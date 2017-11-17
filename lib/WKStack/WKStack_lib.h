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
#define RECONNECT_DELAY_MEDIUM 1000 * 30
#define RECONNECT_DELAY_LONG  1000 * 60

extern const char *WKStack_version;
extern const char *wisper; 
int doStart();


void subscrption_map_set(int id);

void subscrption_map_unset(int id);

int subscrption_map_check(int ids);


#endif
