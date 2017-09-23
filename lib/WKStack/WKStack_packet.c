
#include "common.h"
#include "tlv.h"
#include "mqtt.h"
#include "WKStack.h"
#include "WKStack_lib.h"
#include "WKStack_packet.h"

#include "pushlist.h"
#include "udpserver.h"

mqtt_connect_data_t g_mqtt_data = { {'M', 'Q', 'T', 'C'}, 0, 4, 0, 1, 0, { {'M', 'Q', 'T', 'W'}, 0, 0, 0, 0, 0 }, 0, 0 };

static WKStack_datapoint_t dps[DP_COUNT_MAX];

static WKStack_datapoint_t *fill_dp_chunk(WKStack_datapoint_t *pdp, TLV_t *ptlv) {

    pdp->index = *(unsigned short *)ptlv->tag;
    pdp->type = (int)ptlv->tag[2];

    switch(ptlv->tag[2]) {
        case 1: {
                LOG(LEVEL_DEBUG, "bool index %d, len %d\n", pdp->index, ptlv->len);
                //memcpy(&pdp->value.boolean, ptlv->value, ptlv->len);
                pdp->value.boolean = *(char *)ptlv->value;
                LOG(LEVEL_DEBUG, "val %d\n", pdp->value.boolean);
        }
        break;

        case 2: {

                LOG(LEVEL_DEBUG, "int index %d, len %d\n", pdp->index, ptlv->len);
                memcpy(&pdp->value.integer, ptlv->value, ptlv->len);
                LOG(LEVEL_DEBUG, "val %d\n", pdp->value.integer);
        }
        break;

        case 3: {
                LOG(LEVEL_DEBUG, "float index %d, len %d\n", pdp->index, ptlv->len);
                LOG(LEVEL_DEBUG, "type float\n");
                memcpy(&pdp->value.floatpoint, ptlv->value, ptlv->len);
        }
        break;

        case 4: {
                LOG(LEVEL_DEBUG, "string index %d, len %d\n", pdp->index, ptlv->len);

                pdp->value.string = vg_malloc(ptlv->len + 1);
                memset(pdp->value.string, 0, ptlv->len + 1);
                memcpy(pdp->value.string, ptlv->value, ptlv->len);

                LOG(LEVEL_DEBUG, "%s\n", pdp->value.string);
        }
        break;
    }

    return pdp;
}

//return number of tlv items decoded */
static int decode_payload(char *data, int len) {

    LOG(LEVEL_DEBUG, "decoding tlv ...  E\n "); 

    int offset = 0;
    int item_size = 0;
    int i = 0;

    while(len > 0) {
   
        TLV_t *ptlv = NULL;
        LOG(LEVEL_DEBUG, "parse start at offset %d\n", offset);
        item_size = tlv_next(data+offset, len, &ptlv);
        LOG(LEVEL_DEBUG, "item size %d\n", item_size);

        if(item_size >= 0) {
            LOG(LEVEL_DEBUG, "len :  %d\n", ptlv->len);
            len -= item_size;

            LOG(LEVEL_DEBUG, "offset %d len-item_size %d\n", offset, len);
            LOG(LEVEL_DEBUG, "index %d\n", *(unsigned short *)ptlv->tag);

            LOG(LEVEL_DEBUG, "data type %d\n", ptlv->tag[2]);

            fill_dp_chunk(&dps[i++], ptlv);

            offset += item_size;

        }
        else {
            break;
        }
    }

    LOG(LEVEL_DEBUG, "decoding...  X\n "); 

    return i;
}

int WKStack_publish_ota_request(char *version)
{
    LOG(LEVEL_DEBUG, "publish_ota_request E\n "); 
    char buf[128];
    int buf_size = 128;

	memset(buf, 0, sizeof(buf));

    int offset = 0;

    char tag1[4] = {0, 0, WKSTACK_DATAPOINT_TYPE_INT, 0};
    *(unsigned short*)tag1 = WKSTACK_OTA_INDEX_TYPE;
    int item_size = tlv_put_int(buf+offset, tag1, WKSTACK_OTA_UPGRADE_REQUEST, buf_size - offset);
    if(item_size >= 0)
        offset += item_size;
    
    char tag2[4] = {0, 0, WKSTACK_DATAPOINT_TYPE_STRING, 0};
    *(unsigned short *)tag2 = WKSTACK_OTA_INDEX_MOD_VER;
    item_size = tlv_put_string(buf+offset, tag2, version, buf_size - offset);

    if(item_size >= 0)
        offset += item_size;         
    
    else { 
        LOG(LEVEL_ERROR, "ota publish buffer not large enough\n");
        return -1;
    }

    return mqtt_publish(WKStack.ota_pub_topic, (unsigned char*)buf, offset, MQTT_QOS1, MQTT_RETAIN_FALSE, (mqtt_cb_t)0);
}

static int WKStack_unpack_ota(unsigned char *payload, int len)
{
    LOG(LEVEL_DEBUG, "WKStack_unpack_ota E\n");

    int count = decode_payload((char *)payload, len);
    if(count <= 0)  
        return 0;

    int i = 0;

    int type = -1;
    char *ticket = 0;
    char *uri = 0;
    char *version = 0;

    for(i = 0; i < count; i++) {

        switch(dps[i].index) {
            
            case WKSTACK_OTA_INDEX_TYPE: {
                type = dps[i].value.integer;
            }
            break;

            case WKSTACK_OTA_INDEX_UPGRADE_URI: {
                uri = dps[i].value.string;
            }
            break;
            
            case WKSTACK_OTA_INDEX_TICKET: {
                ticket = dps[i].value.string;
            }
            break;

            case WKSTACK_OTA_INDEX_MOD_VER: {
                version = dps[i].value.string;
            }
            break;
        }
    }

    if(type == WKSTACK_OTA_TYPE_VER) {
        LOG(LEVEL_DEBUG, "WKStack_unpack_ota received version notification %s E\n", version);

        if(!version)
            return 0;

        strcpy(WKStack.ota.mod_ver, version);

        WKStack_publish_ota_request(version);
        LOG(LEVEL_DEBUG, "WKStack_unpack_ota received version notification %s X\n", version);
    }
    else if (type == WKSTACK_OTA_TYPE_MSG) {
        
        if(!uri || !ticket)
            return 0;
       
        strcpy(WKStack.ota.mod_url, uri);
        strcat(WKStack.ota.mod_url, "?ticket=");
        strcat(WKStack.ota.mod_url, ticket);

        LOG(LEVEL_DEBUG, "ota url %s\n", WKStack.ota.mod_url);
        LOG(LEVEL_DEBUG, "unsubscribe %s\n", WKStack.ota_sub_topic);
        mqtt_unsubscribe(WKStack.ota_sub_topic, NULL);    
        msleep(800);
    }
    else {

        LOG(LEVEL_DEBUG, "WKStack_unpack_ota E\n");
    }

    for(i = 0; i < count; i++) {
        //free the string buffer
        if(dps[i].type == 4) {
            vg_free(dps[i].value.string);
            dps[i].value.string = NULL;
        }
    }

    if(WKStack.ota_cb != NULL)
        //TODO target is hard-coded to 0
        WKStack.ota_cb(&(WKStack.ota), 0, type);

	return 0;
}

predicate_t is_client(void *ele, void *arg) {
   
    if(!ele)
        return 0;

    printf("ele %s\n",  ((client_info_t *)ele)->user_id);
    printf("%s\n", (char *)arg);

    if(strcmp(((client_info_t *)ele)->user_id, (char *)arg) == 0) {
        
        return 1;
    }
    else 
        return 0;
}


static int WKStack_unpack_sync(unsigned char *payload, int len)
{

    LOG(LEVEL_DEBUG, "WKStack_unpack_sync E\n");
    int count = decode_payload((char *)payload, len);
    if(count <= 0)  
        return 0;

    int i = 0;
    
    for(; i < count; i++) {

        switch(dps[i].index) {
            
            case WKSTACK_WELCOME_INDEX_DNAME: {
                char *pname = dps[i].value.string;
                LOG(LEVEL_NORMAL, "update name to %s\n", pname);
                strncpy(WKStack.params.name, pname, sizeof(WKStack.params.name));
            }
            break;
        }
    }

    for(i = 0; i < count; i++) {
        //free the string buffer
        if(dps[i].type == 4) {
            vg_free(dps[i].value.string);
            dps[i].value.string = NULL;
        }
    }

	//printf("### MEM FREE : %d.\n", qcom_mem_heap_get_free_size());
    LOG(LEVEL_DEBUG, "WKStack_unpack_sync X\n");
}


static int WKStack_unpack_binding(unsigned char *payload, int len)
{
    LOG(LEVEL_DEBUG, "WKStack_unpack_binding E\n");

    int count = decode_payload((char *)payload, len);
    if(count <= 0)  
        return 0;

    int i = 0;
    
    char *user_id = 0;
    char *bind_error = 0;
    char *bind_ticket = 0;

    for(; i < count; i++) {

        switch(dps[i].index) {
            
            case WKSTACK_BINDING_INDEX_USERID: {
                user_id = dps[i].value.string;
            }
            break;

            case WKSTACK_BINDING_INDEX_TICKET: {
                bind_ticket = dps[i].value.string;
            }
            break;
            
            case WKSTACK_BINDING_INDEX_ERROR: {
                bind_error = dps[i].value.string;
            }
            break;
        }
    }
    

    if(user_id && bind_ticket) {
        LOG(LEVEL_NORMAL, "user: %s, ticket: %s\n", user_id, bind_ticket);
        
        client_info_t *client_info = 0;
        client_info = (client_info_t *)plist_find(is_client, user_id);
        
        char buf[128];
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "VENGAS:BIND:%s#%s:VENGAE", WKStack.params.did, bind_ticket);

        if(client_info) {
            udpserver_sendto(&client_info->addr, buf, strlen(buf));
        }
        else {
            LOG(LEVEL_ERROR, "plist too small ? \n");
        }
    }
    else if(bind_error) {
        //TODO return error msg to client
        LOG(LEVEL_NORMAL, "bind error %s\n", bind_error);
    }

    for(i = 0; i < count; i++) {
        //free the string buffer
        if(dps[i].type == 4) {
            vg_free(dps[i].value.string);
            dps[i].value.string = NULL;
        }
    }

	//printf("### MEM FREE : %d.\n", qcom_mem_heap_get_free_size());
    LOG(LEVEL_DEBUG, "WKStack_unpack_binding X\n");

	return 0;
}

// control include command and set status
// tmp_buffer: |  512  | 512 |
// store:      |  arg1 | arg2|
static int WKStack_unpack_control(unsigned char *payload, int len)
{
    LOG(LEVEL_DEBUG, "WKStack_unpack_control E\n");

    int count = decode_payload((char *)payload, len);
    if(count <= 0)  
        return 0;

    int i = 0;

    if(WKStack.dp_handler) {
         WKStack.dp_handler(dps, count);
    }

    for(i = 0; i < count; i++) {
        //free the string buffer
        if(dps[i].type == 4) {
            vg_free(dps[i].value.string);
            dps[i].value.string = NULL;
        }
    }

    if(g_testmode) {
        
        char * buf = vg_malloc(len<<1);
        if(!buf) {
            LOG(LEVEL_ERROR, "OOM\n");
            return;
        }

        bin2hex(payload, len, buf);
        tcpserver_send(buf, len<<1);

        vg_free(buf);
    }
	//printf("### MEM FREE : %d.\n", qcom_mem_heap_get_free_size());
    LOG(LEVEL_DEBUG, "WKStack_unpack_control X\n");

	return 0;
}


int WKStack_pack_connect(char *client_id, int willflag)
{
    //NOTE: message must be declared static
    static char _id[128]; 

    memset(_id, 0, sizeof(_id));

	g_mqtt_data.MQTTVersion = 3;

    if(client_id) {
        strcpy(_id, client_id);
    }
    else {
        sprintf((char *)_id, "%s#%s", WKStack.params.product_id, WKStack.params.mac);
    }

    g_mqtt_data.clientID = _id;

	//data.clientID = WKStack.params.mac;
	g_mqtt_data.username = NULL;

    // First connect, without last will
	if(willflag == 0){
		g_mqtt_data.willFlag	= 0;
    } else {
        /*
        static char message[128];
        memset(message, 0, sizeof(message));
	    sprintf(message, "{did:%s}", WKStack.params.did);
		g_mqtt_data.willFlag = 1;		// 0 close will message, 1 use will message .
        g_mqtt_data.will.topicName = WKSTACK_TOPIC_OFFLINE;
        g_mqtt_data.will.message = message;
        g_mqtt_data.will.retained = 1;
        g_mqtt_data.will.qos = 1;
        */
    }

	g_mqtt_data.password = NULL;
	g_mqtt_data.cleansession = 0;		// 0 will receive outline message.

	return 0;
}

int WKStack_publish_sync()
{
    int ret = -1;

    LOG(LEVEL_NORMAL, "pub sync\n");

    char buf[128];
    memset(buf, 0, sizeof(buf));

    int offset = 0;

    {
        char tag[4] = {0, 0, WKSTACK_DATAPOINT_TYPE_STRING, 0};
        *(unsigned short*)tag = WKSTACK_SYNC_INDEX_SDKVER;
        int item_size = tlv_put_string(buf+offset, tag, WKStack_version, sizeof(buf) - offset);
        LOG(LEVEL_NORMAL, "sdkver %s\n", WKStack_version);
        if(item_size >= 0)
            offset += item_size;
        else { 
            LOG(LEVEL_ERROR, "knock publish buffer not large enough\n");
            return -1;
        }
    }

    if(strlen(WKStack.params.version) > 0) {

        char tag[4] = {0, 0, WKSTACK_DATAPOINT_TYPE_STRING, 0};
        *(unsigned short*)tag = WKSTACK_SYNC_INDEX_VER;

        LOG(LEVEL_NORMAL, "fwver %s\n", WKStack.params.version);

        int item_size = tlv_put_string(buf+offset, tag, WKStack.params.version, sizeof(buf) - offset);
        if(item_size >= 0)
            offset += item_size;
        else { 
            LOG(LEVEL_ERROR, "knock publish buffer not large enough\n");
            return -1;
        }
    }

    return mqtt_publish(WKStack.sync_pub_topic, (unsigned char*)buf, offset, MQTT_QOS1, MQTT_RETAIN_FALSE, (mqtt_cb_t)NULL);
}

int WKStack_publish_knock()
{
    int ret = -1;

    sprintf(WKStack.topic_knock, WKSTACK_TOPIC_KNOCK_FMT, WKStack.params.mac); 
    LOG(LEVEL_NORMAL, "pub knock\n");

/*
    LOG(LEVEL_DEBUG, "publish to topic %s\n", WKStack.topic_knock);
    char buf[32];
    
    if(strlen(WKStack.params.version) > 0)
        sprintf(buf, "%s %s", WKStack_version, WKStack.params.version);
    else
        sprintf(buf, "%s", WKStack_version);

    ret = mqtt_publish(WKStack.topic_knock, buf, strlen(buf), 0, 0, NULL);
*/

    char buf[(WKSTACK_VALUE_LEN << 1 )+ WKSTACK_SN_LEN];
    memset(buf, 0, sizeof(buf));

    int offset = 0;

    {
        char tag[4] = {0, 0, WKSTACK_DATAPOINT_TYPE_STRING, 0};
        *(unsigned short*)tag = WKSTACK_SYNC_INDEX_SDKVER;
        int item_size = tlv_put_string(buf+offset, tag, WKStack_version, sizeof(buf) - offset);
        LOG(LEVEL_NORMAL, "sdkver %s\n", WKStack_version);
        if(item_size >= 0)
            offset += item_size;
        else { 
            LOG(LEVEL_ERROR, "knock publish buffer not large enough\n");
            return -1;
        }
    }

    if(strlen(WKStack.params.version) > 0) {

        char tag[4] = {0, 0, WKSTACK_DATAPOINT_TYPE_STRING, 0};
        *(unsigned short*)tag = WKSTACK_SYNC_INDEX_VER;

        LOG(LEVEL_NORMAL, "fwver %s\n", WKStack.params.version);

        int item_size = tlv_put_string(buf+offset, tag, WKStack.params.version, sizeof(buf) - offset);
        if(item_size >= 0)
            offset += item_size;
        else { 
            LOG(LEVEL_ERROR, "knock publish buffer not large enough\n");
            return -1;
        }
    }

    if(strlen(WKStack.params.sn) > 0) {

        char tag[4] = {0, 0, WKSTACK_DATAPOINT_TYPE_STRING, 0};
        *(unsigned short*)tag = WKSTACK_SYNC_INDEX_SN;
        LOG(LEVEL_NORMAL, "sn %s\n", WKStack.params.sn);
        int item_size = tlv_put_string(buf+offset, tag, WKStack.params.sn, sizeof(buf) - offset);
        if(item_size >= 0)
            offset += item_size;
        else { 
            LOG(LEVEL_ERROR, "knock publish buffer not large enough\n");
            return -1;
        }
    }
    return mqtt_publish(WKStack.topic_knock, (unsigned char*)buf, offset, MQTT_QOS1, MQTT_RETAIN_FALSE, (mqtt_cb_t)NULL);
}

static int WKStack_publish_answer(char *answer, int len)
{
    int ret = -1;

    LOG(LEVEL_NORMAL, "pub answer\n");

    sprintf(WKStack.topic_answer, WKSTACK_TOPIC_ANSWER_FMT, WKStack.params.mac); 
    ret = mqtt_publish(WKStack.topic_answer, answer, len, 0, 0, NULL);

	return ret;
}


int WKStack_unpack_welcome(unsigned char *payload, int len) {

    LOG(LEVEL_DEBUG, "WKStack_unpack_welcome E\n");


    if(memcmp(payload, REGISTRY_ERR_SYSTEM_FAILURE, strlen(REGISTRY_ERR_SYSTEM_FAILURE)) == 0) {

        LOG(LEVEL_ERROR, "Failed to register device: "REGISTRY_ERR_SYSTEM_FAILURE"\n");

        msleep(RECONNECT_DELAY_SHORT);

        WKStack.state = WKSTACK_OFFLINE;
        //doStart();

        return -1;
    }

    if(memcmp(payload, REGISTRY_ERR_UNAUTHORIZED, strlen(REGISTRY_ERR_UNAUTHORIZED)) == 0) {

         msleep(RECONNECT_DELAY_LONG);
         
         WKStack.state = WKSTACK_OFFLINE;
         //doStart();
 

        LOG(LEVEL_ERROR, "Failed to register device: "REGISTRY_ERR_UNAUTHORIZED"\n");
        return -1;
    }


    int count = decode_payload((char *)payload, len);
    if(count <= 0){
        LOG(LEVEL_ERROR, "Failed to decode welcome msg\n");
        return -1;
    }

    int i = 0;

    int hasDid = 0;
    int hasEndpoint = 0;
    int hasTicket = 0;
    int hasName = 0;

    char *purl = 0;
    char *pname = 0;
    char *pdid = 0;
    char *pticket = 0;

    for(i = 0; i < count; i++) {

        switch(dps[i].index) {
            
            case WKSTACK_WELCOME_INDEX_URL: {


                purl= dps[i].value.string;


                char port[8] = {0,};

                parse_url((char *)purl, WKStack.params.host, port);
                WKStack.params.port = atoi(port);
                LOG(LEVEL_NORMAL, "endpoint:%s:%d\n", WKStack.params.host, WKStack.params.port);

                hasEndpoint = 1;

            }
            break;

            case WKSTACK_WELCOME_INDEX_DID: {
                pdid = dps[i].value.string;
                
                strncpy((char *)WKStack.params.did, pdid, WKSTACK_DID_LEN);

                LOG(LEVEL_NORMAL, "did:%s\n", WKStack.params.did);
                hasDid = 1;
            }
            break;
            
            case WKSTACK_WELCOME_INDEX_TICKET: {
                char *pticket = dps[i].value.string;


                strncpy(WKStack.params.ticket, pticket, sizeof(WKStack.params.ticket));
                LOG(LEVEL_NORMAL, "ticket:%s\n", WKStack.params.ticket);
                hasTicket = 1;
            }
            break;

            case WKSTACK_WELCOME_INDEX_DNAME: {

                pname = dps[i].value.string;
                strncpy(WKStack.params.name, pname, sizeof(WKStack.params.name));
                LOG(LEVEL_NORMAL, "name:%s\n", WKStack.params.name);
                hasName = 1;
            }
            break;
        }
    }

    for(i = 0; i < count; i++) {
        //free the string buffer
        if(dps[i].type == 4) {
            vg_free(dps[i].value.string);
            dps[i].value.string = NULL;
        }
    }

    //hasName = 1;


    if(hasDid && hasEndpoint && hasTicket /*&& hasName*/) {

        mqtt_stop(0);

        return 0;
    }

	//printf("### MEM FREE : %d.\n", qcom_mem_heap_get_free_size());
    return -1;
}

int WKStack_subscribe_welcome()
{
    sprintf(WKStack.topic_welcome, WKSTACK_TOPIC_WELCOME_FMT, WKStack.params.mac); 
    mqtt_subscribe(WKStack.topic_welcome, NULL, WKStack_unpack_welcome);

    return 0;
}

int WKStack_unpack_challenge(unsigned char *payload, int len) {
  
#define AES_LEN 16
    char aes[AES_LEN];
   
    if(!payload || len != AES_LEN) {
        LOG(LEVEL_ERROR, "FATAL: empty challenge payload or invalid len %d\n", len);
        return -1;
    }

    //aes_ecb(aes, (unsigned char*)payload, (unsigned char*)WKStack.params.key);
    aes_ecb(aes, (unsigned char*)payload, (unsigned char*)WKStack.params.key);

    LOG(LEVEL_DEBUG, "received challenge\n");
    vg_print_hex(LEVEL_DEBUG, payload, len);
    LOG(LEVEL_DEBUG, "answer:\n");
    vg_print_hex(LEVEL_DEBUG, aes, AES_LEN);

    WKStack_publish_answer(aes, len);
   
    return 0;
}

int WKStack_subscribe_challenge()
{
    sprintf(WKStack.topic_challenge, WKSTACK_TOPIC_CHALLENGE_FMT, WKStack.params.mac); 
    mqtt_subscribe(WKStack.topic_challenge, NULL, WKStack_unpack_challenge);

    return 0;
}

int WKStack_subscribe_control()
{
    mqtt_subscribe(WKStack.control_topic, NULL, WKStack_unpack_control);

    return 0;
}

int WKStack_subscribe_sync()
{
    mqtt_subscribe(WKStack.sync_sub_topic, NULL, WKStack_unpack_sync);

    return 0;
}


int WKStack_subscribe_ota()
{
	mqtt_subscribe(WKStack.ota_sub_topic, NULL, WKStack_unpack_ota);

    return 0;
}


int WKStack_subscribe_binding()
{
	mqtt_subscribe(WKStack.binding_sub_topic, NULL, WKStack_unpack_binding);

    return 0;
}


int WKStack_publish_bind_request(char *userId) 
{
    LOG(LEVEL_DEBUG, "WKStack_publish_bind_request from userId %s E\n", userId);

    char buf[32];
    int buf_size = 32;

	memset(buf, 0, sizeof(buf));

    int offset = 0;

    char tag[4] = {0, 0, WKSTACK_DATAPOINT_TYPE_STRING, 0};
    *(unsigned short*)tag = WKSTACK_BINDING_INDEX_USERID;

    int item_size = tlv_put_string(buf+offset, tag, userId, buf_size - offset);

    if(item_size >= 0)
        offset += item_size;
    else { 
        LOG(LEVEL_ERROR, "ota publish buffer not large enough\n");
        return -1;
    }

    return mqtt_publish(WKStack.binding_pub_topic, (unsigned char*)buf, offset, MQTT_QOS1, MQTT_RETAIN_FALSE, (mqtt_cb_t)0);
}



