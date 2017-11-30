
#ifndef _TCP_SERVER_H
#define _TCP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

int tcpserver_send(char *buf, int length);
int  tcpserver_start(int port);

#ifdef __cplusplus
}
#endif

#endif
