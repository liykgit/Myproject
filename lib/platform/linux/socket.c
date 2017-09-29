

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


	*sock = socket(AF_INET, SOCK_STREAM, 0);

	if(-1 == *sock){
		LOG(LEVEL_ERROR, "<ERR> creat socket error");
		return -1;
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
    close(sock);
	if(ssl){
		SSL_free(ssl_fd);
		SSL_CTX_free(ctx);
	}
	return 0;
}

int vg_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	LOG(LEVEL_DEBUG, "vg_accept E\n");
    int fd = accept(sockfd, addr, addrlen);
	LOG(LEVEL_DEBUG, "vg_accept X\n");
	return fd;
}

int vg_listen(int sock, int backlog)
{
	LOG(LEVEL_DEBUG, "vg_listen E\n");
    listen(sock, backlog);
	LOG(LEVEL_DEBUG, "vg_listen X\n");
}

int vg_send(int sock, unsigned char *buffer, int length, int ssl)
{

    LOG(LEVEL_DEBUG, "vg_send:\n");
	vg_print_hex(LEVEL_DEBUG, buffer, length);
    LOG(LEVEL_DEBUG, "\n");


	if(ssl)
		return SSL_write(ssl_fd, buffer, length);
	else
		return send(sock, (char*)buffer, length, 0);
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
    fd_set fdRead;
	struct timeval		tm;
	int					ret;
	tm.tv_sec  = TCP_TIMEOUT_S;
	tm.tv_usec = TCP_TIMEOUT_MS; //2S

	FD_ZERO(&fdRead);
	FD_SET(sock, &fdRead);

	ret = 1;

    if(FD_ISSET(sock, &fdRead)){
        if(ssl)
            ret = SSL_read(ssl_fd, buffer, length);
        else
            ret = recv(sock, (char*)buffer, length, 0);
        if(ret > 0){
            LOG(LEVEL_DEBUG, "vg_recv: \n");
            vg_print_hex(LEVEL_DEBUG, (char *)buffer, ret);

            return ret;
        }else if(ret < 0){//recv err
            LOG(LEVEL_ERROR, "<ERR> vg recv error(errno:%d)\n", errno);
            if(errno == 104 || errno == 9)
                return -4;
            else
                return -2;
        }else{// == 0 //socket close
            LOG(LEVEL_ERROR, "mqtt close socket\n");
            return -3;// careful : sometime select =1 ,but read 0 byte data
        }
    }else{
        LOG(LEVEL_ERROR, "mqtt sockfd is not set\n");
    }

	return 0;
}

int vg_udp_socket(int *sock)
{

	*sock = -1;

	*sock = socket(AF_INET, SOCK_DGRAM, 0);

	if(-1 == *sock){
		printf( "<ERR> creat socket error");
		return -1;
	}
    
    int broadcastEnable = 1;
    setsockopt(*sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
	LOG(LEVEL_DEBUG, "Create sockfd(%d)\n", *sock);
	return 0;
}

int vg_udp_close(int sock)
{

	LOG(LEVEL_DEBUG, "Close sockfd(%d)\n", sock);
    close(sock);

	return 0;
}

int vg_recvfrom(int s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen) {
    return recvfrom(s, buf, len, flags, from, fromlen);  
}


int vg_sendto(int sock, unsigned char *buffer, int length, struct sockaddr_in *client_addr){

    return sendto(sock, buffer, length, 0, client_addr, sizeof(struct sockaddr_in));
}

int vg_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {

    return bind(sockfd, addr, addrlen);
}

int vg_connect_server(int *sock, char *url, int port, int ssl/*0:nossl, 1:ssl*/)
{
	struct sockaddr_in address;
	int conn = -1, isip = 0;
	char ip[32];
	int ret = 0;
	int try_times = 0;

	struct hostent *hptr = NULL;

again:

	if(ssl){
		ctx = ssl_init();
	}

	ip[0] = 0;

	isip = is_valid_ip(url);
	if(isip == 0){
		hptr = gethostbyname(url);
		if(hptr){
			switch(hptr->h_addrtype)
			{
			case AF_INET:
			case AF_INET6:
				inet_ntop(hptr->h_addrtype, hptr->h_addr, ip, sizeof(ip));
				break;
			default:
				LOG(LEVEL_ERROR, "<ERR> unknown address type\n");
				return -1;
				break;
			}
		}else
			return -1;
	}else if(isip == 1){
		strcpy(ip, url);
	}else{
		LOG(LEVEL_ERROR, "<ERR> error url.\n");
		return -1;
	}

	address.sin_addr.s_addr	= inet_addr(ip);
	address.sin_port		= htons(port);
	address.sin_family		= AF_INET;
	memset(address.sin_zero,0,sizeof(address.sin_zero));

	LOG(LEVEL_DEBUG, "<LOG> connect ip is %s:%d\n", ip, port);

	conn = connect(*sock,(struct sockaddr*)&address,sizeof(struct sockaddr));
	if(-1 == conn)
	{
		try_times++;
		if(try_times >= TCP_TRY_TIMES)
			return conn;
		else
			goto again;
	}
	if(ssl){
		/* 基于 ctx 产生一个新的 SSL */
    	ssl_fd = SSL_new(ctx);
    	SSL_set_fd(ssl_fd, conn);
    	/* 建立 SSL 连接 */
    	if (SSL_connect(ssl_fd) == -1){
        	ERR_print_errors_fp(stderr);
		}else {
        	printf("Connected with %s encryption\n", SSL_get_cipher(ssl_fd));
        	show_certs(ssl_fd);
    	}
	}
	return 0;
}

int vg_resolve_domain_name(char *domain_name, char *output, int output_max) {

    struct addrinfo* result;
    struct addrinfo* res;
    int error;
    int ret = -1;
    /* resolve the domain name into a list of addresses */
    error = getaddrinfo(domain_name, NULL, NULL, &result);
    if (error != 0) {   
        if (error == EAI_SYSTEM) {
            perror("getaddrinfo");
        } else {
            LOG(LEVEL_ERROR, "error in getaddrinfo: %s\n", gai_strerror(error));
        }   
        exit(EXIT_FAILURE);
    }   

    /* loop over all returned results and do inverse lookup */
    for (res = result; res != NULL; res = res->ai_next) {   
        char hostname[NI_MAXHOST];
        error = getnameinfo(res->ai_addr, res->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, 0); 
        if (error != 0) {
            fprintf(stderr, "error in getnameinfo: %s\n", gai_strerror(error));
            continue;
        }

        int l = strlen(hostname);
        if(l>0 && (l+1) < output_max) {
            LOG(LEVEL_DEBUG, "hostname: %s\n", hostname);
            strcpy(output, hostname);
            ret = 0;
            //break;
        }
    }   

    freeaddrinfo(result);
    return ret;
}

