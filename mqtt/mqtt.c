#include "string.h"
#include "common.h"

#include "mqtt.h"
#include "mqtt_lib.h"
#include "mqtt_process.h"
#include "mqtt_buddle.h"
#include "mqtt_packet.h"

mqtt_state_t mqtt_state()
{
    return mqtt.state;
}

mqtt_errno_t mqtt_errno(void)
{
    return mqtt.error_number;
}

int mqtt_start(char *host, unsigned short port, mqtt_connect_data_t *data, mqtt_connect_cb_t cb)
{
    if(mqtt.state == MQTT_STATE_IDLE || mqtt.state == MQTT_STATE_ERROR){
        strcpy(mqtt.host, host);
        mqtt.port = port;

        memcpy(&(mqtt.connect_data), data, sizeof(mqtt_connect_data_t));

        mqtt.connect_cb = cb;
        mqtt.state = MQTT_STATE_START;

        release_sem(&ctrl_thread_sem);
    }else{
        return -1;
    }

    return 0;
}

int mqtt_stop(void)
{
    if(mqtt.state == MQTT_STATE_RUNNING){
        mqtt.state = MQTT_STATE_STOP;

        release_sem(&ctrl_thread_sem);
    }else{
        return -1;
    }

    return 0;
}

int mqtt_publish(char *topic, unsigned char *msg, int msg_len, mqtt_qos_t qos, mqtt_retain_t retain, mqtt_cb_t cb)
{
    printf("mqtt_publish E\n");
    mqtt_package_t *package;

    if (mqtt.state != MQTT_STATE_RUNNING){
        LOG(LEVEL_ERROR, "<ERR> mqtt cann't connect to cloudy\n");
        return -2;
    }

    package = GetFree2Send();
    if(package == NULL) {
        LOG(LEVEL_ERROR, "<ERR> mqtt send buddle not free\n");
		return -1;
	}

    package->payload = (unsigned char *)mem_alloc(msg_len);
    if(package->payload == NULL){
        LOG(LEVEL_ERROR, "<ERR> mqtt malloc error\n");
        FreeBuddle(package);
        return -3;
    }

    package->msg_id = mqtt_msgid();
    strcpy(package->topic, topic);
    package->retain = retain;
    package->qos = qos;
    package->type = MQTT_PUBLISH;
    package->cb = cb;
    memcpy(package->payload, msg, msg_len);
    package->length = msg_len;
    package->used = BUDDLE_USED;


    release_sem(&ctrl_thread_sem);

    return (int)(package->msg_id);
}

int mqtt_subscribe(char *topic, mqtt_cb_t result_cb, mqtt_topic_cb_t topic_cb)
{
    mqtt_package_t *msg;
    int ret = 0;

    if (mqtt.state != MQTT_STATE_RUNNING){
        LOG(LEVEL_ERROR, "<ERR> mqtt cann't connect to cloudy\n");
        return -2;
    }

	msg	= GetFree2Send();
	if(msg == NULL) {
        LOG(LEVEL_ERROR, "<ERR> mqtt send buddle not free\n");
		return -1;
	}

    msg->msg_id = mqtt_msgid();
    msg->qos = 1;
	msg->type = MQTT_SUBSCRIBE;
	strcpy(msg->topic, topic);
	msg->cb = result_cb;
	msg->used = BUDDLE_USED;

    ret = mqtt_register_topic(topic, topic_cb);
    if(ret != 0){
        LOG(LEVEL_ERROR, "<ERR> mqtt topic too much\n");
        FreeBuddle(msg);
        return -3;
    }

    release_sem(&ctrl_thread_sem);

	return (int)(msg->msg_id);
}

int mqtt_unsubscribe(char *topic, mqtt_cb_t result_cb)
{

    LOG(LEVEL_DEBUG, "<LOG> Mqtt unsubscribe %s\n", topic);
    mqtt_package_t *msg;
    int ret = 0;

    if (mqtt.state != MQTT_STATE_RUNNING){
        LOG(LEVEL_ERROR, "<ERR> mqtt cann't connect to cloudy\n");
        return -2;
    }

	msg	= GetFree2Send();
	if(msg == NULL) {
        LOG(LEVEL_ERROR, "<ERR> mqtt send buddle not free\n");
		return -1;
	}

    msg->msg_id = mqtt_msgid();
    msg->qos = 1;
	msg->type = MQTT_UNSUBSCRIBE;
	strcpy(msg->topic, topic);
	msg->cb = result_cb;
	msg->used = BUDDLE_USED;

    ret = mqtt_unregister_topic(topic);
    if(ret != 0){
        LOG(LEVEL_ERROR, "<ERR> topic %s not found\n", topic);
        FreeBuddle(msg);
        return -3;
    }

    release_sem(&ctrl_thread_sem);

	return (int)(msg->msg_id);
}

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

int mqtt_puback(unsigned short msg_id)
{
    mqtt_package_t *pmsg;

    if (mqtt.state != MQTT_STATE_RUNNING){
        LOG(LEVEL_ERROR, "<ERR> mqtt cann't connect to cloudy\n");
        return -2;
    }

    pmsg = GetFree2Send();
    if (pmsg == 0){
        LOG(LEVEL_ERROR, "<ERR> mqtt send buddle not free\n");
        return -1;
    }

	pmsg->tick = get_tick();
	pmsg->type = MQTT_PUBACK;
	pmsg->cb = NULL;
	pmsg->times = 1;
    pmsg->msg_id = msg_id;
	pmsg->used = BUDDLE_USED;

    release_sem(&ctrl_thread_sem);

    return 0;
}
int mqtt_pubrec(unsigned short msg_id)
{
    mqtt_package_t *pmsg;

    if (mqtt.state != MQTT_STATE_RUNNING){
        LOG(LEVEL_ERROR, "<ERR> mqtt cann't connect to cloudy\n");
        return -2;
    }

    pmsg = GetFree2Send();
    if (pmsg == 0){
        LOG(LEVEL_ERROR, "<ERR> mqtt send buddle not free\n");
        return -1;
    }

	pmsg->tick = get_tick();
	pmsg->type = MQTT_PUBREC;
	pmsg->cb = NULL;
	pmsg->times = 1;
    pmsg->msg_id = msg_id;
	pmsg->used = BUDDLE_USED;

    release_sem(&ctrl_thread_sem);

    return 0;
}
int mqtt_pubrel(unsigned short msg_id)
{
    mqtt_package_t *pmsg;

    if (mqtt.state != MQTT_STATE_RUNNING){
        LOG(LEVEL_ERROR, "<ERR> mqtt cann't connect to cloudy\n");
        return -2;
    }

    pmsg = GetFree2Send();
    if (pmsg == 0){
        LOG(LEVEL_ERROR, "<ERR> mqtt send buddle not free\n");
        return -1;
    }

	pmsg->tick = get_tick();
	pmsg->type = MQTT_PUBREL;
	pmsg->cb = NULL;
	pmsg->times = 1;
    pmsg->msg_id = msg_id;
	pmsg->used = BUDDLE_USED;

    release_sem(&ctrl_thread_sem);

    return 0;
}
int mqtt_pubcomp(unsigned short msg_id)
{
    mqtt_package_t *pmsg;

    if (mqtt.state != MQTT_STATE_RUNNING){
        LOG(LEVEL_ERROR, "<ERR> mqtt cann't connect to cloudy\n");
        return -2;
    }

    pmsg = GetFree2Send();
    if (pmsg == 0){
        LOG(LEVEL_ERROR, "<ERR> mqtt send buddle not free\n");
        return -1;
    }

	pmsg->tick = get_tick();
	pmsg->type = MQTT_PUBCOMP;
	pmsg->cb = NULL;
	pmsg->times = 1;
    pmsg->msg_id = msg_id;
	pmsg->used = BUDDLE_USED;

    release_sem(&ctrl_thread_sem);

    return 0;
}

static int mqtt_connect()
{
    int len = 0;
    int ret = 0;
    int flag = 0;

    LOG(LEVEL_NORMAL, "<LOG> mqtt_connect E\n");
    ret = mqtt_socket(mqtt.host, mqtt.port);
    if(ret < 0){
        LOG(LEVEL_ERROR, "<ERR> Mqtt connect server failed\n");
        if(ret == -1)
            flag = 1;
        goto err;
    }

    memset(mqtt.send_buf, 0, mqtt.window_size);

    len = mqtt_pack_connect(mqtt.send_buf, mqtt.window_size, &(mqtt.connect_data));

    ret = mqtt_send(mqtt.send_buf, len);
    if(ret > 0){
        mqtt.state = MQTT_STATE_WAIT_CONNACK;
        release_sem(&recv_thread_sem);
    }else{
        LOG(LEVEL_ERROR, "<ERR> Mqtt send connect failed\n");
        goto err;
    }

    return 0;
err:
    if(flag == 1)
        mqtt.error_number = MQTT_SYSTEM_ERROR;
    else
        mqtt.error_number = MQTT_SOCKET_ERROR;

    mqtt.state = MQTT_STATE_ERROR;
	release_sem(&ctrl_thread_sem);

    return -1;
}

static int mqtt_disconnect()
{
    int len = 0;
    int ret = 0;

    memset(mqtt.send_buf, 0, mqtt.window_size);

    len = mqtt_pack_disconnect(mqtt.send_buf, mqtt.window_size);

    ret = mqtt_send(mqtt.send_buf, len);
    if(ret > 0){
        mqtt.error_number = MQTT_DISCONNECT_SUCCEED;
        mqtt.state = MQTT_STATE_ERROR; // Not real error, just want to do the clean thing
    }else{
        mqtt.error_number = MQTT_SOCKET_ERROR;
        mqtt.state = MQTT_STATE_ERROR;
    }

	release_sem(&ctrl_thread_sem);
    return 0;
}

static thread_ret_t mqtt_ctrl_thread(thread_params_t args)
{
    int ret = 0;

    while(1){
        LOG(LEVEL_NORMAL, "<LOG> Ctrl thread wait...(%d)\n", mqtt.keepalive);
        ret = wait_sem(&ctrl_thread_sem, mqtt.keepalive);

        if(ret == -1){ // Timeout
            if(mqtt.ping_times > MQTT_RETRY_TIMES){
                mqtt.state = MQTT_STATE_ERROR;

                LOG(LEVEL_ERROR, "<ERR> Mqtt ping timeout\n");
            }else if(mqtt.state == MQTT_STATE_RUNNING){
                mqtt_pingreq();
                continue;
            }
        }

        LOG(LEVEL_NORMAL, "<LOG> Ctrl thread process(%d)...\n", mqtt.state);
        switch(mqtt.state){
        case MQTT_STATE_IDLE:
            /* do nothing */
            break;
        case MQTT_STATE_START:
            mqtt_connect();
            break;
        case MQTT_STATE_STOP:
            // Process recv and send task first
            mqtt_process_pend();
            mqtt_process_send();
            mqtt_disconnect();
            break;
        case MQTT_STATE_WAIT_CONNACK:
            mqtt_process_connack();
            break;
        case MQTT_STATE_RUNNING:
            mqtt_process_pend();
            mqtt_process_timeout();
            
            mqtt_process_send();
            break;
        case MQTT_STATE_ERROR:
            mqtt_process_error();
            break;
        }
    }

    return (thread_ret_t)0;
}

static thread_ret_t mqtt_recv_thread(thread_params_t args)
{
    int len = 0;
start:
    // Wait for connect completed
    LOG(LEVEL_NORMAL, "<LOG> Recv thread IDLE...\n");
    wait_sem(&recv_thread_sem, -1);

    LOG(LEVEL_NORMAL, "<LOG> Recv thread process...\n");

    memset(mqtt.recv_buf, 0, mqtt.window_size);
    while(1){
        if(mqtt.state == MQTT_STATE_IDLE
        || mqtt.state == MQTT_STATE_ERROR
        || mqtt.state == MQTT_STATE_STOP
        || mqtt.state == MQTT_STATE_START)
            goto start;

        // Select in recv
        len = 0;

        LOG(LEVEL_NORMAL, "<LOG> Recv thread wait in recv...\n");
        len = mqtt_recv(mqtt.recv_buf, mqtt.window_size);
        if(len < 0) {
            // If there is not next connect task, then go to error, else goto recv directory
            if (mqtt.state != MQTT_STATE_IDLE
            && mqtt.state != MQTT_STATE_ERROR
            && mqtt.state != MQTT_STATE_START
            && mqtt.state != MQTT_STATE_WAIT_CONNACK){
                mqtt.error_number = MQTT_SOCKET_ERROR;
                mqtt.state = MQTT_STATE_ERROR;

                release_sem(&ctrl_thread_sem);
            }
		}else if(len < 2){
  			//LOG(LEVEL_NORMAL, "<LOG> read buf length less than 2bytes(%d)\n", len);
			continue;
		}else{
            mqtt_unpack(mqtt.recv_buf, len);
            memset(mqtt.recv_buf, 0, mqtt.window_size);
        }
    }

    return (thread_ret_t)0;
}
// keepalive max 65535s(18h 12m 15s)
// min 1s
// window size: byte
int mqtt_init(int keepalive, int window_size)
{
    mqtt.window_size = window_size;

    mqtt.send_buf = (char *)mem_alloc(mqtt.window_size);
    if(mqtt.send_buf == NULL){
        return -1;
    }
    mqtt.recv_buf = (char *)mem_alloc(mqtt.window_size);
    if(mqtt.recv_buf == NULL){
        mem_free(mqtt.send_buf);
        return -1;
    }

    //tick_init();

    mqtt_buddle_init();

    mqtt_topic_init();

    if(keepalive > 0 && keepalive < 65535)
        mqtt.keepalive = keepalive * 1000;
    else
        mqtt.keepalive = 10000; // default is 10s

    mqtt.state = MQTT_STATE_IDLE;

    create_sem(&ctrl_thread_sem, "ctrl_thread");
    create_sem(&recv_thread_sem, "recv_thread");

    start_thread(mqtt_ctrl_thread, NULL, 4096 + 1024);
	start_thread(mqtt_recv_thread, NULL, 4096);

    return 0;
}
