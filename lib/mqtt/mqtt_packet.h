#ifndef _MQTT_PACKET_H_JKASJFKLSAJFKLSDF_
#define _MQTT_PACKET_H_JKASJFKLSAJFKLSDF_

// Mqtt Packet type
#define MQTT_CONNECT 		1
#define MQTT_CONNACK		2
#define MQTT_PUBLISH		3
#define MQTT_PUBACK			4
#define MQTT_PUBREC			5
#define MQTT_PUBREL			6
#define MQTT_PUBCOMP		7
#define MQTT_SUBSCRIBE		8
#define MQTT_SUBACK			9
#define MQTT_UNSUBSCRIBE	10
#define MQTT_UNSUBACK		11
#define MQTT_PINGREQ		12
#define MQTT_PINGRESP		13
#define MQTT_DISCONNECT		14


typedef union{
    unsigned char byte;
	struct{
        unsigned char retain: 1;	// 0
        unsigned char qos  : 2;		// 2:1
        unsigned char dup  : 1;		// 3
        unsigned char type : 4;		// 7:4
	}bits;
}mqtt_fixed_header_t;

typedef union {
    unsigned char all; /**< all connect flags */

    struct
    {
        unsigned int : 1;              /**< unused */
        unsigned int cleansession : 1; /**< cleansession flag */
        unsigned int will : 1;         /**< will flag */
        unsigned int willQoS : 2;      /**< will QoS value */
        unsigned int willRetain : 1;   /**< will retain setting */
        unsigned int password : 1;     /**< 3.1 password */
        unsigned int username : 1;     /**< 3.1 user name */
    } bits;

}mqtt_connect_flags_t; /**< connect flags byte */

typedef union {
    unsigned char all; /**< all connack flags */

    struct
    {
        unsigned int : 7;                /**< unused */
        unsigned int sessionpresent : 1; /**< session present flag */
    } bits;

}mqtt_connack_flags_t; /**< connack flags byte */

int mqtt_pack_connect(unsigned char *buf, int buflen, mqtt_connect_data_t *options);
int mqtt_pack_publish(unsigned char *buf, int buflen, mqtt_package_t *msg);
int mqtt_pack_puback(unsigned char *buf, int buflen, mqtt_package_t *msg);
int mqtt_pack_pubrel(unsigned char *buf, int buflen, mqtt_package_t *msg);
int mqtt_pack_subscribe(unsigned char *buf, int	buflen, mqtt_package_t *msg);
int mqtt_pack_pingreq(unsigned char *buf, int buflen);
int mqtt_pack_disconnect(unsigned char *buf, int buflen);
int mqtt_unpack(unsigned char *buf, int len);


#endif
