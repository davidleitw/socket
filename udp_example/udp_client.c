#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define serverPort 12000
#define serverIP "127.0.0.1"

int main() 
{
    char buf[1024] = {0};
    char recvbuf[1024] = {0};
    // create an udp socket based on ipv4.
    int socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        printf("Create socket fail!\n");
        return -1;
    }

    struct sockaddr_in server = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr(serverIP),
        .sin_port = htons(serverPort)
    };
    int len = sizeof(server);

    while (1) {
        printf("Please input your message: ");
        scanf("%s", buf);

        sendto(socket_fd, buf, sizeof(buf), 0, (struct sockaddr *)&server, sizeof(server));
        if (strcmp(buf, "exit") == 0) 
            break;

        memset(buf, 0, sizeof(buf));

        if (recvfrom(socket_fd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&server, &len) < 0) {
            printf("recvfrom data from %s:%d, failed!", inet_ntoa(server.sin_addr), ntohs(server.sin_port));
        }
        
        printf("get receive message from [%s:%d]: %s\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port), recvbuf);
        memset(recvbuf, 0, sizeof(recvbuf));
    }

    if (close(socket_fd) < 0) {
        perror("close socket failed!");
    }
    return 0;
}