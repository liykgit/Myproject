
#include <string.h>

#include "common.h"
#include "mqtt_lib.h"


static unsigned short g_packetid = 0x8;
mqtt_t mqtt;

unsigned short mqtt_msgid()
{
	return ++g_packetid;
}

int mqtt_set_remain_length(unsigned char *buf, int length)
{
    int rc = 0;
    char d;

    do
    {
        d = length % 128;
        length /= 128;

        /* if there are more digits to encode, set the top bit of this digit */
        if (length > 0)
            d |= 0x80;
        buf[rc++] = d;
    } while (length > 0);

    return rc;
}

void writeCString(unsigned char **pptr, const char *string)
{
    int len = strlen(string);
    writeInt(pptr, len);
    memcpy(*pptr, string, len);
    *pptr += len;
}

void writeChar(unsigned char **pptr, char c)
{
    **pptr = c;
    (*pptr)++;
}

/**
 * Writes an integer as 2 bytes to an output buffer.
 * @param pptr pointer to the output buffer - incremented by the number of bytes used & returned
 * @param anInt the integer to write
 */
void writeInt(unsigned char **pptr, int anInt)
{
    **pptr = (unsigned char)(anInt / 256);
    (*pptr)++;
    **pptr = (unsigned char)(anInt % 256);
    (*pptr)++;
}

void writeMQTTString(unsigned char **pptr, char *mqttstring)
{
    if (mqttstring)
        writeCString(pptr, mqttstring);
    else
        writeInt(pptr, 0);
}

int MQTTstrlen(char *mqttstring)
{
    int rc = 0;

    if (mqttstring)
        rc = strlen(mqttstring);
    return rc;
}

int MQTTSerialize_connectLength(mqtt_connect_data_t *options)
{
    int len = 0;

    if (options->MQTTVersion == 3)
        len = 12; /* variable depending on MQTT or MQIsdp */
    else if (options->MQTTVersion == 4)
        len = 10;

    len += MQTTstrlen(options->clientID) + 2;
    if (options->willFlag)
        len += MQTTstrlen(options->will.topicName) + 2 + MQTTstrlen(options->will.message) + 2;
    if (options->username)
        len += MQTTstrlen(options->username) + 2;
    if (options->password)
        len += MQTTstrlen(options->password) + 2;

    return len;
}
int MQTTSerialize_subscribeLength(char *topic)
{
	//int i;
	int len = 2; // packetid

	//for (i = 0; i < count; ++i)
	len += 2 + MQTTstrlen(topic) + 1; // length + topic + req_qos
	return len;
}

int MQTTSerialize_unsubscribeLength(char *topic)
{
	//int i;
	int len = 2; // packetid

	//for (i = 0; i < count; ++i)
	len += 2 + MQTTstrlen(topic); // length + topic
	return len;
}


int MQTTSerialize_publishLength(int qos, char *topicName, int payloadlen)
{
	int					len = 0;

	len					+= 2 + MQTTstrlen(topicName) + payloadlen;
	if (qos > 0)
		len				+= 2; /* packetid */

	return len;
}
int MQTTPacket_len(int rem_len)
{
    rem_len += 1; /* header byte */

    /* now remaining_length field */
    if (rem_len < 128)
        rem_len += 1;
    else if (rem_len < 16384)
        rem_len += 2;
    else if (rem_len < 2097151)
        rem_len += 3;
    else
        rem_len += 4;
    return rem_len;
}

int mqtt_get_remain_length(unsigned char **plen)
{
	unsigned char len_tem[4];

	len_tem[0] = **plen;
	if((len_tem[0] & 0x80) == 0) {
		(*plen)++;
		return len_tem[0];
	}
	len_tem[0] &= 0x7F;

	(*plen)++;
	len_tem[1] = **plen;
	if((len_tem[1] & 0x80) == 0) {
		(*plen)++;
		return len_tem[1] * 128 + len_tem[0];
	}
	len_tem[1] &= 0x7F;

	(*plen)++;
	len_tem[2] = **plen;
	if((len_tem[2] & 0x80) == 0) {
		(*plen)++;
		return (len_tem[2] * 16384) + len_tem[1]*128 + len_tem[0];
	}
	len_tem[2] &= 0x7F;

	(*plen)++;
	len_tem[3] = **plen;
	if((len_tem[3] & 0x80) == 0) {
		(*plen)++;
		return (len_tem[3] * 2097152) + len_tem[2]*16384 + len_tem[1]*128+ len_tem[0];
	}

	return -1;
}

int mqtt_register_topic(char *topic, mqtt_topic_cb_t cb)
{
	int i = 0;

	for(i = 0; i < MQTT_MAX_TOPIC_NUM; i++){
		if(mqtt.topic[i].name == NULL){
		    printf("register topic %s  at %d\n", topic, i);
			mqtt.topic[i].name = topic;
			mqtt.topic[i].cb = cb;
			break;
		}
	}

	if(i >= MQTT_MAX_TOPIC_NUM){
		return -1;
	}else{
		return 0;
	}
}

int mqtt_unregister_topic(char *topic)
{
	int i = 0;

	for(i = 0; i < MQTT_MAX_TOPIC_NUM; i++){

		if(mqtt.topic[i].name && strcmp(mqtt.topic[i].name, topic) == 0){
			mqtt.topic[i].name = NULL;
			mqtt.topic[i].cb = NULL;

			break;
		}
	}

	if(i >= MQTT_MAX_TOPIC_NUM){
		return -1;
	}else{
		return 0;
	}
}

mqtt_topic_t *mqtt_topic_find(char *topic)
{
	int i = 0;

	for(i = 0; i < MQTT_MAX_TOPIC_NUM; i++){
		if (mqtt.topic[i].name == NULL)
			continue;
		if(strcmp(mqtt.topic[i].name, topic) == 0){
			return &(mqtt.topic[i]);
		}
	}

	return NULL;
}

int mqtt_topic_init(void)
{
	memset(mqtt.topic, 0, sizeof(mqtt_topic_t) * MQTT_MAX_TOPIC_NUM);

	return 0;
}

int mqtt_send(unsigned char *buf, int len)
{
    LOG(LEVEL_ERROR, "mqtt_send E\n");
	int send_repeat = MQTT_RETRY_TIMES;
	int ret =-1;

	if(buf == NULL || len == 0) {

        LOG(LEVEL_ERROR, "mqtt_send X \n");
        LOG(LEVEL_ERROR, "mqtt_send X\n");
		return -1;
    }

repeat:
    ret = vg_send(mqtt.sockfd, buf, len, CONN_MODE);
    if(ret >= 0){
        if(ret == 0){
            LOG(LEVEL_ERROR, "<ERR> tcp send length 0\n");
        }
        LOG(LEVEL_ERROR, "mqtt_send X \n");
        return ret;
    }else if(ret == -100){
        LOG(LEVEL_ERROR, "<ERR> tcp send ret = -100\n");
        if(send_repeat-- > 0){
            msleep(2000);
            goto repeat;
        }else{
            LOG(LEVEL_ERROR, "mqtt_send X \n");
            return -2;
        }
    }else{
        LOG(LEVEL_ERROR, "<ERR> tcp send error\n");
		if(send_repeat-- > 0){
            msleep(2000);
            goto repeat;
        }else{
            LOG(LEVEL_ERROR, "mqtt_send X \n");
			return -3;
        }
    }

        LOG(LEVEL_ERROR, "mqtt_send X \n");
	return -1;
}

int mqtt_recv(unsigned char *buf, int len)
{
	int read_count = MQTT_RETRY_TIMES;
	int rdlen = 0;

repeat:
	rdlen = vg_recv(mqtt.sockfd, buf, len, CONN_MODE);
	if(rdlen >= 0){//read or timeout
#if 0
		if(rdlen == 0){
			LOG(LEVEL_ERROR, "<ERR> Tcp read rdlen = 0\n");
			msleep(500);
		}
#endif
		return rdlen;
	}else if(rdlen == -1){//select return -1
		LOG(LEVEL_ERROR, "<ERR> Tcp read rdlen = -1\n");
		return -1;
	}else if(rdlen == -2){//recv return -1
		LOG(LEVEL_ERROR, "<ERR> Tcp read rdlen = -2\n");
		if(read_count-- > 0){
			msleep(100);
			goto repeat;
		}else{
			return -2;
		}
	}
	else if(rdlen == -3){//recv return 0, socked has been closed.
		LOG(LEVEL_ERROR, "<ERR> Tcp read rdlen = -3\n");
		if(read_count-- > 0){
			msleep(100);
			goto repeat;
		}else{
			return -3;
		}
	}

	return -4;
}

int mqtt_socket(char *ip, int port)
{
	int ret = -1;

	if(vg_tcp_socket(&(mqtt.sockfd)) != 0){
		//printf("<ERR> createSocket fail.\n");
		return -1;
	}

	ret = vg_connect_server(&(mqtt.sockfd), ip, port, CONN_MODE);
	if(ret != 0){
		return -2;
	}
	return 0;
}

