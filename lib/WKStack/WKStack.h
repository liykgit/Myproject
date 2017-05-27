#ifndef _WKSTACK_H_KLSJKLSDJFKLSDJFKLD_
#define _WKSTACK_H_KLSJKLSDJFKLSDJFKLD_

/* Please do not modify this file */

// WKStack (command or datapoint)'s name max length
#define WKSTACK_VALUE_LEN 128

// WKStack max datapoint count
#define WKSTACK_MAX_DP_NUM  64
// WKStack max command count
#define WKSTACK_MAX_CMD_NUM  64

// WKStack params max length
#define WKSTACK_SN_LEN 48
//4: padding 128=24*4*8/6, where 24: max allowed characters, 4 : each utf-8 character up to 4 bytes, *8/6 base64 length
#define WKSTACK_NAME_LEN (128 + 4) 
#define WKSTACK_KEY_LEN 32
#define WKSTACK_DEVTYPE_LEN 16
#define WKSTACK_MAC_LEN 16

#define WKSTACK_HOST_LEN 64
#define WKSTACK_DID_LEN 16 

// WKStack ota max length
#define WKSTACK_OTA_VER_LEN 32
#define WKSTACK_OTA_URL_LEN 128

// WKStack datapoint value type
#define WKSTACK_TYPE_FLOAT	0x01
#define WKSTACK_TYPE_STRING	0x02
#define WKSTACK_TYPE_RAW 0x05
#define WKSTACK_TYPE_BOOL 0x08
#define WKSTACK_TYPE_INT 0x10
#define WKSTACK_TYPE_LONG 0x12

#define WKSTACK_DATAPOINT_NR 32
#define WKSTACK_DATAPOINT_TYPE_BOOL 0x01
#define WKSTACK_DATAPOINT_TYPE_INT  0x02
#define WKSTACK_DATAPOINT_TYPE_FLOAT 0x03
#define WKSTACK_DATAPOINT_TYPE_STRING 0x04

typedef enum {
    WKSTACK_WAIT_INIT = 0, // Wait init
    WKSTACK_OFFLINE = 1,

    WKSTACK_INIT = 2, // in init

    WKSTACK_WAIT_ONLINE = 3, // Wait online
    WKSTACK_ONLINE = 4,
    WKSTACK_OTA = 5,
    WKSTACK_ERROR = 6,
}WKStack_state_t;

// Must be 6 and 7
typedef enum{
    WKSTACK_PUBLISH_FAILED = 6,
    WKSTACK_PUBLISH_SUCCEED = 7,
}WKStack_publish_state_t;

typedef enum {
	WKSTACK_OTA_TARGET_MOD = 0,
	WKSTACK_OTA_TARGET_MCU = 1,
}WKStack_ota_target_t;

typedef enum {
	WKSTACK_OTA_STATE_START = 0,
	WKSTACK_OTA_STATE_ONGOING,
	WKSTACK_OTA_STATE_END
}WKStack_ota_state_t;

typedef enum {
	WKSTACK_OTA_RESULT_SUCCESS = 0,
	WKSTACK_OTA_RESULT_FAILURE = 1
}WKStack_ota_result_t;



typedef struct {
    WKStack_ota_state_t state;
    WKStack_ota_result_t result;
    unsigned int percent; 
    char *curr_version; 
    char *err_msg;
} WKStack_ota_report_t;

typedef enum {
	WKSTACK_OTA_TYPE_VER = 0,
	WKSTACK_OTA_TYPE_MSG = 1,
}WKStack_ota_type_t;

typedef struct {
    char sn[WKSTACK_SN_LEN];
    char key[WKSTACK_KEY_LEN];
    char devtype[WKSTACK_DEVTYPE_LEN];
    char mac[WKSTACK_MAC_LEN];

    char name[WKSTACK_NAME_LEN];
    char did[WKSTACK_SN_LEN];
    char host[WKSTACK_HOST_LEN];
    unsigned short port;
} WKStack_params_t;

typedef struct {

	char mod_ver[WKSTACK_OTA_VER_LEN];
	char mod_url[WKSTACK_OTA_URL_LEN];
	int mod_port;

	char mcu_ver[WKSTACK_OTA_VER_LEN];
	char mcu_url[WKSTACK_OTA_URL_LEN];
	int mcu_port;
}WKStack_ota_t;

//TODO change struct name to include ota
typedef struct{

    int index;
	int type;
	union VAL{
        int integer;
        float floatpoint;
        int boolean;
        char *string;
    } value;
} WKStack_datapoint_t;


typedef int (*WKStack_cb_t)(WKStack_state_t state);
typedef int (*WKStack_report_cb_t)(unsigned short id, WKStack_publish_state_t state);
typedef int (*WKStack_ota_cb_t)(WKStack_ota_t *info, WKStack_ota_target_t target, WKStack_ota_type_t type);
typedef void (*WKStack_datapoint_handler_t)(WKStack_datapoint_t *dps, int size);


int WKStack_init(WKStack_params_t *params);

int WKStack_register_datapoint_handler(WKStack_datapoint_handler_t cb);

int WKStack_start(WKStack_cb_t connect_cb, WKStack_ota_cb_t cb);

int WKStack_report_datapoint(WKStack_datapoint_t *dp_group, unsigned int group_size, WKStack_report_cb_t cb);

int WKStack_stop();


// -1 no new version available
int WKStack_ota_request(WKStack_ota_target_t target);

int WKStack_report_ota_progress(WKStack_ota_target_t target, WKStack_ota_report_t report, WKStack_report_cb_t cb);

WKStack_state_t WKStack_state(void);

extern const char *WKStack_version;

#endif
