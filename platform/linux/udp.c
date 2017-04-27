
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "plat.h"

#include <sys/types.h>
#include <sys/socket.h>

int create_udp_socket(int *sock)
{

	*sock = -1;

	*sock = socket(AF_INET, SOCK_DGRAM, 0);

	if(-1 == *sock){
		printf( "<ERR> creat socket error");
		return -1;
	}

	printf( "Create sockfd(%d)\n", *sock);
	return 0;
}

int close_udp_socket(int sock)
{

	printf( "Close sockfd(%d)\n", sock);
    close(sock);

	return 0;
}

int udp_recvfrom(int s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen) {
    return recvfrom(s, buf, len, flags, from, fromlen);  
}

int socket_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {

    return bind(sockfd, addr, addrlen);
}
