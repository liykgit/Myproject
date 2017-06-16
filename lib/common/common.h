#ifndef _COMMON_H_SJKDFJSLDJFSDFSDF_
#define _COMMON_H_SJKDFJSLDJFSDFSDF_

#include "plat.h"

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

#endif
