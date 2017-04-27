#include "plat.h"

int create_sem(void *handle, char *name)
{

	sem_init((sem_t *)handle, 0, 0);

	return 0;
}

int release_sem(void *handle)
{
	int ret = 0;

	ret = sem_post ((sem_t *)handle);

	printf( "release sem ret = %d\n", ret);
	return ret;
}

int wait_sem(void *handle, int tm)
{
	int ret = -1;
	struct timespec timeout;


	if(tm < 0){
		ret = sem_wait((sem_t *)handle);
	}else{
		ret = clock_gettime(CLOCK_REALTIME, &timeout);
		if(ret < 0)
			return -1;

		timeout.tv_sec += tm/1000;

		ret = sem_timedwait((sem_t *)handle, &timeout);
	}


	if(ret < -1 && errno == ETIMEDOUT){
		ret = -1;
	}

	printf( "wait sem ret = %d\n", ret);
	return ret;
}

void delete_sem(void *handle)
{

	sem_destroy((sem_t *)handle);
}

int create_mutex(void *lock, char *name)
{

	pthread_mutex_init((pthread_mutex_t *)lock, NULL);

	return 0;
}

int get_mutex(void *lock)
{

	pthread_mutex_lock((pthread_mutex_t *)lock);

	return 0;
}

int put_mutex(void *lock)
{

	pthread_mutex_unlock((pthread_mutex_t *)lock);

	return 0;
}

