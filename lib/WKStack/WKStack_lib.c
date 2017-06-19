
#include <string.h>

#include "common.h"
#include "WKStack_lib.h"

//#include "mqtt.h"
#include "WKStack_packet.h"

#include "tlv.h"

WKStack_t WKStack;

int g_testmode = 0;
const char *WKStack_version = "1.0.0";


int WKStack_connect_cb(mqtt_errno_t err);

void WKStack_connect_ep(void)
{
    LOG(LEVEL_NORMAL,"WKStack_connect_ep E state: %d\n", WKStack.state);

    char client_id[32];
    memset(client_id, 0, 32);


    WKStack_params_t *params = &WKStack.params;

    //clientId: {productId}.{did}
    sprintf((char *)client_id, "%s.%s", params->product_id, WKStack.did);

    WKStack_pack_connect(client_id, 0);
    
    g_mqtt_data.username= "VENGA_DEV";
    g_mqtt_data.password = WKStack.ticket;

    mqtt_start(WKStack.host, WKStack.port, &g_mqtt_data, WKStack_connect_cb);
    return;
};

void WKStack_announce() {

        int i = 0;

        char buf[192];
        memset(buf, 0, sizeof(buf));

        sprintf(buf, "VENGAS:ANNOUNCE:%s#%s#%s#%s:VENGAE", WKStack.params.product_id,
                            WKStack.params.mac,
                            WKStack.did,
                            WKStack.name);

        while (i++ < WKSTACK_ANNOUNCE_COUNT) {
            udpserver_broadcast(buf, strlen(buf), WKSTACK_ANNOUNCE_PORT);
            //TODO vg_msleep
            msleep(WKSTACK_ANNOUNCE_INTERVAL);
        }
}

int WKStack_connect_cb(mqtt_errno_t err)
{
    LOG(LEVEL_NORMAL,"WKStack_connect_cb E state: %d\n", WKStack.state);

    if(WKStack.state == WKSTACK_INIT){
        if(err == MQTT_CONNECT_SUCCEED){
            LOG(LEVEL_NORMAL,"We are on entrypoint, state %d\n", WKStack.state);

            WKStack_subscribe_welcome();
            WKStack_subscribe_challenge();
            WKStack_publish_knock();
        }else if(err == MQTT_DISCONNECT_SUCCEED){

            LOG(LEVEL_NORMAL,"We are off, state %d\n", WKStack.state);
            if(WKStack.state == WKSTACK_WAIT_ONLINE) 
                WKStack_connect_ep();

        } else {
            LOG(LEVEL_ERROR,"mqtt err %d\n", err);
            //TODO check return error and add sleep then retry
            //go back to init state
            WKStack.state = WKSTACK_INIT;
        }
    } else if(WKStack.state == WKSTACK_WAIT_ONLINE) {

        switch(err) {
            
            case MQTT_CONNECT_SUCCEED:
                {
                    LOG(LEVEL_NORMAL,"We are on endpoint, state %d\n", WKStack.state);
                    WKStack.state = WKSTACK_ONLINE;

                    WKStack_subscribe_control();
                    WKStack_subscribe_ota();
                    WKStack_subscribe_binding();

                }
                break;

            case MQTT_DISCONNECT_SUCCEED:
                {
                    LOG(LEVEL_NORMAL,"We are off, state %d\n", WKStack.state);
                    WKStack.state = WKSTACK_OFFLINE;
                }
                break;

            case MQTT_SOCKET_ERROR:
            case MQTT_SEVICE_NOT_AVAILABLE:
                {
                    WKStack_connect_ep();
                }
                break;

            case MQTT_CLIENT_ID_ERROR:
            case MQTT_INVALID_USER: //!< Connection Refused: bad user name or password
            case MQTT_UNAUTHORIZED: //!< Connection Refused: not authorized
                {
                    memset(WKStack.ticket, 0, sizeof(WKStack.ticket));
                    WKStack.state = WKSTACK_OFFLINE;
                }
                break;

            default:

                WKStack.state = WKSTACK_ERROR;
        }

        if(WKStack.connect_cb != NULL)
            WKStack.connect_cb(WKStack.state);
    }
    else if(WKStack.state == WKSTACK_ONLINE){
        if(err == MQTT_SOCKET_ERROR) {
            WKStack_connect_ep();
        }
        else if(err == MQTT_DISCONNECT_SUCCEED) {
             WKStack.state = WKSTACK_OFFLINE;
        }
    }

    return 0;
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
    LOG(LEVEL_NORMAL,"WKStack_connect E state: %d\n", WKStack.state);

    WKStack_pack_connect(NULL, 0);
    mqtt_start(WKSTACK_FIRST_CONNECT_HOST, WKSTACK_FIRST_CONNECT_PORT, &g_mqtt_data, WKStack_connect_cb);

    return;
}

int doStart() {

    LOG(LEVEL_NORMAL,"doStart E state: %d\n", WKStack.state);

    if(WKStack.state == WKSTACK_OFFLINE) {

        WKStack_params_t *params = &WKStack.params;

        if(strlen(WKStack.did) != 0 && strlen(WKStack.host) != 0) { 
            LOG(LEVEL_NORMAL,"My did is %s\n", WKStack.did);
            LOG(LEVEL_NORMAL,"host is %s\n", WKStack.host);

            
            WKStack.state = WKSTACK_WAIT_ONLINE;
        }
        else {

            memset(WKStack.host, 0, WKSTACK_HOST_LEN);
            WKStack.port = 0;

            WKStack.state = WKSTACK_INIT;
        }
    }

    if(WKStack.state == WKSTACK_INIT){

        WKStack_connect();

    }else if(WKStack.state == WKSTACK_WAIT_ONLINE){

        WKStack_connect_ep();

    }else if(WKStack.state == WKSTACK_ONLINE){
        return -1;
    }
    else
        return -2;

    return 0;
}

