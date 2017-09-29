
#ifndef _PLAT_H
#define _PLAT_H

#include <unistd.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <semaphore.h>
#include <pthread.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

//----------------------- sys ---------------------------------


#define vg_fd_set fd_set

#define msleep(ms) usleep(ms * 1000)

//#define vg_malloc malloc
//#define vg_free free

void *vg_malloc(size_t sz);
void vg_free(void *ptr);

unsigned long vg_get_tick(void);
//-------------------- mutex & sem --------------------------------

typedef sem_t vg_sem_t;
typedef pthread_mutex_t vg_mutex_t;

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

int vg_listen(int sock, int backlog);
int vg_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

int vg_tcp_close(int sock, int ssl);

//------------ thread ----------------------

typedef void * thread_ret_t;
typedef void * thread_params_t;


typedef thread_ret_t (*thread_fun_t)(thread_params_t args);
int vg_start_thread(thread_fun_t fun, void **stk_addr, int stk_size);

int vg_resolve_domain_name(char *domain_name, char *ip_buf, int ip_buf_length);

enum log_level {
	LEVEL_DEBUG = 0,
	LEVEL_NORMAL = 1,
	LEVEL_ERROR = 2
} ;

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
