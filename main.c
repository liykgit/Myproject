#include <stdio.h> #include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "WKStack.h"
#include "tlv.h"

#include "common.h"


//85
//#define PRODUCTID   "E3GF3s6f8WOC2bsh"
//




#define PRODUCTID   "zT6b3nuvSt0i8kIt"
#define KEY         "k1gyW7pefc02oowe"
#define MAC         "A1B2C3D4E916"
#define VERSION     "0.1"


static int not_started = 1;

int connect_cb()
{
    printf("connected\n");
    not_started = 0;
    return 0;
}
//----------------------- udp ---------------------------------------

int disconnect_cb()
{
    printf("disconnected \n");
    not_started = 1;

    return 0;
}


int send_callback(int msg_id, publish_result_t err_code)
{

    printf("send result: %d\n", err_code);
    return 0;
}


int raw_data_handler(char *raw_data, int len) {
    
    printf("received raw data: %s\n", raw_data);

    WKStack_send_raw(raw_data, len, send_callback);

    return 0;
}

int load_params(void *buf, int size) {

    printf("loading params\n");
   
    FILE *ptr_file;

    ptr_file =fopen("flash.bin", "rb");

    if (!ptr_file) {
        printf("open flash.bin failed\n");
        memset(buf, 0, size); 
    }
    else {
 
        fread(buf, size, 1, ptr_file);

        fclose(ptr_file);
    }

    return 0;
}


int save_params(void *buf, int size) {

    printf("saving params\n");

    FILE *ptr_file;

    ptr_file =fopen("flash.bin", "wb+");

    if (!ptr_file) {
        printf("open flash.bin failed\n");
        exit(0);
    }
    
    fwrite(buf, size, 1, ptr_file);

    fclose(ptr_file);

    return 0;
}

int error_handler(int error_code) {
    printf("error : %d\n", error_code);
    not_started = 1;
    return 0;
}

int ota_event_handler(const WKStack_ota_msg_t *msg)
{
    printf("ota_event_handler E\n");

    printf("ota msg  type: %d\n", msg->type);
    printf("ota target :  %d\n", msg->target);
    printf("ota version :  %s\n", msg->firmware->version);
    printf("ota url:  %s\n", msg->firmware->url);
    printf("ota port:  %d\n", msg->firmware->port);

    printf("ota_event_handler X\n");
    return 0;
}

void datapoint_handler(WKStack_datapoint_t dps[], int size)
{
    printf("received %d datapoints\n", size);
    int i = 0;
    for(;i < size;i++) {
        switch(dps[i].type) {
            
            case WKSTACK_DATAPOINT_TYPE_STRING:  {
                printf("%d string: %s\n", dps[i].index, dps[i].value.string);
            }
            break;

            case WKSTACK_DATAPOINT_TYPE_BOOL:  {
                printf("%d bool : %d\n", dps[i].index, dps[i].value.boolean);

            }
            break;

            case WKSTACK_DATAPOINT_TYPE_INT:  {
                printf("%d int : %d\n", dps[i].index, dps[i].value.integer);

            }
            break;

            case WKSTACK_DATAPOINT_TYPE_FLOAT:  {
                printf("%d float : %f\n", dps[i].index, dps[i].value.floatpoint);
            }
            break;
        }
    }

    WKStack_send_datapoint(dps, size, NULL);
}

int main(int argc, char **argv)
{
    WKStack_register_callback(CALLBACK_CONNECTED, connect_cb);
    WKStack_register_callback(CALLBACK_DISCONNECTED, disconnect_cb);

    WKStack_register_callback(CALLBACK_ERROR, error_handler);

    WKStack_register_callback(CALLBACK_SAVE_PARAMS, save_params);
    WKStack_register_callback(CALLBACK_LOAD_PARAMS, load_params);

    WKStack_register_callback(CALLBACK_RAW_DATA, raw_data_handler);

    WKStack_register_callback(CALLBACK_OTA_EVENT, ota_event_handler);

    WKStack_register_callback(CALLBACK_DATAPOINT_EVENT, datapoint_handler);

    WKStack_init(MAC, PRODUCTID, VERSION, 0, KEY);

    while(1){
        if(not_started) {
            WKStack_start();
        }
        msleep(2000);

    }

	return 0;
}
