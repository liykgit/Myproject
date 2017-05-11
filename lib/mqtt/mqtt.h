#ifndef _MQTT_H_SKLJFKSLFJKLSDJFKSDJFKLSD_
#define _MQTT_H_SKLJFKSLFJKLSDJFKSDJFKLSD_

#define MQTT_MAX_TOPIC_NUM 64

/**
* @enum typedef mqtt_qos_t
* @brief A type definition for The quality of service.
*/
typedef enum{
    MQTT_QOS0 = 0, //!< The quality of service is 0
    MQTT_QOS1 = 1, //!< The quality of service is 1
    MQTT_QOS2 = 2, //!< The quality of service is 2
}mqtt_qos_t;

/**
* @enum typedef mqtt_retain_t
* @brief A type definition for The retain flag of PUBLISH.
*/
typedef enum{
    MQTT_RETAIN_FALSE = 0, //!< The retain flag is false
    MQTT_RETAIN_TRUE = 1, //!< The retain flag is true
}mqtt_retain_t;

/**
* @enum typedef mqtt_state_t
* @brief A type definition for The mqtt state.
*/
typedef enum{
    MQTT_STATE_IDLE = 0, //!< Free state
    MQTT_STATE_START = 1, //!< after user call mqtt_start()
    MQTT_STATE_WAIT_CONNACK = 2, //!< send mqtt_connect() already, wait connect ack from server
    MQTT_STATE_STOP = 3, //!< after user call mqtt_stop()
    MQTT_STATE_RUNNING = 4, //!< mqtt running
    MQTT_STATE_ERROR = 5, //!< error
}mqtt_state_t;

/**
* @enum typedef mqtt_errno_t
* @brief A type definition for The mqtt errno.
*/
typedef enum{
    MQTT_CONNECT_SUCCEED = 0, //!< Connection Accepted
    MQTT_VERSION_ERROR = 1, //!< Connection Refused: unacceptable protocol version
    MQTT_CLIENT_ID_ERROR = 2, //!< Connection Refused: identifier rejected
    MQTT_SEVICE_NOT_AVAILABLE = 3, //!< Connection Refused: server unavailable
    MQTT_INVALID_USER = 4, //!< Connection Refused: bad user name or password
    MQTT_UNAUTHORIZED = 5, //!< Connection Refused: not authorized

    MQTT_PUBLISH_FAILED = 6, //!< Publish failed
    MQTT_PUBLISH_SUCCEED = 7, //!< Publish succeed

    MQTT_SUBSCRIBE_MAX_QOS0 = 8, //!< Subscribe succeed: publish this topic max qos is 0
    MQTT_SUBSCRIBE_MAX_QOS1 = 9, //!< Subscribe succeed: publish this topic max qos is 1
    MQTT_SUBSCRIBE_MAX_QOS2 = 10, //!< Subscribe succeed: publish this topic max qos is 2
    MQTT_SUBSCRIBE_FAILED = 11, //!< Subscribe failed

    MQTT_SOCKET_ERROR = 12, //!< Socket error, socket/recv/send error
    MQTT_SYSTEM_ERROR = 13, //!< System error, create socket error
    MQTT_STATION_ERROR = 14, // !< Station error, disconnect from AP
    MQTT_DISCONNECT_SUCCEED = 15, //!< Disconect succeed

    MQTT_UNSUBSCRIBE_FAILED, //!< Unsubscribe failed

}mqtt_errno_t;

/**
* @struct typedef mqtt_lastwill_options_t
* @brief Defines the MQTT "Last Will and Testament" (LWT) settings for the connect packet
*/
typedef struct{
    char struct_id[4]; //!< The eyecatcher for this structure. must be MQTW.
    int struct_version; //!< The version number of this structure. Must be 0
    char *topicName; //!< The LWT topic to which the LWT message will be published.
    char *message; //!< The LWT payload.
    unsigned char retained; //!< The retained flag for the LWT message.
    char qos; //!< QOS for the LWT message.
}mqtt_lastwill_options_t;

/**
* @struct typedef mqtt_connect_data_t
* @brief Defines the MQTT CONNECT data
*/
typedef struct{
    char struct_id[4]; //!< The eyecatcher for this structure. must be MQTC
    int struct_version; //!< The version number of this structure. Must be 0
    unsigned char MQTTVersion; //!< Version of MQTT to be used.  3 = 3.1 4 = 3.1.1
    char *clientID; //!< The Client Identifier, 1~23 bytes, is the uniquely identifies the client to the server
    unsigned char cleansession; //!< clean session flag
    unsigned char willFlag; //!< LWT flag, 1 means this connection have LWT, 0 means doesn't have
    mqtt_lastwill_options_t will; //!< LWT message option, see mqtt_lastwill_options_t for detail
    char *username; //!< The user name identifies the name of the user who is connecting, which can be used for authentication
    char *password; //!< The password corresponding to the user who is connecting, which can be used for authentication
}mqtt_connect_data_t;

// publish result callback and subscribe result callback, message id is the return code when call mqtt_publish() or mqtt_suscribe()
typedef int (*mqtt_cb_t)(unsigned short msg_id, mqtt_errno_t err);
// connect result callback
typedef int (*mqtt_connect_cb_t)(mqtt_errno_t err);
// receive topic callback
typedef int (*mqtt_topic_cb_t)(unsigned char *payload, int len);

/**
* @fn mqtt_state_t mqtt_state(void)
* @brief get the mqtt state.
* @return mqtt state, see mqtt_state_t for detail
*/
mqtt_state_t mqtt_state(void);
/**
* @fn mqtt_errno_t mqtt_errno(void)
* @brief get the mqtt errno.
* @return mqtt errno,
*         if the mqtt state is IDLE or ERROR,
*         you can check errno with this API to found what error have happen.
*         see mqtt_errno_t for detail
*/
mqtt_errno_t mqtt_errno(void);
/**
* @fn int mqtt_init(int keepalive, int window_size)
* @brief Init mqtt client, must do this before all other mqtt function.
* @param keepalive keepalive interval, unit is second, 1~65535 seconds, 0 means the client is not disconnected
* @param window_size tcp recveive and send window size
* @return 0 succeed
          -1 malloc window size buffer failed
*/
int mqtt_init(int keepalive, int window_size);
/**
* @fn void mqtt_init(int keepalive)
* @brief Init mqtt client, must do this before all other mqtt function.
* @param keepalive keepalive interval, unit is second, 1~65535 seconds
* @return 0 stop succeed, -1 mqtt state is not MQTT_STATE_IDLE or MQTT_STATE_ERROR
*/
int mqtt_start(char *host, unsigned short port, mqtt_connect_data_t *data, mqtt_connect_cb_t cb);
/**
* @fn int mqtt_stop(void)
* @brief disconnect the mqtt.
* @return 0 stop succeed, -1 mqtt state is not MQTT_STATE_RUNNING
*/
int mqtt_stop(void);
/**
* @fn int mqtt_publish(char *topic, unsigned char *msg, int msg_len, mqtt_qos_t qos, mqtt_cb_t cb)
* @brief Init mqtt client, must do this before all other mqtt function.
* @param topic publish topic
* @param msg publish message
* @param msg_len publish message length
* @param qos publish qos
* @param cb publish result callback
* @return >0 this publish's message id
*         -1 mqtt send buddle is full
*         -2 mqtt state is not MQTT_STATE_RUNNING
*         -3 mqtt buffer malloc error
*/
int mqtt_publish(char *topic, unsigned char *msg, int msg_len, mqtt_qos_t qos, mqtt_retain_t retain, mqtt_cb_t cb);
/**
* @fn int mqtt_subscribe(char *topic, mqtt_cb_t result_cb, mqtt_topic_cb_t topic_cb)
* @brief Subscribe a topic.
* @param topic subscribe topic, this must not be a temporary variable
* @param result_cb subscribe result callback
* @param topic_cb callback when recieve this topic' message
* @return >0 this subscribe's message id
*         -1 mqtt send buddle is full
*         -2 mqtt state is not MQTT_STATE_RUNNING
*         -3 mqtt topic too much, max topic num is MQTT_MAX_TOPIC_NUM
*/
int mqtt_subscribe(char *topic, mqtt_cb_t result_cb, mqtt_topic_cb_t topic_cb);


int mqtt_unsubscribe(char *topic, mqtt_cb_t result_cb);

#endif
