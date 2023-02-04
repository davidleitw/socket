#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

int main()
{
    const char *addr1 = "8.8.8.8";
    const char *addr2 = "192.168.1.256";

    struct in_addr *naddr1 = (struct in_addr*)malloc(sizeof(struct in_addr));
    struct in_addr *naddr2 = (struct in_addr*)malloc(sizeof(struct in_addr));

    if (inet_aton(addr1, naddr1) == 0) {
        printf("inet_aton failed! Please check your input address.\n");
    }
    printf("address %s, after inet_addr function, ip address = %u\n", addr1, naddr1->s_addr);
    
    // 輸入不合法的 ip 地址
    if (inet_aton(addr2, naddr2) == 0) {
        printf("inet_aton failed! Please check your input address.\n");
    }
    return 0;
}