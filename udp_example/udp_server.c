#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc , char *argv[])
{
    char buf[1024] = {0};
    
    int socket_fd = socket(AF_INET , SOCK_DGRAM , 0);
    if (socket_fd < 0){
        printf("Fail to create a socket.");
    }

    //socket的連線
    struct sockaddr_in serverAddr = {
        .sin_family = PF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(12000)
    };
    struct sockaddr_in clientAddr;
    int len = sizeof(clientAddr);
    
    
    if (bind(socket_fd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind socket failed!");
        close(socket_fd);
    }
    
    printf("Server ready!\n");

    while (1) {
        if (recvfrom(socket_fd, buf, sizeof(buf), 0, (struct sockaddr *)&clientAddr, &len) < 0) {
            break;
        }

        printf("get message from [%s:%d]: ", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        printf("%s\n", buf);

        if (strcmp(buf, "exit") == 0) {
            break;
        }
        memset(buf, 0, sizeof(buf));
    }

    close(socket_fd);
    return 0;
}