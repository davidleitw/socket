#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define ncu "www.csie.ncu.edu.tw"

int main()
{
    int status;
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_PASSIVE,
    };
    struct addrinfo *server_info;

    status = getaddrinfo(ncu, "443", &hints, &server_info);

    if (status != 0) {
        printf("getaddrinfo error: %s\n", gai_strerror(status));
        exit(0);
    }

    for (struct addrinfo *iter = server_info; iter != NULL; iter = iter->ai_next) {
        void *addr;
        char ip_address[INET6_ADDRSTRLEN];

        switch (iter->ai_family) 
        {
            case AF_INET: {
                struct sockaddr_in *ipv4 = (struct sockaddr_in *)iter->ai_addr;
                addr = &(ipv4->sin_addr);
                break;
            }
            case AF_INET6: {
                struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)iter->ai_addr;
                addr = &(ipv6->sin6_addr);
                break;
            }
        }

        inet_ntop(iter->ai_family, addr, ip_address, sizeof(ip_address));
        printf("%s -> %s\n", ncu, ip_address);
    }
    return 0;
}