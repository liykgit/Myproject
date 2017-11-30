
#include "plat.h" 

extern u32_t os_time;

unsigned long vg_get_tick(void)
{
    return os_time;
}

