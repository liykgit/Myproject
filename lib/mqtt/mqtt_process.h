#ifndef _MQTT_PROCESS_H_KSDJFKSLDJFKLSDJF_
#define _MQTT_PROCESS_H_KSDJFKSLDJFKLSDJF_

#ifdef __cplusplus
extern "C" {
#endif

int mqtt_process_pend();
int mqtt_process_timeout();
int mqtt_process_send();
int mqtt_process_error();
int mqtt_process_connack();

#ifdef __cplusplus
}
#endif

#endif
