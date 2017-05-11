
#include <sys/time.h>
#include "plat.h"

unsigned long vg_get_tick(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return (tv.tv_sec * 1000 + (tv.tv_usec / 1000));
}

