#include "difference_measurement.h"
#include "memory.h"

t_diff_measurement * diff_timer_init( uint32_t base_addr )
{
	t_diff_measurement * diff_timer;

	diff_timer = (t_diff_measurement *)mmap(0, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, base_addr);
	if (diff_timer == MAP_FAILED) {
		printf("Timer Init: Could not map memory\n");
		return NULL;
	}

	return diff_timer;
}