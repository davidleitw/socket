.PHONY: all

all: tcp_client tcp_server

tcp_client: 
	gcc -o tcp_client tcp_client.c
tcp_server: 
	gcc -o tcp_server tcp_server.c -lpthread
clean:
	rm -f tcp_client tcp_server