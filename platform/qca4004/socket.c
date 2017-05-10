

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <stdlib.h>
#include "hash.h"
#include "plat.h"

SSL *ssl_fd;
SSL_CTX *ctx;

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

int vg_tcp_socket(int *sock)
{
	*sock = -1;

	*sock = qcom_socket(AF_INET, SOCK_STREAM, 0);

	if(-1 == *sock){
		LOG(LEVEL_ERROR, "<ERR> creat socket error");
		return -1;
	}

	ret = qcom_setsockopt(*sock, SOL_SOCKET, TCP_NODELAY, (char *)&flag, sizeof(flag));
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

static void show_certs(SSL * ssl)
{
    X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl);
    if (cert != NULL) {
        printf("数字证书信息:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("证书: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("颁发者: %s\n", line);
        free(line);
        X509_free(cert);
    } else
        printf("无证书信息！\n");

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

int vg_tcp_send(int sock, unsigned char *buffer, int length, int ssl)
{
	int ret = 0;

	LOG(LEVEL_DEBUG, "<LOG> send :\n");

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
int vg_tcp_recv(int sock, unsigned char *buffer, int length, int ssl)
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
		LOG(LEVEL_ERROR, "<ERR> mqtt sockfd select error\n");
		return -1;
	}else if(ret == 0){
		LOG(LEVEL_DEBUG, "<LOG> mqtt sockfd select timeout\n");
		return 0;
	}else{
	    if(FD_ISSET(sock, &fdRead)){

			if(ssl)
				ret = qca_ssl_read((unsigned char *)(buffer), length);
			else
				ret = qcom_recv(sock, (char *)(buffer), length, 0);
			if(ret > 0){
				LOG(LEVEL_DEBUG, "<LOG> recv: \n");
				log_buffer(LEVEL_DEBUG, (char *)buffer, ret);

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

int vg_connect_server(int *sock, char *url, int port, int ssl/*0:nossl, 1:ssl*/)
{

	struct sockaddr_in address;
	int conn = -1, isip = 0;
	char ip[32];
	int ret = 0;
	int try_times = 0;

	unsigned int ipaddr = 0;

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

	if(SOCKET_ERROR == conn)
	{
		try_times++;
		if(try_times >= TCP_TRY_TIMES)
			return conn;
		else
			goto again;
	}

	return 0;
}

