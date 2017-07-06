


#include "plat.h"

#if (CONN_MODE == CONN_SSL)
const unsigned char calist_cert[] = {

   0x00, 0x00, 0x00, 0x01, 0x56, 0x65, 0x6E, 0x67,
   0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   0x30, 0x82, 0x03, 0x81, 0x30, 0x82, 0x02, 0x69,
   0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x09, 0x00,
   0xE2, 0xE4, 0x37, 0x15, 0xDA, 0xD5, 0x4E, 0x01,
   0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86,
   0xF7, 0x0D, 0x01, 0x01, 0x0B, 0x05, 0x00, 0x30,
   0x57, 0x31, 0x0B, 0x30, 0x09, 0x06, 0x03, 0x55,
   0x04, 0x06, 0x13, 0x02, 0x43, 0x4E, 0x31, 0x0B,
   0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x08, 0x0C,
   0x02, 0x47, 0x44, 0x31, 0x0B, 0x30, 0x09, 0x06,
   0x03, 0x55, 0x04, 0x07, 0x0C, 0x02, 0x53, 0x5A,
   0x31, 0x0E, 0x30, 0x0C, 0x06, 0x03, 0x55, 0x04,
   0x0A, 0x0C, 0x05, 0x56, 0x65, 0x6E, 0x67, 0x61,
   0x31, 0x0E, 0x30, 0x0C, 0x06, 0x03, 0x55, 0x04,
   0x0B, 0x0C, 0x05, 0x56, 0x65, 0x6E, 0x67, 0x61,
   0x31, 0x0E, 0x30, 0x0C, 0x06, 0x03, 0x55, 0x04,
   0x03, 0x0C, 0x05, 0x56, 0x65, 0x6E, 0x67, 0x61,
   0x30, 0x1E, 0x17, 0x0D, 0x31, 0x37, 0x30, 0x36,
   0x31, 0x36, 0x30, 0x37, 0x34, 0x32, 0x30, 0x33,
   0x5A, 0x17, 0x0D, 0x31, 0x37, 0x30, 0x37, 0x31,
   0x36, 0x30, 0x37, 0x34, 0x32, 0x30, 0x33, 0x5A,
   0x30, 0x57, 0x31, 0x0B, 0x30, 0x09, 0x06, 0x03,
   0x55, 0x04, 0x06, 0x13, 0x02, 0x43, 0x4E, 0x31,
   0x0B, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x08,
   0x0C, 0x02, 0x47, 0x44, 0x31, 0x0B, 0x30, 0x09,
   0x06, 0x03, 0x55, 0x04, 0x07, 0x0C, 0x02, 0x53,
   0x5A, 0x31, 0x0E, 0x30, 0x0C, 0x06, 0x03, 0x55,
   0x04, 0x0A, 0x0C, 0x05, 0x56, 0x65, 0x6E, 0x67,
   0x61, 0x31, 0x0E, 0x30, 0x0C, 0x06, 0x03, 0x55,
   0x04, 0x0B, 0x0C, 0x05, 0x56, 0x65, 0x6E, 0x67,
   0x61, 0x31, 0x0E, 0x30, 0x0C, 0x06, 0x03, 0x55,
   0x04, 0x03, 0x0C, 0x05, 0x56, 0x65, 0x6E, 0x67,
   0x61, 0x30, 0x82, 0x01, 0x22, 0x30, 0x0D, 0x06,
   0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01,
   0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0F,
   0x00, 0x30, 0x82, 0x01, 0x0A, 0x02, 0x82, 0x01,
   0x01, 0x00, 0xB4, 0x91, 0x1F, 0x30, 0x6C, 0xB0,
   0x2B, 0xAC, 0x7D, 0xB3, 0xEE, 0xE2, 0x8D, 0x9E,
   0x75, 0x96, 0x09, 0x10, 0xE7, 0x35, 0x1F, 0x03,
   0x01, 0x39, 0x5E, 0x98, 0x8E, 0x7C, 0xF2, 0xE1,
   0x7B, 0x5E, 0xFC, 0xDD, 0xD8, 0xA5, 0x73, 0x25,
   0x4B, 0x37, 0xC5, 0xE6, 0xA0, 0x94, 0x4F, 0xB1,
   0xDF, 0x75, 0xC4, 0x40, 0x53, 0x1A, 0xE8, 0x3D,
   0xD1, 0x58, 0xB3, 0x07, 0xAB, 0xF3, 0xF5, 0x12,
   0xC5, 0x15, 0xAF, 0x90, 0xA9, 0x02, 0xD7, 0x68,
   0x31, 0xAD, 0xD4, 0x58, 0xC5, 0x55, 0xDC, 0x3D,
   0xC6, 0x01, 0xA3, 0xAB, 0x34, 0xD2, 0xAB, 0x05,
   0x4D, 0x81, 0xDA, 0x72, 0x11, 0xBC, 0xFA, 0xC8,
   0xB6, 0x3F, 0x93, 0x56, 0x1B, 0xCB, 0x3F, 0x12,
   0x0E, 0xCA, 0x3D, 0xA1, 0xB7, 0xEF, 0x09, 0x2F,
   0xF2, 0x00, 0xF0, 0x82, 0x22, 0x05, 0x5A, 0xDB,
   0x87, 0x8F, 0xE3, 0xC4, 0xFB, 0x0F, 0xC9, 0xC2,
   0xFA, 0xE8, 0x82, 0xA5, 0xDB, 0xA4, 0x79, 0x4B,
   0x2C, 0x80, 0x09, 0xE2, 0xBE, 0x7F, 0xCA, 0x75,
   0x70, 0xC6, 0xBA, 0x23, 0x04, 0x0C, 0x5C, 0xB1,
   0x01, 0x85, 0x68, 0xF1, 0x6A, 0x2E, 0xE8, 0x0A,
   0x8D, 0xFC, 0x09, 0x2D, 0xD2, 0xDF, 0xA7, 0xD2,
   0x75, 0x30, 0xFB, 0xC7, 0x13, 0x78, 0x41, 0xF9,
   0xA7, 0xFA, 0xC6, 0x8E, 0x72, 0x1D, 0xD9, 0x35,
   0xB4, 0x7C, 0x0F, 0x68, 0xB2, 0x1C, 0xF0, 0xCD,
   0xC7, 0x33, 0x80, 0x6F, 0x32, 0xF2, 0x6A, 0x55,
   0x8B, 0xCA, 0x37, 0x15, 0x4E, 0x7F, 0xB6, 0x1A,
   0x33, 0x31, 0x29, 0xD9, 0xA3, 0x71, 0x24, 0xCB,
   0x62, 0xB2, 0x59, 0xA7, 0x48, 0x42, 0xBB, 0xDE,
   0x15, 0x81, 0xFA, 0xF4, 0xED, 0xAE, 0x72, 0xB5,
   0xE6, 0x5B, 0x22, 0xB7, 0xE9, 0x58, 0xF0, 0x00,
   0xFB, 0x72, 0x61, 0x98, 0xBA, 0xF6, 0x0B, 0x43,
   0x06, 0x71, 0x54, 0x41, 0x8F, 0xEE, 0x1B, 0xD5,
   0x5A, 0x3B, 0x02, 0x03, 0x01, 0x00, 0x01, 0xA3,
   0x50, 0x30, 0x4E, 0x30, 0x1D, 0x06, 0x03, 0x55,
   0x1D, 0x0E, 0x04, 0x16, 0x04, 0x14, 0x1E, 0x28,
   0xE8, 0x16, 0xF9, 0xF2, 0x66, 0xA7, 0xBA, 0x6A,
   0xD7, 0x61, 0x33, 0xE0, 0xF4, 0xBD, 0xDE, 0x37,
   0x21, 0x59, 0x30, 0x1F, 0x06, 0x03, 0x55, 0x1D,
   0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0x1E,
   0x28, 0xE8, 0x16, 0xF9, 0xF2, 0x66, 0xA7, 0xBA,
   0x6A, 0xD7, 0x61, 0x33, 0xE0, 0xF4, 0xBD, 0xDE,
   0x37, 0x21, 0x59, 0x30, 0x0C, 0x06, 0x03, 0x55,
   0x1D, 0x13, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01,
   0xFF, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48,
   0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0B, 0x05, 0x00,
   0x03, 0x82, 0x01, 0x01, 0x00, 0x88, 0x3C, 0xB8,
   0xF6, 0xD9, 0x00, 0xF9, 0xE6, 0x95, 0x3A, 0xD7,
   0x46, 0x23, 0x45, 0x8A, 0xAD, 0xD5, 0xCD, 0x46,
   0x24, 0xC8, 0xB0, 0xBF, 0xD6, 0x44, 0x49, 0x8A,
   0x3D, 0x3E, 0x5D, 0xDB, 0x7D, 0x84, 0x05, 0xEF,
   0xD4, 0x6B, 0xAB, 0xAF, 0x36, 0x42, 0x5E, 0x76,
   0xCF, 0x07, 0xBE, 0x16, 0xCD, 0x60, 0x1D, 0x3A,
   0xDE, 0xBC, 0x2E, 0x90, 0x7D, 0xF2, 0xE1, 0x83,
   0x07, 0xCC, 0x74, 0x33, 0x1B, 0x79, 0x3D, 0xB0,
   0xDA, 0x7E, 0xEB, 0xDA, 0xA8, 0x11, 0x66, 0x21,
   0xC8, 0xE4, 0x42, 0xAD, 0xDB, 0x67, 0x8B, 0x9D,
   0x29, 0x82, 0x48, 0xF9, 0xD0, 0xE9, 0x36, 0x8E,
   0xCF, 0xCA, 0x0F, 0x8D, 0x2C, 0x93, 0x80, 0x42,
   0xE2, 0xC6, 0xFF, 0x87, 0x97, 0xCD, 0x66, 0xDF,
   0xAF, 0x79, 0x20, 0x7D, 0x3F, 0x56, 0x21, 0x70,
   0x85, 0xBE, 0x66, 0x96, 0x5B, 0x1D, 0x0D, 0xF4,
   0xFE, 0x6C, 0xE6, 0x04, 0x44, 0x61, 0x3E, 0xED,
   0xA4, 0x12, 0x1D, 0x74, 0xA1, 0xD7, 0x8A, 0x30,
   0xA2, 0xB4, 0xFB, 0x79, 0x2C, 0x83, 0x94, 0xC4,
   0xBA, 0x0E, 0x62, 0x02, 0x9D, 0xD7, 0x73, 0xAA,
   0x5B, 0xCB, 0xF5, 0x05, 0xDD, 0x0C, 0x4F, 0x89,
   0xC5, 0x83, 0x45, 0x98, 0x37, 0x60, 0xCB, 0xEB,
   0x7B, 0xDA, 0x2A, 0x28, 0xBA, 0xE2, 0x6F, 0x38,
   0x97, 0x25, 0x45, 0xE7, 0xA2, 0x51, 0x89, 0x5E,
   0x4E, 0x66, 0xDC, 0x3A, 0x3F, 0x80, 0xC3, 0x71,
   0x53, 0x37, 0x4C, 0xA6, 0x12, 0x13, 0xA4, 0xDC,
   0xD5, 0x6D, 0x96, 0xBA, 0xD2, 0xC2, 0x7E, 0x6F,
   0xEE, 0xD6, 0xAA, 0xC6, 0xE0, 0x01, 0xE4, 0xB3,
   0xFB, 0x34, 0x9B, 0xC7, 0xB1, 0x41, 0xAF, 0xC1,
   0x8D, 0xCE, 0xE3, 0x5C, 0xD2, 0x4C, 0x0E, 0x6A,
   0x00, 0x51, 0x26, 0xAE, 0x7F, 0xB2, 0x76, 0xC8,
   0xD0, 0x86, 0xFB, 0xF6, 0x0B, 0x82, 0xB4, 0x37,
   0x9D, 0x81, 0xFB, 0x4D, 0x6B
};
#else

const unsigned char calist_cert[] = {};

#endif


SSL *ssl_fd;
SSL_CTX *ctx;

/*
static SSL_CTX *ssl_init()
{
	SSL_CTX *ctx;

	SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ctx = SSL_CTX_new(SSLv23_client_method());
    if (ctx == NULL) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }

	return ctx;
}
*/

int vg_tcp_socket(int *sock)
{
	*sock = -1;

	*sock = qcom_socket(AF_INET, SOCK_STREAM, 0);

	if(-1 == *sock){
		LOG(LEVEL_ERROR, "<ERR> creat socket error");
		return -1;
	}
    
    int flag = 1;

	int ret = qcom_setsockopt(*sock, SOL_SOCKET, TCP_NODELAY, (char *)&flag, sizeof(flag));
	if (ret < 0){
		printf("tcp client setsockopt(TCP_NODELAY) error\r\n");
	}
	ret = qcom_setsockopt(*sock, SOL_SOCKET, TCP_NOACKDELAY, (char *)&flag, sizeof(flag));
	if (ret < 0){
		printf("tcp client setsockopt(TCP_NOACKDELAY) error\r\n");
	}

	LOG(LEVEL_DEBUG, "Create sockfd(%d)\n", *sock);
	return 0;
}

// this is to be implemented elsewhere
int vg_tcp_close(int sock, int ssl)
{
	LOG(LEVEL_DEBUG, "Close sockfd(%d)\n", sock);

 	if(ssl){
		//todo
		qca_ssl_close(&sock);
		qca_ssl_client_stop();
	}else
		qcom_socket_close(sock);

	return 0;
}

int vg_send(int sock, unsigned char *buffer, int length, int ssl)
{
	int ret = 0;

    LOG(LOG_DEBUG, "vg_send:\n");
	vg_print_hex(LEVEL_DEBUG, buffer, length);
    LOG(LOG_DEBUG, "\n");

	if(ssl)
		return qca_ssl_write(buffer, length);
	else
		return qcom_send(sock, (char*)buffer, length, 0);
}

/************************************************************************
 * @brief ÅœÃÃÃžÃÃ§Â¶ÃÃÂ¡ÃÂ»Â°ÃŒÂ£Â¬ÂµÂ±ÃÃžÃÃ§ÃÃ¬Â³Â£ÂµÃÂ£Â¬ÃÂ²ÅœÃÃÃ¢ÅŸÃ¶Â·ÂµÂ»ÃÃÂµÃÃÂ·ÂµÂ»ÃÂ£Â¬ÃÃÂ±Ã£ÃÃ·ÃÂ­Â»Â·ÂµÃÂµÅÅœÅ ÃÃ­
 * @param
 * @return 	>0 : Â³ÃÂ¹Å Â¶ÃÂµÅÂµÃÃÃÅÃÃÃœ
 *        		0 : Â³Â¬ÃÂ±Â¶ÃÃÂ¡ÃÂ§Â°ÃÂ£Â¬ÃÃžÃÃ§ÃÂ»ÃÃÃÃœÅžÃ
 *        		-1 : ÃÃžÃÃ§ÃÃ¬Â³Â£
 * @note
************************************************************************/
int vg_recv(int sock, unsigned char *buffer, int length, int ssl)
{
    q_fd_set fdRead;
	struct timeval		tm;
	int					ret;

	tm.tv_sec  = TCP_TIMEOUT_S;
	tm.tv_usec = TCP_TIMEOUT_MS; //2S

	FD_ZERO(&fdRead);
	FD_SET(sock, &fdRead);

	ret = qcom_select(sock + 1, &fdRead, NULL, NULL, &tm);

	if(ret < 0){// error
		LOG(LEVEL_ERROR, "mqtt sockfd select error\n");
		return -1;
	}else if(ret == 0){
		LOG(LEVEL_DEBUG, "mqtt sockfd select timeout\n");
		return 0;
	}else{
	    if(FD_ISSET(sock, &fdRead)){

			if(ssl)
				ret = qca_ssl_read((unsigned char *)(buffer), length);
			else
				ret = qcom_recv(sock, (char *)(buffer), length, 0);
			if(ret > 0){
				LOG(LEVEL_DEBUG, "vg_recv: \n");
				vg_print_hex(LEVEL_DEBUG, (char *)buffer, ret);

				return ret;
			}else if(ret < 0){//recv err
				LOG(LEVEL_ERROR, "<ERR> mqtt recv error(errno:%d)\n", ret);
				return -1;
			}else{// == 0 //socket close
				LOG(LEVEL_ERROR, "<ERR> mqtt close socket\n");
				return -3;// careful : sometime select =1 ,but read 0 byte data
			}
	    }else{
			LOG(LEVEL_ERROR, "<ERR> mqtt sockfd is not set\n");
		}
	}
	return 0;
}

int vg_udp_socket(int *sock)
{

	*sock = -1;

	*sock = qcom_socket(AF_INET, SOCK_DGRAM, 0);

	if(-1 == *sock){
		printf( "<ERR> creat socket error");
		return -1;
	}

	printf( "Create sockfd(%d)\n", *sock);
	return 0;
}

int vg_udp_close(int sock)
{

	printf( "Close sockfd(%d)\n", sock);
    qcom_socket_close(sock);

	return 0;
}

int vg_recvfrom(int s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen) {
    return qcom_recvfrom(s, buf, len, flags, from, fromlen);  
}

int vg_sendto(int sock, unsigned char *buffer, int length, struct sockaddr_in *client_addr){
    return qcom_sendto(sock, buffer, length, 0, client_addr, sizeof(struct sockaddr_in));
}

int vg_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {

    return qcom_bind(sockfd, addr, addrlen);
}

static int get_host_by_name(unsigned int *ipaddr, char *url)
{
	int count = 0;

	while(count++ < 3)
	{
		if(qcom_dnsc_get_host_by_name(url, ipaddr) == A_OK)
		{
			LOG(LEVEL_DEBUG, "<LOG> Get ip address is %u.%u.%u.%u\n",
				(unsigned)(*ipaddr >> 24), (unsigned)((*ipaddr >> 16) & 0xff),
				(unsigned)((*ipaddr >> 8) & 0xff), (unsigned)(*ipaddr & 0xff));
			return 0;
		}

		LOG(LEVEL_DEBUG, "<LOG> get ip address of host %s failed, retry...\n", url);
		msleep(1000);
	}

	return -1;
}


int vg_connect_server(int *sock, char *url, int port, int ssl/*0:nossl, 1:ssl*/)
{

	struct sockaddr_in address;
	int conn = -1, isip = 0;
	char ip[32];
	int ret = 0;
	int try_times = 0;

	unsigned int ipaddr = 0;

    qcom_tcp_conn_timeout(5);

again:

	ip[0] = 0;

	isip = is_valid_ip(url);
	if(isip == 0){
	}else if(isip == 1){
		strcpy(ip, url);
	}else{
		LOG(LEVEL_ERROR, "<ERR> error url.\n");
		return -1;
	}

	ret = get_host_by_name(&ipaddr, url);
	if(ret == -1){
		return -2;
	}

	sprintf(ip, "%u.%u.%u.%u", (unsigned)(ipaddr >> 24),
            (unsigned)((ipaddr >> 16) & 0xff), (unsigned)((ipaddr >> 8) & 0xff), (unsigned)(ipaddr & 0xff));
	address.sin_addr.s_addr		= htonl(ipaddr);

	address.sin_port		= htons(port);
	address.sin_family		= AF_INET;
	memset(address.sin_zero,0,sizeof(address.sin_zero));

	LOG(LEVEL_DEBUG, "<LOG> connect ip is %s:%d\n", ip, port);

	if(ssl)
	{
		//todo
		conn = qca_ssl_client_start((A_UINT8 *)calist_cert, sizeof(calist_cert));
		if(conn == A_ERROR)
		{
			return conn;
		}
		conn = qca_ssl_connect(sock, ipaddr, port);
	}
	else
		conn = qcom_connect(*sock,(struct sockaddr*)&address, sizeof(struct sockaddr_in));

	LOG(LEVEL_DEBUG, "connect return %d\n", conn);
/*
	if(SOCKET_ERROR == conn)
	{
		try_times++;
		if(try_times >= TCP_TRY_TIMES)
			return conn;
		else
			goto again;
	}
*/
	return conn;
}

int vg_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	LOG(LEVEL_DEBUG, "vg_accept E\n");
    int fd = qcom_accept(sockfd, addr, addrlen);
	LOG(LEVEL_DEBUG, "vg_accept X\n");
	return fd;
}

int vg_listen(int sock, int backlog)
{
	LOG(LEVEL_DEBUG, "vg_listen E\n");
    qcom_listen(sock, backlog);
	LOG(LEVEL_DEBUG, "vg_listen X\n");
}

