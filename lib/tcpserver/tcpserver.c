
#include "tcpserver.h"
#include "udp_atcmd.h"
#include "common.h"

static int tcpserver_sock = -1;
//store client socket fd here as we accept only one connection
static int client_sock = -1; 

//static unsigned char udpserver_buffer[TCP_RECV_BUFFER_SIZE];


static thread_ret_t tcpserver_handler_thread() {

    LOG(LEVEL_DEBUG, "tcp_handler_thread E\n");

    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    int addr_len = sizeof(struct sockaddr_in);
	struct timeval tm;
	int	ret;

	tm.tv_sec  = 5;
	tm.tv_usec = 0; 


    vg_listen(tcpserver_sock, 1);


    while(1) {

        vg_fd_set sockSet;
        FD_ZERO(&sockSet);
        FD_SET(tcpserver_sock, &sockSet);

	    ret	= vg_select(tcpserver_sock + 1, &sockSet, 0, 0, &tm);

		if (ret < 0)
		{
			LOG(LEVEL_ERROR, "tcp listen socket selece failed %d.\n", ret);
			break;
		}
		else if (ret == 0)
		{
			continue;
		}
        else {

            ret = FD_ISSET(tcpserver_sock, &sockSet);
            if (ret == 1)       
            {
                //== 5. accept a new socket connect
                client_sock = vg_accept(tcpserver_sock, (struct sockaddr *)&client_addr, &addr_len);
                if (client_sock < 0)
                {
                    LOG(LEVEL_ERROR, "Failed to accept socket %d.\n", client_sock);
                }
                else
                    LOG(LEVEL_NORMAL, "Accepted tcp socket %d.\n", client_sock);

            }
        }
    }
    LOG(LEVEL_DEBUG, "tcp_handler_thread X");
}

int  tcpserver_start(int port) {

    int ret = 0;
   
    if(tcpserver_sock >= 0)
        goto socket_fail;

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
    
    LOG(LEVEL_DEBUG, "tcpserver_send E\n");
    if(client_sock < 0) {
        LOG(LEVEL_ERROR, "client_sock not connected\n");
        return -1;
    }

    int r = vg_send(client_sock, buf, length, 0);
    if(r < 0) {
        LOG(LEVEL_ERROR, "client_sock send failed\n");
        return -1;
    }

    LOG(LEVEL_DEBUG, "tcpserver_send X\n");

    return r;
}



