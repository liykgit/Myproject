#ifndef _TLV_H_SKFJKSDHGSKJDFKLSD_
#define _TLV_H_SKFJKSDHGSKJDFKLSD_

#define TLV_VALUE_MAX_SIZE 512
#define TLV_TAG_SIZE 4
#define TLV_LEN_SIZE (sizeof(unsigned short))

typedef struct tlv {
        unsigned char   tag[TLV_TAG_SIZE];
        unsigned int    len;
        char            value[TLV_VALUE_MAX_SIZE];
} TLV_t;

#ifdef __cplusplus
extern "C" {
#endif

int tlv_next(char *buf, int len, TLV_t **output);

int tlv_put_int(char *pOut, char* tag, int value, int maxlen);
int tlv_put_bool(char *pOut, char* tag, char value, int maxlen);
int tlv_put_float(char *pOut, char* tag, float value, int maxlen);
int tlv_put_string(char *pOut, char* tag,  const char *str, int maxlen);

#ifdef __cplusplus
}
#endif

#endif

