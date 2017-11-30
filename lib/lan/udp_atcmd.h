#ifndef _USER_CMD_H
#define _USER_CMD_H


#define AT                  "AT+"
#define OK                  "+OK\r\n"

#define FORMAT_ERROR        "+ERR=1\r\n" //command format error\r\n"
#define NONE_COMMAND        "+ERR=2\r\n" //unknow this command\r\n"
#define PARAM_ERROR         "+ERR=3\r\n" //param error\r\n"
#define TARGET_ERROR         "+ERR=4\r\n" //param error\r\n"
#define EXEC_ERROR          "FAILED\r\n" //param error\r\n"

#ifdef __cplusplus
extern "C" {
#endif

int udp_user_cmd_handle(unsigned char *cmd, int len, struct sockaddr_in *client_addr);

#ifdef __cplusplus
}
#endif

#endif
