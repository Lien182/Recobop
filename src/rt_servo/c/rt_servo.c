/********************************************************************          
* rt_servo.c  -- software servo thread implementation      			*
*                                                                   *
*                                                                   *  
* Author(s):  Christian Lienen                                      *   
*                                                                   *   
********************************************************************/

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
	struct reconos_thread * rt;
	uint32_t cmd = 0;


	THREAD_INIT();
	rt = (struct reconos_thread *)GET_INIT_DATA();
	rb_info = (struct recobop_info*)rt->init_data;

	while (1) {
		
		switch(rb_info->demo_nr)
		{
			case 0: cmd = mbox_get(servo_0_cmd); break;
			case 1: cmd = mbox_get(servo_1_cmd); break;
			case 2: cmd = mbox_get(servo_2_cmd); break;
		}
#if DEBUG == 1
		printf("[rt_servo %d] Got new data from inverse\n",rb_info->demo_nr);
#endif
		((uint32_t*)(rb_info->pServo))[(cmd >> 18) & 7] = cmd >> 21;


	}
}