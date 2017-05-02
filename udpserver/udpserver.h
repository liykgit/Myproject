#ifndef _UDPSERVER_H
#define _UDPSERVER_H

#include <netinet/in.h>
#include <sys/types.h>  
#include <sys/socket.h>

#define UDP_RECV_BUFFER_SIZE 512

//int udp_recv(int sock, unsigned char *buffer, int length, struct sockaddr *client_addr, unsigned int *client_addr_len);


//int udpserver_sendto(struct sockaddr_int *client_addr, char *buf, int length);

typedef struct client_info {
    struct sockaddr_in addr;
    char user_id[16];
} client_info_t;

#endif 
