#ifndef _PLAT_H
#define _PLAT_H

#include "qcom_common.h"
#include "qcom_uart.h"
#include "qcom/basetypes.h"
#include "qcom_internal.h"
#include "qcom/qcom_cli.h"
#include "qcom/socket_api.h"
#include "qcom/select_api.h"
#include "qcom_ssl.h"
#include "qca_ssl_client.h"
#include "tx_api.h"

#define strcat strcat_ext
#define msleep qcom_thread_msleep

#define vg_alloc mem_alloc
#define vg_free mem_free


#define vg_fd_set q_fd_set


unsigned long vg_get_tick(void);
//-------------------- mutex & sem --------------------------------


//-------------------- mutex & sem --------------------------------

typedef TX_MUTEX vg_mutex_t;
typedef TX_SEMAPHORE vg_sem_t;

int vg_create_sem(void *handle, char *name);

int vg_release_sem(void *handle);

int vg_wait_sem(void *handle, int tm);

void vg_delete_sem(void *handle);

int vg_create_mutex(void *lock, char *name);

int vg_get_mutex(void *lock);

int vg_put_mutex(void *lock);

//-------------- socket ------------------------

#define UDP_RECV_TIMEOUT_S 5
#define UDP_RECV_TIMEOUT_MS 0


int vg_udp_socket(int *sock);


int vg_udp_close(int sock);

int vg_recvfrom(int s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen);

int vg_sendto(int sock, unsigned char *buffer, int length, struct sockaddr_in *client_addr);


#define CONN_NOSSL 0
#define CONN_SSL   1

#define CONN_MODE CONN_NOSSL

#define TCP_TIMEOUT_S  			(6)
#define TCP_TIMEOUT_MS 			(0)

#define TCP_TRY_TIMES           (3)

int vg_tcp_socket(int *sock);

int vg_connect_server(int *sock, char *url, int port, int ssl);

int vg_send(int sock, unsigned char *buffer, int length, int ssl);
int vg_recv(int sock, unsigned char *buffer, int length, int ssl);

int vg_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

//------------ thread ----------------------

typedef void * thread_ret_t;
typedef void * thread_params_t;


typedef thread_ret_t (*thread_fun_t)(thread_params_t args);
int vg_start_thread(thread_fun_t fun, void **stk_addr, int stk_size);


//------------- log -----------------------

enum log_level {
	LEVEL_DEBUG = 0,
	LEVEL_NORMAL = 1,
	LEVEL_ERROR = 2
};

#define LOG_LEVEL LEVEL_DEBUG

#ifdef LOG_IN_FILE
extern FILE *log_fp;
#endif

void now(char *time_buf);

#ifdef LOG_IN_FILE
#define LOG(level, fmt, args...) do{if(level >= LOG_LEVEL){char buf[64];now(buf);fprintf(log_fp,"%s", buf);fprintf(log_fp, fmt, ##args);fflush(log_fp);}}while(0)
#else
#define LOG(level, fmt, args...) do{if(level >= LOG_LEVEL){printf(fmt, ##args);}}while(0)
#endif

#endif
