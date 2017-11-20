#ifndef _WKSTACK_H_KLSJKLSDJFKLSDJFKLD_
#define _WKSTACK_H_KLSJKLSDJFKLSDJFKLD_


//---------------------------  new for passthrough project

#define ERROR_CODE_NETWORK_ERROR (-1)
#define ERROR_CODE_AUTH_FAILURE (-2)

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

int WKStack_start();
int WKStack_stop();
int WKStack_register_callback(callback_index_t id, generic_callback_fp fp);


#endif
