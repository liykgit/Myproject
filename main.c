#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "tlv.h"
#include "WKStack.h"

#if (_PLATFORM_ == win)
#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#elif (_PLATFORM_ == linux)
#include <sys/select.h>
#include <unistd.h>
#include <netdb.h>
#endif

#include "common.h"

#define MAX_BUFFER_LEN 1024
#define DP_VALUE_MAX_SIZE 64
#define PUBLISH_QUEUE_NUM 6


#define TLV_LENGTH_ERROR -1
#define TAG_DUPLICATE -2
#define TAG0_NOT_FOUND -3
#define SYSTEM_ERROR -4
#define DP_NOT_FOUND -5
#define DP_NUMS_ERROR -6
#define PUBLISH_QUEUE_FULL -7
#define TABLE_NOT_FOUND -8
#define TABLE_EXIST_ALREADY -9
#define CRC_ERROR -10
#define NAME_TOO_LONG -11
#define DEVTYPE_NOT_SET -12
#define DP_PUBULISH_SUCCEED -13
#define DP_PUBULISH_FAILED -14
#define GET_CMD -15
#define SET_STATUS -16
#define DEV_IS_ONLINE -17
#define DEV_IS_OFFLINE -18
#define TYPE_ERROR -19
#define TAG_ERROR -20

static int AT_sock_fd = 0;

char sim_tcp_ip[32];
int sim_tcp_port;

int publish_cb(unsigned short id, WKStack_publish_state_t state)
{
#if 0
    unsigned char tmp[5];
    unsigned int i, j;
    tmp[0] = 'W';
    tmp[1] = 'K';
    tmp[2] = ':';

    if(state == 0){
        /* WK:<group index>,0x01 */
        tmp[4] = 1;
    }else{
        /* WK:<group index>,0x00 */
        tmp[4] = 0;
    }

	for (i = 0; i < PUBLISH_QUEUE_NUM; i++) {
		if (dp_group_size[i] != 0 && memcmp(((WK_Package *)msg)->pTDP, dp_group[i], dp_group_size[i]) == 0){
			printf("dp_publish_index:%d\n", i);
			tmp[3] = (unsigned char)i;

			if (dp_group[i] != NULL) {
				j = 0;
				while (dp_group[i][j] != 0) {
					if (dp_group[i][j]->type == WKSTACK_TYPE_STRING) {
						free(((WKStack_string_t *)dp_group[i][j])->string);
					}
					else if (dp_group[i][j]->type == WKSTACK_TYPE_RAW) {
						free(((WKStack_raw_t *)dp_group[i][j])->raw);
					}
					j++;
				}

				free(dp_group[i]);
				dp_group[i] = NULL;
				dp_group_size[i] = 0;

			}
			break;
		}
	}
	tcp_send(AT_sock_fd, tmp, 5, 0);
#endif
	return 0;
}

int connect_cb(WKStack_state_t state)
{
/*
    unsigned char tmp[5];

    tmp[0] = 'W';
    tmp[1] = 'K';
    tmp[2] = ':';

    tmp[3] = 0xFF;
    if(state == WKSTACK_ONLINE){
        // Device Online, send WK:0xFF 0x01 
        tmp[4] = 0x01;
    }else{
        // Device Offline, send WK:0xFF 0x00 
        tmp[4] = 0x00;
    }

    tcp_send(AT_sock_fd, tmp, 5, 0);
*/
    printf("connect_cb state %d\n", state);
    return 0;
}

int set_cb(void *datapoint)
{
	#if 0
	int offset = 0;
	int i;
	unsigned char tmp[DP_VALUE_MAX_SIZE + 8];

	/* Send data below to user MCU */
    /* WK:0xff,0x05 <index>,<value len>,<value> */
	tmp[offset++] = 'W';
	tmp[offset++] = 'K';
	tmp[offset++] = ':';

	tmp[offset++] = 0xFF;
	tmp[offset++] = 0x05;

	for (i = 0; i < DATAPOINT_NUM; i++) {
		if (strcmp(dp->name, dpList[i]->name) == 0) {
			tmp[offset++] = i + 1;
			break;
		}
	}

    switch(dp->type){
        case WKSTACK_TYPE_FLOAT:
			tmp[offset++] = sizeof(dp->Float);
			memcpy(tmp + offset, (unsigned char *)(&dp->Float), sizeof(dp->Float));
			offset += sizeof(dp->Float);
            break;
        case WKSTACK_TYPE_STRING:
			tmp[offset++] = dp->len;
			memcpy(tmp + offset, dp->String, dp->len);
			offset += dp->len;
            break;
        case WKSTACK_TYPE_RAW:
			tmp[offset++] = dp->len;
			memcpy(tmp + offset, dp->Raw, dp->len);
			offset += dp->len;
            break;
        case WKSTACK_TYPE_BOOL:
			tmp[offset++] = sizeof(dp->Bool);
			memcpy(tmp + offset, (unsigned char *)(&(dp->Bool)), sizeof(dp->Bool));
			offset += sizeof(dp->Bool);
            break;
        case WKSTACK_TYPE_INT:
			tmp[offset++] = sizeof(dp->Int);
			memcpy(tmp + offset, (unsigned char *)(&(dp->Int)), sizeof(dp->Int));
			offset += sizeof(dp->Int);
            break;
        case WKSTACK_TYPE_LONG:
			tmp[offset++] = sizeof(dp->Long);
			memcpy(tmp + offset, (unsigned char *)(&(dp->Long)), sizeof(dp->Long));
			offset += sizeof(dp->Long);
            break;
		default:
			break;
    }

	tcp_send(AT_sock_fd, tmp, offset, 0);
#endif
	return 0;
}

int cmd_cb(void *command)
{
	#if 0
	int offset = 0;
	int i;
	static unsigned char tmp[MAX_ARGS_LEN+8];

	tmp[offset++] = 'W';
	tmp[offset++] = 'K';
	tmp[offset++] = ':';

	tmp[offset++] = 0xFF;
	tmp[offset++] = 0x04;

	for (i = 0; i < CMD_NUM; i++) {
		if (strcmp(((WK_CMDInfo *)cmd)->cmd, cmdList[i]->cmd) == 0) {
			tmp[offset++] = DATAPOINT_NUM + i + 1;
			break;
		}
	}

	tmp[offset++] = strlen(((WK_CMDInfo *)cmd)->args);

	memcpy(tmp + offset, ((WK_CMDInfo *)cmd)->args, strlen(((WK_CMDInfo *)cmd)->args));
	offset += strlen(((WK_CMDInfo *)cmd)->args);

	tcp_send(AT_sock_fd, tmp, offset, 0);
#endif
	return 0;
}

static WKStack_params_t params;

static void datapoint_handler(WKStack_datapoint_t *dps, int size) {

    printf("reporting status ...  \n "); 
    WKStack_report_datapoint(dps, size, 0);
}

   

void WK_Start(char *productId, char *mac, char *key)
{

    memcpy(params.devtype, productId, strlen(productId));
    memcpy(params.key, key, strlen(key));
    memcpy(params.mac, mac, strlen(mac));

    WKStack_register_datapoint_handler(datapoint_handler);

    WKStack_init(&params);

    WKStack_start(connect_cb, NULL);
}

void WK_Stop() {

    WKStack_stop();
}

//----------------------- udp ---------------------------------------



int main(int argc, char **argv)
{
/*
	FILE *config_fp = NULL;
	FILE *tcp_config_fp = NULL;

	if(argv[1] == NULL){
		printf("usage:\nWKdevice [device number]\n");
		getchar();
		return -1;
	}

	if (argv[1] != NULL) {
		config_fp = fopen("devices.config", "r");
		if (config_fp == NULL) {
			printf("Open config file error\n");
			while (1) { msleep(1000); }
			return 1;
		}
		fseek(config_fp, 74 * (atoi((const char *)argv[1]) - 1), SEEK_SET);

		fscanf(config_fp, "%s %s %s %s", params.mac, params.sn, params.devtype, params.key);

		printf("%s %s %s %s\n", params.mac, params.sn, params.devtype, params.key);
	}
*/
    #define PRODUCTID "k1fNmNng"
    #define KEY     "JkgzNQ4InxXjhp4g"
    #define MAC "845DD74A6C5C"

/*
	tcp_config_fp = fopen("Ctrlserver.config", "r");
	if (tcp_config_fp == NULL) {
		printf("Open QTPserver config file error\n");
		while (1) { msleep(1000); }
		return 1;
	}

	fscanf(tcp_config_fp, "%s %d", sim_tcp_ip, &sim_tcp_port);
	printf("%s %d\n", sim_tcp_ip, sim_tcp_port);
*/
    WK_Start(PRODUCTID, MAC, KEY);

    #define UDP_SERVER_PORT 30319
    UDPServer_start(UDP_SERVER_PORT); 


    while(1){
        msleep(2000);
        if(WKStack_state() == WKSTACK_ONLINE){
            //printf("Init succeed\n");
            //usr_thread(1);
        }
    }

	return 0;
}
