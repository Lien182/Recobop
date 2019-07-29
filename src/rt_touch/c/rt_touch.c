#include "recobop.h"
#include "reconos.h"
#include "reconos_thread.h"
#include "reconos_calls.h"
#include "utils.h"
#include "cycle_timer.h"
#include <math.h>
#include <stdio.h>

THREAD_ENTRY() {
	struct recobop_info *rb_info;
	int i;
	struct reconos_thread * rt;
	uint32_t x_pos, y_pos;

	THREAD_INIT();
	rt = (struct reconos_thread *)GET_INIT_DATA();
	rb_info = (struct recobop_info*)rt->init_data;

	printf("Hello from touch thread of demonstrator %d \n", rb_info->demo_nr );

	
	while (1) {
		
		cycle_timer_wait(&cycle_timer);

		x_pos = ((int32_t*)rb_info->pTouch)[0] & 0x0fff;	
		if(x_pos & 0x0800) x_pos |= 0xfffff000;
		y_pos = ((int32_t*)rb_info->pTouch)[1] & 0x0fff;
		if(y_pos & 0x0800) y_pos |= 0xfffff000;

		printf("AXI: X: %08x, Y: %08x;\n", x_pos, y_pos); 


		switch(rb_info->demo_nr)
		{
			case 0: mbox_put(touch_0_pos, 0 | ((x_pos & 0xfff) << 12) | ((y_pos & 0xfff) << 0));
			case 1: mbox_put(touch_1_pos, 0 | ((x_pos & 0xfff) << 12) | ((y_pos & 0xfff) << 0));
			case 2: mbox_put(touch_2_pos, 0 | ((x_pos & 0xfff) << 12) | ((y_pos & 0xfff) << 0));
		}
	}
}