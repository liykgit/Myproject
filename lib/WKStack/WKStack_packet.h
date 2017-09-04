#ifndef _WKSTACK_PACKET_H_JSKFJLKSDJFKSDF_
#define _WKSTACK_PACKET_H_JSKFJLKSDJFKSDF_

#include "mqtt.h"

// First host, connect this host to get endpoint
//#define WKSTACK_FIRST_CONNECT_HOST "35.165.194.227"
#define WKSTACK_FIRST_CONNECT_HOST "192.168.3.3"
//#define WKSTACK_FIRST_CONNECT_HOST "120.24.216.85"
#define WKSTACK_FIRST_CONNECT_PORT 1884

#define DP_COUNT_MAX 32
// 10 s for keepalive time
#define WKSTACK_KEEPALIVE_TIME 10

// Topic
#define WKSTACK_TOPIC_ONLINE "/iot/dm/online"
#define WKSTACK_TOPIC_OFFLINE "/iot/dm/offline"


#define WKSTACK_TOPIC_KNOCK_FMT "/iot/netin/%s/knock"
#define WKSTACK_TOPIC_ANSWER_FMT "/iot/netin/%s/answer"
#define WKSTACK_TOPIC_WELCOME_FMT "/iot/netin/%s/welcome"
#define WKSTACK_TOPIC_CHALLENGE_FMT "/iot/netin/%s/challenge"

#define WKSTACK_TOPIC_REPORT_FMT "/iot/dm/%s/%s/report"
#define WKSTACK_TOPIC_CONTROL_FMT "/iot/dm/%s/%s/control"
#define WKSTACK_TOPIC_OTA_SUB_FMT "/iot/ota/%s/%s/sub" // For both Module and MCU ota (Server -> Device)
#define WKSTACK_TOPIC_OTA_PUB_FMT "/iot/ota/%s/%s/pub" // For Module ota (Device -> Server)

#define WKSTACK_TOPIC_BINDING_SUB_FMT "/iot/binding/%s/%s/sub" // For both Module and MCU ota (Server -> Device)
#define WKSTACK_TOPIC_BINDING_PUB_FMT "/iot/binding/%s/%s/pub" // For Module ota (Device -> Server)

#define WKSTACK_TOPIC_SYNC_SUB_FMT "/iot/sync/%s/%s/sub" 
#define WKSTACK_TOPIC_SYNC_PUB_FMT "/iot/sync/%s/%s/pub" // For Module ota (Device -> Server)

#define WKSTACK_TOPIC_OTA_MCU_OUT_FMT "/iot/in/mcu/%s/%s/ota" // For MCU ota (Device -> Server)

// Json field
#define WKSTACK_FIELD_DEVTYPE "type"
#define WKSTACK_FIELD_SN "sn"
#define WKSTACK_FIELD_DID "did"
#define WKSTACK_FIELD_ENDPOINT "endpoint"
#define WKSTACK_FIELD_CTRL_TYPE "action"
#define WKSTACK_FIELD_CMD_NAME "command"
#define WKSTACK_FIELD_CMD_ARGUMENTS "arguments"
#define WKSTACK_FIELD_CMD_ARGUMENTS2 "paramsType"

// ota field
#define WKSTACK_OTA_TYPE "TYPE"
#define WKSTACK_OTA_MCU_VER "MCU_VER"
#define WKSTACK_OTA_MOD_VER "MOD_VER"
#define WKSTACK_OTA_DEVTYPE "DEVTYPE"
#define WKSTACK_OTA_HOST "HOST"
#define WKSTACK_OTA_PORT "PORT"
#define WKSTACK_OTA_SESSION "SESSION"
#define WKSTACK_OTA_RESULT "RESULT"

#define WKSTACK_OTA_RET "OTA_RET"
#define WKSTACK_OTA_MSG "OTA_MSG"
#define WKSTACK_OTA_REQ "OTA_REQ"
#define WKSTACK_OTA_VER "OTA_VER"

#define WKSTACK_OTA_SESSION_FLAG "/ota?session="

// Control topic type
#define WKSTACK_CTRL_TYPE_COMMAND "command" // For command
#define WKSTACK_CTRL_TYPE_STATUES "commandbson" // For set status

#define WKSTACK_OTA_INDEX_TYPE          65001
#define WKSTACK_OTA_INDEX_DEVTYPE       65002
#define WKSTACK_OTA_INDEX_UPGRADE_URI   65003
#define WKSTACK_OTA_INDEX_TICKET        65004
#define WKSTACK_OTA_INDEX_STATE         65005
#define WKSTACK_OTA_INDEX_PERCENT       65006
#define WKSTACK_OTA_INDEX_RESULT        65007
#define WKSTACK_OTA_INDEX_ERR_MSG       65008
#define WKSTACK_OTA_INDEX_MOD_VER       65020

#define WKSTACK_BINDING_INDEX_USERID    65040
#define WKSTACK_BINDING_INDEX_TICKET    65041
#define WKSTACK_BINDING_INDEX_ERROR     65042

#define WKSTACK_SYNC_INDEX_SN           65060
#define WKSTACK_SYNC_INDEX_VER          65061
#define WKSTACK_SYNC_INDEX_SDKVER       65062
#define WKSTACK_SYNC_INDEX_RESTORE      65063

#define WKSTACK_WELCOME_INDEX_DID          65081
#define WKSTACK_WELCOME_INDEX_URL          65082
#define WKSTACK_WELCOME_INDEX_TICKET       65083
#define WKSTACK_WELCOME_INDEX_DNAME        65084

#define INDEX_SZ (4)
#define TYPE_SZ (4)
#define BUFSZ_SZ (4)

#define INT_SZ (4)
#define FLOAT_SZ (4)
#define BOOL_SZ (1)


typedef enum {
    WKSTACK_OTA_UPGRADE_REQUEST = 2,
    WKSTACK_OTA_UPGRADE_REPORT
} WKStack_ota_msg_type_t;


extern mqtt_connect_data_t g_mqtt_data;

int WKStack_pack_connect(char *client_id, int flag);
int WKStack_pack_connect_cb(char *client_id, int flag);
int WKStack_subscribe_welcome();
int WKStack_publish_knock();

int WKStack_publish_realonline();
int WKStack_subscribe_control();
int WKStack_subscribe_ota();
int WKStack_subscribe_binding();

#endif
