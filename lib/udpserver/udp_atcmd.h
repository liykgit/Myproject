#ifndef _USER_CMD_H
#define _USER_CMD_H

int udp_user_cmd_handle(unsigned char *cmd, int len, struct sockaddr_in *client_addr);

#endif
