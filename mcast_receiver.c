/* multicast-отримувач */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MCAST_GROUP "239.0.0.1"
#define MCAST_PORT  5000
#define BUF         512

int main(void)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) { perror("socket"); return EXIT_FAILURE; }

    int reuse = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(MCAST_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(fd);
        return EXIT_FAILURE;
    }

    struct ip_mreq mreq;
    inet_pton(AF_INET, MCAST_GROUP, &mreq.imr_multiaddr);
    mreq.imr_interface.s_addr = INADDR_ANY;
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == -1) {
        perror("setsockopt IP_ADD_MEMBERSHIP");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("Отримувач підписаний на %s:%d (PID=%d)\n",
           MCAST_GROUP, MCAST_PORT, getpid());

    char buf[BUF];
    for (;;) {
        ssize_t n = recvfrom(fd, buf, sizeof(buf) - 1, 0, NULL, NULL);
        if (n == -1) { perror("recvfrom"); continue; }
        buf[n] = '\0';
        printf("[PID %d] Отримано: %s", getpid(), buf);
    }

    close(fd);
    return EXIT_SUCCESS;
}
