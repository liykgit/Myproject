
#include <string.h>

#include "string.h"
#include "common.h"

#include "mqtt.h"
#include "mqtt_buddle.h"
#include "mqtt_lib.h"
#include "mqtt_packet.h"
#include "mqtt_process.h"

extern int mqtt_pubrel(unsigned short msg_id);
extern int mqtt_puback(unsigned short msg_id);
extern int mqtt_pubrec(unsigned short msg_id);
extern int mqtt_pubcomp(unsigned short msg_id);

static int mqtt_pingreq()
{
    int len = 0;
    int ret = 0;

    memset(mqtt.send_buf, 0, mqtt.window_size);

    len = mqtt_pack_pingreq(mqtt.send_buf, mqtt.window_size);

    ret = mqtt_send(mqtt.send_buf, len);
    if(ret <= 0){
        LOG(LEVEL_DEBUG, "<LOG> Mqtt send ping error\n");
        mqtt.error_number = MQTT_SOCKET_ERROR;
        mqtt.state = MQTT_STATE_ERROR;
    }

    mqtt.ping_times++;

    return 0;
}



int mqtt_process_pend()
{
    mqtt_package_t *pMsg = NULL;
    mqtt_package_t *pFlight = NULL;
    mqtt_topic_t *topic = NULL;
    int offset = 0;

    while((pMsg = GetLastPendingBuddle()) != NULL){
        switch(pMsg->type){
		case MQTT_PUBLISH:
            if(pMsg->qos != MQTT_QOS0){ // QOS0 have no msg id
                pMsg->msg_id = (((unsigned short)(pMsg->payload[0])) << 8) + pMsg->payload[1];
                LOG(LEVEL_DEBUG, "<LOG> Get PUBLISH, msg id is %d\n", pMsg->msg_id);
                if(pMsg->qos == MQTT_QOS1){
                    mqtt_puback(pMsg->msg_id);
                }else if(pMsg->qos == MQTT_QOS2){
                    mqtt_pubrec(pMsg->msg_id);
                }
                offset = 2;
            }
            printf("offset: %d, length: %d\n", offset, pMsg->length);
            // TODO: If it's a qos2 message, than do not tell user in this time,tell user the message when recv the PUBREL
            topic = mqtt_topic_find(pMsg->topic);
            if(topic != NULL){
                if(topic->cb != NULL)
                    topic->cb(pMsg->payload + offset, pMsg->length - offset);
            }
            break;
        case MQTT_PUBACK:
            pMsg->msg_id = (((unsigned short)(pMsg->payload[0])) << 8) + pMsg->payload[1];

            pFlight	= GetFromFlightByID(pMsg->msg_id);
            if(pFlight != NULL && pFlight->type == MQTT_PUBLISH){
                if(pFlight->cb != NULL){
                    pFlight->cb(pFlight->msg_id, MQTT_PUBLISH_SUCCEED);
                }
                FreeBuddle(pFlight);
            }
            break;
        case MQTT_PUBREC:
            pMsg->msg_id = (((unsigned short)(pMsg->payload[0])) << 8) + pMsg->payload[1];

            pFlight	= GetFromFlightByID(pMsg->msg_id);
            if(pFlight != NULL && pFlight->type == MQTT_PUBLISH && pFlight->qos == MQTT_QOS2){
                mqtt_pubrel(pMsg->msg_id);
            }
            break;
        case MQTT_PUBREL:
            //TODO: tell user the message and delete the message
            mqtt_pubcomp(pMsg->msg_id);
            break;
        case MQTT_PUBCOMP:
            pMsg->msg_id = (((unsigned short)(pMsg->payload[0])) << 8) + pMsg->payload[1];

            pFlight	= GetFromFlightByID(pMsg->msg_id);
            if(pFlight != NULL && pFlight->type == MQTT_PUBLISH && pFlight->qos == MQTT_QOS2){
                if(pFlight->cb != NULL){
                    pFlight->cb(pFlight->msg_id, MQTT_PUBLISH_SUCCEED);
                }
                FreeBuddle(pFlight);
            }
            break;
        case MQTT_SUBACK:
            pMsg->msg_id = (((unsigned short)(pMsg->payload[0])) << 8) + pMsg->payload[1];

            pFlight	= GetFromFlightByID(pMsg->msg_id);
            if(pFlight != NULL && pFlight->type == MQTT_SUBSCRIBE){
                if(pFlight->cb != NULL){
                    mqtt.error_number = MQTT_SUBSCRIBE_MAX_QOS0 + pMsg->payload[2];
                    pFlight->cb(pFlight->msg_id, mqtt.error_number);
                    if(mqtt.error_number == MQTT_SUBSCRIBE_FAILED){
                        mqtt_unregister_topic(pFlight->topic);
                    }
                }
                FreeBuddle(pFlight);
            }
            break;
        case MQTT_UNSUBACK:
            pMsg->msg_id = (((unsigned short)(pMsg->payload[0])) << 8) + pMsg->payload[1];
            pFlight	= GetFromFlightByID(pMsg->msg_id);
            if(pFlight != NULL && pFlight->type == MQTT_UNSUBSCRIBE){
                if(pFlight->cb != NULL){
                    mqtt.error_number = MQTT_SUBSCRIBE_MAX_QOS0 + pMsg->payload[2];
                    pFlight->cb(pFlight->msg_id, mqtt.error_number);
                    if(mqtt.error_number == MQTT_UNSUBSCRIBE_FAILED){
                        mqtt_unregister_topic(pFlight->topic);
                    }
                }

                FreeBuddle(pFlight);
            }
            break;

        case MQTT_PINGRESP:
            LOG(LEVEL_DEBUG, "<LOG> mqtt ping finish\n");
            mqtt.ping_times = 0;
            break;
        default:
            break;
        }
        FreeBuddle(pMsg);
    }

    return 0;
}

int mqtt_process_timeout()
{
    mqtt_package_t *msg = NULL;

    msg = check_flight(mqtt.ping_times);

    // Send err, close mqtt
    if(msg != NULL){
        if(msg->type == MQTT_PUBLISH)
            mqtt.error_number = MQTT_PUBLISH_FAILED;
        else if(msg->type == MQTT_SUBSCRIBE)
            mqtt.error_number = MQTT_SUBSCRIBE_FAILED;
        else
            mqtt.error_number = MQTT_SOCKET_ERROR;

        if(msg->cb != NULL){
            msg->cb(msg->msg_id, mqtt.error_number);
        }

        mqtt.state = MQTT_STATE_ERROR;

        LOG(LEVEL_ERROR, "<ERR> Mqtt timeout\n");
    }

    return 0;
}



int mqtt_process_send()
{
    mqtt_package_t *msg = NULL;
	int ret = -1;
    int len = 0;

    while(1){
        if(GetFlightCount() < FLIGHT_MAX){
            // Send message, if no message to send then wait semaphore
            msg = GetLastSendBuddle();
            if(msg != NULL){
                switch (msg->type){
                case MQTT_PUBLISH:
                    len = mqtt_pack_publish((unsigned char *)mqtt.send_buf, mqtt.window_size, msg);
                    break;
                case MQTT_PUBACK:
                    len = mqtt_pack_puback((unsigned char *)mqtt.send_buf, mqtt.window_size, msg);
                    break;
                case MQTT_PUBREL:
                    len = mqtt_pack_pubrel((unsigned char *)mqtt.send_buf, mqtt.window_size, msg);
                    break;
                case MQTT_PUBCOMP:
                    len = mqtt_pack_pubcomp((unsigned char *)mqtt.send_buf, mqtt.window_size, msg);
                    break;
                case MQTT_SUBSCRIBE:
                    len = mqtt_pack_subscribe((unsigned char *)mqtt.send_buf, mqtt.window_size, msg);
                    break;
                case MQTT_UNSUBSCRIBE:
                    len = mqtt_pack_unsubscribe((unsigned char *)mqtt.send_buf, mqtt.window_size, msg);
                    break;

                default:
                    FreeBuddle(msg);
                    continue;
                }
                if(len <= 0){
                    continue;
                }

                ret = mqtt_send(mqtt.send_buf, len);
                if(ret > 0){
                    // Send PUBLISH succeed, move send to flight
                    msg->tick = vg_get_tick();
                    if((msg->type == MQTT_PUBLISH && msg->qos == MQTT_QOS0)
                        || (msg->type == MQTT_PUBACK)
                        || (msg->type == MQTT_PUBREC)
                        || (msg->type == MQTT_PUBREL)
                        || (msg->type == MQTT_PUBCOMP)){
                        FreeBuddle(msg);
                    }else{
                        MoveSend2Flight(msg);
                    }
                }else{
                    mqtt.error_number = MQTT_SOCKET_ERROR;
                    mqtt.state = MQTT_STATE_ERROR;
                    break;
                }
            }else{
                break;
            }
        }
    }
    return 0;
}

int mqtt_process_error()
{
    LOG(LEVEL_DEBUG, "mqtt_process_error E\n");

    if(mqtt.sockfd >=0) {
        vg_tcp_close(mqtt.sockfd, CONN_MODE);

        //do a short sleep to wait for possible platform socket release latency
        msleep(200);
    }

    mqtt_topic_init();

    ResetBuddle();

    if (mqtt.error_number != MQTT_DISCONNECT_SUCCEED)
        LOG(LEVEL_ERROR, "<ERR> Mqtt error(%d)\n", mqtt.error_number);

    memset(mqtt.host, 0, MAX_HOST_NAME);
    mqtt.port = 0;
    mqtt.sockfd = -1;
    mqtt.ping_times = 0;
    memset(&(mqtt.connect_data), 0, sizeof(mqtt_connect_data_t));

    mqtt.state = MQTT_STATE_IDLE;

    LOG(LEVEL_DEBUG, "mqtt_process_error X\n");

    return mqtt.error_number;
}

int mqtt_process_connack()
{
    LOG(LEVEL_DEBUG, "mqtt_process_connack E\n");
    mqtt_package_t *pMsg = NULL;

    int ret = 0;

    if((pMsg = GetLastPendingBuddle()) != NULL){
        if(pMsg->type == MQTT_CONNACK){
            mqtt.error_number = pMsg->payload[1];

            if(mqtt.error_number == MQTT_CONNECT_SUCCEED){
                mqtt.state = MQTT_STATE_RUNNING;

                if(mqtt.connect_cb != NULL)
                    mqtt.connect_cb(mqtt.error_number);

            }else{

                LOG(LEVEL_NORMAL, "connack with error %d\n", mqtt.error_number);
                mqtt.state = MQTT_STATE_ERROR;
                //mqtt_process_error();
                ret = -1;
            }
        }
    }
    LOG(LEVEL_DEBUG, "mqtt_process_connack X\n");

    return ret;
}
