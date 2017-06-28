#ifndef _WKSTACK_LIB_H_KLASJFKSDJFKLSJDFKLJSD_
#define _WKSTACK_LIB_H_KLASJFKSDJFKLSJDFKLJSD_

#include "WKStack.h"

#define WKSTACK_TOPIC_LEN 96
#define TICKET_LEN (32) 
#define TICKET_LEN_PADDING (4) //4 as padding


//4: padding 128=24*4*8/6, where 24: max allowed characters, 4 : each utf-8 character up to 4 bytes, *8/6 base64 length
#define WKSTACK_NAME_LEN (128 + 4) 
#define WKSTACK_HOST_LEN 64
#define WKSTACK_DID_LEN (36+4)


// WKStack (command or datapoint)'s name max length


typedef struct{

    char name[WKSTACK_NAME_LEN];
    char did[WKSTACK_DID_LEN];
    char host[WKSTACK_HOST_LEN];
    unsigned short port;
    WKStack_state_t state;

    WKStack_params_t params;

    char ticket[TICKET_LEN + TICKET_LEN_PADDING];
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
    //char ota_mcu_out_topic[WKSTACK_TOPIC_LEN];

    WKStack_ota_t ota;

    WKStack_cb_t connect_cb;
    WKStack_ota_cb_t ota_cb;
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
#define RECONNECT_DELAY_LONG  1000 * 60

extern const char *WKStack_version;

int doStart();


#endif
