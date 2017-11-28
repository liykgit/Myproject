
#include "WKStack.h"
#include "WKStack_lib.h"
#include "udpserver.h"
#include "udp_atcmd.h"
#include "tcpserver.h"
#include "common.h"


#define HANDLER_MAX 32
#define PARAM_MAX 10
#define CMD_BUF_SIZE 2048
#define SHELL_BUF_SIZE 1024
#define CMD_PREFIX_SIZE 3

#define TCPSERVER_PORT      30321

extern WKStack_t WKStack;

typedef enum {
    AT_COMMAND = 0,
    NONE_AT_COMMAND,
    AT_COMMAND_ERROR,
} user_command_t;

typedef struct {
    char *command;
    void (*execute)(int argc, char *argv[], struct sockaddr_in *client_addr);
}cmd_handle_t;

#define FIND                "FIND"
#define BIND                "BIND"
#define RAW                 "RAW"
#define NETSTAT             "NETSTAT"
#define ONLINE              "ONLINE"
#define OFFLINE             "OFFLINE"
#define BROADCAST           "BROADCAST"
#define TESTMODE            "TESTMODE"
#define TCPSEND             "TCPSEND"

void exec_testmode(int argc, char *argv[], struct sockaddr_in *client_addr)
{
    LOG(LEVEL_DEBUG, "exec_testmode E\n");
       
    int r = tcpserver_start(TCPSERVER_PORT);
    
    if(r == 0) {

        udpserver_sendto(client_addr, OK, strlen(OK));

        g_testmode = 1;
    }
    else {
        udpserver_sendto(client_addr, EXEC_ERROR, strlen(EXEC_ERROR));
    }

    LOG(LEVEL_DEBUG, "exec_testmode X\n");
}

void exec_tcpsend(int argc, char *argv[], struct sockaddr_in *client_addr)
{
    LOG(LEVEL_DEBUG, "exec_tcpsend E\n");
       
    if(argc == 2){
        printf("tcp sending %s\n", argv[1]);
        tcpserver_send(argv[1], strlen(argv[1]));
        udpserver_sendto(client_addr, OK, strlen(OK));
    }
    else {
        udpserver_sendto(client_addr, PARAM_ERROR, strlen(PARAM_ERROR));
    }

    LOG(LEVEL_DEBUG, "exec_tcpsend X\n");
}

void exec_broadcast(int argc, char *argv[], struct sockaddr_in *client_addr)
{
    LOG(LEVEL_DEBUG, "exec_broadcast E\n");

    if(argc == 2){
        printf("broadcasting %s\n", argv[1]);
        udpserver_broadcast(argv[1], strlen(argv[1]), 30320);
        //udpserver_sendto(client_addr, OK, strlen(OK));
    }
    else {
        udpserver_sendto(client_addr, PARAM_ERROR, strlen(PARAM_ERROR));
    }
    LOG(LEVEL_DEBUG, "exec_broadcast X\n");
}

void exec_netstat(int argc, char *argv[], struct sockaddr_in *client_addr)
{
    LOG(LEVEL_DEBUG, "exec_netstat E\n");
    char stat = '0' + WKStack_state();

    udpserver_sendto(client_addr, &stat, 1);
    LOG(LEVEL_DEBUG, "exec_netstat X\n");

}


void exec_online(int argc, char *argv[], struct sockaddr_in *client_addr)
{
    WKStack_start();
    udpserver_sendto(client_addr, OK, strlen(OK));
}


void exec_offline(int argc, char *argv[], struct sockaddr_in *client_addr)
{    
    WKStack_stop();
    udpserver_sendto(client_addr, OK, strlen(OK));
}

void exec_passthrough(int argc, char *argv[], struct sockaddr_in *client_addr)
{
    LOG(LEVEL_DEBUG, "exec_passthrough E\n");
    if(argc <= 1) {
        udpserver_sendto(client_addr, PARAM_ERROR, strlen(PARAM_ERROR));
    }
    else if(argc == 2){
        
        printf("passthrough to server %s\n", argv[1]);
        
        int buf_sz = strlen(argv[1]) >> 1 ;

        char *buf = vg_malloc(buf_sz);

        memset(buf, 0, buf_sz);

        hex2bin(argv[1], buf);

        WKStack_report_raw(buf, buf_sz);

        int i = 0;
        char *pbuf = buf;
        for (i = 0; i < buf_sz; i++)
        {
            printf("%02x ", *pbuf++);
        }

        vg_free(buf);
        udpserver_sendto(client_addr, OK, strlen(OK));
    }
    else {
        udpserver_sendto(client_addr, PARAM_ERROR, strlen(PARAM_ERROR));
    }
    LOG(LEVEL_DEBUG, "exec_passthrough X\n");
}

void exec_find(int argc, char *argv[], struct sockaddr_in *client_addr)
{
    LOG(LEVEL_DEBUG, "exec_find E\n");
    
    if(strlen(WKStack.params.did) > 0) {

        char buf[216];
        memset(buf, 0, sizeof(buf));
        sprintf((char *)buf, "VENGAS:FIND:%s#%s#%s#%s:VENGAE", WKStack.params.product_id, WKStack.params.mac, WKStack.params.did, WKStack.params.name);
        udpserver_sendto(client_addr, buf, strlen(buf));
    } 
    else {
        //sprintf((char *)buf, "VENGAS:FIND:%s#%s:VENGAE", WKStack.params.devtype, WKStack.params.mac);
    }

    LOG(LEVEL_DEBUG, "exec_find X\n");
}

extern int WKStack_publish_bind_request(char *userId);

void exec_bind(int argc, char *argv[], struct sockaddr_in *client_addr)
{
    LOG(LEVEL_DEBUG, "exec_bind E\n");
    if(argc <= 1) {
        udpserver_sendto(client_addr, PARAM_ERROR, strlen(PARAM_ERROR));
    }
    else if(argc == 2){

        if(memcmp(argv[1], WKStack.params.did, strlen(WKStack.params.did))) {
            udpserver_sendto(client_addr, TARGET_ERROR, strlen(TARGET_ERROR));
            return;
        }
       
        char *p = strstr(argv[1], "#");
            
        if(!p || !(p+1)) {
            udpserver_sendto(client_addr, PARAM_ERROR, strlen(PARAM_ERROR));
            return;
        }

        p++;

        LOG(LEVEL_NORMAL, "bind request from user %s\n", p);
        WKStack_publish_bind_request(p);

        client_info_t *cinfo = vg_malloc(sizeof(client_info_t));
        memcpy(&cinfo->addr, client_addr, sizeof(struct sockaddr_in));

        strncpy(cinfo->user_id, p, 16);
        void *victim = plist_push(cinfo);
        if(victim)
            vg_free(victim);
    }
    else {
        udpserver_sendto(client_addr, PARAM_ERROR, strlen(PARAM_ERROR));
    }
    LOG(LEVEL_DEBUG, "exec_bind X\n");
}

static cmd_handle_t at_command_handler[HANDLER_MAX] = {
    {
        .command = FIND, //!< restart system
        .execute = exec_find
    },
    {
        .command = BIND, //!< restart system
        .execute = exec_bind
    },

    {
        .command = RAW, 
        .execute = exec_passthrough
    },
    {
        .command = ONLINE, 
        .execute = exec_online
    },

    {
        .command = OFFLINE, 
        .execute = exec_offline
    },

    {
        .command = NETSTAT, 
        .execute = exec_netstat
    },

    {
        .command = BROADCAST, 
        .execute = exec_broadcast
    },

    {
        .command = TESTMODE, 
        .execute = exec_testmode
    },
    {
        .command = TCPSEND, 
        .execute = exec_tcpsend
    }
};

static int handler_nr = 9;


int udp_user_cmd_handle(unsigned char *cmd, int len, struct sockaddr_in *client_addr)
{
    int i;
    int j;
    char *p = cmd;
    char *p2 = NULL;
    char *p_next_cmd = NULL;

    int argc = 0;
    char *argv[PARAM_MAX] = {NULL};

REPEATED_PARSING:

    if (memcmp(cmd, AT, CMD_PREFIX_SIZE) == 0){
        cmd += CMD_PREFIX_SIZE;

        //查找结束符
        if ((p = strstr(cmd, "\r\n")) != NULL){
            *p = '\0';

            //如果有连续的指令，则指向下一条指令
            p_next_cmd = strstr(p + 2, AT);
        }

        // 如果数据中带有0x00的话，上面步骤查找"\r\n"会失败，则应该再进行下面的步骤
        if(p == NULL){
            while(len > 1){
                if(cmd[len - 1] == '\n' && cmd[len - 2] == '\r'){
                    p = &(cmd[len - 2]);

                    *p = '\0';
                    //如果有连续的指令，则指向下一条指令
                    p_next_cmd = strstr(p + 2, AT);
                    break;
                }
                len--;
            }
        }

        if (p == NULL)
        {
            printf("\r\n" FORMAT_ERROR);

            return AT_COMMAND_ERROR;
        }

        if (*cmd != '\0')
        {
            i = 0;

            p = cmd;

            //参数预处理，处理转义字符
            while (*p != '\0')
            {
                if (*p++ == '\\')
                {
                    *p -= 1;
                }
            }

            //抽取参数
            //第一次抽取参数以'=\r\n'为分割符
            if ((argv[argc++] = vg_strtok(cmd, "=\r\n")) == NULL)
            {
                printf(FORMAT_ERROR);

                return AT_COMMAND_ERROR;
            }

            while (i < handler_nr)
            {
                if (strcmp(cmd, at_command_handler[i].command) == 0)
                {
                    do
                    {
                        if ((argv[argc] = vg_strtok(NULL, ",")) != NULL)
                        {
                            if (argc++ > PARAM_MAX - 1)
                            {
                                //SHELL_PRINTF(channel, "too many parameters.\n" PARAM_ERROR);
                                printf(PARAM_ERROR);

                                return AT_COMMAND_ERROR;
                            }

                            continue;
                        }

                        break;
                    } while (1);

                    //恢复预处理后的参数
                    for (j = 0; j < argc; j++)
                    {
                        p = argv[j];

                        while (*p != '\0')
                        {
                            p2 = p++;

                            if (*p2++ == '\\')
                            {
                                //恢复转义参数
                                *p2 += 1;

                                //去除转义标志
                                while (*p2 != '\0')
                                {
                                    *(p2 - 1) = *p2;
                                    p2++;
                                }

                                *(p2 - 1) = '\0';
                            }
                        }
                    }

                    //调用命令handle
                    at_command_handler[i].execute(argc, argv, client_addr);

                    break;
                }

                i++;
            }

            if (i >= handler_nr)
            {
                printf(NONE_COMMAND);
            }
        }
        else
        {
            //AT+ 测试指令
            printf(OK);
        }

        if (p_next_cmd != NULL)
        {
            argc = 0;

            cmd = p_next_cmd;

            goto REPEATED_PARSING;
        }

        return AT_COMMAND;
    }

    return NONE_AT_COMMAND;
}


int WKStack_register_lan_hook(char *cmd, lan_cmd_hook_t hook) {
    
    if(!cmd || !hook)
        return -1;

    at_command_handler[handler_nr].command = cmd;
    at_command_handler[handler_nr].execute = hook;

    handler_nr++;

    return 0;
}

