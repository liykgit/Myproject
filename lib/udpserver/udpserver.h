#ifndef _UDPSERVER_H
#define _UDPSERVER_H

#include "plat.h"

#define UDP_RECV_BUFFER_SIZE 512


typedef struct client_info {
    struct sockaddr_in addr;
    char user_id[16];
} client_info_t;

int udpserver_sendto(struct sockaddr_in *client_addr, char *buf, int length);
int udpserver_broadcast(char *buf, int length, int port);

#endif 
