#include "recobop.h"
#include "reconos.h"
#include "reconos_thread.h"
#include "reconos_calls.h"
#include "utils.h"
#include "cycle_timer.h"
#include <math.h>
#include <stdio.h>

#define DEBUG 1

THREAD_ENTRY() {
	struct recobop_info *rb_info;
	int i;
	struct reconos_thread * rt;
	uint32_t x_pos, y_pos;

	uint32_t cmd = 0;
	
#if DEBUG == 1
	double dd = 0.0;
#endif


	THREAD_INIT();
	rt = (struct reconos_thread *)GET_INIT_DATA();
	rb_info = (struct recobop_info*)rt->init_data;

	while (1) {
		
		switch(rb_info->demo_nr)
		{
			case 0: cmd = mbox_get(servo_0_cmd);break;
			case 1: cmd = mbox_get(servo_1_cmd);break;
			case 2: cmd = mbox_get(servo_2_cmd);break;
		}

		printf("[rt_servo %d] Got new data from inverse\n",rb_info->demo_nr);

		((uint32_t*)(rb_info->pServo))[(cmd >> 18) & 7] = cmd >> 21;


	}
}