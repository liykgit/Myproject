#include <stdio.h>
#include <string.h>

#include "tlv.h"

#define PADDING_SIZE 1 //dont touch this

static TLV_t _tlv;

int tlv_next(char *buf, int len, TLV_t **output)
{
        int offset = 0;
        if(len < (TLV_TAG_SIZE + TLV_LEN_SIZE))
            return -1;
       
        unsigned short vlen;
        memcpy(&vlen, buf+TLV_TAG_SIZE, TLV_LEN_SIZE);
    
        if(vlen > TLV_VALUE_MAX_SIZE ) {
            printf("Error :TLV value len exceeds max\n");
            return -1; 
        }

        memcpy(_tlv.tag, buf, TLV_TAG_SIZE);
        offset += TLV_TAG_SIZE;

        _tlv.len = vlen;
        offset += TLV_LEN_SIZE;
                
        memcpy(_tlv.value, buf + offset, vlen);
        offset += vlen;
                                       
        *output = &_tlv;
        return offset;
}

int tlv_put_string(char *pOut, char* tag,  const char *str, int maxlen) {

    int offset = 0;
    int vlen = strlen(str);
    if(TLV_TAG_SIZE + sizeof(unsigned short) + vlen > maxlen) {
        return -1;
    }
    
    memcpy(pOut, tag, TLV_TAG_SIZE);
    offset += TLV_TAG_SIZE;

    memcpy(pOut + offset, &vlen, TLV_LEN_SIZE);
    offset += TLV_LEN_SIZE;

    strcpy(pOut + offset, str);
    offset += vlen;

    return offset;
}

int tlv_put_int(char *pOut, char* tag, int value, int maxlen) {

    int offset = 0;

    if(TLV_TAG_SIZE + TLV_LEN_SIZE  + sizeof(value) > maxlen) {
        return -1;
    }

    memcpy(pOut, tag, TLV_TAG_SIZE);
    offset += TLV_TAG_SIZE;

    unsigned short sz = sizeof(int);
    memcpy(pOut + offset, &sz, TLV_LEN_SIZE);
    offset += TLV_LEN_SIZE;

    memcpy(pOut + offset, &value, sizeof(int));
    offset += sizeof(int); 
    
    return offset;
}

int tlv_put_bool(char *pOut, char* tag, char value, int maxlen) {

    int offset = 0;

    if(TLV_TAG_SIZE + TLV_LEN_SIZE + sizeof(value) > maxlen) {
        return -1;
    }

    memcpy(pOut, tag, TLV_TAG_SIZE);
    offset += TLV_TAG_SIZE;

    unsigned short sz = sizeof(char);
    memcpy(pOut + offset, &sz, TLV_LEN_SIZE);
    offset += TLV_LEN_SIZE;

    *(char *)(pOut + offset) = value;
    offset += sizeof(value); 

    return offset;
}

int tlv_put_float(char *pOut, char* tag, float value, int maxlen) {
    
    int offset = 0;
    if(TLV_TAG_SIZE + TLV_LEN_SIZE + sizeof(float) > maxlen) {
        return -1;
    }

    memcpy(pOut, (const void *)tag, TLV_TAG_SIZE);
    offset += TLV_TAG_SIZE;

    unsigned short sz = sizeof(float);
    memcpy(pOut + offset, &sz, TLV_LEN_SIZE);
    offset += TLV_LEN_SIZE;

    memcpy(pOut + offset, &value, sizeof(float));
    offset += sizeof(value); 

    return offset;
}



