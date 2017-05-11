#include <string.h>


#include "common.h"
#include "mqtt.h"
#include "WKStack_lib.h"
#include "WKStack_packet.h"
#include "WKStack.h"
#include "tlv.h"


static int WKStack_connect_cb(mqtt_errno_t err);

static void WKStack_connect_ep(void)
{
    printf("WKStack_connect_ep E state: %d\n", WKStack.state);

    char client_id[24];
    memset(client_id, 0, 24);


    WKStack_params_t *params = &WKStack.params;

    //use long id
    sprintf((char *)client_id, "%s%s", params->devtype, params->did);

    WKStack_pack_connect(params->did, 0);
    
    data.username= "VENGA_DEV";
    data.password = WKStack.ticket;

    mqtt_start(WKStack.params.host, WKStack.params.port, &data, WKStack_connect_cb);
    return;
};

static int WKStack_connect_cb(mqtt_errno_t err)
{
    printf("WKStack_connect_cb E state: %d\n", WKStack.state);

    if(WKStack.state == WKSTACK_INIT){
        if(err == MQTT_CONNECT_SUCCEED){
            printf("We are on entrypoint, state %d\n", WKStack.state);

            WKStack_subscribe_welcome();
            WKStack_subscribe_challenge();
            WKStack_publish_knock();
        }else if(err == MQTT_DISCONNECT_SUCCEED){

            printf("We are off, state %d\n", WKStack.state);
            if(WKStack.state == WKSTACK_WAIT_ONLINE) 
                WKStack_connect_ep();

        } else {
            printf("mqtt err %d\n", err);
            //TODO check return error and add sleep then retry
            //go back to init state
            WKStack.state = WKSTACK_INIT;
        }
    } else if(WKStack.state == WKSTACK_WAIT_ONLINE) {

        switch(err) {
            
            case MQTT_CONNECT_SUCCEED:
                {
                    printf("We are on endpoint, state %d\n", WKStack.state);
                    WKStack.state = WKSTACK_ONLINE;

                    WKStack_subscribe_control();
                    WKStack_subscribe_ota();
                    WKStack_subscribe_binding();

                }
                break;

            case MQTT_DISCONNECT_SUCCEED:
                {
                    printf("We are off, state %d\n", WKStack.state);
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
static void WKStack_connect(void)
{
    printf("WKStack_connect E state: %d\n", WKStack.state);

    WKStack_pack_connect(NULL, 0);
    mqtt_start(WKSTACK_FIRST_CONNECT_HOST, WKSTACK_FIRST_CONNECT_PORT, &data, WKStack_connect_cb);

    return;
}

int WKStack_init(WKStack_params_t *params)
{

    if(WKStack.state == WKSTACK_WAIT_INIT){
       
        memcpy(&WKStack.params, params, sizeof(WKStack_params_t));

        //TODO get sn in future
        //sprintf(WKStack.devinfo_topic, WKSTACK_TOPIC_DEVINFO_FMT, WKStack.params.sn);

        log_init(WKStack.params.mac);

        mqtt_init(WKSTACK_KEEPALIVE_TIME, 1024);
    }

    WKStack.state = WKSTACK_OFFLINE;

    
    return 0;
}
//-1 WKStack is online
//-2 WKStack not init, please call WKStack_init() first
int WKStack_start(WKStack_cb_t connect_cb, WKStack_ota_cb_t ota_cb)
{
    WKStack.connect_cb = connect_cb;
    WKStack.ota_cb = ota_cb;

    //TODO make sure params is all 0 if not set. 
    //TODO check ep here also.
    printf("WKSTACK_start E state: %d\n", WKStack.state);
    if(WKStack.state == WKSTACK_OFFLINE) {

        WKStack_params_t *params = &WKStack.params;

        if(strlen(params->did) != 0 && strlen(params->host) != 0) { 
            printf("My did is %s\n", params->did);
            printf("host is %s\n", params->host);

            memset(params->host, 0, WKSTACK_HOST_LEN);
            params->port = 0;
            
            WKStack.state = WKSTACK_WAIT_ONLINE;
        }
        else {
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

int WKStack_stop()
{
    return mqtt_stop();
}

int WKStack_report_datapoint(WKStack_datapoint_t *dp_group, unsigned int group_size, WKStack_report_cb_t cb)
{

    char buf[512];
    int buf_size = 512;

	if(dp_group == NULL)
	if(dp_group == NULL || group_size == 0)
		return -1;

	memset((void *)buf, 0, sizeof(buf));

    int offset = 0;
    int i = 0;

	while(i < group_size){
        WKStack_datapoint_t dp = dp_group[i++];

		switch(dp.type) {
            case WKSTACK_DATAPOINT_TYPE_BOOL: {
                char tag[4] = {0, 0, WKSTACK_DATAPOINT_TYPE_BOOL, 0};
                *(unsigned short *)tag = dp.index;

                int item_size = tlv_put_bool(buf+offset, tag, dp.value.boolean, buf_size - offset);
                if(item_size >= 0)
                    offset += item_size;
                else
                    i = group_size;
            }
            break;

            case WKSTACK_DATAPOINT_TYPE_INT: {
                 char tag[4] = {0, 0, WKSTACK_DATAPOINT_TYPE_INT, 0};
                *(unsigned short *)tag = dp.index;

                int value;
                memcpy(&value, &dp.value.integer, sizeof(int));
                int item_size = tlv_put_int(buf+offset, tag, value, buf_size - offset);
                if(item_size >= 0)
                    offset += item_size;            
                else
                    i = group_size;
            }
            break;

            case WKSTACK_DATAPOINT_TYPE_FLOAT: {
                  char tag[4] = {0, 0, WKSTACK_DATAPOINT_TYPE_FLOAT, 0};
                *(unsigned short *)tag = dp.index;

                float value;
                memcpy(&value, &dp.value.floatpoint, sizeof(float));

                int item_size = tlv_put_float(buf+offset, tag, value, buf_size - offset);
                if(item_size >= 0)
                    offset += item_size;           
                else
                    i = group_size;
            }
            break;

            case WKSTACK_DATAPOINT_TYPE_STRING: {
                char tag[4] = {0, 0, WKSTACK_DATAPOINT_TYPE_STRING, 0};
                *(unsigned short *)tag = dp.index;

                int item_size = tlv_put_string(buf+offset, tag, dp.value.string, buf_size - offset);
                if(item_size >= 0)
                    offset += item_size;         
                else
                    i = group_size;
            }
            break;
		}
	}

    return mqtt_publish(WKStack.report_topic, (unsigned char*)buf, offset, MQTT_QOS1, MQTT_RETAIN_FALSE, (mqtt_cb_t)cb);

}

int WKStack_report_raw(unsigned char *buf, unsigned int size) {
    return mqtt_publish(WKStack.report_topic, buf, size, MQTT_QOS1, MQTT_RETAIN_FALSE, (mqtt_cb_t)0);
}

int WKStack_register_datapoint_handler(WKStack_datapoint_handler_t handler)
{
    if(handler == NULL)
		return -1;

	WKStack.dp_handler = handler;

	return 0;
}

WKStack_state_t WKStack_state()
{
    return WKStack.state;
}

int WKStack_report_ota_progress(WKStack_ota_target_t target, WKStack_ota_report_t report, WKStack_report_cb_t cb)
{

    static char buf[512];
    int buf_size = 512;
    int offset = 0;

	memset((void *)buf, 0, sizeof(buf));


    //put OTA_TYPE
    char tag[4] = {0, 0, WKSTACK_DATAPOINT_TYPE_INT, 0};
    *(unsigned short*)tag = WKSTACK_OTA_INDEX_TYPE;

    int item_size = tlv_put_int(buf+offset, tag, WKSTACK_OTA_UPGRADE_REPORT, buf_size - offset);
    if(item_size >= 0)
        offset += item_size;

    switch(report.state) {
        case WKSTACK_OTA_STATE_START : 
            {
               tag[2] = WKSTACK_DATAPOINT_TYPE_INT;
                *(unsigned short*)tag = WKSTACK_OTA_INDEX_STATE;

                item_size = tlv_put_int(buf+offset, tag, WKSTACK_OTA_STATE_START, buf_size - offset);
                if(item_size >= 0)
                    offset += item_size;
                else {
                    printf("ota publish buffer not large enough\n");
                    return 0;
                }
            }
            break;
        case WKSTACK_OTA_STATE_ONGOING: 
            {
                tag[2] = WKSTACK_DATAPOINT_TYPE_INT;
                *(unsigned short*)tag = WKSTACK_OTA_INDEX_STATE;

                item_size = tlv_put_int(buf+offset, tag, WKSTACK_OTA_STATE_ONGOING, buf_size - offset);
                if(item_size >= 0)
                    offset += item_size;
                else {
                    printf("ota publish buffer not large enough\n");
                    return 0;
                }

                if(report.percent != 0 && report.percent <= 100) {

                    tag[2] = WKSTACK_DATAPOINT_TYPE_INT;
                    *(unsigned short*)tag = WKSTACK_OTA_INDEX_PERCENT;

                    item_size = tlv_put_int(buf+offset, tag, report.percent, buf_size - offset);
                    if(item_size >= 0)
                        offset += item_size;
                    else {
                        printf("ota publish buffer not large enough\n");
                        return 0;
                    }
                }
            }
            break;

        case WKSTACK_OTA_STATE_END: 
            {
               tag[2] = WKSTACK_DATAPOINT_TYPE_INT;
                *(unsigned short*)tag = WKSTACK_OTA_INDEX_STATE;

                item_size = tlv_put_int(buf+offset, tag, WKSTACK_OTA_STATE_END, buf_size - offset);
                if(item_size >= 0)
                    offset += item_size;
                else {
                    printf("ota publish buffer not large enough\n");
                    return 0;
                }

                
               tag[2] = WKSTACK_DATAPOINT_TYPE_INT;
                *(unsigned short*)tag = WKSTACK_OTA_INDEX_RESULT;

                item_size = tlv_put_int(buf+offset, tag, !!report.result, buf_size - offset);
                if(item_size >= 0)
                    offset += item_size;
                else {
                    printf("ota publish buffer not large enough\n");
                    return 0;
                }
                
               tag[2] = WKSTACK_DATAPOINT_TYPE_STRING;
               *(unsigned short *)tag = WKSTACK_OTA_INDEX_MOD_VER;
               if(!!report.result)
                   //failure
                    item_size = tlv_put_string(buf+offset, tag, WKStack_version, buf_size - offset);
               else
                    item_size = tlv_put_string(buf+offset, tag, WKStack.ota.mod_ver, buf_size - offset);

               if(item_size >= 0)
                   offset += item_size;         
               else {
                   printf("ota publish buffer not large enough\n");
                   return 0;
               }


               if(report.err_msg != NULL) {

                   tag[2] = WKSTACK_DATAPOINT_TYPE_STRING;
                   *(unsigned short*)tag = WKSTACK_OTA_INDEX_ERR_MSG;

                   item_size = tlv_put_string(buf+offset, tag, report.err_msg, buf_size - offset);
                   if(item_size >= 0)
                       offset += item_size;
                   else {
                       printf("ota publish buffer not large enough\n");
                       return 0;
                   }
                }
            }
            break;
    }
    mqtt_unsubscribe(WKStack.ota_sub_topic, NULL);    
  
    return mqtt_publish(WKStack.ota_pub_topic, (unsigned char*)buf, offset, MQTT_QOS1, MQTT_RETAIN_FALSE, (mqtt_cb_t)cb);
}


void WKStack_rabbit() {
    printf("WKStack_rabbit\n");
    mqtt_unsubscribe(WKStack.ota_sub_topic, NULL);    
}
