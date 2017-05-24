
#include "udpserver.h"
#include "udp_atcmd.h"
#include "common.h"

static int udpserver_sock = -1;

static unsigned char udpserver_buffer[UDP_RECV_BUFFER_SIZE];


static int udp_recv(int sock, unsigned char *buffer, int length, struct sockaddr *client_addr, unsigned int *client_addr_len)
{
    LOG(LEVEL_DEBUG, "udp_recv E\n");

    vg_fd_set fdRead;
	struct timeval tm;
	int	ret;

	tm.tv_sec  = UDP_RECV_TIMEOUT_S;
	tm.tv_usec = UDP_RECV_TIMEOUT_MS; //2S

	FD_ZERO(&fdRead);
	FD_SET(sock, &fdRead);

	ret	= vg_select(sock + 1, &fdRead, 0, 0, &tm);


	if(ret < 0){// error
		printf( "udp sockfd select error\n");
		return -1;
	}else if(ret == 0){
		//printf( "udp sockfd select timeout\n");
		return 0;
	} else {
	    if(FD_ISSET(sock, &fdRead)){
            ret = vg_recvfrom(sock, (char*)buffer, length, 0, (struct sockaddr *)client_addr, client_addr_len);
			if(ret > 0){
                
                udp_user_cmd_handle(buffer, ret, client_addr);

				return ret;
			}else if(ret < 0){//recv err
				printf( "<ERR> udp recv error \n");
			    return ret;
			}else{// == 0 //socket close
				printf( "<ERR> udp close socket\n");
				return -3;// careful : sometime select =1 ,but read 0 byte data
			}
	    }else{
			printf( "<ERR> udp sockfd is not set\n");
		}
	}

    LOG(LEVEL_DEBUG, "udp_recv X");

	return 0;
}

 static thread_ret_t udp_handler_thread() {

    LOG(LEVEL_DEBUG, "udp_handler_thread E");

    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));

    while(1) {
        
        unsigned int client_addr_len;
        udp_recv(udpserver_sock, (unsigned char *)&udpserver_buffer, sizeof(udpserver_buffer), &client_addr, &client_addr_len);
    }
    LOG(LEVEL_DEBUG, "udp_handler_thread X");
}

int  UDPServer_start(int port) {

    int ret = 0;
    
    ret = vg_udp_socket(&udpserver_sock);
    if(ret != 0){
		printf("Create_udp_socket failed\n");
		goto socket_fail;
    }

    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(struct sockaddr_in));

    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);

    ret = vg_bind(udpserver_sock, (const struct sockaddr *)&local_addr, sizeof(struct sockaddr_in));
    if(ret != 0) {
		printf("bind failed\n");
		goto bind_fail;
    }

    vg_start_thread(udp_handler_thread, NULL, 2048);

    return 0;

bind_fail:
    
    vg_udp_close(udpserver_sock);

socket_fail:

    return -1;

}

int udpserver_sendto(struct sockaddr_in *client_addr, char *buf, int length) {
    
    LOG(LEVEL_DEBUG, "udpserver_sendto E");
    int r = vg_sendto(udpserver_sock, buf, length, client_addr);
    LOG(LEVEL_DEBUG, "udpserver_sendto X");

    return r;
}

