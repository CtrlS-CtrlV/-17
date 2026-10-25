/* UDP echo-сервер */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9090
#define BUF  512

int main(void)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("UDP echo-сервер слухає на порту %d...\n", PORT);

    char buf[BUF];
    struct sockaddr_in cli;
    socklen_t cli_len;

    for (;;) {
        cli_len = sizeof(cli);
        ssize_t n = recvfrom(fd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&cli, &cli_len);

        if (n == -1) { 
            perror("recvfrom"); 
            continue; 
        }

        buf[n] = '\0';
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli.sin_addr, ip, sizeof(ip));
        printf("Від %s:%d (%zd байт): %s\n", ip, ntohs(cli.sin_port), n, buf);

        // Відправляємо ехо назад відправнику
        if (sendto(fd, buf, n, 0, (struct sockaddr *)&cli, cli_len) == -1) {
            perror("sendto");
        }
    }

    close(fd);
    return EXIT_SUCCESS;
}
