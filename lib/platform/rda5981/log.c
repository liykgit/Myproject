
#include "plat.h"

#ifdef LOG_IN_FILE
FILE *log_fp = NULL;
static int init_flag = 0;
#endif

int log_init(char *mac)
{
#ifdef LOG_IN_FILE
	if(init_flag == 0){
		int ret = 0;
		char file_name[24];

		if(access("./log/", 0/*F_OK*/) != 0){
			ret = mkdir("./log/", 0777);
			if(ret != 0){
				return -1;
			}
			printf("Create log dir OK\n");
		}

		memset(file_name, '\0', 24);

		sprintf(file_name, "log/%s.txt", mac);

		log_fp = fopen(file_name, "w+");
		if (log_fp == NULL) {
			return -1;
		}

		init_flag = 1;
	}
#endif

	return 0;
}

void now(char *time_buf)
{
#ifdef LOG_IN_FILE
	struct tm *tm;
	time_t timep;

	if(time_buf == NULL)
		return;

	time(&timep);
	tm = localtime(&timep);

	sprintf(time_buf, "[ %04d-%02d-%02d %02d:%02d:%02d ]", (tm->tm_year+1900), (tm->tm_mon+1)
			, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
#endif
}

void log_buffer(enum log_level level, char *buf, unsigned int size)
{
	if (level >= LOG_LEVEL) {
		unsigned int i = 0;

		for (i = 0; i < size; i++) {
#ifdef LOG_IN_FILE
			fprintf(log_fp, "0x%02x, ", buf[i]);
#else
			printf("0x%02x, ", buf[i]);
#endif
		}
#ifdef LOG_IN_FILE
		fprintf(log_fp, "\n");
		fflush(log_fp);
#else
		printf("\n");
#endif
	}
}

void log_uninit()
{
#ifdef LOG_IN_FILE
	fclose(log_fp);
#endif
}
