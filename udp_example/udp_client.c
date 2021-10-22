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

    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        printf("Create socket fail!\n");
        return -1;
    }

    struct sockaddr_in clientSocket = {
        .sin_family = PF_INET,
        .sin_addr.s_addr = inet_addr(serverIP),
        .sin_port = htons(serverPort)
    };
    int len = sizeof(clientSocket);
    
    while (1) {
        printf("Please input your message: ");
        scanf("%s", buf);

        sendto(socket_fd, buf, strlen(buf), 0, (struct sockaddr *)&clientSocket, sizeof(clientSocket));
        if (strcmp(buf, "exit") == 0) 
            break;

        memset(buf, 0, sizeof(buf));

        recvfrom(socket_fd, buf, strlen(buf), 0, (struct sockaddr *)&clientSocket, &len);
        printf("get receive message from [%s:%d]: %s\n", inet_ntoa(clientSocket.sin_addr), ntohs(clientSocket.sin_port), buf);
    }

    if (close(socket_fd) < 0) {
        perror("close socket failed!");
    }
    return 0;
}