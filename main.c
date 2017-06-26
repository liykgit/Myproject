#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "WKStack.h"
#include "tlv.h"

#include "common.h"


#define PRODUCTID   "rDLFXB8d"
#define KEY         "yyEheWBmbdHdLMTu"

#define MAC         "A1B2C3D4E916"


int connect_cb(WKStack_state_t state)
{

    printf("connect_cb state %d\n", state);
    return 0;
}



static void datapoint_handler(WKStack_datapoint_t *dps, int size) {

    printf("reporting status ...  \n "); 
    WKStack_report_datapoint(dps, size, 0);
}

   


void WK_Init(char *productId, char *mac, char *key)
{
    static WKStack_params_t params;

    memcpy(params.key, key, strlen(key));

    memcpy(params.product_id, productId, strlen(productId));

    memcpy(params.mac, mac, strlen(mac));

    strcpy(params.version, "0.0.1");

    strcpy(params.sn, "ddssllsffmmrrd");

    WKStack_register_datapoint_handler(datapoint_handler);

    WKStack_init(&params);

}

void WK_Start() {

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

		fscanf(config_fp, "%s %s %s %s", params.mac, params.sn, params.product_id, params.key);

		printf("%s %s %s %s\n", params.mac, params.sn, params.product_id, params.key);
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
    WK_Init(PRODUCTID, MAC, KEY);

    while(1){
        WK_Start();
        msleep(2000);
    }

	return 0;
}
