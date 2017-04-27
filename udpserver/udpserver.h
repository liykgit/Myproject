#ifndef _UDPSERVER_H
#define _UDPSERVER_H

#include <sys/types.h>  
#include <sys/socket.h>

#define UDP_RECV_BUFFER_SIZE 512

int udp_recv(int sock, unsigned char *buffer, int length, struct sockaddr *client_addr, unsigned int *client_addr_len);

#endif 
