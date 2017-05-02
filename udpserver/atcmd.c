
#include "WKStack_lib.h"
#include "udpserver.h"
#include "common.h"


#define PARAM_MAX 10
#define CMD_BUF_SIZE 2048
#define SHELL_BUF_SIZE 1024
#define CMD_PREFIX_SIZE 3

#define AT                  "AT+"
#define OK                  "+OK\r\n"

#define FORMAT_ERROR        "+ERR=1\r\n" //command format error\r\n"
#define NONE_COMMAND        "+ERR=2\r\n" //unknow this command\r\n"
#define PARAM_ERROR         "+ERR=3\r\n" //param error\r\n"

extern WKStack_t WKStack;

extern int udp_fd;

typedef enum{
    AT_COMMAND = 0,
    NONE_AT_COMMAND,
    AT_COMMAND_ERROR,
}user_command_t;

typedef struct {
    char *command;
    void (*execute)(int argc, char *argv[], struct socketaddr_in *client_addr);
}cmd_handle_t;

#define FIND                "FIND"
#define BIND                "BIND"
#define RAW                 "RAW"

void exec_passthrough(int argc, char *argv[], struct socketaddr_in *client_addr)
{

    if(argc <= 1) {
        udpserver_sendto(client_addr, PARAM_ERROR, strlen(PARAM_ERROR));
    }
    else if(argc == 2){
        
        printf("passthrough to server %s\n", argv[1]);
        
        int buf_sz = strlen(argv[1]) >> 1 ;
        char *buf = malloc(buf_sz);

        memset(buf, 0, buf_sz);

        hex2bin(argv[1], buf);
        

        WKStack_report_raw(buf, buf_sz);

        int i = 0;
        for (i = 0; i < buf_sz; i++)
        {
            printf("%02x ", *buf++);
        }

    }
    else {
        udpserver_sendto(client_addr, PARAM_ERROR, strlen(PARAM_ERROR));
    }
}


void exec_find(int argc, char *argv[], struct socketaddr_in *client_addr)
{
    printf("find argc %d\n", argc);

    char buf[128];
    memset(buf, 0, sizeof(buf));
    
    if(strlen(WKStack.params.did) > 0) {
        sprintf((char *)buf, "%s#%s#%s", WKStack.params.devtype, WKStack.params.mac, WKStack.params.did);
    } 
    else {
        sprintf((char *)buf, "%s#%s", WKStack.params.devtype, WKStack.params.mac);
    }

    udpserver_sendto(client_addr, buf, strlen(buf));
}


extern int WKStack_publish_bind_request(char *userId);

void exec_bind(int argc, char *argv[], struct socketaddr_in *client_addr)
{

    if(argc <= 1) {
        udpserver_sendto(client_addr, PARAM_ERROR, strlen(PARAM_ERROR));
    }
    else if(argc == 2){
        
        printf("bind request from user %s\n", argv[1]);
        WKStack_publish_bind_request(argv[1]);

        client_info_t *cinfo = malloc(sizeof(client_info_t));
        memcpy(&cinfo->addr, client_addr, sizeof(struct sockaddr_in));

        strncpy(cinfo->user_id, argv[1], 16);

        void *victim = plist_push(cinfo);
        if(victim)
            free(victim);
    }
    else {
        udpserver_sendto(client_addr, PARAM_ERROR, strlen(PARAM_ERROR));
    }
}

const cmd_handle_t command_handle[]= {
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

};

#define COMMAND_COUNT (sizeof(command_handle) / sizeof(cmd_handle_t))

int user_cmd_handle(unsigned char *cmd, int len, struct sockaddr_in *client_addr)
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
            if ((argv[argc++] = strtok(cmd, "=\r\n")) == NULL)
            {
                printf(FORMAT_ERROR);

                return AT_COMMAND_ERROR;
            }

            while (i < COMMAND_COUNT)
            {
                if (strcmp(cmd, command_handle[i].command) == 0)
                {
                    do
                    {
                        if ((argv[argc] = strtok(NULL, ",")) != NULL)
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
                    command_handle[i].execute(argc, argv, client_addr);

                    break;
                }

                i++;
            }

            if (i >= COMMAND_COUNT)
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
