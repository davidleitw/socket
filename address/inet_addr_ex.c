#include <stdio.h>
#include <arpa/inet.h>

int main()
{
    const char *addr1 = "8.8.8.8";
    const char *addr2 = "192.168.1.256";

    // 8.8.8.8
    // bin: 00001000 00001000 00001000 00001000
    // dec: 134744072
    in_addr_t naddr1 = inet_addr(addr1);
    
    // INADDR_NONE = 0xffffffff
    if (naddr1 == INADDR_NONE) {
        printf("inet_addr failed! Please check your input address.\n");
    }
    printf("address %s, after inet_addr function, ip address = %u\n", addr1, naddr1);

    // 輸入不合法的 ip 地址
    in_addr_t naddr2 = inet_addr(addr2);
    if (naddr2 == INADDR_NONE) {
        printf("inet_addr failed! Please check your input address.\n");
    }
}