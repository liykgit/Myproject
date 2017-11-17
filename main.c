#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "WKStack.h"
#include "tlv.h"

#include "common.h"


#define PRODUCTID   "yEOBNYFxfyNfbm0P"
#define MAC         "A1B2C3D4E916"
#define VERSION     "0.1"


#define MAC         "A1B2C3D4E916"


int connect_cb()
{
    printf("connected\n");
    return 0;
}
//----------------------- udp ---------------------------------------

int disconnect_cb()
{
    printf("disconnected \n");
    return 0;
}

int raw_data_handler(char *raw_data, int len) {
    
    printf("received raw data %s\n", raw_data);
    return 0;
}

int load_params(void *buf, int size) {
    
    printf("loading params \n");
    return 0;
}

int save_params(void *buf, int size) {
    
    printf("saving params\n");
    return 0;
}

int main(int argc, char **argv)
{

    WKStack_init(MAC, PRODUCTID, VERSION, 0, 0);

    WKStack_register_callback(CALLBACK_CONNECTED, connect_cb);
    WKStack_register_callback(CALLBACK_DISCONNECTED, disconnect_cb);

    WKStack_register_callback(CALLBACK_SAVE_PARAMS, save_params);
    WKStack_register_callback(CALLBACK_LOAD_PARAMS, load_params);

    WKStack_register_callback(CALLBACK_RAW_DATA, raw_data_handler);

    while(1){
        WKStack_start();
        msleep(2000);
    }

	return 0;
}
