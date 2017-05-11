
#include "common.h"

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
