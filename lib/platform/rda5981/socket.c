#include "plat.h"

#define DEBUG_LEVEL 1

#if DEBUG_LEVEL > 0
#include "mbedtls/debug.h"
#endif

#if (CONN_MODE == CONN_SSL)
const unsigned char SSL_CA_PEM[] = 
/* GlobalSign Root certificate */
"-----BEGIN CERTIFICATE-----\n"
"MIIDgTCCAmmgAwIBAgIJAKvFPKJaCCNzMA0GCSqGSIb3DQEBCwUAMFcxCzAJBgNV\n"
"BAYTAkNOMQswCQYDVQQIDAJHRDELMAkGA1UEBwwCU1oxDjAMBgNVBAoMBVZlbmdh\n"
"MQ4wDAYDVQQLDAVWZW5nYTEOMAwGA1UEAwwFVmVuZ2EwHhcNMTcxMTE2MDMzNTI3\n"
"WhcNMzcxMTExMDMzNTI3WjBXMQswCQYDVQQGEwJDTjELMAkGA1UECAwCR0QxCzAJ\n"
"BgNVBAcMAlNaMQ4wDAYDVQQKDAVWZW5nYTEOMAwGA1UECwwFVmVuZ2ExDjAMBgNV\n"
"BAMMBVZlbmdhMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAtJEfMGyw\n"
"K6x9s+7ijZ51lgkQ5zUfAwE5XpiOfPLhe1783dilcyVLN8XmoJRPsd91xEBTGug9\n"
"0VizB6vz9RLFFa+QqQLXaDGt1FjFVdw9xgGjqzTSqwVNgdpyEbz6yLY/k1Ybyz8S\n"
"Dso9obfvCS/yAPCCIgVa24eP48T7D8nC+uiCpdukeUssgAnivn/KdXDGuiMEDFyx\n"
"AYVo8Wou6AqN/Akt0t+n0nUw+8cTeEH5p/rGjnId2TW0fA9oshzwzcczgG8y8mpV\n"
"i8o3FU5/thozMSnZo3Eky2KyWadIQrveFYH69O2ucrXmWyK36VjwAPtyYZi69gtD\n"
"BnFUQY/uG9VaOwIDAQABo1AwTjAdBgNVHQ4EFgQUHijoFvnyZqe6atdhM+D0vd43\n"
"IVkwHwYDVR0jBBgwFoAUHijoFvnyZqe6atdhM+D0vd43IVkwDAYDVR0TBAUwAwEB\n"
"/zANBgkqhkiG9w0BAQsFAAOCAQEAE+9uL0tToE5qtYYRo0C4abpzxvpUa7DcbROd\n"
"PDYPi95zuOunWc32i8w5fHxs3LPAswfI5I8+BfH3/6DWDFDX0rmryoQ75X92JtDI\n"
"ktPr+M1W+3bl2ELRrPz/gTESXhHoorhI0AS646sGukQlPCefesqrR77yiOkpd560\n"
"O9Yi4f9SWSS6Z6ARujM0sjljRE0LQw4t7fc1xWuo89drIom4oVpYuBNFEvn24IYY\n"
"DVc07+S/nUwdTzlze6w5AyfoPyHAPGlyCTBJzt+4VAB1kwNhGR7F6vsqtt5dKChx\n"
"iyC6l7d1G6ycNJ2HeyxXvj/d085h2b6wcf5xDJXWOPCrB5ixqA==\n"
"-----END CERTIFICATE-----\n";
#else
unsigned char SSL_CA_PEM[] ={};
#endif

const char *DRBG_PERS = "TLS client";

// mbedtls_ssl_context *ssl_fd;
// SSL_CTX *ctx;
#ifndef MBEDTLS_DRBG_ALT
mbedtls_entropy_context _entropy;
mbedtls_ctr_drbg_context _ctr_drbg;
#endif

mbedtls_x509_crt _cacert;
mbedtls_ssl_context _ssl;
mbedtls_ssl_config _ssl_conf;

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

	*sock = socket(AF_INET, SOCK_STREAM, 0);

	if(-1 == *sock){
		LOG(LEVEL_ERROR, "<ERR> creat socket error");
		return -1;
	}
    
    int flag = 1;

	int ret = setsockopt(*sock, SOL_SOCKET, TCP_NODELAY, (char *)&flag, sizeof(flag));
	if (ret < 0){
		printf("tcp client setsockopt(TCP_NODELAY) error\r\n");
	}
	// ret = setsockopt(*sock, SOL_SOCKET, TCP_NOACKDELAY, (char *)&flag, sizeof(flag));
	// if (ret < 0){
	// 	printf("tcp client setsockopt(TCP_NOACKDELAY) error\r\n");
	// }

	LOG(LEVEL_DEBUG, "Create sockfd(%d)\n", *sock);
	return 0;
}

// this is to be implemented elsewhere
int vg_tcp_close(int sock, int ssl)
{
	LOG(LEVEL_DEBUG, "Close sockfd(%d)\n", sock);

 	if(ssl){
		//todo
		//qca_ssl_close(&sock);
		//qca_ssl_client_stop();
#ifndef MBEDTLS_DRBG_ALT
        mbedtls_entropy_free(&_entropy);
        mbedtls_ctr_drbg_free(&_ctr_drbg);
#endif
        mbedtls_x509_crt_free(&_cacert);
        mbedtls_ssl_free(&_ssl);
        mbedtls_ssl_config_free(&_ssl_conf);

	}else
		closesocket(sock);

	return 0;
}

int vg_send(int sock, unsigned char *buffer, int length, int ssl)
{
	LOG(LEVEL_DEBUG, "<LOG> send :\n");

	if(ssl)
		return mbedtls_ssl_write(&_ssl, (const unsigned char *)buffer, (unsigned int)length);
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

	ret = select(sock + 1, &fdRead, NULL, NULL, &tm);

	if(ret < 0){// error
		LOG(LEVEL_ERROR, "<ERR> mqtt sockfd select error\n");
		return -1;
	}else if(ret == 0){
		LOG(LEVEL_DEBUG, "<LOG> mqtt sockfd select timeout\n");
		return 0;
	}else{
	    if(FD_ISSET(sock, &fdRead)){

			if(ssl)
				ret = mbedtls_ssl_read(&_ssl, (unsigned char *)buffer, (unsigned int)length);
			else
				ret = recv(sock, (char *)buffer, length, 0);
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

	*sock = socket(AF_INET, SOCK_DGRAM, 0);

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
    closesocket(sock);

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

// static int get_host_by_name(unsigned int *ipaddr, char *url)
// {
// 	int count = 0;

// 	while(count++ < 3)
// 	{
// 		if(qcom_dnsc_get_host_by_name(url, ipaddr) == A_OK)
// 		{
// 			LOG(LEVEL_DEBUG, "<LOG> Get ip address is %u.%u.%u.%u\n",
// 				(unsigned)(*ipaddr >> 24), (unsigned)((*ipaddr >> 16) & 0xff),
// 				(unsigned)((*ipaddr >> 8) & 0xff), (unsigned)(*ipaddr & 0xff));
// 			return 0;
// 		}

// 		LOG(LEVEL_DEBUG, "<LOG> get ip address of host %s failed, retry...\n", url);
// 		msleep(1000);
// 	}

// 	return -1;
// }

int is_valid_ip_sock(char* str)  
{  
    if(str == NULL)  
    {  
        return 0;  
	} 
	 
    char a[40] = {0};  
	
	if(sscanf(str,"%d.%d.%d.%d",&a[0],&a[1],&a[2],&a[3]) != 4)  
    {  
        return 0;  
    }  
  
    for(int i = 0; i < 4; ++i)  
    {  
        if(a[i] < 0 || a[i] > 255)  
        {  
            return 0;  
        }  
	}  
	
    return 1;  
}  

static int ssl_recv(void *ctx, unsigned char *buf, size_t len) 
{
	int recv = -1;
	int *sock = (int *)ctx;

	recv = recv(*sock, buf, len, 0);

	if(recv < 0){
		return -1;
	}else{
		return recv;
	}
}

static int ssl_send(void *ctx, const unsigned char *buf, size_t len) 
{
   	int size = -1;
	int *sock = (int *)ctx;

	size = send(*sock, buf, len, 0);

	if(size < 0){
		return -1;
	}else{
		return size;
	}
}

#if DEBUG_LEVEL > 0
    /**
     * Debug callback for mbed TLS
     * Just prints on the USB serial port
     */
    static void my_debug(void *ctx, int level, const char *file, int line,
                         const char *str)
    {
        const char *p, *basename;
        (void) ctx;

        /* Extract basename from file */
        for(p = basename = file; *p != '\0'; p++) {
            if(*p == '/' || *p == '\\') {
                basename = p + 1;
            }
        }

        mbedtls_printf("%s:%04d: |%d| %s", basename, line, level, str);
    }

    /**
     * Certificate verification callback for mbed TLS
     * Here we only use it to display information on each cert in the chain
     */
    static int my_verify(void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags)
    {
        char buf[1024];
        (void) data;

        mbedtls_printf("\nVerifying certificate at depth %d:\n", depth);
        mbedtls_x509_crt_info(buf, sizeof (buf) - 1, "  ", crt);
        mbedtls_printf("%s", buf);

        if (*flags == 0)
            mbedtls_printf("No verification issue for this certificate\n");
        else
        {
            mbedtls_x509_crt_verify_info(buf, sizeof (buf), "  ! ", *flags);
            mbedtls_printf("%s\n", buf);
        }

        return 0;
    }
#endif

static void print_mbedtls_error(const char *name, int err) {
        char buf[128];
        mbedtls_strerror(err, buf, sizeof (buf));
        mbedtls_printf("%s() failed: -0x%04x (%d): %s\r\n", name, -err, err, buf);
}

int vg_connect_server(int *sock, char *url, int port, int ssl/*0:nossl, 1:ssl*/)
{

	struct sockaddr_in address;
	int conn = -1, isip = 0;
	char ip[32];
	int ret = 0;
	int try_times = 0;

	//unsigned int ipaddr = 0;
	struct hostent *hptr = NULL;	

again:

	//if(ssl){
	//	ctx = ssl_init();
	//}

	ip[0] = 0;

	isip = is_valid_ip_sock(url);
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
	memset(address.sin_zero, 0, sizeof(address.sin_zero));

	LOG(LEVEL_DEBUG, "<LOG> connect ip is %s:%d\n", ip, port);

	if(ssl)
	{
		//todo
		//conn = qca_ssl_client_start((A_UINT8 *)calist_cert, sizeof(calist_cert));
		//if(conn == A_ERROR)
		//{
		//	return conn;
		//}
		//conn = qca_ssl_connect(sock, ipaddr, port);
#ifndef MBEDTLS_DRBG_ALT
        mbedtls_entropy_init(&_entropy);
        mbedtls_ctr_drbg_init(&_ctr_drbg);
#endif
		mbedtls_x509_crt_init(&_cacert);
		mbedtls_ssl_init(&_ssl);
		mbedtls_ssl_config_init(&_ssl_conf);	

#ifndef MBEDTLS_DRBG_ALT
		if ((ret = mbedtls_ctr_drbg_seed(&_ctr_drbg, mbedtls_entropy_func, &_entropy,
                          (const unsigned char *) DRBG_PERS,
                          sizeof (DRBG_PERS))) != 0) {
            printf("mbedtls_crt_drbg_init", ret);
            return -1;
        }
#endif
			
		if ((ret = mbedtls_x509_crt_parse(&_cacert, (const unsigned char *) SSL_CA_PEM,
                           sizeof (SSL_CA_PEM))) != 0) {
			printf("mbedtls_x509_crt_parse() error(%d)\n", ret);
			return -1;
		}	

		conn = mbedtls_ssl_config_defaults(&_ssl_conf, MBEDTLS_SSL_IS_CLIENT, 
			MBEDTLS_SSL_TRANSPORT_STREAM, 
			MBEDTLS_SSL_PRESET_DEFAULT);
		if(conn)
		{
			printf("mbedtls_ssl_config_defaults() error\n");
			return -1;
		}

		mbedtls_ssl_conf_ca_chain(&_ssl_conf, &_cacert, NULL);

#ifndef MBEDTLS_DRBG_ALT
        mbedtls_ssl_conf_rng(&_ssl_conf, mbedtls_ctr_drbg_random, &_ctr_drbg);
#else
        mbedtls_ssl_conf_rng(&_ssl_conf, mbedtls_ctr_drbg_random, NULL);
#endif
		
#if 0
        mbedtls_ssl_conf_authmode(&_ssl_conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
#endif

#if DEBUG_LEVEL > 0
        mbedtls_ssl_conf_verify(&_ssl_conf, my_verify, NULL);
        mbedtls_ssl_conf_dbg(&_ssl_conf, my_debug, NULL);
        mbedtls_debug_set_threshold(DEBUG_LEVEL);
#endif

		if ((ret = mbedtls_ssl_setup(&_ssl, &_ssl_conf)) != 0) {
            printf("mbedtls_ssl_setup() error(%d)\n", ret);
            return -1;
        }

		//mbedtls_ssl_set_hostname(&_ssl, "venga");

		mbedtls_ssl_set_bio(&_ssl, (void *)sock, ssl_send, ssl_recv, NULL);
		
		printf("Connecting with %s\r\n", ip);
		conn = connect(*sock, (struct sockaddr*)&address, sizeof(struct sockaddr_in));
		if(conn)
		{
			int flag = 0;
			getsockopt(*sock, SOL_SOCKET, SO_ERROR, (char *)&flag, sizeof(flag));
			printf("Socket connect failed(conn=%d, err=%d)\n", conn, flag);
		}
		else
		{
			printf("Connected...\n");
		}

		ret = mbedtls_ssl_handshake(&_ssl);
		if (ret < 0) 
		{
            if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) 
			{
                print_mbedtls_error("mbedtls_ssl_handshake", ret);
            }
            return -1;
        }
	}
	else
	{
		conn = connect(*sock, (struct sockaddr*)&address, sizeof(struct sockaddr_in));

		if(-1 == conn)
		{
			try_times++;
			if(try_times >= TCP_TRY_TIMES)
				return conn;
			else
			{
				int flag = 0;
				getsockopt(*sock, SOL_SOCKET, SO_ERROR, (char *)&flag, sizeof(flag));
				printf("socket connect failed(conn=%d, err=%d)\n", conn, flag);
				goto again;
			}
		}
		else
		{
			printf("connected...\n");
		}
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

