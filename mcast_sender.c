/* multicast-відправник */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MCAST_GROUP "239.0.0.1"
#define MCAST_PORT  5000
#define TTL_VALUE   1

int main(void)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) { perror("socket"); return EXIT_FAILURE; }

    int ttl = TTL_VALUE;
    if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) == -1) {
        perror("setsockopt IP_MULTICAST_TTL");
        close(fd);
        return EXIT_FAILURE;
    }

    struct sockaddr_in group_addr;
    memset(&group_addr, 0, sizeof(group_addr));
    group_addr.sin_family = AF_INET;
    group_addr.sin_port   = htons(MCAST_PORT);
    inet_pton(AF_INET, MCAST_GROUP, &group_addr.sin_addr);

    printf("Multicast-відправник → %s:%d\n", MCAST_GROUP, MCAST_PORT);
    printf("Введіть повідомлення (Ctrl+D для виходу):\n");

    char buf[256];
    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&group_addr, sizeof(group_addr));
        printf("Надіслано: %s", buf);
    }

    close(fd);
    return EXIT_SUCCESS;
}
