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
    int socket_fd = 0;
    socket_fd = socket(AF_INET , SOCK_DGRAM , 0);

    if (socket_fd < 0){
        printf("Fail to create a socket.");
    }

    //socket的連線
    struct sockaddr_in socketSocket;
    // bzero(&socketSocket,sizeof(socketSocket));

    socketSocket.sin_family = PF_INET;
    socketSocket.sin_addr.s_addr = INADDR_ANY;
    socketSocket.sin_port = htons(12000);
    int ret = bind(socket_fd, (const struct sockaddr *)&socketSocket, sizeof(socketSocket));
    
    if (ret < 0) {
        perror("Bind socket failed!");
        close(socket_fd);
    }
    
    printf("Server ready!");

    struct sockaddr_in src;
    size_t len = sizeof(src);
    char buf[1024] = {0};
    
    while (1) {
        ret = recvfrom(socket_fd, buf, sizeof(buf), 0, (struct sockaddr *)&src, &len);
        if (ret < 0) {
            break;
        }

        printf("get message from [%s:%d]: ", inet_ntoa(src.sin_addr), ntohs(src.sin_port));
        printf("%s\n", buf);

        if (strcmp(buf, "exit") == 0) {
            break;
        }
        memset(buf, 0, sizeof(buf));
    }

    close(socket_fd);
    return 0;
}