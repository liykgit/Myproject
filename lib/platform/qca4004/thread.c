
#include "plat.h"

int vg_start_thread(thread_fun_t fun, void **stk_addr, int stk_size)
{
	//extern int qca_task_start(Thread_fun_t fn, unsigned int arg, int stk_size, int tk_ms);
	int handle = qca_task_start(fun, 0, stk_size);
	return handle;
}

