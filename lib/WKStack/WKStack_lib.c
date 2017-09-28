#include <string.h>

#include "common.h"
#include "WKStack_lib.h"

//#include "mqtt.h"
#include "WKStack_packet.h"

#include "tlv.h"

WKStack_t WKStack;

int g_testmode = 0;
const char *WKStack_version = "1.5.1";


int WKStack_connect_cb(mqtt_errno_t err);

static void doPrepare() {
    sprintf(WKStack.report_topic, WKSTACK_TOPIC_REPORT_FMT, WKStack.params.product_id, WKStack.params.did);

    sprintf(WKStack.control_topic, WKSTACK_TOPIC_CONTROL_FMT, WKStack.params.product_id, WKStack.params.did);
    sprintf(WKStack.ota_sub_topic, WKSTACK_TOPIC_OTA_SUB_FMT, WKStack.params.product_id, WKStack.params.did);
    sprintf(WKStack.ota_pub_topic, WKSTACK_TOPIC_OTA_PUB_FMT, WKStack.params.product_id, WKStack.params.did);

    sprintf(WKStack.binding_sub_topic, WKSTACK_TOPIC_BINDING_SUB_FMT, WKStack.params.product_id, WKStack.params.did);
    sprintf(WKStack.binding_pub_topic, WKSTACK_TOPIC_BINDING_PUB_FMT, WKStack.params.product_id, WKStack.params.did);


    sprintf(WKStack.sync_pub_topic, WKSTACK_TOPIC_SYNC_PUB_FMT, WKStack.params.product_id, WKStack.params.did);
    sprintf(WKStack.sync_sub_topic, WKSTACK_TOPIC_SYNC_SUB_FMT, WKStack.params.product_id, WKStack.params.did);


    WKStack_subscribe_ota();
    WKStack_subscribe_sync();
    WKStack_subscribe_control();
    WKStack_subscribe_binding();

    WKStack_publish_sync();
}

void WKStack_connect_ep(void)
{
    LOG(LEVEL_NORMAL,"WKStack_connect_ep E state: %d\n", WKStack.state);

    char client_id[72];
    memset(client_id, 0, 72);

    WKStack_params_t *params = &WKStack.params;

    //clientId: {productId}.{did}
    sprintf((char *)client_id, "%s.%s", params->product_id, WKStack.params.did);

    WKStack_pack_connect(client_id, 0);
    
    g_mqtt_data.username= "VENGA_DEV";
    g_mqtt_data.password = WKStack.params.ticket;
    //g_mqtt_data.password = "test";

    mqtt_start(WKStack.params.host, WKStack.params.port, &g_mqtt_data, WKStack_connect_cb);
    return;
};

void WKStack_announce() {

        int i = 0;

        char buf[216];
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "VENGAS:ANNOUNCE:%s#%s#%s#%s:VENGAE", WKStack.params.product_id,
                            WKStack.params.mac,
                            WKStack.params.did,
                            WKStack.params.name);

        while (i++ < WKSTACK_ANNOUNCE_COUNT) {
            udpserver_broadcast(buf, strlen(buf), WKSTACK_ANNOUNCE_PORT);
            //TODO vg_msleep
            msleep(WKSTACK_ANNOUNCE_INTERVAL);
        }
}

int WKStack_connect_cb(mqtt_errno_t err)
{
    LOG(LEVEL_DEBUG, "WKStack_connect_cb E state: %d\n", WKStack.state);

    if(WKStack.state == WKSTACK_REGISTER){
        if(err == MQTT_CONNECT_SUCCEED){
            LOG(LEVEL_NORMAL,"Registry connected. Subscribe to topics\n");

            WKStack_subscribe_welcome();
            WKStack_subscribe_challenge();
            WKStack_publish_knock();

        }else if(err == MQTT_DISCONNECT_SUCCEED){

            LOG(LEVEL_NORMAL,"Disconnected from registry, state %d\n", WKStack.state);
            WKStack.state = WKSTACK_CONNECT_ENDPOINT;
            LOG(LEVEL_NORMAL,"Immediate connect to ep\n");

            WKStack_connect_ep();

        } else {
            LOG(LEVEL_ERROR,"mqtt err %d, go offline\n", err);
            WKStack.state = WKSTACK_OFFLINE;
        }
    } else if(WKStack.state == WKSTACK_CONNECT_ENDPOINT) {

        switch(err) {
            
            case MQTT_CONNECT_SUCCEED:
                {
                    WKStack.state = WKSTACK_ONLINE;

                    LOG(LEVEL_NORMAL,"Connected to endpoint\n");
                    doPrepare();
                }
                break;

            case MQTT_DISCONNECT_SUCCEED:
                {
                    LOG(LEVEL_NORMAL,"Disconnected state %d\n", WKStack.state);
                    WKStack.state = WKSTACK_OFFLINE;
                    //WKStack_connect_ep();
                }
                break;

            case MQTT_SOCKET_ERROR:
            case MQTT_SEVICE_NOT_AVAILABLE:
                {
                    LOG(LEVEL_ERROR,"MQTT or socket error, get offline\n");
                    WKStack.state = WKSTACK_OFFLINE;
                    //WKStack_connect_ep();
                }
                break;

            case MQTT_CLIENT_ID_ERROR:
            case MQTT_INVALID_USER: //!< Connection Refused: bad user name or password
            case MQTT_UNAUTHORIZED: //!< Connection Refused: not authorized
                {
                    LOG(LEVEL_ERROR,"MQTT auth failed by endpoint\n");
                    memset(WKStack.params.ticket, 0, sizeof(WKStack.params.ticket));
                    WKStack.state = WKSTACK_OFFLINE;
                }
                break;

            default:
                WKStack.state = WKSTACK_OFFLINE;
        }

        if(WKStack.connect_cb != NULL)
            WKStack.connect_cb(WKStack.state);
    }
    else if(WKStack.state == WKSTACK_ONLINE){
        if(err == MQTT_SOCKET_ERROR) {
            LOG(LEVEL_ERROR,"Socket error, get offline\n");
            WKStack.state = WKSTACK_OFFLINE;
            //WKStack_connect_ep();
        }
        else if(err == MQTT_DISCONNECT_SUCCEED) {
             LOG(LEVEL_ERROR,"Disconnected from endpoint, get offline\n");
             WKStack.state = WKSTACK_OFFLINE;
        }
    }
    else if(WKStack.state == WKSTACK_RECONNECT_ENDPOINT) {

        switch(err) {
            
            case MQTT_CONNECT_SUCCEED:
                {
                    LOG(LEVEL_NORMAL,"Reconnected to ep\n Sub topics\n");
                    WKStack.state = WKSTACK_ONLINE;

                    doPrepare();

                    
                }
                break;

            case MQTT_DISCONNECT_SUCCEED:
                {
                    LOG(LEVEL_NORMAL,"Disconnected, state %d\n", WKStack.state);
                    WKStack.state = WKSTACK_OFFLINE;
                
                    //WKStack_connect_ep();
                }
                break;

            case MQTT_SOCKET_ERROR:
            case MQTT_SEVICE_NOT_AVAILABLE:
                {
                    LOG(LEVEL_ERROR,"MQTT or socket error during reconnect, get offline\n");
                    WKStack.state = WKSTACK_OFFLINE;
                    //WKStack_connect_ep();
                }
                break;

            case MQTT_CLIENT_ID_ERROR:
            case MQTT_INVALID_USER: //!< Connection Refused: bad user name or password
            case MQTT_UNAUTHORIZED: //!< Connection Refused: not authorized
                {
                    LOG(LEVEL_ERROR,"MQTT auth failed during reconnect\n");
                    memset(WKStack.params.ticket, 0, sizeof(WKStack.params.ticket));
                    WKStack.state = WKSTACK_OFFLINE;
                }
                break;

            default:
                WKStack.state = WKSTACK_OFFLINE;
        }

        if(WKStack.connect_cb != NULL)
            WKStack.connect_cb(WKStack.state);
    }


    return 0;
}

static int is_ip_address(char *str) {

    int segs = 0;   /* Segment count. */
    int chcnt = 0;  /* Character count within segment. */
    int accum = 0;  /* Accumulator for segment. */

    /* Catch NULL pointer. */

    if (str == NULL)
        return 0;

    /* Process every character in string. */

    while (*str != '\0') {
        /* Segment changeover. */

        if (*str == '.') {
            /* Must have some digits in segment. */

            if (chcnt == 0)
                return 0;

            /* Limit number of segments. */

            if (++segs == 4)
                return 0;

            /* Reset segment values and restart loop. */

            chcnt = accum = 0;
            str++;
            continue;
        }
 

        /* Check numeric. */

        if ((*str < '0') || (*str > '9'))
            return 0;

        /* Accumulate and check segment. */

        if ((accum = accum * 10 + *str - '0') > 255)
            return 0;

        /* Advance other segment specific stuff and continue loop. */

        chcnt++;
        str++;
    }

    /* Check enough segments and enough characters in last segment. */

    if (segs != 3)
        return 0;

    if (chcnt == 0)
        return 0;

    /* Address okay. */

    return 1;
}


// First step:
// 1. connect to broker
// 2. subsribe a online topic for getting endpoint and wait msg from broker
// 3. when get the endpoint msg, publish a online msg to broker

// Second step:
// 1. connect to broker with last will
// 2. publish realonline topic
// 3. subscribe control topic(for recv cmd and set status)
void WKStack_connect(void)
{
    LOG(LEVEL_DEBUG,"WKStack_connect E state: %d\n", WKStack.state);

    WKStack_pack_connect(NULL, 0);

    if(is_ip_address(WKSTACK_FIRST_CONNECT_HOST)) {
        LOG(LEVEL_DEBUG, "connect to ip %s\n", WKSTACK_FIRST_CONNECT_HOST);
        mqtt_start(WKSTACK_FIRST_CONNECT_HOST, WKSTACK_FIRST_CONNECT_PORT, &g_mqtt_data, WKStack_connect_cb);
    }
    else {
        LOG(LEVEL_DEBUG, "connect to domain %s\n", WKSTACK_FIRST_CONNECT_HOST);
       
        unsigned int ip = vg_dns_get_ip_by_domain_name(WKSTACK_FIRST_CONNECT_HOST); 
        
        if(ip == 0) {
            LOG(LEVEL_ERROR, "Cannot resolve the given domain name\n");
        }
        else {
            char ip_str[16];
            memset(ip_str, 0, sizeof(ip_str));
            sprintf(ip_str, "%d.%d.%d.%d", (ip>>24) & 0xff, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip  & 0xFF);

            LOG(LEVEL_DEBUG, "resolved domain to %s\n", ip_str);
            mqtt_start(ip_str, WKSTACK_FIRST_CONNECT_PORT, &g_mqtt_data, WKStack_connect_cb);
        }
         
    }

    return;
}

int doStart() {

    LOG(LEVEL_DEBUG, ".. %d\n", WKStack.state); 
    if(WKStack.state == WKSTACK_OFFLINE) {

        WKStack_params_t *params = &WKStack.params;
         

        if(strlen(WKStack.params.did) != 0 && strlen(WKStack.params.host) != 0 && strlen(WKStack.params.ticket) != 0) { 
            LOG(LEVEL_NORMAL,"My did is %s\n", WKStack.params.did);
            LOG(LEVEL_NORMAL,"Offline, reconnecting %s\n", WKStack.params.host);

            WKStack.state = WKSTACK_RECONNECT_ENDPOINT;
            WKStack_connect_ep();
        }
        else {
            LOG(LEVEL_NORMAL,"Offline, connect registry\n");

            memset(WKStack.params.host, 0, WKSTACK_HOST_LEN);
            WKStack.params.port = 0;

            WKStack.state = WKSTACK_REGISTER;
            WKStack_connect();
        }
    }

    return 0;
}

