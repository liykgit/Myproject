#ifndef _PLAT_H
#define _PLAT_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mbedtls/platform.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"

#include "lwip/sockets.h"
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/netdb.h"
#include "lwip/inet_chksum.h"

#include "rda_sys_wrapper.h"

#include "WKStack.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

//#define strcat strcat_ext
#define msleep osDelay


void *vg_malloc(unsigned int sz); 
void vg_free(void *ptr);

#define vg_fd_set fd_set


unsigned long vg_get_tick(void);
//-------------------- mutex & sem --------------------------------


//-------------------- mutex & sem --------------------------------

typedef struct{
	void *lock;
	char *name;
} vg_mutex_t;

typedef struct{
	void *handle;
	char *name;
} vg_sem_t;

int vg_create_sem(vg_sem_t *handle, char *name);

int vg_release_sem(vg_sem_t *handle);

int vg_wait_sem(vg_sem_t *handle, int tm);

void vg_delete_sem(vg_sem_t *handle);

int vg_create_mutex(vg_mutex_t *lock, char *name);

int vg_get_mutex(vg_mutex_t *lock);

int vg_put_mutex(vg_mutex_t *lock);

void vg_delete_mutex(vg_mutex_t *lock);

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

int vg_resolve_domain_name(char *domain_name, char *ip_buf, int ip_buf_length);

//------------- log -----------------------

void now(char *time_buf);

//------------- flash ---------------------

int vg_erase_flash(unsigned int addr, unsigned int len);

int vg_write_flash(unsigned int addr, void *buf, unsigned int buf_len);

int vg_read_flash(unsigned int addr, void *buf, unsigned int buf_len);

#ifdef __cplusplus
}
#endif

#endif
