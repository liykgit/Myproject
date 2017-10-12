#include <string.h>
#include "common.h"

#include "mqtt_buddle.h"
#include "mqtt_packet.h"

extern vg_sem_t ctrl_thread_sem;

int mqtt_pack_connect(unsigned char *buf, int buflen, mqtt_connect_data_t *options)
{
    unsigned char *ptr = buf;
	mqtt_fixed_header_t header = {0};
	mqtt_connect_flags_t flags = {0};
	int	len = 0;
	int rc = -1;

	if (MQTTPacket_len(len = MQTTSerialize_connectLength(options)) > buflen){
		rc = -1;
		goto exit;
	}

	header.byte	= 0;
	header.bits.type = MQTT_CONNECT;
	writeChar(&ptr, header.byte); /* write header */

	ptr	+= mqtt_set_remain_length(ptr, len); /* write remaining length */

	if (options->MQTTVersion == 4){
		writeCString(&ptr, "MQTT");
		writeChar(&ptr, (char) 4);
	}else{
		writeCString(&ptr, "MQIsdp");
		writeChar(&ptr, (char) 3);
	}

	flags.all = 0;
	flags.bits.cleansession = options->cleansession;
	flags.bits.will = (options->willFlag) ? 1 : 0;
	if (flags.bits.will){
		flags.bits.willQoS = options->will.qos;
		flags.bits.willRetain = options->will.retained;
	}

	if (options->username )
		flags.bits.username	= 1;
	if (options->password )
		flags.bits.password	= 1;

	writeChar(&ptr, flags.all);
	writeInt(&ptr, mqtt.keepalive/1000);
	writeMQTTString(&ptr, options->clientID);
	if (options->willFlag){
		writeMQTTString(&ptr, options->will.topicName);
		writeMQTTString(&ptr, options->will.message);
	}
	if (flags.bits.username)
		writeMQTTString(&ptr, options->username);
	if (flags.bits.password)
		writeMQTTString(&ptr, options->password);

	rc = ptr - buf;

exit:
	return rc;
}

int mqtt_pack_publish(unsigned char *buf, int buflen, mqtt_package_t *msg)
{
    unsigned char *ptr	= buf;
	mqtt_fixed_header_t header	= {0};
	int	rem_len = 0;
	int rc = 0;

	if (MQTTPacket_len(rem_len = MQTTSerialize_publishLength(msg->qos, msg->topic, msg->length)) > buflen){
		rc = -1;
		goto exit;
	}

	header.bits.type	= MQTT_PUBLISH;
	header.bits.dup		= msg->dup;
	header.bits.qos		= msg->qos;
	header.bits.retain	= msg->retain;
	writeChar(&ptr, header.byte); /* write header */

	ptr += mqtt_set_remain_length(ptr, rem_len); /* write remaining length */;

	writeMQTTString(&ptr, msg->topic);


	if (header.bits.qos > 0)
		writeInt(&ptr, msg->msg_id);

	memcpy(ptr, msg->payload, msg->length);
	ptr += msg->length;

	rc = ptr - buf;

exit:
	return rc;
}

int mqtt_pack_puback(unsigned char *buf, int buflen, mqtt_package_t *msg)
{
    mqtt_fixed_header_t header = {0};
	int rc = 0;
	unsigned char *ptr = buf;

	if (buflen < 4){
		// buf is too short
		rc = -1;
		goto exit;
	}
	header.bits.type	= MQTT_PUBACK;
	header.bits.dup		= 0;
	header.bits.qos		= 0;
	writeChar(&ptr, header.byte); 		/* write header */

	ptr	+= mqtt_set_remain_length(ptr, 2); 		/* write remaining length */

	writeInt(&ptr, msg->msg_id);	/* write packet ID */
	rc = ptr - buf;

exit:
	return rc;
}
int mqtt_pack_pubrel(unsigned char *buf, int buflen, mqtt_package_t *msg)
{
    mqtt_fixed_header_t header = {0};
	int rc = 0;
	unsigned char *ptr = buf;

	if (buflen < 4){
		// buf is too short
		rc = -1;
		goto exit;
	}
	header.bits.type	= MQTT_PUBREL;
	header.bits.dup		= 0;
	header.bits.qos		= 1;
	writeChar(&ptr, header.byte); 		/* write header */

	ptr	+= mqtt_set_remain_length(ptr, 2); 		/* write remaining length */

	writeInt(&ptr, msg->msg_id);	/* write packet ID */
	rc = ptr - buf;

exit:
	return rc;
}
int mqtt_pack_pubrec(unsigned char *buf, int buflen, mqtt_package_t *msg)
{
	mqtt_fixed_header_t header = {0};
	int rc = 0;
	unsigned char *ptr = buf;

	if (buflen < 4){
		// buf is too short
		rc = -1;
		goto exit;
	}
	header.bits.type	= MQTT_PUBREC;
	header.bits.dup		= 0;
	header.bits.qos		= 0;
	writeChar(&ptr, header.byte); 		/* write header */

	ptr	+= mqtt_set_remain_length(ptr, 2); 		/* write remaining length */

	writeInt(&ptr, msg->msg_id);	/* write packet ID */
	rc = ptr - buf;

exit:
	return rc;
}
int mqtt_pack_pubcomp(unsigned char *buf, int buflen, mqtt_package_t *msg)
{
	mqtt_fixed_header_t header = {0};
	int rc = 0;
	unsigned char *ptr = buf;

	if (buflen < 4){
		// buf is too short
		rc = -1;
		goto exit;
	}
	header.bits.type	= MQTT_PUBCOMP;
	header.bits.dup		= 0;
	header.bits.qos		= 0;
	writeChar(&ptr, header.byte); 		/* write header */

	ptr	+= mqtt_set_remain_length(ptr, 2); 		/* write remaining length */

	writeInt(&ptr, msg->msg_id);	/* write packet ID */
	rc = ptr - buf;

exit:
	return rc;
}

int mqtt_pack_subscribe(unsigned char *buf, int	buflen, mqtt_package_t *msg)
{
    unsigned char *ptr = buf;
	mqtt_fixed_header_t header = {0};
	int rem_len	= 0;
	int rc = 0;

	if (MQTTPacket_len(rem_len = MQTTSerialize_subscribeLength(msg->topic)) > buflen){
		rc = -2;
		goto exit;
	}

	header.byte			= 0;
	header.bits.type	= MQTT_SUBSCRIBE;
	header.bits.dup		= msg->dup;
	header.bits.qos		= msg->qos;
	writeChar(&ptr, header.byte);				// write header

	ptr	+= mqtt_set_remain_length(ptr, rem_len);		// write remaining length

	writeInt(&ptr, msg->msg_id);

	writeMQTTString(&ptr, msg->topic);
    writeChar(&ptr, header.bits.qos);

	rc = ptr - buf;

exit:
	return rc;
}

int mqtt_pack_unsubscribe(unsigned char *buf, int buflen, mqtt_package_t *msg)
{
    unsigned char *ptr = buf;
	mqtt_fixed_header_t header = {0};
	int rem_len	= 0;
	int rc = 0;

	if (MQTTPacket_len(rem_len = MQTTSerialize_unsubscribeLength(msg->topic)) > buflen){
		rc = -2;
		goto exit;
	}

	header.byte			= 0;
	header.bits.type	= MQTT_UNSUBSCRIBE;
	header.bits.dup		= 0;
	header.bits.qos		= 1;

	writeChar(&ptr, header.byte);				// write header

	ptr	+= mqtt_set_remain_length(ptr, rem_len);		// write remaining length

	writeInt(&ptr, msg->msg_id);

	writeMQTTString(&ptr, msg->topic);
	rc = ptr - buf;
    
exit:
	return rc;
}

int mqtt_pack_pingreq(unsigned char *buf, int buflen)
{
    mqtt_fixed_header_t	header = {0};
	int rc = -1;
	unsigned char *ptr = buf;

	if(buflen < 2) {
		return -1;
	}

	header.byte			= 0;
	header.bits.type	= MQTT_PINGREQ;
	writeChar(&ptr, header.byte);			// write header

	ptr += mqtt_set_remain_length(ptr, 0); // write remaining length
	rc = ptr - buf;

	return rc;
}

int mqtt_pack_disconnect(unsigned char *buf, int buflen)
{
    mqtt_fixed_header_t	header = {0};
	int rc = -1;
	unsigned char *ptr = buf;

	if(buflen < 2) {
		return -1;
	}

	header.byte			= 0;
	header.bits.type	= MQTT_DISCONNECT;
	writeChar(&ptr, header.byte);			// write header

	ptr += mqtt_set_remain_length(ptr, 0); // write remaining length
	rc = ptr - buf;

	return rc;
}



int mqtt_unpack(unsigned char *buf, int len)
{
    unsigned char *p = NULL;
    int length_feild_size = 0;
    mqtt_package_t *pmsg = NULL;
    mqtt_fixed_header_t header;
    int msg_len = 0;
	int topic_len = 0;
	int flag = 0;

another:
	p = buf;
    header.byte = *p++;

	if(header.bits.type == 0x0 || header.bits.type == 0xF)
		return -6;

    msg_len = mqtt_get_remain_length(&p);
    length_feild_size = p - buf - 1;

	#if 0
    if(msg_len > MAX_BUFFER_LEN || msg_len + length_feild_size + 1 != len){
        LOG(LEVEL_ERROR, "<ERR> mqtt message length error\n");
        return -2;
    }
	#endif
	if(msg_len + length_feild_size + 1 < len){
		flag = 1;
	}else if(msg_len + length_feild_size + 1 > len){
		LOG(LEVEL_ERROR, "message length error\n");
        return -2;
	}

	if(msg_len > mqtt.window_size){
        LOG(LEVEL_ERROR, "message length error2\n");
        return -2;
    }
    pmsg = GetFree2Pending();
    if (pmsg == 0){
        LOG(LEVEL_ERROR, "pending buddle not free\n");
        return -1;
    }

    pmsg->type = header.bits.type;
    pmsg->qos = header.bits.qos;
    pmsg->tick = vg_get_tick();

	pmsg->length = msg_len;

    if(pmsg->type == MQTT_PUBLISH){
        topic_len = (((unsigned short)p[0]) << 8) + p[1];
        p += 2;
        if(topic_len + 1 < MAX_TOPIC_LEN){
			memset(pmsg->topic, 0, MAX_TOPIC_LEN);
            memcpy(pmsg->topic, p, topic_len);
            p += topic_len;
        }else{
            FreeBuddle(pmsg);
            LOG(LEVEL_ERROR, "topic too long\n");
            return -3;
        }

		pmsg->length = msg_len - topic_len - 2;
    }

	LOG(LEVEL_DEBUG, "<LOG>msg len: %x, topic len: %x\n", msg_len, topic_len);

    if(pmsg->length > 0){
		pmsg->payload = (unsigned char *)vg_malloc(pmsg->length);
		if(pmsg->payload == NULL){
			FreeBuddle(pmsg);
			LOG(LEVEL_ERROR, "OOM!!!\n");
			return -4;
		}
        memcpy(pmsg->payload, p, pmsg->length);
        
        
        LOG(LEVEL_DEBUG, "mqtt received : \n");
        vg_print_hex(LEVEL_DEBUG, pmsg->payload, pmsg->length);  

    }else if(pmsg->length < 0){
		FreeBuddle(pmsg);
        LOG(LEVEL_ERROR, "payload too long\n");
        return -5;
	}

    pmsg->used = BUDDLE_USED;

	vg_release_sem(&ctrl_thread_sem);

	// another packet
	if(flag == 1){
		flag = 0;

		buf += msg_len + length_feild_size + 1;
		len -= msg_len + length_feild_size + 1;
		goto another;
	}
    return 0;
}
