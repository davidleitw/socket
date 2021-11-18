#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define serverPort 48763
#define BACKLOG 5

pthread_t clients[BACKLOG];
pthread_t tid;

char *convert(char *src) {
    char *iter = src;
    char *result = malloc(sizeof(src));
    char *it = result;
    if (iter == NULL) return iter;

    while (*iter) {
        *it++ = toupper(*iter++);
    }
    return result;
}

struct client_info{
    int sockfd;
    struct sockaddr_in clientAddr;
};

void* clientSocket(void *param){
    char buf[1024];
    struct client_info * info = param;
    // 不斷接收 client 資料
    while (recv(info->sockfd, buf, sizeof(buf), 0)) {
        
        // 收到 exit 指令就離開迴圈
        if (strcmp(buf, "exit") == 0) {
            memset(buf, 0, sizeof(buf));
            break;
        }

        // 將收到的英文字母換成大寫
        char *conv = malloc(sizeof(buf));
        memcpy(conv, convert(buf), sizeof(buf));

        // 顯示資料來源，原本資料，以及修改後的資料
        printf("get message from [%s:%d]: ",
                inet_ntoa(info->clientAddr.sin_addr), ntohs(info->clientAddr.sin_port));
        printf("%s -> %s\n", buf, conv);

        // 傳回 client 端
        // 不需要填入 client 端的位置資訊，因為已經建立 TCP 連線
        if (send(info->sockfd, conv, sizeof(conv), 0) < 0) {
            printf("send data to %s:%d, failed!\n", 
                    inet_ntoa(info->clientAddr.sin_addr), ntohs(info->clientAddr.sin_port));
            memset(buf, 0, sizeof(buf));
            free(conv);
            break;
        }

        // 清空 message buffer
        memset(buf, 0, sizeof(buf));
        free(conv);
    }

    // 關閉 reply socket，並檢查是否關閉成功
    if (close(info->sockfd) < 0) {
        perror("close socket failed!");
    }else{
        printf("Socket closed from %s:%d success!\n", 
            inet_ntoa(info->clientAddr.sin_addr), ntohs(info->clientAddr.sin_port));
    }

}

int main() 
{
    // message buffer
    char buf[1024] = {0};

    int client_index = 0;

    // 建立 socket
    int socket_fd = socket(PF_INET , SOCK_STREAM , 0);
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

    // 初始化，準備接受 connect
    // backlog = 5，在 server accept 動作之前，最多允許五筆連線申請
    // 回傳 -1 代表 listen 發生錯誤
    if (listen(socket_fd, BACKLOG) == -1) {
        printf("socket %d listen failed!\n", socket_fd);
        close(socket_fd);
        exit(0);
    }

    printf("server [%s:%d] --- ready\n", 
            inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

    while(1) {

        // 建立新socket的參數
        int reply_sockfd;
        struct sockaddr_in clientAddr;
        int client_len = sizeof(clientAddr);

        // 從 complete connection queue 中取出已連線的 socket
        // 之後用 reply_sockfd 與 client 溝通
        reply_sockfd = accept(socket_fd, (struct sockaddr *)&clientAddr, &client_len);
        printf("Accept connect request from [%s:%d]\n", 
                inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        // 建立 thread 與 thread 的參數
        struct client_info *newClientinfo = malloc(sizeof(struct client_info));
        memcpy(&newClientinfo->sockfd, &reply_sockfd, sizeof(int));
        memcpy(&newClientinfo->clientAddr, &clientAddr, sizeof(struct sockaddr_in));

        if (pthread_create(&clients[client_index++], NULL, clientSocket, newClientinfo) != 0){
            printf("Failed to create new Thread!");
        }

    }

    // 關閉 socket，並檢查是否關閉成功
    if (close(socket_fd) < 0) {
        perror("close socket failed!");
    }
    return 0;
}