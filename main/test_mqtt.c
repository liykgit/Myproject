#include "common.h"
#include "mqtt.h"

mqtt_connect_data_t data = { {'M', 'Q', 'T', 'C'}, 0, 4, 0, 1, 0, { {'M', 'Q', 'T', 'W'}, 0, 0, 0, 0, 0 }, 0, 0 };

int connect_cb(mqtt_errno_t err)
{
    switch(err){
    case MQTT_CONNECT_SUCCEED:
        printf("Mqtt running\n");
        break;
    default:
		printf("Mqtt error\n");
        break;
    }
}

#define TEST_TOPIC1 "/subscribe/data1"
#define TEST_TOPIC2 "/subscribe/data2"
#define TEST_TOPIC3 "/subscribe/data3"
#define TEST_TOPIC4 "/publish/data"

#define TEST_DATA_QOS0 "QOS0"
#define TEST_DATA_QOS1 "QOS1"
#define TEST_DATA_QOS2 "QOS2"

int topic1_cb(unsigned char *payload, int len)
{
	int i = 0;

	printf("topic:%s\n", TEST_TOPIC1);
	printf("msg:");
	for (i = 0; i < len; i++) {
		printf("%c ", payload[i]);
	}
	printf("\n");
}

int topic2_cb(unsigned char *payload, int len)
{
	int i = 0;

	printf("topic:%s\n", TEST_TOPIC2);
	printf("msg:");
	for (i = 0; i < len; i++) {
		printf("%c ", payload[i]);
	}
	printf("\n");
}

int topic3_cb(unsigned char *payload, int len)
{
	int i = 0;

	printf("topic:%s\n", TEST_TOPIC3);
	printf("msg:");
	for (i = 0; i < len; i++) {
		printf("%c ", payload[i]);
	}
	printf("\n");
}

int main()
{
	log_init("test_mqtt");
    mqtt_init(10, 1024);

	data.cleansession = 1;

    mqtt_start("192.168.7.98", 1883, &data, connect_cb);

    while(1){
        if(mqtt_state() == MQTT_STATE_RUNNING){
            break;
        }
        msleep(1000);
    }

	printf("subscribe: %s\n", TEST_TOPIC1);
    mqtt_subscribe(TEST_TOPIC1, NULL, topic1_cb);
	printf("subscribe: %s\n", TEST_TOPIC2);
    mqtt_subscribe(TEST_TOPIC2, NULL, topic2_cb);
	printf("subscribe: %s\n", TEST_TOPIC3);
	mqtt_subscribe(TEST_TOPIC3, NULL, topic3_cb);

	printf("publish: topic: %s, msg: %s, qos: 0\n", TEST_TOPIC4, TEST_DATA_QOS0);
    mqtt_publish(TEST_TOPIC4, TEST_DATA_QOS0, sizeof(TEST_DATA_QOS0), MQTT_QOS0, NULL);

	printf("publish: topic: %s, msg: %s, qos: 1\n", TEST_TOPIC4, TEST_DATA_QOS1);
    mqtt_publish(TEST_TOPIC4, TEST_DATA_QOS1, sizeof(TEST_DATA_QOS1), MQTT_QOS1, NULL);

	printf("publish: topic: %s, msg: %s, qos: 2\n", TEST_TOPIC4, TEST_DATA_QOS2);
    mqtt_publish(TEST_TOPIC4, TEST_DATA_QOS2, sizeof(TEST_DATA_QOS2), MQTT_QOS2, NULL);

    while(1){
        msleep(1000);
    }

    return 0;
}
