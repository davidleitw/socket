#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main() 
{
    const char *addr1 = "8.8.8.8";

    struct in_addr *addr;
    
    if (inet_aton(addr1, addr) == 0) {
        printf("inet_aton failed!\n");
        exit(0);
    }
    printf("address %s, after inet_addr function, ip address = %u\n", addr1, addr->s_addr);

    char *addr2 = inet_ntoa(*addr);
    if (addr2 == NULL) {
        printf("inet_ntoa failed!\n");
        exit(0);
    } 
    
    printf("after inet_ntoa function, address is %s\n", addr2);
}