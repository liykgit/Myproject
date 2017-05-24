#ifndef _COMMON_H_SJKDFJSLDJFSDFSDF_
#define _COMMON_H_SJKDFJSLDJFSDFSDF_

#include "plat.h"

int is_valid_ip(const char *url);

int parse_url(char *ep, char* www, char *port);

unsigned long get_tick(void);

void aes_ecb(char *aes, unsigned char *session, unsigned char *key);

char* vg_strtok(char * string, const char * control);

void vg_print_hex(unsigned char *buf, int len);

#endif
