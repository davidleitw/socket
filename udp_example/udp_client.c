#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define serverPort 12000
#define serverIP "127.0.0.1"

int main() {
    int socket_fd;
    
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        printf("Create socket fail!\n");
        return -1;
    }

    struct sockaddr_in clientSocket;
    // bzero(&clientSocket, sizeof(clientSocket));
    clientSocket.sin_family = PF_INET;
    clientSocket.sin_addr.s_addr = inet_addr(serverIP);
    clientSocket.sin_port = htons(serverPort);

    char buf[1024] = {0};
    while (1) {
        printf("Please input your message: ");
        scanf("%s", buf);

        sendto(socket_fd, buf, strlen(buf), 0, (const struct sockaddr *)&clientSocket, sizeof(clientSocket));
        if (strcmp(buf, "exit") == 0) 
            break;

        memset(buf, 0, sizeof(buf));
    }

    if (close(socket_fd) < 0) {
        perror("close socket failed!");
    }
    return 0;
}