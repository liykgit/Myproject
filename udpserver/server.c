

static int udp_fd = -1;

#define UDP_SERVER_BUFFER_SIZE 512
static unsigned char recv_buf[UDP_SERVER_BUFFER_SIZE];

void UDPServer_start_handler() {

    struct sockaddr client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));

    while(1) {
        
        unsigned int client_addr_len;
        udp_recv(udp_fd, (unsigned char *)&recv_buf, sizeof(recv_buf), &client_addr, &client_addr_len);
    }
}

int UDPServer_start(int port) {

    int ret = 0;
    
    ret = create_udp_socket(&udp_fd);
    if(ret != 0){
		printf("Create_udp_socket failed(errno:%d)\n", errno);
		goto socket_fail;
    }

    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(struct sockaddr_in));

    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);

    //TODO implement bind in platform
    ret = bind(udp_fd, (const struct sockaddr *)&local_addr, sizeof(struct sockaddr_in));
    if(ret != 0) {
		printf("bind failed(errno:%d)\n", errno);
		goto bind_fail;
    }

    UDPServer_start_handler();


    return 0;

bind_fail:
    
    close_udp_socket(udp_fd);

socket_fail:

    return -1;
}
