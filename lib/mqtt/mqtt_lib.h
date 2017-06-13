#ifndef _MQTT_LIB_H_SDFKJSDKFJSDKFDSF_
#define _MQTT_LIB_H_SDFKJSDKFJSDKFDSF_

#include "mqtt.h"

#define MAX_HOST_NAME 64

#define MAX_TOPIC_LEN 96

#define MQTT_ACK_TIME 10000 // 10 s for ACK timeout

#define MQTT_RETRY_TIMES 3


typedef struct{
    char *name;
    mqtt_topic_cb_t cb;
}mqtt_topic_t;

typedef struct
{
    char host[MAX_HOST_NAME];
    unsigned short port;
    int sockfd;
    int ping_times;
    int keepalive;
    int window_size;
    char *send_buf;
    char *recv_buf;
    mqtt_topic_t topic[MQTT_MAX_TOPIC_NUM];
    mqtt_state_t state;
    mqtt_errno_t error_number;
    mqtt_connect_data_t connect_data;
    mqtt_connect_cb_t connect_cb;
    mqtt_stop_cb_t stop_cb;
}mqtt_t;

extern mqtt_t mqtt;

unsigned short mqtt_msgid();

int mqtt_set_remain_length(unsigned char *buf, int length);
int mqtt_get_remain_length(unsigned char **plen);

void writeCString(unsigned char **pptr, const char *string);
void writeChar(unsigned char **pptr, char c);
void writeInt(unsigned char **pptr, int anInt);
void writeMQTTString(unsigned char **pptr, char *mqttstring);

int MQTTstrlen(char *mqttstring);
int MQTTSerialize_connectLength(mqtt_connect_data_t *options);
int MQTTSerialize_subscribeLength(char *topic);
int MQTTSerialize_publishLength(int qos, char *topicName, int payloadlen);
int MQTTPacket_len(int rem_len);

int mqtt_register_topic(char *topic, mqtt_topic_cb_t);
int mqtt_unregister_topic(char *topic);
mqtt_topic_t *mqtt_topic_find(char *topic);
int mqtt_topic_init(void);

int mqtt_send(unsigned char *buf, int len);
int mqtt_recv(unsigned char *buf, int len);
int mqtt_socket(char *ip, int port);


#endif
