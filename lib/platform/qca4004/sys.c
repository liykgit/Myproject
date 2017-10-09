#include "plat.h"

int vg_create_sem(void *handle, char *name)
{
	tx_semaphore_create(handle, name, 0);

	return 0;
}

int vg_release_sem(void *handle)
{
	int ret = 0;

	tx_semaphore_put((TX_SEMAPHORE*)handle);

	return ret;
}

int vg_wait_sem(void *handle, int tm)
{
	int ret = -1;
	ret = tx_semaphore_get((TX_SEMAPHORE*)handle, (tm < 0) ? 0xFFFFFFFF : tm);
	if(ret != TX_SUCCESS){
		ret = -1;
	}

	return ret;
}

void vg_delete_sem(void *handle)
{
	tx_semaphore_delete((TX_SEMAPHORE*)handle);
}

int vg_create_mutex(void *lock, char *name)
{

	tx_mutex_create(lock, "mutex tsk", TX_NO_INHERIT);

	return 0;
}

int vg_get_mutex(void *lock)
{

	int status =  tx_mutex_get(lock, TX_WAIT_FOREVER);
	if (status != TX_SUCCESS) {
        LOG(LEVEL_ERROR, "<ERR> get mutex ret = %d\n", status);
		return -1;
	}

	return 0;
}

int vg_put_mutex(void *lock)
{

	int status =  tx_mutex_put((TX_MUTEX*)lock);
	if (status != TX_SUCCESS) {
        LOG(LEVEL_ERROR, "<ERR> put mutex ret = %d\n", status);
		return -1;
	}
	return 0;
}

int vg_select(int nfds, vg_fd_set *readfds, vg_fd_set *writefds, vg_fd_set *exceptfds, struct timeval *timeout) {
    return qcom_select(nfds, readfds, writefds, exceptfds, timeout);
}

void *vg_malloc(unsigned int sz) {
    void * p = mem_alloc(sz);

    return p;
}

void vg_free(void *ptr) {
    if(ptr) {

        mem_free(ptr);
    }
}

unsigned long vg_time_ms() {
    
    return time_ms();
}

