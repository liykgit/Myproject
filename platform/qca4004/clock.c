
#include <sys/time.h>
#include <string.h>

unsigned long vg_get_tick(void)
{
    return time_ms();
}

