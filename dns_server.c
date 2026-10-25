/* простий DNS-подібний UDP-сервіс */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5353
#define BUF  256

typedef struct {
    char hostname[64];
    char ip[INET_ADDRSTRLEN];
} DnsEntry;

static const DnsEntry dns_table[] = {
    { "server1.local", "192.168.1.10" },
    { "server2.local", "192.168.1.20" },
    { "db.local",      "192.168.1.30" },
    { "web.local",     "192.168.1.40" },
    { "mail.local",    "192.168.1.50" },
};

#define TABLE_SIZE (sizeof(dns_table) / sizeof(dns_table[0]))

static const char *lookup(const char *hostname)
{
    for (size_t i = 0; i < TABLE_SIZE; i++) {
        if (strcmp(dns_table[i].hostname, hostname) == 0)
            return dns_table[i].ip;
    }
    return NULL;
}

int main(void)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) { perror("socket"); return EXIT_FAILURE; }

    struct sockaddr_in srv;
    memset(&srv, 0, sizeof(srv));
    srv.sin_family      = AF_INET;
    srv.sin_port        = htons(PORT);
    srv.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (struct sockaddr *)&srv, sizeof(srv)) == -1) {
        perror("bind");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("DNS-сервер слухає на порту %d...\n", PORT);

    char buf[BUF];
    struct sockaddr_in cli;
    socklen_t cli_len;

    for (;;) {
        cli_len = sizeof(cli);
        ssize_t n = recvfrom(fd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&cli, &cli_len);
        if (n == -1) continue;

        buf[n] = '\0';
        // Видалення можливого символу нового рядка
        buf[strcspn(buf, "\r\n")] = 0;

        const char *ip = lookup(buf);
        const char *response = ip ? ip : "NOT_FOUND";

        sendto(fd, response, strlen(response), 0, (struct sockaddr *)&cli, cli_len);
    }

    close(fd);
    return EXIT_SUCCESS;
}
