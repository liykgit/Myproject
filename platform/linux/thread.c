
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <pthread.h>

#include "plat.h"

int start_thread(thread_fun_t fun, void **stk_addr, int stk_size)
{
    int ret = 0;
	pthread_t mythread;
	ret = pthread_create(&mythread, 0, fun, NULL);
	if(ret != 0)
		return -1;
	else
		return 0;
}
