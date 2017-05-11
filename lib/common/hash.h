#ifndef _HASH_H_
#define _HASH_H_

#define BDKHASH

unsigned int Calc_binhash(unsigned char *buf, int len);
unsigned int Calc_hash(char *str);


#ifdef BDKHASH
#define HASH_IN_DP		0x41e9a216
#define HASH_OUT_DP		0x23b8b661

#define HASH_IN_TEST		0x78c4a860
#define HASH_OUT_TEST		0x1b5c0003

// change only for debug !!
//#define HASH_IN_DP		0x23b8b661
//#define HASH_OUT_DP		0x23b8b661

#define IBM_MQTT_ONLINE			0x0306fe05
#define IBM_MQTT_ONLINE_PUBLISH 0x594f4901
#endif

#ifdef APHASH

#endif

#endif
