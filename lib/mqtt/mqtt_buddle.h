#ifndef _MQTT_BUDDLE_H_KSJDFKSKLFDJKSLJFDKJSDF_
#define _MQTT_BUDDLE_H_KSJDFKSKLFDJKSLJFDKJSDF_

#include "mqtt_lib.h"

// Free+TobeSend+TobeHandle
#define SEND_MAX		8
#define FLIGHT_MAX		12
#define PEND_MAX		8
#define BUDDLE_MAX		(SEND_MAX+FLIGHT_MAX+PEND_MAX)

#define BUDDLE_USED		1
#define BUDDLE_FREE		0

#define ATTR_FREE		0
#define ATTR_SEND		1
#define ATTR_FLIGHT		2
#define ATTR_PEND		3


typedef struct {
    unsigned char used:2;
	unsigned char attr:3;
	unsigned char retain:1;
	unsigned char qos:2;
	unsigned char dup:4;
	unsigned char type:4;
	unsigned short msg_id;
	char topic[MAX_TOPIC_LEN];
	unsigned char *payload;
	int length;
	unsigned long tick;
    int times; // Ping times + times <= MQTT_RETRY_TIMES
	mqtt_cb_t cb;
}mqtt_package_t;

int get_send_count();
int get_pend_count();
int get_flight_count();

void mqtt_buddle_init();
void FreeBuddle(mqtt_package_t *pItem);
void ResetBuddle();

mqtt_package_t *GetFree2Send();
mqtt_package_t *GetLastSendBuddle();
void MoveSend2Flight(mqtt_package_t *pItem);
mqtt_package_t *GetFree2Flight();
mqtt_package_t *GetFromFlightByID(unsigned short msg_id);
mqtt_package_t *GetFromFlightByType(int type);

mqtt_package_t *GetFree2Pending();
mqtt_package_t *GetLastPendingBuddle();
void MoveFlight2Pending(mqtt_package_t *pItem);
void RemoveTimeoutBuddle();
void ClearSendBuddle();
int GetFlightCount();
int GetPendingCount();
mqtt_package_t *check_flight(int ping_times);

#endif



