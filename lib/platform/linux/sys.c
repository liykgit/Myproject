#include "plat.h"

int vg_create_sem(void *handle, char *name)
{

	sem_init((sem_t *)handle, 0, 0);

	return 0;
}

int vg_release_sem(void *handle)
{
	int ret = 0;

	ret = sem_post ((sem_t *)handle);

	LOG(LEVEL_DEBUG, "release sem ret = %d\n", ret);
	return ret;
}

int vg_wait_sem(void *handle, int tm)
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

	LOG(LEVEL_DEBUG, "wait sem ret = %d\n", ret);
	return ret;
}

void vg_delete_sem(void *handle)
{

	sem_destroy((sem_t *)handle);
}

int vg_create_mutex(void *lock, char *name)
{

	pthread_mutex_init((pthread_mutex_t *)lock, NULL);

	return 0;
}

int vg_get_mutex(void *lock)
{

	pthread_mutex_lock((pthread_mutex_t *)lock);

	return 0;
}

int vg_put_mutex(void *lock)
{

	pthread_mutex_unlock((pthread_mutex_t *)lock);

	return 0;
}

int vg_select(int nfds, vg_fd_set *readfds, vg_fd_set *writefds, vg_fd_set *exceptfds, struct timeval *timeout) {
    return select(nfds, readfds, writefds, exceptfds, timeout);
}


void *vg_malloc(size_t sz) {
    return malloc(sz);
}

void vg_free(void *ptr) {
    if(!ptr) {
        free(ptr);
    }
}
