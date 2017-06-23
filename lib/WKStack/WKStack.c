
#include <string.h>

#include "common.h"

#include "WKStack_lib.h"
#include "WKStack_packet.h"
#include "WKStack.h"

#include "mqtt.h"


int WKStack_init(WKStack_params_t *params)
{

    if(strlen(params->key) <= 0 || strlen(params->key) > WKSTACK_KEY_LEN) {
        LOG(LEVEL_ERROR, "Error: invalid product key %s\n", params->key);
        return -1;
    }

    if(strlen(params->product_id) <= 0 || strlen(params->product_id) > WKSTACK_DEVTYPE_LEN) {
        LOG(LEVEL_ERROR, "Error: invalid product id %s\n", params->product_id);
        return -1;
    }

    if(strlen(params->mac) <= 0 || strlen(params->mac) > WKSTACK_MAC_LEN || !isValidMacAddress(params->mac)) {
        LOG(LEVEL_ERROR, "Error: invalid mac %s\n", params->mac);
        return -1;
    }

    if(strlen(params->sn) > WKSTACK_SN_LEN) {
        LOG(LEVEL_ERROR, "Error: max allowed sn length %d\n", WKSTACK_SN_LEN);
        return -1;
    }

    if(strlen(params->version) > WKSTACK_VER_LEN) {
        LOG(LEVEL_ERROR, "Error: max allowed version length %d\n", WKSTACK_VER_LEN);
        return -1;
    }

    if(WKStack.state == WKSTACK_WAIT_INIT){
       
        memcpy(&WKStack.params, params, sizeof(WKStack_params_t));

        //TODO get sn in future
        //sLOG(LEVEL_NORMAL,WKStack.devinfo_topic, WKSTACK_TOPIC_DEVINFO_FMT, WKStack.params.sn);

        log_init(WKStack.params.mac);

        mqtt_init(WKSTACK_KEEPALIVE_TIME, 1024);
    }

    LOG(LEVEL_NORMAL, "WKStack inited \n");
    WKStack.state = WKSTACK_OFFLINE;
    
    UDPServer_start(UDP_SERVER_PORT); 

    return 0;
}

//-1 WKStack is online
//-2 WKStack not init, please call WKStack_init() first
int WKStack_start(WKStack_cb_t connect_cb, WKStack_ota_cb_t ota_cb)
{
    WKStack.connect_cb = connect_cb;
    WKStack.ota_cb = ota_cb;

    return doStart();
}

int WKStack_stop(WKStack_stop_cb_t cb)
{
    return mqtt_stop(cb);
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

int WKStack_did(char *buf, int size)
{
    int did_len = strlen(WKStack.did);
    
    if(did_len + 1 > size)
        return -1;

    strcpy(buf, WKStack.did);
    return 0;
}

int WKStack_name(char *buf, int size)
{
    int name_len = strlen(WKStack.name);
    
    if(name_len + 1 > size)
        return -1;

    strcpy(buf, WKStack.name);
    return 0;
}

int WKStack_report_ota_progress(WKStack_ota_target_t target, WKStack_ota_report_t report, WKStack_report_cb_t cb)
{

    static char buf[128];
    int buf_size = 128;
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
                    LOG(LEVEL_NORMAL,"ota publish buffer not large enough\n");
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
                    LOG(LEVEL_NORMAL,"ota publish buffer not large enough\n");
                    return 0;
                }

                if(report.percent != 0 && report.percent <= 100) {

                    tag[2] = WKSTACK_DATAPOINT_TYPE_INT;
                    *(unsigned short*)tag = WKSTACK_OTA_INDEX_PERCENT;

                    item_size = tlv_put_int(buf+offset, tag, report.percent, buf_size - offset);
                    if(item_size >= 0)
                        offset += item_size;
                    else {
                        LOG(LEVEL_NORMAL,"ota publish buffer not large enough\n");
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
                    LOG(LEVEL_NORMAL,"ota publish buffer not large enough\n");
                    return 0;
                }

                
               tag[2] = WKSTACK_DATAPOINT_TYPE_INT;
                *(unsigned short*)tag = WKSTACK_OTA_INDEX_RESULT;

                item_size = tlv_put_int(buf+offset, tag, !!report.result, buf_size - offset);
                if(item_size >= 0)
                    offset += item_size;
                else {
                    LOG(LEVEL_NORMAL,"ota publish buffer not large enough\n");
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
                   LOG(LEVEL_NORMAL,"ota publish buffer not large enough\n");
                   return 0;
               }


               if(report.err_msg != NULL) {

                   tag[2] = WKSTACK_DATAPOINT_TYPE_STRING;
                   *(unsigned short*)tag = WKSTACK_OTA_INDEX_ERR_MSG;

                   item_size = tlv_put_string(buf+offset, tag, report.err_msg, buf_size - offset);
                   if(item_size >= 0)
                       offset += item_size;
                   else {
                       LOG(LEVEL_NORMAL,"ota publish buffer not large enough\n");
                       return 0;
                   }
                }
            }
            break;
    }
    //mqtt_unsubscribe(WKStack.ota_sub_topic, NULL);    
  
    return mqtt_publish(WKStack.ota_pub_topic, (unsigned char*)buf, offset, MQTT_QOS1, MQTT_RETAIN_FALSE, (mqtt_cb_t)cb);
}

void WKStack_rabbit() {
    LOG(LEVEL_NORMAL,"WKStack_rabbit\n");
    mqtt_unsubscribe(WKStack.ota_sub_topic, NULL);    
}
