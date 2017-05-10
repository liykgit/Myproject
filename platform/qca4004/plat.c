
#include <string.h>
#include "plat.h"

#include "socket_api.h"

int create_udp_socket(int *sock)
{

	*sock = -1;

	*sock = qcom_socket(AF_INET, SOCK_STREAM, 0);

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
    qcom_socket_close(sock);

	return 0;
}

int udp_recvfrom(int s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen) {
    return qcom_recvfrom(s, buf, len, flags, from, fromlen);  
}


int udp_sendto(int sock, unsigned char *buffer, int length, struct sockaddr_in *client_addr){
    return qcom_sendto(sock, buffer, length, 0, client_addr, sizeof(struct sockaddr_in));
}

int socket_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {

    return socket_bind(sockfd, addr, addrlen);
}

int sys_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) {
    return qcom_select(nfds, readfds, writefds, exceptfds, timeout);
}


