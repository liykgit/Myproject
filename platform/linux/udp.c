

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "plat.h"


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

int udp_recv(int sock, unsigned char *buffer, int length, struct sockaddr *client_addr, unsigned int *client_addr_len)
{
    fd_set fdRead;
	struct timeval tm;
	int	ret;

	tm.tv_sec  = UDP_RECV_TIMEOUT_S;
	tm.tv_usec = UDP_RECV_TIMEOUT_MS; //2S

	FD_ZERO(&fdRead);
	FD_SET(sock, &fdRead);

	ret	= select(sock + 1, &fdRead, 0, 0, &tm);

	if(ret < 0){// error
		printf( "<ERR> udp sockfd select error\n");
		return -1;
	}else if(ret == 0){
		printf( "<LOG> udp sockfd select timeout\n");
		return 0;
	} else {
	    if(FD_ISSET(sock, &fdRead)){
            ret = recvfrom(sock, (char*)buffer, length, 0, (struct sockaddr *)client_addr, client_addr_len);
			if(ret > 0){
				printf( "<LOG> recvd buffer: \n");

				return ret;
			}else if(ret < 0){//recv err
				printf( "<ERR> udp recv error(errno:%d)\n", errno);
				if(errno == 104 || errno == 9)
					return -4;
				else
					return -2;
			}else{// == 0 //socket close
				printf( "<ERR> udp close socket\n");
				return -3;// careful : sometime select =1 ,but read 0 byte data
			}
	    }else{
			printf( "<ERR> udp sockfd is not set\n");
		}
	}
	return 0;
}

