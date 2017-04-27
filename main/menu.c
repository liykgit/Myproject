
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "WKStack_data.h"

#if (_PLATFORM_ == _WIN32_)

#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>

#elif ((_PLATFORM_ == _LINUX_))

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <netdb.h>
#include <ncurses.h>

#endif

#include "linkList.h"
#include "common.h"
#include "network_process.h"

#define BUFFER_MAX_SIZE 64

extern WKStack_datapoint_t *dpList[];
extern WKStack_command_t *cmdList[];

WKStack_datapoint_t **dpList_part;

int show_main_menu = 0;

WKStack_params_t params;

void show_raw(unsigned char *raw, unsigned int size)
{
    unsigned int i;

    for(i = 0; i < size; i++){
        printf("%02x ", raw[i]);
    }
}

int connect_cb(WKStack_state_t state)
{
    if(state == WKSTACK_ONLINE){
        printf("<LOG> Device online\n");
    }else{
        printf("<LOG> Device offline\n");
    }

    return 0;
}


int wk_set_cb(void *dp)
{
    int i = 0;

    for(i = 0; i < DATAPOINT_NUM; i++){
        if(strcmp(((WKStack_datapoint_t *)dp)->name, dpList[i]->name) == 0)
            break;
    }

    if(i >= DATAPOINT_NUM){
        return 0;
    }

    switch(dpList[i]->type){
        case WKSTACK_TYPE_FLOAT:
            ((WKStack_float_t *)(dpList[i]))->value = ((WKStack_float_t *)dp)->value;
            printf("<LOG> Set %s = %f\n", ((WKStack_float_t *)(dpList[i]))->name, ((WKStack_float_t *)(dpList[i]))->value);
            break;
        case WKSTACK_TYPE_STRING:
            strcpy(((WKStack_string_t *)(dpList[i]))->value, ((WKStack_string_t *)dp)->value);
            printf("<LOG> Set %s = %s\n", ((WKStack_string_t *)(dpList[i]))->name, ((WKStack_string_t *)(dpList[i]))->value);
            break;
        case WKSTACK_TYPE_RAW:
            strcpy((char *)(((WKStack_raw_t *)(dpList[i]))->value), (char *)(((WKStack_raw_t *)dp)->value));
            ((WKStack_raw_t *)(dpList[i]))->length = ((WKStack_raw_t *)dp)->length;
            printf("<LOG> Set %s = ", ((WKStack_raw_t *)(dpList[i]))->name);
            show_raw(((WKStack_raw_t *)(dpList[i]))->value, ((WKStack_raw_t *)(dpList[i]))->length);
            break;
        case WKSTACK_TYPE_BOOL:
            ((WKStack_bool_t *)(dpList[i]))->value = ((WKStack_bool_t *)dp)->value;
            printf("<LOG> Set %s = %d\n", ((WKStack_bool_t *)(dpList[i]))->name, ((WKStack_bool_t *)(dpList[i]))->value);
            break;
        case WKSTACK_TYPE_INT:
            ((WKStack_int_t *)(dpList[i]))->value = ((WKStack_int_t *)dp)->value;
            printf("<LOG> Set %s = %d\n", ((WKStack_bool_t *)(dpList[i]))->name, ((WKStack_bool_t *)(dpList[i]))->value);
            break;
        case WKSTACK_TYPE_LONG:
            ((WKStack_long_t *)(dpList[i]))->value = ((WKStack_long_t *)dp)->value;
            printf("<LOG> Set %s = %d\n", ((WKStack_long_t *)(dpList[i]))->name, ((WKStack_long_t *)(dpList[i]))->value);
            break;
    }

    return 0;
}

int wk_cmd_cb(void *cmd)
{
    printf("<LOG> Get command = %s, args = %s, args2 = %s\n", ((WKStack_command_t *)cmd)->name, ((WKStack_command_t *)cmd)->args, ((WKStack_command_t *)cmd)->args2);
    return 0;
}

int publish_cb(unsigned short id, WKStack_publish_state_t state)
{
    if(state == WKSTACK_PUBLISH_SUCCEED)
        printf("<LOG> Publish success(id:%d)\n", id);
    else if(state == WKSTACK_PUBLISH_FAILED)
        printf("<LOG> Publish failed(id:%d)\n", id);

    return 0;
}

void show_dp_list_menu()
{
    int i;

    printf("---------------------------------2----------------------------\n");
    printf("Datapoint list:\n");
    for(i = 0; dpList[i] != 0; i++){
        printf("%d, %s", i + 1, dpList[i]->name);
        switch(dpList[i]->type){
            case WKSTACK_TYPE_STRING:
                printf(" = %s\n", ((WKStack_string_t *)(dpList[i]))->value);
                break;
            case WKSTACK_TYPE_INT:
                printf(" = %d\n", ((WKStack_int_t *)(dpList[i]))->value);
                break;
            case WKSTACK_TYPE_BOOL:
                printf(" = %d\n", ((WKStack_bool_t *)(dpList[i]))->value);
                break;
            case WKSTACK_TYPE_LONG:
                printf(" = %d\n", ((WKStack_long_t *)(dpList[i]))->value);
                break;
            case WKSTACK_TYPE_FLOAT:
                printf(" = %f\n", ((WKStack_float_t *)(dpList[i]))->value);
                break;
            case WKSTACK_TYPE_RAW:
                //printf(" = %s\n", ((WK_RawDP *)(dpList[i]))->raw);
                show_raw(((WKStack_raw_t *)(dpList[i]))->value, ((WKStack_raw_t *)(dpList[i]))->length);
                break;
            default:
                break;
        }
    }
    printf("----------------------------------2---------------------------\n");
}
int get_choose_set_value()
{
    char index = 0;
    char tmp[BUFFER_MAX_SIZE] = {'\0',};
    int i;
    //int input_count = 0;

    memset(tmp, 0, sizeof(tmp));
    i = 0;

    while(i < BUFFER_MAX_SIZE){
        tmp[i] = getch();
        putchar(tmp[i]);
        if(tmp[i] == 'r'){
            printf("\n");
            return 1;
        }
        if(tmp[i] == 0x0D){
            break;
        }

        i++;
    }

    index =  (char)atoi(tmp);
    if(index < 0 && index > DATAPOINT_NUM + 1){
        printf("Input failed, datapoint not found\n");
        return -1;
    }
    memset(tmp, 0, sizeof(tmp));
    printf("Please input the value:");
    scanf("%s", tmp);
    getchar();
    printf("Set %s = %s, input ENTER to continue\n", dpList[index - 1]->name, tmp);
    getch();
    switch(dpList[index - 1]->type){
        case WKSTACK_TYPE_STRING:
            strcpy(((WKStack_string_t *)(dpList[index - 1]))->value, tmp);
            break;
        case WKSTACK_TYPE_INT:
            ((WKStack_int_t *)(dpList[index - 1]))->value = atoi(tmp);
            break;
        case WKSTACK_TYPE_BOOL:
            ((WKStack_int_t *)(dpList[index - 1]))->value = atoi(tmp);
            break;
        case WKSTACK_TYPE_FLOAT:
            ((WKStack_float_t *)(dpList[index - 1]))->value = atof(tmp);
            break;
        case WKSTACK_TYPE_RAW:
            strcpy((char *)(((WKStack_raw_t *)(dpList[index - 1]))->value), tmp);
            break;
        case WKSTACK_TYPE_LONG:
            ((WKStack_long_t *)(dpList[index - 1]))->value = atol(tmp);
            break;
        default:
            break;
    }
    return 0;
}
void get_choose_creat_group()
{
    char index_buf[DATAPOINT_NUM + 1] = {0,};
    char tmp[BUFFER_MAX_SIZE] = {'\0'};
    int i;
    int input_count = 0;
    int dp_num = 0;
    int index = 0;

    memset(index_buf, 0, sizeof(index_buf));
    memset(tmp, 0, sizeof(tmp));
    i = 0;
    while(i < BUFFER_MAX_SIZE && (tmp[i++] = getchar()) != '\n');

    input_count = 0;
    if((char)atoi(tmp) > 0 && (char)atoi(tmp) < DATAPOINT_NUM + 1)
        index_buf[input_count++] =  (char)atoi(tmp);
    for(i = 0; input_count < (DATAPOINT_NUM + 1) && tmp[i] != '\0'; i++){
        if(tmp[i] == ' '){
            tmp[i] = (char)atoi(&(tmp[i+1]));
            if(tmp[i] > 0 && tmp[i] < DATAPOINT_NUM + 1)
                index_buf[input_count++] = tmp[i];
        }
    }

    dp_num = input_count;
    if(dp_num != 0){
        dpList_part = (WKStack_datapoint_t **)mem_alloc((dp_num + 1) * sizeof(WKStack_datapoint_t *));
        if(dpList_part == NULL){
            printf("Malloc failed\n");
            return;
        }

        for(i = 0; i < dp_num; i++){
            dpList_part[i] = dpList[index_buf[i] - 1];
        }
        dpList_part[i] = 0;

        index = link_list_insert(dpList_part);
        if(index == -1){
            printf("Add group failed\n");
        }else{
            printf("Add group succeed\n");
            link_list_show_one(index);
        }
    }
}

void usr_thread(unsigned int v)
{
    int i;
    char choose = 0;
    char index = 0;
    dp_group_t *find_group = NULL;
    int ret = 0;
    unsigned short msg_id = 0;

    while(1){
        if(show_main_menu){
            printf("---------------------------------1----------------------------\n");
            printf("1. Make datapoint group\n");
            printf("2. Print datapoint group\n");
            printf("3. Delete datapoint group\n");
            printf("4. Publish datapoint group\n");
            printf("5. Set datapoint value\n");
            printf("\n");
            printf("6. Don't show this menu, show publish menu forever\n");
			printf("q. Quit\n");
            printf("---------------------------------1----------------------------\n");
            choose = (char)getch();
        }else{
            choose = '4';
        }

        switch(choose){
        case '1':
            show_dp_list_menu();
            printf("Choose datapoint number serial(etc. 1 4 10 12 5), and input ENTER\n");
            get_choose_creat_group();
            break;
        case '2':
            link_list_show_all();
            break;
        case '3':
            link_list_show_all();
            printf("Please choose a datapoint group to delete\n");
            index = (char)getch();
            ret = link_list_delete(atoi(&index));
            if(ret == 0){
                printf("Delete succeed\n");
            }else{
                printf("Group not found\n");
            }
            break;
        case '4':
            link_list_show_all();
            printf("Please choose a datapoint group number to publish(input 'r' to return to main menu)\n");
            index = (char)getch();
            if(index == 'r'){
                show_main_menu = 1;
                continue;
            }
            find_group = link_list_find(atoi(&index), NULL);
            if(find_group != NULL){
				msg_id = WKStack_report_datapoint(find_group->dp_group, publish_cb);
                printf("Publish group %d(id: %d)\n", atoi(&index), msg_id);
            }else{
                printf("Group not found\n");
            }
            break;
        case '5':
            while(1){
                show_dp_list_menu();
                printf("Choose datapoint number(etc. 5), and input ENTER(input 'r' return to main menu)\n");
                if(get_choose_set_value()){
                    break;
                }
            }
            continue;
        case '6':
            show_main_menu = 0;
            continue;
        case '7':
            WKStack_ota_request(WKSTACK_OTA_TARGET_MOD);
            break;
        case 'q':
        case 'Q':
			WKStack_stop();
            for(i = 0; i < DATAPOINT_NUM; i++){
                if(dpList[i]->type == WKSTACK_TYPE_STRING){
                    free(((WKStack_string_t *)(dpList[i]))->value);
                    ((WKStack_string_t *)(dpList[i]))->value = NULL;
                }else if(dpList[i]->type == WKSTACK_TYPE_RAW){
                    free(((WKStack_raw_t *)(dpList[i]))->value);
                    ((WKStack_raw_t *)(dpList[i]))->value = NULL;
                }
            }
            return;
        default:
            printf("Please input 1 - 5\n");
            break;
        }
        printf("Input ENTER to return\n");
        getch();
    }
}

void cmd_init()
{
    int i;

    for(i = 0; i < CMD_NUM; i++){
		WKStack_register_command(cmdList[i], wk_cmd_cb);
    }
}

void dp_init()
{
    int i;

    for(i = 0; i < DATAPOINT_NUM; i++){
        if(dpList[i]->type == WKSTACK_TYPE_STRING){
            ((WKStack_string_t *)(dpList[i]))->value = (char *)malloc(BUFFER_MAX_SIZE);
			memset(((WKStack_string_t *)(dpList[i]))->value, '\0', BUFFER_MAX_SIZE);
			((WKStack_string_t *)(dpList[i]))->value[0] = '0';
        }else if(dpList[i]->type == WKSTACK_TYPE_RAW){
            ((WKStack_raw_t *)(dpList[i]))->value = (unsigned char *)malloc(BUFFER_MAX_SIZE);
			memset(((WKStack_raw_t *)(dpList[i]))->value, 0, BUFFER_MAX_SIZE);
            ((WKStack_raw_t *)(dpList[i]))->length = 1;
        }
		WKStack_register_datapoint(dpList[i], wk_set_cb);
    }
}

void WKdevice_start()
{
    link_list_init(dpList);
    show_main_menu = 1;

    printf("Init WKStack, please wait...\n");
    WKStack_init(&params, 1);

    dp_init();
    cmd_init();

	WKStack_start(connect_cb, NULL);

    while(1){
        msleep(2000);
        if(WKStack_state() == WKSTACK_ONLINE){
            printf("Init succeed\n");
            usr_thread(1);
            break;
        }
    }
}

int main(int argc, char **argv)
{
	FILE				*config_fp;

	memset(&params, 0, sizeof(WKStack_params_t));

#if 1
	if(argv[1] == NULL){
		printf("usage:\nWKdevice [device number]\n");
		getchar();
		return -1;
	}
#endif

	if (argv[1] != NULL) {
		config_fp = fopen("devices.config", "r");
		if (config_fp == NULL) {
			printf("Open config file error\n");
			while (1) { msleep(1000); }
			return 1;
		}
		fseek(config_fp, 74 * (atoi((const char *)argv[1]) - 1), SEEK_SET);

		fscanf(config_fp, "%s %s %s %s", params.mac, params.sn, params.devtype, params.key);
		printf("%s %s %s %s\n", params.mac, params.sn, params.devtype, params.key);
	}

    WKdevice_start();

	return 0;
}
