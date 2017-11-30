
#include "common.h"

static int isxdigit(c)
{
        /*
         *   * Depends on ASCII-like character ordering.
         *       */
        return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) ? 1 : 0;
}

int isValidMacAddress(const char* mac) {
    int i = 0;
    int s = 0;

    while (*mac) {
       if (isxdigit(*mac)) {
          i++;
       }
       else {
        s = -1;
       }

/*
       else if (*mac == ':' || *mac == '-') {

          if (i == 0 || i / 2 - 1 != s)
            break;

          ++s;
       }
       else {
           s = -1;
       }
*/

       ++mac;
    }

    return (i == 12 && (s == 5 || s == 0));
}

int is_valid_ip(const char *url)
{
	int section = 0;  //ÿһ�ڵ�ʮ����ֵ
	int dot = 0;       //�������ָ���
	int last = -1;     //ÿһ������һ���ַ�

	if(url == NULL)
		return -1;

	while(*url){
		if(*url == '.'){
			dot++;
			if(dot > 3){
				return 0;
			}
			if(section >= 0 && section <=255){
				section = 0;
			}else{
				return 0;
			}
		}else if(*url >= '0' && *url <= '9'){
			section = section * 10 + *url - '0';
			if(last == '0'){
				return 0;
			}
		}else{
			return 0;
		}
		last = *url;
		url++;
	}

	if(section >= 0 && section <=255){
		if(3 == dot){
			section = 0;
			return 1;
		}
	}
	return 0;
}

int parse_url(char *ep, char* url, char *port)
{
    char *pseptr1;
    char *pseptr2;
    int len;
    int i;
    pseptr2 = ep;
    pseptr1 = strchr(pseptr2, ':');
    if ( NULL == pseptr1 ) {
        return -1;
    }

    len = pseptr1 - pseptr2;

    pseptr1++;
    pseptr2 = pseptr1;
    for ( i = 0; i < 2; i++ ) {
        if ( '/' != *pseptr2 ) {
			LOG(LEVEL_ERROR, "url(pseptr2) failed!\n");
            return -1;
        }
        pseptr2++;
    }
    pseptr1 = strchr(pseptr2, ':');
    if ( NULL == pseptr1 ){
        pseptr1 = strchr(pseptr2, '/');
        if ( NULL == pseptr1 ) {
			LOG(LEVEL_ERROR, "url(pseptr1) failed!\n");
			return -1;
		}
        len = pseptr1 - pseptr2;

		LOG(LEVEL_DEBUG, "host: ");
        for(i=0;i<len;i++)
		    LOG(LEVEL_DEBUG, "%c",pseptr2[i]);
		LOG(LEVEL_DEBUG, "\n");

    }
    else{
		len = pseptr1 - pseptr2;
		memcpy(url, pseptr2, len);
		url[len] = 0;

		pseptr1++;
		strcpy(port, pseptr1);
	}

	return 0;
}



void aes_ecb(char *output, unsigned char *input, unsigned char *key)
{
    extern void AES128_ECB_encrypt(const uint8_t* input, const uint8_t* key, uint8_t *output);
    AES128_ECB_encrypt(input, key, output);
}


/********************************************************
*函数名：vg_strtok(A_CHAR *str, const A_CHAR *token)
*功  能：按token分割字符串
*输  入：A_CHAR *str, const A_CHAR token
*返  回：*part,分割的部分
*此代码为移植代码
*********************************************************/
char* vg_strtok(char * string, const char * control)
{
    unsigned char *str;
    const unsigned char *ctrl = (const unsigned char *)control;
    static unsigned char* _TOKEN = NULL;

    unsigned char map[32];//32*8 = 256 bit
    int count;

    // Clear control map  
    for (count = 0; count < 32; count++)
        map[count] = 0;

    // Set bits in delimiter table  
    do {
        map[*ctrl >> 3] |= (1 << (*ctrl & 7));
    } while (*ctrl++);

    // Initialize str   
    // If string is NULL, set str to the saved pointer   
    //(i.e., continue breaking tokens out of the string from the last strtok call)  
    if (string != NULL)
        str = (unsigned char *)string;
    else
        str = (unsigned char *)_TOKEN;

    // Find beginning of token (skip over leading delimiters). Note that  
    // there is no token iff this loop sets str to point to the terminal  
    // null (*str == '\0')  
    while ( (map[*str >> 3] & (1 << (*str & 7))) && *str )
        str++;

    string = (char *)str;

    // Find the end of the token. If it is not the end of the string,  
    // put a null there.   
    for (; *str; str++)
    {
        if ( map[*str >> 3] & (1 << (*str & 7)) ) 
        {
            *str++ = '\0';
            break;
        }
    }

    // Update nextoken (or the corresponding field in the per-thread data structure  
    _TOKEN = str;

    // Determine if a token has been found.   
    if (string == (char *)str)
        return NULL;
    else
        return string;
}

void vg_print_hex(enum log_level level, unsigned char *buf, int len)
{
	int			i;

	for (i = 0; i < len; i++)
	{
		LOG(level, "%02x ", *buf++);
	}
    LOG(level, "\n");
}

int substr_length(char *src, char *delimiter) {
    
    char *start = src;
    
    char *end = strstr(src, delimiter);    

    if(end) 
        return end-start;
    else
        return strlen(src);
}


unsigned char cal_crc8(unsigned char *vptr, unsigned char len)
{
    const unsigned char *data = vptr;
    unsigned crc = 0;
    int i, j;

    for (j = len; j; j--, data++) {
        crc ^= (*data << 8);
        for (i = 8; i; i--) {
            if (crc & 0x8000)
                crc ^= (0x1070 << 3);
            crc <<= 1;
        }
    }

    return (unsigned char)(crc >> 8);
}



