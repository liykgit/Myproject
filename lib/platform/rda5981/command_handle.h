#ifndef __COMMAND_HANDLE_H__
#define __COMMAND_HANDLE_H__

#define PARAM_ADDR 0x180fd000

typedef enum {
    SOFTAP_MODE = 1,
    STATION_MODE = 2,
} conn_mode_t;

typedef struct system_param_s {
    conn_mode_t mode_flag;
    char sta_ssid[33];
    char sta_psd[65];
} system_param_t;

#ifdef __cplusplus
extern "C" {
#endif

void stainfo_func(int argc, char *argv[], struct sockaddr_in *client_addr);

#ifdef __cplusplus
}
#endif

#endif










