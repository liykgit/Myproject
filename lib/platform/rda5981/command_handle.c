#include "plat.h"
#include "command_handle.h"
#include "udp_atcmd.h"
#include "udpserver.h"
#include "rda_wdt_api.h"

void stainfo_func(int argc, char *argv[], struct sockaddr_in *client_addr)
{
    system_param_t sys_param;
    
    int ret = vg_read_flash(PARAM_ADDR, (void *)&sys_param, sizeof(system_param_t));
    if(ret)
    {
        printf("Read system param failed(err=%d).\r\n", ret);
        goto exit;
    }

    if(argc == 2)
	{
		memset(sys_param.sta_ssid, 0, sizeof(sys_param.sta_ssid));
        memset(sys_param.sta_psd, 0, sizeof(sys_param.sta_psd));
        
        memcpy(sys_param.sta_ssid, argv[1], strlen(argv[1]));
	}
	else if(argc == 3)
	{
		memset(sys_param.sta_ssid, 0, sizeof(sys_param.sta_ssid));
        memset(sys_param.sta_psd, 0, sizeof(sys_param.sta_psd));
        
        memcpy(sys_param.sta_ssid, argv[1], strlen(argv[1]));
		memcpy(sys_param.sta_psd, argv[2], strlen(argv[2]));
    }
    else
    {
        printf(PARAM_ERROR);
        udpserver_sendto(client_addr, PARAM_ERROR, strlen(PARAM_ERROR));
        return;
    }

    printf("Get ssid: %s, psd: %s\r\n", sys_param.sta_ssid, sys_param.sta_psd);

    sys_param.mode_flag = STATION_MODE;
    ret  = vg_erase_flash(PARAM_ADDR, 4096);
    if(ret)
        printf("Erase flash failed(err=%d).\r\n", ret);
    ret = vg_write_flash(PARAM_ADDR, (void *)&sys_param, sizeof(system_param_t));
    if(ret)
    {
        printf("Write system param failed(err=%d).\r\n", ret);
        goto exit;   
    }
    udpserver_sendto(client_addr, OK, strlen(OK));

    osDelay(100);

    rda_wdt_softreset();

    return;

exit:
    udpserver_sendto(client_addr, TARGET_ERROR, strlen(TARGET_ERROR));
    return;
}








