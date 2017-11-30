
#include "plat.h"
#include "http.h"

//#include "lwip/sockets.h"

#define HTTP_CLIENT_BUF_SIZE 1024
#define SERVER_NAME_MAX_SIZE 32

//static unsigned char *http_client_data_buf = NULL;
static unsigned char http_client_data_buf[HTTP_CLIENT_BUF_SIZE];
int http_socket_fd = 0;
static char server_addr[SERVER_NAME_MAX_SIZE];


unsigned int str2ip(char *str)
{
	unsigned int ipaddr;
	unsigned int data[4];
	unsigned int ret;

	ret = sscanf(str, "%3d.%3d.%3d.%3d", data, data + 1, data + 2, data + 3);

	if (ret < 0) {
		return 0;
	}
	else {
		ipaddr = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
	}
	return ipaddr;
}

int IsaIPv4(const char *psz)
{
	extern unsigned int isdigit(int c);
	if(strlen(psz) > 15 || strlen(psz) < 7) return -1;
	int iPoint = 0;
	const char *b = psz;
	while(*psz)
	{
		if(*psz == '.') 
		{
			if(psz-b < 1) return -1;
			if(psz-b > 1 && *b == '0') return -1;
			if(atoi(b) > 255) return -1;
			iPoint++;
			b = psz+1;
		}
		else if(!isdigit(*psz)) return -1;
		psz++;
	}
	if(iPoint != 3) return -1;
	if(psz-b < 1) return -1;
	if(psz-b > 1 && *b == '0') return -1;
	if(atoi(b) > 255) return -1;

	return 0;
}

/**
***************************************************************************
*@brief     separate string by separator
*@param[in] from: buffer that want to separate
            delim: separator
            to: save return separate result
            item_num: splited data nums
*@return    succeed: 0
            failed: 1
*@warning   remenber to free *to, so have to call explode_data_free() after called explode()
*@see
***************************************************************************
*/
unsigned char explode(char *from, char delim, char ***to, unsigned int *item_num)
{
    unsigned int i, k, n;
    unsigned int from_len = strlen(from);
    char **ret;

    for(i=0, n=1; from[i]!='\0'; i++){
        if (from[i] == delim){
            n++;
            from[i] = '\0';
        }
    }

    ret = (char **)vg_malloc(n*sizeof(char *));
    if(ret == NULL){
        LOG(LEVEL_ERROR, "Malloc failed\n");
        return 1;
    }

    ret[0] = from;
    for(i = 0, k = 1; i  < from_len && k < n; i++){
        if(from[i] == '\0'){
            ret[k] = &from[i + 1];
            k++;
        }
    }
    *to = ret;
    *item_num = n;

    return 0;
}
/**
***************************************************************************
*@brief     Free data which malloc by explode
            if you call explode once, and after used 'to',
            then you have to call this functions
*@param[in] to: pointer which point to malloc buffer
*@return    none
*@warning
*@see
***************************************************************************
*/
static void explode_data_free(char ***to)
{
    if(*to != NULL){
        vg_free((void *)*to);
        *to = NULL;
    }
}

static int send_header(char *method, char *path, unsigned int content_len, char *content_type)
{
    int ret = 0;
    char *column = (char *)http_client_data_buf;

    memset(column, '\0', HTTP_CLIENT_BUF_SIZE);

    sprintf(column, "%s %s HTTP/1.1\r\n", method, path);

    sprintf(column + strlen(column), "Host: %s\r\n", server_addr);

    sprintf(column + strlen(column), "Accept: text/html, */*\r\n");

    if(content_type == NULL){
        sprintf(column + strlen(column), "Content-Type: application/octet-stream\r\n");
    }else{
        sprintf(column + strlen(column), "Content-Type: %s\r\n", content_type);
    }

    sprintf(column + strlen(column), "Content-length: %d\r\n", content_len);

    sprintf(column + strlen(column), "Connection: keep-alive\r\n");

    sprintf(column + strlen(column), "\r\n");

    ret = vg_send(http_socket_fd, column, strlen(column), 0);
//	printf("Send:%s\n", column);

    return ret;
}

static int send_one_block_body(unsigned char *data, unsigned int length)
{
    int ret = 0;
    int remain = 0;
    int element = 0;
    int already_send = 0;

    if(data == NULL)
        return -1;

    remain = length;
    already_send = 0;
    while(remain > 0){
        element = remain > HTTP_CLIENT_BUF_SIZE ? HTTP_CLIENT_BUF_SIZE : remain;

        ret = vg_send(http_socket_fd, (char *)(data + already_send), element, 0);
        if(ret == -100)
            continue;
        else if(ret < 0)
            break;

        remain -= element;
        already_send += element;
    }

    return ret;
}

static int parse_header(char **content, unsigned int *content_length, char *pkg)
{
    char *plen = NULL;

    if (memcmp(pkg, "HTTP", 4) != 0)
    {
        return -1;
    }

    if (strstr(pkg, "200 OK") == NULL)
    {
        return -1;
    }

    plen = strstr(pkg, "Content-Length:");
    if(plen != NULL){
        *content_length = atoi(plen + sizeof("Content-Length:"));
        printf("content_length:%d\r\n", *content_length);
    }else{
        return -1;
    }

    *content = strstr(pkg, "\r\n\r\n");
    if(*content != NULL){
        *content += 4;
    }else{
        return -1;
    }

    return 0;
}

static int get_response(get_callback_t get_cb)
{
    int				ret = 0;
    int				recvBytes = 0;
    char			*content = NULL;
    unsigned int	content_length = 0;
    unsigned int	header_length = 0;

	static fd_set sockSet;
	struct timeval	tmo;

    while(1)
	{
		//== Init fd_set
		FD_ZERO(&sockSet);
		FD_SET(http_socket_fd, &sockSet);

		//== select time
		tmo.tv_sec = 30;
		tmo.tv_usec = 0;

		//== select
		ret = vg_select(http_socket_fd + 1, &sockSet, NULL, NULL, &tmo);
		if (ret <= 0)
		{
			break;
		}

		//== check event
		ret = FD_ISSET(http_socket_fd, &sockSet);
		if (ret == 0)
		{
	
			continue;
		}
		else if (ret == -1)
		{
			printf("Server close socket.\n");
			break;
		}


        recvBytes = vg_recv(http_socket_fd, (char*)http_client_data_buf, HTTP_CLIENT_BUF_SIZE, 0);
        if(recvBytes > 0)
		{
            content = (char *)http_client_data_buf;

            // First block
            if(content_length == 0){
                ret = parse_header(&content, &content_length, (char *)http_client_data_buf);
                if(ret != 0)
                    return -1;

                header_length = (unsigned int)content - (unsigned int)http_client_data_buf;

                recvBytes -= header_length;
            }

            content_length -= recvBytes;

	//		printf("content len:%d, recvbyte:%d\n", content_length, recvBytes);

            get_cb((unsigned char *)content, recvBytes);
            if(content_length == 0){
                break;
            }
        }
		else
		{
            break;
        }
    }

	return 0;
}


int vg_http_client_startup(unsigned char *domain, unsigned short port)
{
    unsigned int		server_ip;
    int					ret = 0;
    struct sockaddr_in	remoteAddr;
    char				**obj_buf = NULL;
	unsigned int				obj_nums;

//     http_client_data_buf = mem_alloc(HTTP_CLIENT_BUF_SIZE);
//     if (NULL == http_client_data_buf)
// 	{
//         A_PRINTF("malloc httpc_data_buf failed.\n");
//         return -1;
//     }
// 	

    explode((char *)domain, '/', &obj_buf, &obj_nums);

	memset(server_addr, 0, sizeof(server_addr));
    if(obj_nums >= 3 && strncmp(obj_buf[0], "http", 4) == 0)
	{
        strcpy(server_addr, obj_buf[2]);
    }
	else
	{
        strcpy(server_addr, obj_buf[0]);
    }

 	
// 	if (server_ip == 0)
	if (IsaIPv4(server_addr) == 0)
	{
		printf("ipv4\n");
		server_ip = str2ip(server_addr);
	}
	else
	{
		LOG(LEVEL_ERROR, "dns resolve is not implemented\n");

        //vg_resolve_domain_name(server_addr, char *output, int output_max) {

	}

    LOG(LEVEL_DEBUG, "Server Addr:%s, ip:%x\n", server_addr, server_ip);
	LOG(LEVEL_DEBUG, "%d.%d.%d.%d\r\n", 
		(server_ip) & 0xFF, (server_ip) >> 8 & 0xFF,
		(server_ip) >> 16 & 0xFF, (server_ip) >> 24 & 0xFF);

    http_socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (http_socket_fd < 0)
	{
        LOG(LEVEL_ERROR, "Open http client socket error.\n");
        goto out;
    }

	struct linger ling= { 1, 1 };
	ret = setsockopt(http_socket_fd, SOL_SOCKET, SO_LINGER, (void *)&ling, sizeof(ling));
	if (ret < 0) 
	{
		printf("tcp client setsockopt error\r\n");			
	}

    /* Set socket param */
    memset(&remoteAddr, 0, sizeof (struct sockaddr_in));
    remoteAddr.sin_addr.s_addr = htonl(server_ip);
    remoteAddr.sin_port = htons(port);
    remoteAddr.sin_family = AF_INET;

    ret = connect(http_socket_fd, (struct sockaddr *)&remoteAddr, sizeof(struct sockaddr_in));
    if(ret != 0)
	{
        LOG(LEVEL_ERROR, "Connect to server failed.\n");
        goto out;
    }

	LOG(LEVEL_DEBUG, "Connect %lu.%lu.%lu.%lu OK, %d\r\n",
		(remoteAddr.sin_addr.s_addr) & 0xFF, (remoteAddr.sin_addr.s_addr) >> 8 & 0xFF,
		(remoteAddr.sin_addr.s_addr) >> 16 & 0xFF, (remoteAddr.sin_addr.s_addr) >> 24 & 0xFF, http_socket_fd);

	if(obj_buf != NULL)
    explode_data_free(&obj_buf);

    return 0;

out:
//     if (http_client_data_buf != NULL)
// 	{
//         mem_free(http_client_data_buf);
//         http_client_data_buf = NULL;
//     }
    if(obj_buf != NULL)
	{
		explode_data_free(&obj_buf);
	}

    return -1;
}

int vg_http_client_post(char *url, unsigned int content_length, post_callback_t post_cb, get_callback_t get_cb)
{
    unsigned char *send_buf = NULL;
    unsigned int buf_len = 0;
    int ret = 0;


    ret = send_header("POST", url, content_length, NULL);
    if(ret < 0)
        return -1;

    // Send post body
    while(content_length != 0){
        post_cb(&send_buf, &buf_len);

        if(buf_len > content_length)
            return -1;

        if(send_buf == NULL){
            return -1;
        }else{
            ret = send_one_block_body(send_buf, buf_len);
            if(ret < 0){
                return -1;
            }
        }

        content_length -= buf_len;
    }

    // Recv response
    if(get_cb != NULL){
        return get_response(get_cb);
    }

    return 0;
}

int vg_http_client_get(char *url, get_callback_t get_cb)
{
    int ret = 0;

    ret = send_header("GET", url, 0, NULL);
    if(ret < 0){
        return -1;
    }

    if(get_cb == NULL){
        return -1;
    }

    return get_response(get_cb);
}

int vg_http_client_stop(void)
{

	if (http_socket_fd > 0)
	{
		 vg_tcp_close(http_socket_fd, 0);
	}
 

    return 0;
}
