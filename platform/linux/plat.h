

#include <unistd.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

//----------------------- sys ---------------------------------

#define msleep(ms) usleep(ms * 1000)

#define mem_alloc malloc
#define mem_free free


unsigned long get_tick(void);


//-------------------- mutex & sem --------------------------------

pthread_mutex_t lock;

sem_t ctrl_thread_sem;
sem_t recv_thread_sem;
sem_t g_sendbuddle_sem;
sem_t g_flightbuddle_sem;
sem_t g_pendbuddle_sem;


int create_sem(void *handle, char *name);

int release_sem(void *handle);

int wait_sem(void *handle, int tm);

void delete_sem(void *handle);

int create_mutex(void *lock, char *name);

int get_mutex(void *lock);

int put_mutex(void *lock);



//-------------- socket ------------------------

#define UDP_RECV_TIMEOUT_S 5
#define UDP_RECV_TIMEOUT_MS 0


int create_udp_socket(int *sock);


int close_udp_socket(int sock);


int udp_recvfrom(int s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen);

int udp_sendto(int sock, unsigned char *buffer, int length, struct sockaddr_in *client_addr);


#define CONN_NOSSL 0
#define CONN_SSL   1

#define CONN_MODE CONN_NOSSL

#define TCP_TIMEOUT_S  			(6)
#define TCP_TIMEOUT_MS 			(0)

#define TCP_TRY_TIMES 3

int create_socket(int *sock);
int connect_server(int *sock, char *url, int port, int ssl);

int tcp_close(int sock, int ssl);

int tcp_send(int sock, unsigned char *buffer, int length, int ssl);
int tcp_recv(int sock, unsigned char *buffer, int length, int ssl);

int socket_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

//------------ thread ----------------------

typedef void * thread_ret_t;
typedef void * thread_params_t;


typedef thread_ret_t (*thread_fun_t)(thread_params_t args);
int start_thread(thread_fun_t fun, void **stk_addr, int stk_size);


//------------- log -----------------------

enum log_level {
	LEVEL_DEBUG = 0,
	LEVEL_NORMAL = 1,
	LEVEL_ERROR = 2,
	LEVEL_CLOSE = 3,
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

// optional helper function 
int  log_init(char *mac);
void log_buffer(enum log_level level, char *buf, unsigned int size);
void log_uninit();


//-------------- tcp --------------------


