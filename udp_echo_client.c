/* UDP-клієнт */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 9090
#define BUF         512

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Використання: %s <повідомлення>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in srv;
    memset(&srv, 0, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_port   = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &srv.sin_addr);

    if (sendto(fd, argv[1], strlen(argv[1]), 0, (struct sockaddr *)&srv, sizeof(srv)) == -1) {
        perror("sendto");
        close(fd);
        return EXIT_FAILURE;
    }

    struct timeval tv = { .tv_sec = 3, .tv_usec = 0 };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    char buf[BUF];
    struct sockaddr_in from;
    socklen_t from_len = sizeof(from);

    ssize_t n = recvfrom(fd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&from, &from_len);

    if (n == -1) {
        perror("recvfrom (таймаут або помилка)");
    } else { 
        buf[n] = '\0'; 
        printf("Відповідь: %s\n", buf); 
    }

    close(fd);
    return EXIT_SUCCESS;
}
