#ifndef _WKSTACK_H_KLSJKLSDJFKLSDJFKLD_
#define _WKSTACK_H_KLSJKLSDJFKLSDJFKLD_

//------------- CLOUD definitions ---------------
#define AWS_CLOUD 1
#define ALI_CLOUD 2
#define LOCAL   100


//------------- connection mode definition ---------------
#define CONN_NOSSL 0
#define CONN_SSL   1



#define CLOUD ALI_CLOUD
#define CONN_MODE CONN_NOSSL

#define PASSTHROUGH 0

#if CLOUD == ALI_CLOUD

#define WKSTACK_FIRST_CONNECT_HOST "cn.vengasz.com"

#if CONN_MODE == 1
#define WKSTACK_FIRST_CONNECT_PORT 3884
#else
#define WKSTACK_FIRST_CONNECT_PORT 1884
#endif

#elif CLOUD == AWS_CLOUD

#define WKSTACK_FIRST_CONNECT_HOST "www.vengasz.com"

#if CONN_MODE == 1
#define WKSTACK_FIRST_CONNECT_PORT 3884
#else
#define WKSTACK_FIRST_CONNECT_PORT 1884
#endif



#else

#define WKSTACK_FIRST_CONNECT_HOST "192.168.3.3"
#define WKSTACK_FIRST_CONNECT_PORT 1884

#endif

//---------------------------  new for passthrough project

#define ERROR_CODE_NETWORK_ERROR (-1)
#define ERROR_CODE_AUTH_FAILURE (-2)

#define WKSTACK_VER_LEN 20
#define WKSTACK_OTA_URL_LEN 128

typedef enum{

    PUBLISH_FAILED = 6,
    PUBLISH_SUCCEED = 7,

}publish_result_t;

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
    CALLBACK_OTA_EVENT,

    CALLBACK_NR_MAX 
} callback_index_t;


typedef enum {
	OTA_TARGET_MOD,
	OTA_TARGET_MCU,
}WKStack_ota_target_t;

typedef enum {
	OTA_STATE_START = 0,
	OTA_STATE_ONGOING,
	OTA_STATE_END
}WKStack_ota_state_t;

typedef enum {
	OTA_RESULT_SUCCESS,
	OTA_RESULT_FAILURE
}WKStack_ota_result_t;

typedef enum {
	OTA_MSG_TYPE_NEW_VERSION = 0,
	OTA_MSG_TYPE_FIRMWARE_URL = 1,
} WKStack_ota_msg_type_t;

typedef struct _firmware_info {
    char version[WKSTACK_VER_LEN];
    char url[WKSTACK_OTA_URL_LEN];
    int  port;
} firmware_info_t;

typedef struct {

    WKStack_ota_target_t target;
    WKStack_ota_msg_type_t type;

    const firmware_info_t *firmware;

} WKStack_ota_msg_t;


typedef struct {
    WKStack_ota_state_t state;
    WKStack_ota_result_t result;
    unsigned int percent; 
    char *curr_version; 
    char *err_msg;
} WKStack_ota_report_t;

typedef int (*generic_callback_fp)(void);

typedef void (*WKStack_raw_data_handler_t)(char *buf, int size);
typedef int (*WKStack_save_params_fn_t)(void *buf, int size);
typedef int (*WKStack_load_params_fn_t)(char *buf, int size);
typedef int (*WKStack_error_handler_t)(int err_code);
typedef int (*WKStack_send_cb_t)(int msg_id, publish_result_t err_code);
typedef int (*WKStack_ota_event_cb_t)(const WKStack_ota_msg_t *msg);

int WKStack_init(const char *mac, const char *product_id, const char *version, const char *sn,  const char *key);

int WKStack_start(void);
int WKStack_stop(void);
int WKStack_register_callback(callback_index_t id, generic_callback_fp fp);

//int WKStack_send_mcu_ota_request(WKStack_ota_target_t target, char *version);
//int WKStack_send_mcu_ota_request(WKStack_ota_target_t target, char *version);
int WKStack_send_ota_progress(WKStack_ota_target_t target, WKStack_ota_report_t report, WKStack_send_cb_t cb);

#if PASSTHROUGH
int WKStack_send_raw(unsigned char *raw_data, unsigned int size, WKStack_send_cb_t cb);
#endif


typedef void (*lan_cmd_hook_t)(int argc, char *argv[], struct sockaddr_in *client_addr);

int WKStack_register_lan_hook(char *cmd, lan_cmd_hook_t hook);
//int WKStack_lan_sendto(struct sockaddr_in *client_addr, char *buf, int length);

int WKStack_lan_atcmd_start();

int WKStack_lan_atcmd_stop();

#endif
