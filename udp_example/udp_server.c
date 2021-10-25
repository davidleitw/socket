#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define serverPort 48763

char *convert(char *src) {
    char *iter = src;
    char *result = malloc(sizeof(src));
    char *it = result;
    if (iter == NULL) return iter;

    while (*iter) {
        *it++ = *iter++ & ~0x20;
    }
    return result;
}

int main(int argc , char *argv[])
{
    // message buffer
    char buf[1024] = {0};

    // 建立 socket
    int socket_fd = socket(PF_INET , SOCK_DGRAM , 0);
    if (socket_fd < 0){
        printf("Fail to create a socket.");
    }
    
    // server 地址
    struct sockaddr_in serverAddr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(serverPort)
    };
    
    // 將建立的 socket 綁定到 serverAddr 指定的 port
    if (bind(socket_fd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind socket failed!");
        close(socket_fd);
        exit(0);
    }
    
    printf("Server ready!\n");

    struct sockaddr_in clientAddr;
    int len = sizeof(clientAddr);
    while (1) {
        // 當有人使用 UDP 協定送資料到 48763 port
        // 會觸發 recvfrom()，並且把來源資料寫入 clientAddr 當中
        if (recvfrom(socket_fd, buf, sizeof(buf), 0, (struct sockaddr *)&clientAddr, &len) < 0) {
            break;
        }

        // 收到 exit 指令就關閉 server
        if (strcmp(buf, "exit") == 0) {
            printf("get exit order, closing the server...\n");
            break;
        }
        
        // 將收到的英文字母換成大寫
        char *conv = convert(buf);

        // 顯示資料來源，原本資料，以及修改後的資料
        printf("get message from [%s:%d]: ", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        printf("%s -> %s\n", buf, conv);

        // 根據 clientAddr 的資訊，回傳至 client 端                
        sendto(socket_fd, conv, sizeof(conv), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));

        // 清空 message buffer
        memset(buf, 0, sizeof(buf));
        free(conv);
    }
    // 關閉 socket，並檢查是否關閉成功
    if (close(socket_fd) < 0) {
        perror("close socket failed!");
    }
    
    return 0;
}