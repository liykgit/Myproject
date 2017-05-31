
#include "tcpserver.h"
#include "udp_atcmd.h"
#include "common.h"

static int tcpserver_sock = -1;
//store client socket fd here as we accept only one connection
static int client_sock = -1; 

//static unsigned char udpserver_buffer[TCP_RECV_BUFFER_SIZE];

static thread_ret_t tcpserver_handler_thread() {

    LOG(LEVEL_DEBUG, "tcp_handler_thread E");

    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));


    vg_listen(tcpserver_sock, 1);

    while(1) {
        
        int addr_len = sizeof(client_addr);

        client_sock = vg_accept(tcpserver_sock, (struct sockaddr *) &client_addr, &addr_len);
        if(client_sock < 0) {
            LOG(LEVEL_ERROR, "Failed to accept socket %d.\n", client_sock);
            continue;
        }
        
        //struct linger ling = { 1, 1 };

        //vg_setsockopt(client_sock, SOL_SOCKET, SO_LINGER, (void *)&ling, sizeof(ling));

    }
    LOG(LEVEL_DEBUG, "tcp_handler_thread X");
}

int  tcpserver_start(int port) {

    int ret = 0;
    
    ret = vg_tcp_socket(&tcpserver_sock);
    if(ret != 0){
		printf("Create tcp_socket failed\n");
		goto socket_fail;
    }

    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(struct sockaddr_in));

    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);

    ret = vg_bind(tcpserver_sock, (const struct sockaddr *)&local_addr, sizeof(struct sockaddr_in));
    if(ret != 0) {
		printf("bind failed\n");
		goto bind_fail;
    }

    vg_start_thread(tcpserver_handler_thread, NULL, 2048);

    return 0;

bind_fail:
    
    vg_tcp_close(tcpserver_sock, 0);

socket_fail:

    return -1;

}

int tcpserver_send(char *buf, int length) {
    
    LOG(LEVEL_DEBUG, "tcpserver_send E");
    if(client_sock < 0) {
        LOG(LEVEL_ERROR, "client_sock not connected\n");
        return -1;
    }

    int r = vg_send(client_sock, buf, length, 0);
    if(r < 0) {
        LOG(LEVEL_ERROR, "client_sock send failed\n");
        return -1;
    }

    LOG(LEVEL_DEBUG, "tcpserver_send X");

    return r;
}



