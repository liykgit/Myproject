
#include <sys/time.h>
#include <string.h>

unsigned long get_tick(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return (tv.tv_sec * 1000 + (tv.tv_usec / 1000));
}

