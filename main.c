#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "WKStack.h"
#include "tlv.h"

#include "common.h"




#define PRODUCTID   "E3GF3s6feqLDznr5"
#define KEY         "yJODSSiB-QrW-dnq"

#define MAC         "A1B2C3D4E916"



//------------- for at command ----------------------
//
#define OK                  "+OK\r\n"

#define FORMAT_ERROR        "+ERR=1\r\n" //command format error\r\n"
#define NONE_COMMAND        "+ERR=2\r\n" //unknow this command\r\n"
#define PARAM_ERROR         "+ERR=3\r\n" //param error\r\n"
#define TARGET_ERROR         "+ERR=4\r\n" //param error\r\n"
#define EXEC_ERROR          "FAILED\r\n" //param error\r\n"




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
void handle_hsl(int argc, char *argv[], struct socketaddr_in *client_addr)
{
    LOG(LEVEL_DEBUG, "hanle_hsl E\n");

    if(argc <= 1) {
        WKStack_lan_sendto(client_addr, PARAM_ERROR, strlen(PARAM_ERROR));
    }
    else if(argc == 2){
        
        WKStack_lan_sendto(client_addr, OK, strlen(OK));
    }
    else {
        WKStack_lan_sendto(client_addr, PARAM_ERROR, strlen(PARAM_ERROR));
    }
    LOG(LEVEL_DEBUG, "handle_hsl X\n");
}

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



    char cmd[] = "HSL";
    WKStack_register_lan_hook(cmd, handle_hsl);


    WK_Init(PRODUCTID, MAC, KEY);

    while(1){
        WK_Start();
        msleep(2000);
    }

	return 0;
}
