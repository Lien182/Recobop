#include "a9timer.h"
#include "memory.h"

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)
#define MAP_BASEADR 0xF8F00200

t_a9timer * a9timer_init( void )
{
	t_a9timer * timer;

	timer = (t_a9timer *)mmap(0, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, MAP_BASEADR & ~MAP_MASK );
	if (timer == MAP_FAILED) {
		printf("Timer Init: Could not map memory\n");
		return NULL;
	}

    timer = (t_a9timer*)((void*)timer + (MAP_BASEADR & MAP_MASK));
	return timer;
}


uint32_t a9timer_getvalue(t_a9timer * a9timer)
{
    return a9timer->TMR_CNT_REG_L;
}

void     a9timer_caputure(t_a9timer * a9timer, t_a9timer_capture * capture, uint32_t mode )
{
    if(mode == A9TIMER_CAPTURE_START)
        capture->tStart = a9timer_getvalue(a9timer);
    else
        capture->tStop  = a9timer_getvalue(a9timer);
};