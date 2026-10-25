/* відправник файлу через UDP */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "udp_file_transfer.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Використання: %s <файл>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) { perror("fopen"); return EXIT_FAILURE; }

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) { perror("socket"); fclose(fp); return EXIT_FAILURE; }

    struct timeval tv = { .tv_sec = ACK_TIMEOUT, .tv_usec = 0 };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct sockaddr_in srv;
    memset(&srv, 0, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_port   = htons(UDP_PORT);
    inet_pton(AF_INET, "127.0.0.1", &srv.sin_addr);

    Packet pkt;
    AckPacket ack;
    uint32_t seq = 0;
    size_t total_sent = 0;

    while (!feof(fp)) {
        size_t bytes_read = fread(pkt.data, 1, CHUNK_SIZE, fp);
        if (bytes_read == 0) break;

        pkt.type     = PKT_DATA;
        pkt.seq_num  = htonl(seq);
        pkt.data_len = htons((uint16_t)bytes_read);

        int retries = 0;
        int acked = 0;

        while (retries < MAX_RETRIES && !acked) {
            sendto(fd, &pkt, sizeof(Packet), 0, (struct sockaddr *)&srv, sizeof(srv));

            struct sockaddr_in from;
            socklen_t from_len = sizeof(from);
            ssize_t n = recvfrom(fd, &ack, sizeof(AckPacket), 0, (struct sockaddr *)&from, &from_len);

            if (n > 0 && ack.type == PKT_ACK && ntohl(ack.ack_num) == seq) {
                acked = 1;
            } else {
                retries++;
            }
        }

        if (!acked) {
            fprintf(stderr, "Помилка: пакет %u не підтверджено.\n", seq);
            break;
        }

        seq++;
        total_sent += bytes_read;
    }

    // Відправка сигналу завершення PKT_FIN
    pkt.type = PKT_FIN;
    pkt.seq_num = htonl(seq);
    pkt.data_len = 0;
    for (int r = 0; r < 3; r++) {
        sendto(fd, &pkt, sizeof(Packet), 0, (struct sockaddr *)&srv, sizeof(srv));
    }

    printf("Передано: %zu байт, %u пакетів\n", total_sent, seq);

    fclose(fp);
    close(fd);
    return EXIT_SUCCESS;
}
