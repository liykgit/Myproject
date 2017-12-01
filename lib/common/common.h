#ifndef _COMMON_H_SJKDFJSLDJFSDFSDF_
#define _COMMON_H_SJKDFJSLDJFSDFSDF_


#include "WKStack.h"
#include "plat.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifdef LOG_IN_FILE
#define LOG(level, fmt, args...) do{if(level >= LOG_LEVEL){char buf[64];now(buf);fprintf(log_fp,"%s", buf);fprintf(log_fp, fmt, ##args);fflush(log_fp);}}while(0)
#else
#define LOG(level, fmt, args...) do{if(level >= LOG_LEVEL){printf((level==LEVEL_ERROR)?"<E>"fmt:fmt, ##args);}}while(0)
#endif


int isValidMacAddress(const char* mac);

int is_valid_ip(const char *url);

int parse_url(char *ep, char* www, char *port);

unsigned long get_tick(void);

void aes_ecb(char *aes, unsigned char *session, unsigned char *key);

int substr_length(char *src, char *delimiter);

char* vg_strtok(char * string, const char * control);

void vg_print_hex(enum log_level, unsigned char *buf, int len);

void bin2hex(const char* src, int length, char* target);

int hex2bin(const char* src, char* target);

unsigned char cal_crc8(unsigned char *vptr, unsigned char len);

#ifdef __cplusplus
}
#endif

#endif
