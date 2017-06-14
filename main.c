#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "WKStack.h"
#include "tlv.h"

#include "common.h"


#define PRODUCTID   "k1fNmNng"
#define KEY         "JkgzNQ4InxXjhp4g"
#define MAC         "A1B2C3D4E916"




int connect_cb(WKStack_state_t state)
{

    printf("connect_cb state %d\n", state);
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

    WKStack_stop(0);
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


    while(1){
        msleep(2000);
        if(WKStack_state() == WKSTACK_ONLINE){
            //printf("Init succeed\n");
            //usr_thread(1);
        }
    }

	return 0;
}
