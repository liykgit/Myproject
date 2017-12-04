
#include <string.h>

#include "common.h"

#include "WKStack_lib.h"
#include "WKStack_packet.h"
#include "WKStack.h"

#include "mqtt.h"

int WKStack_init(const char *mac, const char *product_id, const char *version, const char *sn,  const char *key)
{

    if(WKStack.state == WKSTACK_WAIT_INIT){

        if(key && strlen(key) != WKSTACK_KEY_LEN) {
            LOG(LEVEL_ERROR, "key len\n");
            return -1;
        }

        if(!product_id || strlen(product_id) > WKSTACK_DEVTYPE_LEN) {
            LOG(LEVEL_ERROR, "product_id len\n");
            return -1;
        }

        if(!mac || strlen(mac) <= 0 || strlen(mac) > WKSTACK_MAC_LEN || !isValidMacAddress(mac)) {
            LOG(LEVEL_ERROR, "mac %s\n", mac);
            return -1;
        }

        if(sn && strlen(sn) > WKSTACK_SN_LEN) {
            LOG(LEVEL_ERROR, "Max sn len %d\n", WKSTACK_SN_LEN);
            return -1;
        }

        if(!version || strlen(version) > WKSTACK_VER_LEN) {
            LOG(LEVEL_ERROR, "Max version len %d\n", WKSTACK_VER_LEN);
            return -1;
        }

        if(vg_callbacks[CALLBACK_LOAD_PARAMS]) {
            
            int sz = sizeof(WKStack.params) + 8;
            void *pbuf = vg_malloc(sz);
            if(!pbuf) {
                LOG(LEVEL_ERROR, "OOM\n");
                return -1;
            }

            int r = ((WKStack_load_params_fn_t)vg_callbacks[CALLBACK_LOAD_PARAMS])(pbuf, sz);

            if(r != 0) {
                LOG(LEVEL_ERROR, "Loading params\n");
                //memset(&WKStack.params, 0, sizeof(WKStack.params));
            }
            else if((strcmp(((WKStack_params_t*)pbuf)->magic, WKSTACK_MAGIC)) != 0){
               
                LOG(LEVEL_DEBUG, "params magic mismatch\n");
                //memset(&WKStack.params, 0, sizeof(WKStack.params));
            }
            else {
                unsigned char crc = cal_crc8(pbuf, sizeof(WKStack.params));
                unsigned char saved_crc8 = *(((unsigned char*)pbuf)+sizeof(WKStack.params));                   
                
                if(crc == saved_crc8) {

                    LOG(LEVEL_DEBUG, "load params successfully\n");   
                    if((strcmp(((WKStack_params_t*)pbuf)->product_id, product_id)) != 0){
                        LOG(LEVEL_NORMAL, "new product_id, restore\n");   
                        //memset(&WKStack.params, 0, sizeof(WKStack.params));
                    }
                    else
                        memcpy(&WKStack.params, pbuf, sizeof(WKStack.params)); 
                }
                else {
                    LOG(LEVEL_ERROR, "params crc\n");
                    //memset(&WKStack.params, 0, sizeof(WKStack.params));
                }
            }
            vg_free(pbuf);
        }

        strcpy(WKStack.params.magic, WKSTACK_MAGIC);

        memcpy(WKStack.params.product_id, product_id, strlen(product_id));

        memcpy(WKStack.params.mac, mac, strlen(mac));

        strcpy(WKStack.params.version, version);

        if(key)
            memcpy(WKStack.params.key, key, strlen(key));
        else
            memset(WKStack.params.key, 0, sizeof(WKStack.params.key));

        if(sn)
            strcpy(WKStack.params.sn, sn);
        else
            memset(WKStack.params.sn, 0, sizeof(WKStack.params.sn));

        log_init(WKStack.params.mac);

        mqtt_init(WKSTACK_KEEPALIVE_TIME, MQTT_BUFFER_SIZE);

        UDPServer_start(UDP_SERVER_PORT); 
    }

    LOG(LEVEL_NORMAL, "_inited \n");
    WKStack.state = WKSTACK_OFFLINE;

    return 0;
}

//-1 WKStack is online
//-2 WKStack not init, please call WKStack_init() first
int WKStack_start(void)
{
    if(WKStack.state == WKSTACK_WAIT_INIT)
        return -1;
    else
        return doStart();
}

int WKStack_stop(void)
{
    return mqtt_stop((mqtt_stop_cb_t)vg_callbacks[CALLBACK_STOPPED]);
}

int WKStack_register_callback(callback_index_t id, generic_callback_fp fp)
{
    if(id >= CALLBACK_NR_MAX)
        return -1;

    vg_callbacks[id] = fp;
}

int WKStack_send_raw(unsigned char *raw_data, unsigned int size, WKStack_send_cb_t cb) {

    int ret = 0;
    int buf_sz = 0;

    char buf_small[128];
    buf_small[128-1] = 0;
    
    char *buf = 0;
    int free_buf = 0;

    if(size < 128) {
        buf = buf_small;
        buf_sz = 128;
    }
    else {
        buf = vg_malloc(size + 1); 
        if(!buf) {
            LOG(LEVEL_ERROR, "OOM\n");
            return -1;
        }

        buf[size] = 0;

        free_buf = 1;
        buf_sz = size + 1;
    }

    char tag[4] = {0, 0, WKSTACK_DATAPOINT_TYPE_STRING, 0};
    *(unsigned short *)tag = WKSTACK_PASSTHROUGH_INDEX_RAW_DATA;

    int packet_size = tlv_put_string(buf, tag, raw_data, buf_sz);
    if(packet_size > 0)
        ret = mqtt_publish(WKStack.passthrough_pub_topic, buf, packet_size, MQTT_QOS1, MQTT_RETAIN_FALSE, (mqtt_cb_t)cb);
    else
        ret = -1;

    if(free_buf)
        vg_free(buf);

    return ret;
}

int WKStack_send_datapoint(WKStack_datapoint_t *dp_group, unsigned int group_size, WKStack_send_cb_t cb)
{
    if(WKStack.state != WKSTACK_READY)
        return -1;

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


#ifdef RESTORE_CB_WRAPPER
static WKStack_restore_cb_t _restore_cb = 0;

static int restore_cb_wrapper(unsigned short msg_id, mqtt_errno_t err) {

    if(_restore_cb) {
        _restore_cb(err);

        _restore_cb = 0;
    }
}

int WKStack_restore(WKStack_restore_cb_t restore_cb)
{
    LOG(LEVEL_NORMAL, "WKStack_restore_all E\n");

    if(WKStack.state != WKSTACK_READY)
        return -1;

            
            
    if(restore_cb)
        _restore_cb = restore_cb;

    int ret = -1;


    char buf[32];
    memset(buf, 0, sizeof(buf));

    char tag[4] = {0, 0, WKSTACK_DATAPOINT_TYPE_INT, 0};
    *(unsigned short*)tag = WKSTACK_SYNC_INDEX_RESTORE;
    int offset = tlv_put_int(buf, tag, 1, sizeof(buf));


    return mqtt_publish(WKStack.sync_pub_topic, (unsigned char*)buf, offset, MQTT_QOS1, MQTT_RETAIN_FALSE, (mqtt_cb_t)restore_cb_wrapper);

}

#endif

int WKStack_restore(WKStack_send_cb_t restore_cb)
{
    LOG(LEVEL_NORMAL, "WKStack_restore_all E\n");

    if(WKStack.state != WKSTACK_READY)
        return -1;


    char buf[32];
    memset(buf, 0, sizeof(buf));

    char tag[4] = {0, 0, WKSTACK_DATAPOINT_TYPE_INT, 0};
    *(unsigned short*)tag = WKSTACK_SYNC_INDEX_RESTORE;
    int offset = tlv_put_int(buf, tag, 1, sizeof(buf));


    return mqtt_publish(WKStack.sync_pub_topic, (unsigned char*)buf, offset, MQTT_QOS1, MQTT_RETAIN_FALSE, (mqtt_cb_t)restore_cb);

}



WKStack_state_t WKStack_state()
{
    return WKStack.state;
}

int WKStack_did(char *buf, int size)
{
    int did_len = strlen(WKStack.params.did);
    
    if(did_len + 1 > size)
        return -1;

    strcpy(buf, WKStack.params.did);
    return 0;
}

int WKStack_name(char *buf, int size)
{
    int name_len = strlen(WKStack.params.name);
    
    if(name_len + 1 > size)
        return -1;

    strcpy(buf, WKStack.params.name);
    return 0;
}

int WKStack_params(char *buf, int size)
{
    int name_len = strlen(WKStack.params.name);
    
    if(sizeof(WKStack_params_t) > size)
        return -1;

    memcpy(buf, &WKStack.params, sizeof(WKStack_params_t));
    return 0;
}

int WKStack_send_ota_progress(WKStack_ota_target_t target, WKStack_ota_report_t report, WKStack_send_cb_t cb)
{

    if(WKStack.state != WKSTACK_READY)
        return -1;

    char buf[128];
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
        case OTA_STATE_START : 
            {
               tag[2] = WKSTACK_DATAPOINT_TYPE_INT;
                *(unsigned short*)tag = WKSTACK_OTA_INDEX_STATE;

                item_size = tlv_put_int(buf+offset, tag, OTA_STATE_START, buf_size - offset);
                if(item_size >= 0)
                    offset += item_size;
                else {
                    //LOG(LEVEL_DEBUG,"ota publish buffer not large enough\n");
                    return 0;
                }
            }
            break;
        case OTA_STATE_ONGOING: 
            {
                tag[2] = WKSTACK_DATAPOINT_TYPE_INT;
                *(unsigned short*)tag = WKSTACK_OTA_INDEX_STATE;

                item_size = tlv_put_int(buf+offset, tag, OTA_STATE_ONGOING, buf_size - offset);
                if(item_size >= 0)
                    offset += item_size;
                else {
                    //LOG(LEVEL_NORMAL,"ota publish buffer not large enough\n");
                    return 0;
                }

                if(report.percent != 0 && report.percent <= 100) {

                    tag[2] = WKSTACK_DATAPOINT_TYPE_INT;
                    *(unsigned short*)tag = WKSTACK_OTA_INDEX_PERCENT;

                    item_size = tlv_put_int(buf+offset, tag, report.percent, buf_size - offset);
                    if(item_size >= 0)
                        offset += item_size;
                    else {
                        //LOG(LEVEL_NORMAL,"ota publish buffer not large enough\n");
                        return 0;
                    }
                }
            }
            break;

        case OTA_STATE_END: 
            {
                
                char *ERR_PUBLISH_BUF_TOO_SMALL = "ota publish buffer too small";

               tag[2] = WKSTACK_DATAPOINT_TYPE_INT;
                *(unsigned short*)tag = WKSTACK_OTA_INDEX_STATE;

                item_size = tlv_put_int(buf+offset, tag, OTA_STATE_END, buf_size - offset);
                if(item_size >= 0)
                    offset += item_size;
                else {
                    //LOG(LEVEL_NORMAL,"ota publish buffer not large enough\n");
                    return 0;
                }

                
               tag[2] = WKSTACK_DATAPOINT_TYPE_INT;
                *(unsigned short*)tag = WKSTACK_OTA_INDEX_RESULT;

                item_size = tlv_put_int(buf+offset, tag, !!report.result, buf_size - offset);
                if(item_size >= 0)
                    offset += item_size;
                else {
                    //LOG(LEVEL_NORMAL,"ota publish buffer not large enough\n");
                    return 0;
                }
               
               tag[2] = WKSTACK_DATAPOINT_TYPE_STRING;
               *(unsigned short *)tag = WKSTACK_OTA_INDEX_MOD_VER;
               if(!!report.result)
                   //failure
                    item_size = tlv_put_string(buf+offset, tag, WKStack_version, buf_size - offset);
               else
                    item_size = tlv_put_string(buf+offset, tag, WKStack.module_firmware.version, buf_size - offset);

               if(item_size >= 0)
                   offset += item_size;         
               else {
                   LOG(LEVEL_ERROR,"%s\n", ERR_PUBLISH_BUF_TOO_SMALL);
                   return 0;
               }


               if(report.err_msg != NULL) {

                   tag[2] = WKSTACK_DATAPOINT_TYPE_STRING;
                   *(unsigned short*)tag = WKSTACK_OTA_INDEX_ERR_MSG;

                   item_size = tlv_put_string(buf+offset, tag, report.err_msg, buf_size - offset);
                   if(item_size >= 0)
                       offset += item_size;
                   else {
                       LOG(LEVEL_ERROR, "%s\n", ERR_PUBLISH_BUF_TOO_SMALL);
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
    mqtt_unsubscribe(WKStack.ota_sub_topic, NULL);    
}

int WKStack_report_raw(unsigned char *buf, unsigned int size) {
    return mqtt_publish(WKStack.report_topic, buf, size, MQTT_QOS1, MQTT_RETAIN_FALSE, (mqtt_cb_t)0);
}

int WKStack_lan_sendto(struct sockaddr_in *client_addr, char *buf, int length) {
    return udpserver_sendto(client_addr, buf, length);
}

int WKStack_lan_atcmd_start() {

    UDPServer_start(UDP_SERVER_PORT); 
}

int WKStack_lan_atcmd_stop() {

    UDPServer_stop(); 
}

