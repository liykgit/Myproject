
#include <string.h>
#include "common.h"
#include "tlv.h"
#include "mqtt.h"
#include "WKStack.h"
#include "WKStack_lib.h"
#include "WKStack_packet.h"

#include "pushlist.h"
#include "udpserver.h"

mqtt_connect_data_t data = { {'M', 'Q', 'T', 'C'}, 0, 4, 0, 1, 0, { {'M', 'Q', 'T', 'W'}, 0, 0, 0, 0, 0 }, 0, 0 };

static WKStack_datapoint_t dps[DP_COUNT_MAX];



static WKStack_datapoint_t *fill_dp_chunk(WKStack_datapoint_t *pdp, TLV_t *ptlv) {

    printf("fill_dp_chunk E\n");

    pdp->index = *(unsigned short *)ptlv->tag;
    pdp->type = (int)ptlv->tag[2];

    switch(ptlv->tag[2]) {
        case 1: {
                printf("bool index %d, len %d\n", pdp->index, ptlv->len);
                memcpy(&pdp->value.boolean, ptlv->value, ptlv->len);
                printf("val %d\n", pdp->value.boolean);
        }
        break;

        case 2: {

                printf("int index %d, len %d\n", pdp->index, ptlv->len);
                memcpy(&pdp->value.integer, ptlv->value, ptlv->len);
                printf("val %d\n", pdp->value.integer);
        }
        break;

        case 3: {
                printf("float index %d, len %d\n", pdp->index, ptlv->len);
                printf("type float\n");
                memcpy(&pdp->value.floatpoint, ptlv->value, ptlv->len);
        }
        break;

        case 4: {
                printf("string index %d, len %d\n", pdp->index, ptlv->len);

                pdp->value.string = vg_malloc(ptlv->len + 1);
                memset(pdp->value.string, 0, ptlv->len + 1);
                memcpy(pdp->value.string, ptlv->value, ptlv->len);

                printf("%s\n", pdp->value.string);
        }
        break;
    }

    return pdp;
}

//return number of tlv items decoded */
static int decode_payload(char *data, int len) {

    printf("decoding tlv ...  E\n "); 

    int offset = 0;
    int item_size = 0;
    int i = 0;

    while(len > 0) {
   
        TLV_t *ptlv = NULL;
        printf("parse start at offset %d\n", offset);
        item_size = tlv_next(data+offset, len, &ptlv);
        printf("item size %d\n", item_size);

        if(item_size >= 0) {
            printf("len :  %d\n", ptlv->len);
            len -= item_size;

            printf("offset %d len-item_size %d\n", offset, len);
            printf("index %d\n", *(unsigned short *)ptlv->tag);

            printf("data type %d\n", ptlv->tag[2]);

            fill_dp_chunk(&dps[i++], ptlv);

            offset += item_size;

        }
        else {
            break;
        }
    }

    printf("decoding...  X\n "); 

    return i;
}

int WKStack_publish_ota_request(char *version)
{
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
        printf("ota publish buffer not large enough\n");
        return -1;
    }

    printf("publishing ota request, buf size  %d\n", offset);
    return mqtt_publish(WKStack.ota_pub_topic, (unsigned char*)buf, offset, MQTT_QOS1, MQTT_RETAIN_FALSE, (mqtt_cb_t)0);
}




static int WKStack_unpack_ota(unsigned char *payload, int len)
{
    LOG(LEVEL_DEBUG, "<LOG> WKStack_unpack_ota E\n");

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
        LOG(LEVEL_DEBUG, "<LOG> WKStack_unpack_ota received version notification %s E\n", version);

        if(!version)
            return 0;

        strcpy(WKStack.ota.mod_ver, version);

        WKStack_publish_ota_request(version);
        LOG(LEVEL_DEBUG, "<LOG> WKStack_unpack_ota received version notification %s X\n", version);
    }
    else if (type == WKSTACK_OTA_TYPE_MSG) {
        
        if(!uri || !ticket)
            return 0;
       
        strcpy(WKStack.ota.mod_url, uri);
        strcat(WKStack.ota.mod_url, "?ticket=");
        strcat(WKStack.ota.mod_url, ticket);

        LOG(LEVEL_DEBUG, "<LOG> ota url %s\n", WKStack.ota.mod_url);
        LOG(LEVEL_DEBUG, "<LOG> unsubscribe %s\n", WKStack.ota_sub_topic);
        mqtt_unsubscribe(WKStack.ota_sub_topic, NULL);    
        msleep(800);
    }
    else {

        LOG(LEVEL_DEBUG, "<LOG> WKStack_unpack_ota E\n");
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

static int WKStack_unpack_binding(unsigned char *payload, int len)
{
    printf("<LOG> WKStack_unpack_control E\n");

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
    
    if(bind_error) {
        printf("bind error %s\n", bind_error);

    }

    if(user_id && bind_ticket) {
        printf("%s, %s\n", user_id, bind_ticket);
        
        client_info_t *client_info = 0;
        client_info = (client_info_t *)plist_find(is_client, user_id);
        if(client_info) {
            udpserver_sendto(&client_info->addr, bind_ticket, strlen(bind_ticket));
        }
        else {
            printf("No user for bind ticket, plist too small ? \n");
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

	return 0;
}

// control include command and set status
// tmp_buffer: |  512  | 512 |
// store:      |  arg1 | arg2|
static int WKStack_unpack_control(unsigned char *payload, int len)
{
    printf("<LOG> WKStack_unpack_control E\n");

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

	//printf("### MEM FREE : %d.\n", qcom_mem_heap_get_free_size());

	return 0;
}


int WKStack_pack_connect(char *client_id, int willflag)
{
    //NOTE: message must be declared static
    static char message[128];
    static char _id[24]; //mqtt allows 23 bytes at max

	data.MQTTVersion = 3;

    if(client_id) {
        //TODO strncpy ?? tailing 0 must be asured
        strcpy(_id, client_id);
    }
    else {
        sprintf((char *)_id, "%s#%s", WKStack.params.devtype, WKStack.params.mac);
    }

    data.clientID = _id;

	//data.clientID = WKStack.params.mac;
	data.username = NULL;

    // First connect, without last will
	if(willflag == 0){
		data.willFlag	= 0;
    } else {
        //TODO obsolete code
        memset(message, 0, sizeof(message));
	    sprintf(message, "{did:%s}", WKStack.params.did);
		data.willFlag = 1;		// 0 close will message, 1 use will message .
        data.will.topicName = WKSTACK_TOPIC_OFFLINE;
        data.will.message = message;
        data.will.retained = 1;
        data.will.qos = 1;
    }

	data.password = NULL;
	data.cleansession = 0;		// 0 will receive outline message.

	return 0;
}

int WKStack_publish_knock()
{
    int ret = -1;

    sprintf(WKStack.topic_knock, WKSTACK_TOPIC_KNOCK_FMT, WKStack.params.mac); 
    printf("publish to topic %s\n", WKStack.topic_knock);
    ret = mqtt_publish(WKStack.topic_knock, (unsigned char *)WKStack_version, strlen(WKStack_version), 0, 0, NULL);

	return ret;
}

static int WKStack_publish_answer(char *answer, int len)
{
    int ret = -1;

    sprintf(WKStack.topic_answer, WKSTACK_TOPIC_ANSWER_FMT, WKStack.params.mac); 
    printf("publish to topic %s\n", WKStack.topic_answer);
    ret = mqtt_publish(WKStack.topic_answer, answer, len, 0, 0, NULL);

	return ret;
}

int WKStack_unpack_welcome(unsigned char *payload, int len) {

    int hasDid = 0;
    int hasEndpoint = 0;
    int hasTicket = 0;
    
    unsigned char *p1 = strstr(payload, "*");
    int did_len = p1 - payload;

    if(did_len <= 0) {
        printf("Invalid did_len\n");
        printf("\n");
        return 0;
    }

    strncpy((char *)WKStack.params.did, (char *)payload, did_len);
    LOG(LEVEL_DEBUG, "<LOG> did:%s\n", WKStack.params.did);
    hasDid = 1;

    p1 += 1;
    unsigned char *p2 = strstr(p1, "*");
    int urllen = p2 - p1;

    if(urllen <= 0) {
        printf("Invalid urllen\n");
        printf("\n");

        return 0;
    }

    int ticket_len = len - did_len - urllen - 2*strlen("*");
    if(ticket_len != TICKET_LEN) {
        printf("Invalid ticket length len %d, %d-%d-%d-2\n", ticket_len, len, did_len, urllen);
        return 0;
    }

    char endpoint[urllen + 1];
    memcpy(endpoint, p1, urllen);
    endpoint[urllen] = 0;

    char port[8] = {0,};

    parse_url((char *)endpoint, WKStack.params.host, port);
    WKStack.params.port = atoi(port);
    LOG(LEVEL_DEBUG, "<LOG> host:%s, port:%d\n", WKStack.params.host, WKStack.params.port);

    hasEndpoint = 1;
    
    p2 += 1;
    
    memcpy(WKStack.ticket, p2, TICKET_LEN);


    hasTicket = 1;

    if(hasDid && hasEndpoint && hasTicket) {

        WKStack.state = WKSTACK_WAIT_ONLINE;

        sprintf(WKStack.report_topic, WKSTACK_TOPIC_REPORT_FMT, WKStack.params.devtype, WKStack.params.did);

        sprintf(WKStack.control_topic, WKSTACK_TOPIC_CONTROL_FMT, WKStack.params.did);
        sprintf(WKStack.ota_sub_topic, WKSTACK_TOPIC_OTA_SUB_FMT, WKStack.params.did);
        sprintf(WKStack.ota_pub_topic, WKSTACK_TOPIC_OTA_PUB_FMT, WKStack.params.did);

        sprintf(WKStack.binding_sub_topic, WKSTACK_TOPIC_BINDING_SUB_FMT, WKStack.params.did);
        sprintf(WKStack.binding_pub_topic, WKSTACK_TOPIC_BINDING_PUB_FMT, WKStack.params.did);
    }

	//printf("### MEM FREE : %d.\n", qcom_mem_heap_get_free_size());
    return 0;
}


int WKStack_subscribe_welcome()
{
    printf("We are on\n");
    sprintf(WKStack.topic_welcome, WKSTACK_TOPIC_WELCOME_FMT, WKStack.params.mac); 
    printf("subscribe to topic %s\n", WKStack.topic_welcome);
    mqtt_subscribe(WKStack.topic_welcome, NULL, WKStack_unpack_welcome);

    return 0;
}

int WKStack_unpack_challenge(unsigned char *payload, int len) {
    
    char aes[len];

    aes_ecb(aes, (unsigned char*)payload, (unsigned char*)WKStack.params.key);

    printf("nonce : ");
    printf("signed output: ");

    WKStack_publish_answer(aes, len);
   
    return 0;
}

int WKStack_subscribe_challenge()
{
    sprintf(WKStack.topic_challenge, WKSTACK_TOPIC_CHALLENGE_FMT, WKStack.params.mac); 
    printf("subscribe to topic %s\n", WKStack.topic_challenge);
    mqtt_subscribe(WKStack.topic_challenge, NULL, WKStack_unpack_challenge);

    return 0;
}

int WKStack_subscribe_control()
{
    mqtt_subscribe(WKStack.control_topic, NULL, WKStack_unpack_control);

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
    char buf[64];
    int buf_size = 64;

	memset(buf, 0, sizeof(buf));

    int offset = 0;

    char tag[4] = {0, 0, WKSTACK_DATAPOINT_TYPE_STRING, 0};
    *(unsigned short*)tag = WKSTACK_BINDING_INDEX_USERID;

    int item_size = tlv_put_string(buf+offset, tag, userId, buf_size - offset);

    if(item_size >= 0)
        offset += item_size;
    else { 
        printf("ota publish buffer not large enough\n");
        return -1;
    }

    printf("publishing binding request, buf size  %d\n", offset);
    return mqtt_publish(WKStack.binding_pub_topic, (unsigned char*)buf, offset, MQTT_QOS1, MQTT_RETAIN_FALSE, (mqtt_cb_t)0);
}

