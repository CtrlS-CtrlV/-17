/* клієнт з повторною передачею */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 5353
#define BUF         256
#define MAX_RETRIES 3
#define TIMEOUT_SEC 2

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Використання: %s <hostname>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) { perror("socket"); return EXIT_FAILURE; }

    struct sockaddr_in srv;
    memset(&srv, 0, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_port   = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &srv.sin_addr);

    struct timeval tv = { .tv_sec = TIMEOUT_SEC, .tv_usec = 0 };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    char buf[BUF];
    int retries = 0;

    while (retries < MAX_RETRIES) {
        printf("Запит: %s (спроба %d/%d)\n", argv[1], retries + 1, MAX_RETRIES);

        sendto(fd, argv[1], strlen(argv[1]), 0, (struct sockaddr *)&srv, sizeof(srv));

        struct sockaddr_in from;
        socklen_t from_len = sizeof(from);
        ssize_t n = recvfrom(fd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&from, &from_len);

        if (n == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("Таймаут — повторна спроба...\n");
                retries++;
                continue;
            }
            perror("recvfrom");
            break;
        }

        buf[n] = '\0';
        printf("Відповідь: %s → %s\n", argv[1], buf);
        break;  /* успішно отримано */
    }

    if (retries == MAX_RETRIES)
        printf("Сервер не відповідає після %d спроб.\n", MAX_RETRIES);

    close(fd);
    return EXIT_SUCCESS;
}
