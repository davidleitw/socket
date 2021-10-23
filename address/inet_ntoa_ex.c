#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main() 
{
    const char *addr1 = "8.8.8.8";

    in_addr_t naddr1 = inet_addr(addr1);
    if (naddr1 == 0xffffffff) {
        printf("inet_addr failed! Please check your input address.\n");
    }
    printf("address %s, after inet_addr function, ip address = %u\n", addr1, naddr1);

    struct in_addr na = {
        .s_addr = naddr1,
    };
    printf("%s\n", inet_ntoa(na));
}