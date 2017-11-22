#ifndef _WKSTACK_H_KLSJKLSDJFKLSDJFKLD_
#define _WKSTACK_H_KLSJKLSDJFKLSDJFKLD_



#define PASSTHROUGH 1

#define AWS_CLOUD 1
#define ALI_CLOUD 2

#define CLOUD ALI_CLOUD

#if CLOUD == ALI_CLOUD
#define WKSTACK_FIRST_CONNECT_HOST "cn.vengasz.com"
#define WKSTACK_FIRST_CONNECT_PORT 3884

#elif CLOUD == AWS_CLOUD


#define WKSTACK_FIRST_CONNECT_HOST "www.vengasz.com"
#define WKSTACK_FIRST_CONNECT_PORT 3884

#else

#define WKSTACK_FIRST_CONNECT_HOST "192.168.3.2"
#define WKSTACK_FIRST_CONNECT_PORT 1884

#endif

//---------------------------  new for passthrough project

#define ERROR_CODE_NETWORK_ERROR (-1)
#define ERROR_CODE_AUTH_FAILURE (-2)


enum log_level {
	LEVEL_DEBUG = 0,
	LEVEL_NORMAL = 1,
	LEVEL_ERROR = 2
} ;

#define LOG_LEVEL LEVEL_DEBUG


typedef enum {
    CALLBACK_CONNECTED, 
    CALLBACK_DISCONNECTED,
    CALLBACK_RAW_DATA,
    CALLBACK_DP_DATA,
    CALLBACK_STOPPED,
    CALLBACK_SAVE_PARAMS,
    CALLBACK_LOAD_PARAMS,
    CALLBACK_ERROR,

    CALLBACK_NR_MAX 
} callback_index_t;


typedef int (*generic_callback_fp)(void);

typedef void (*WKStack_raw_data_handler_t)(char *buf, int size);
typedef int (*WKStack_save_params_fn_t)(void *buf, int size);
typedef int (*WKStack_load_params_fn_t)(char *buf, int size);
typedef int (*WKStack_error_handler_t)(int err_code);

int WKStack_init(const char *mac, const char *product_id, const char *version, const char *sn,  const char *key);

int WKStack_start(void);
int WKStack_stop(void);
int WKStack_register_callback(callback_index_t id, generic_callback_fp fp);

#if PASSTHROUGH
int WKStack_send_raw(unsigned char *raw_data, unsigned int size);
#endif

#endif
