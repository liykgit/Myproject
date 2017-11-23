#ifndef _WKSTACK_LIB_H_KLASJFKSDJFKLSJDFKLJSD_
#define _WKSTACK_LIB_H_KLASJFKSDJFKLSJDFKLJSD_

#include "WKStack.h"

#define WKSTACK_TOPIC_LEN 96
#define WKSTACK_SUBSCRIPTION_MAX sizeof(int)

#define SUBSCRIPTION_CONTROL    (1 << 0)
#define SUBSCRIPTION_BINDING    (1 << 1) 
#define SUBSCRIPTION_OTA        (1 << 2) 
#define SUBSCRIPTION_SYNC       (1 << 3) 

// WKStack (command or datapoint)'s name max length


#define WKSTACK_VALUE_LEN 128

// WKStack max datapoint count
#define WKSTACK_MAX_DP_NUM  64
// WKStack max command count

// WKStack params max length

#define WKSTACK_SN_LEN 48
#define WKSTACK_KEY_LEN 16
#define WKSTACK_DEVTYPE_LEN 16
#define WKSTACK_MAC_LEN 16

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

#define PADDING (4)

//4: padding 128=24*4*8/6, where 24: max allowed characters, 4 : each utf-8 character up to 4 bytes, *8/6 base64 length
#define WKSTACK_NAME_LEN (128 + PADDING) 
#define WKSTACK_HOST_LEN 64
#define WKSTACK_DID_LEN (36 + PADDING)

#define TICKET_LEN (32) 
#define TICKET_LEN_PADDING (4) //4 as padding

typedef enum {

    WKSTACK_WAIT_INIT = 0, // Wait init
    WKSTACK_OFFLINE = 1,

    WKSTACK_REGISTER = 2, // in init

    WKSTACK_CONNECT_ENDPOINT = 3, // Wait online
    WKSTACK_ONLINE = 4,
    WKSTACK_OTA = 5,
    WKSTACK_ERROR = 6,
    WKSTACK_RECONNECT_ENDPOINT = 7, // Wait online

    WKSTACK_READY = 8,
    WKSTACK_QUERY_ENDPOINT = 9

} WKStack_state_t;

// Must be 6 and 7
typedef enum {
    WKSTACK_PUBLISH_FAILED = 6,
    WKSTACK_PUBLISH_SUCCEED = 7,
} WKStack_publish_state_t;

typedef struct {

    char name[WKSTACK_NAME_LEN];
    char did[WKSTACK_DID_LEN];
    char host[WKSTACK_HOST_LEN];
    char ticket[TICKET_LEN + TICKET_LEN_PADDING];
    unsigned short port;
    char key[WKSTACK_KEY_LEN + PADDING];

    char version[WKSTACK_VER_LEN + PADDING];
    char sn[WKSTACK_SN_LEN +  PADDING];
    char product_id[WKSTACK_DEVTYPE_LEN + PADDING];
    char mac[WKSTACK_MAC_LEN + PADDING];
} WKStack_params_t;

//TODO change struct name to include ota
typedef struct{

    int index;
	int type;
	union VAL{
        int integer;
        float floatpoint;
        char boolean;
        char *string;
    } value;
} WKStack_datapoint_t;


typedef int (*WKStack_cb_t)(WKStack_state_t state);
typedef int (*WKStack_stop_cb_t)();
typedef int (*WKStack_report_cb_t)(unsigned short id, WKStack_publish_state_t state);
typedef void (*WKStack_datapoint_handler_t)(WKStack_datapoint_t *dps, int size);
typedef int (*WKStack_restore_cb_t)(WKStack_publish_state_t state);


int WKStack_register_datapoint_handler(WKStack_datapoint_handler_t cb);

int WKStack_report_datapoint(WKStack_datapoint_t *dp_group, unsigned int group_size, WKStack_report_cb_t cb);

WKStack_state_t WKStack_state(void);

int WKStack_did(char *buf, int size);

int WKStack_name(char *buf, int size);

int WKStack_params(char *buf, int size);

int WKStack_restore_all(WKStack_restore_cb_t restore_cb);



typedef struct{

    WKStack_state_t state;

    WKStack_params_t params;

    unsigned int subscription_map;

    char topic_knock[WKSTACK_TOPIC_LEN];
    char topic_answer[WKSTACK_TOPIC_LEN];
    char topic_welcome[WKSTACK_TOPIC_LEN];
    char topic_challenge[WKSTACK_TOPIC_LEN];

    char control_topic[WKSTACK_TOPIC_LEN];
    char report_topic[WKSTACK_TOPIC_LEN];

    char devinfo_topic[WKSTACK_TOPIC_LEN];

    char ota_pub_topic[WKSTACK_TOPIC_LEN];
    char ota_sub_topic[WKSTACK_TOPIC_LEN];


    char binding_pub_topic[WKSTACK_TOPIC_LEN];
    char binding_sub_topic[WKSTACK_TOPIC_LEN];

    char sync_pub_topic[WKSTACK_TOPIC_LEN];
    char sync_sub_topic[WKSTACK_TOPIC_LEN];

    char passthrough_sub_topic[WKSTACK_TOPIC_LEN];
    char passthrough_pub_topic[WKSTACK_TOPIC_LEN];

    firmware_info_t module_firmware;
    firmware_info_t mcu_firmware;

    WKStack_datapoint_handler_t dp_handler;
} WKStack_t;


extern WKStack_t WKStack;
extern int g_testmode;

#define UDP_SERVER_PORT 30319

#define WKSTACK_ANNOUNCE_PORT 30320
#define WKSTACK_ANNOUNCE_COUNT 6
#define WKSTACK_ANNOUNCE_INTERVAL 50

#define REGISTRY_ERR_SYSTEM_FAILURE "500"
#define REGISTRY_ERR_UNAUTHORIZED   "403"

#define RECONNECT_DELAY_SHORT 1000 * 10
#define RECONNECT_DELAY_MEDIUM 1000 * 30
#define RECONNECT_DELAY_LONG  1000 * 60

extern const char *WKStack_version;
extern const char *wisper; 
int doStart(void);


void subscrption_map_set(int id);

void subscrption_map_unset(int id);

int subscrption_map_check(int ids);


extern generic_callback_fp vg_callbacks[CALLBACK_NR_MAX];



#endif
