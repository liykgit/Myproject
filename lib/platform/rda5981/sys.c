#include "plat.h"

int vg_create_sem(vg_sem_t *handle, char *name)
{
	//tx_semaphore_create(handle, name, 0);
	handle->handle = rda_sem_create(0);

	handle->name = (char *)malloc(strlen(name)+1);
	memset(handle->name, 0, strlen(name)+1);
	memcpy(handle->name, name, strlen(name));
	
	return 0;
}

int vg_release_sem(vg_sem_t *handle)
{
	int ret = 0;

	//tx_semaphore_put((TX_SEMAPHORE*)handle);
	rda_sem_release(handle->handle);

	printf( "release sem ret = %d\n", ret);
	return ret;
}

int vg_wait_sem(vg_sem_t *handle, int tm)
{
	int ret = -1;
	//ret = tx_semaphore_get((TX_SEMAPHORE*)handle, (tm < 0) ? 0xFFFFFFFF : tm);
	ret = rda_sem_wait(handle->handle, (tm < 0) ? 0xFFFFFFFF : tm);
	if(ret != 0){
		ret = -1;
	}

	LOG(LEVEL_DEBUG, "<LOG> wait sem ret = %d\n", ret);
	return ret;
}

void vg_delete_sem(vg_sem_t *handle)
{
	//tx_semaphore_delete((TX_SEMAPHORE*)handle);
	rda_sem_delete(handle->handle);
	free(handle->name);
	handle->name = NULL;
}

int vg_create_mutex(vg_mutex_t *lock, char *name)
{

	//tx_mutex_create(lock, "mutex tsk", TX_NO_INHERIT);
	lock->lock = rda_mutex_create();

	lock->name = (char *)malloc(strlen(name)+1);
	memset(lock->name, 0, strlen(name)+1);
	memcpy(lock->name, name, strlen(name));

	return 0;
}

int vg_get_mutex(vg_mutex_t *lock)
{

	//int status =  tx_mutex_get(lock, TX_WAIT_FOREVER);
	int status = rda_mutex_wait(lock->lock, 0xFFFFFFFF);
	if (status != 0) {
        LOG(LEVEL_ERROR, "<ERR> get mutex ret = %d\n", status);
		return -1;
	}

	return 0;
}

int vg_put_mutex(vg_mutex_t *lock)
{

	//int status =  tx_mutex_put((TX_MUTEX*)lock);
	int status = rda_mutex_realease(lock->lock);
	if (status != 0) {
        LOG(LEVEL_ERROR, "<ERR> put mutex ret = %d\n", status);
		return -1;
	}
	return 0;
}

void vg_delete_mutex(vg_mutex_t *lock)
{
	//tx_mutex_create(lock, "mutex tsk", TX_NO_INHERIT);
	rda_mutex_delete(lock->lock);
	free(lock->name);	
	lock->name = NULL;
}

int vg_select(int nfds, vg_fd_set *readfds, vg_fd_set *writefds, vg_fd_set *exceptfds, struct timeval *timeout) {
    return select(nfds, readfds, writefds, exceptfds, timeout);
}

void *vg_malloc(unsigned int sz) {
    void * p = malloc(sz);

    return p;
}

void vg_free(void *ptr) {
    if(ptr) {

        free(ptr);
    }
}

int vg_erase_flash(unsigned int addr, unsigned int len)
{
	return rda5981_erase_flash(addr, len);
}

int vg_write_flash(unsigned int addr, void *buf, unsigned int buf_len)
{
	return rda5981_write_flash(addr, (char *)buf, buf_len);
}

int vg_read_flash(unsigned int addr, void *buf, unsigned int buf_len)
{
	return rda5981_read_flash(addr, (char *)buf, buf_len);
}

