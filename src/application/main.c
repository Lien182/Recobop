#include "recobop.h"

#include "reconos.h"
#include "reconos_app.h"
#include "mbox.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <signal.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>

#include "log.h"
#include "axi_timer.h"
#include "a9timer.h"
#include "difference_measurement.h"
#include "memory.h"
#include "cycle_timer.h"
#include "axi_touch.h"
#include "axi_servo.h"

#include "reconfig.h"
#include "hdmi.h"

#define MAX_ANGLE 60
#define SLEEP 20000

#define BOP_0_TOUCH_BASE_ADDR 0x43C10000
#define BOP_0_SERVO_BASE_ADDR 0x43C00000
#define BOP_1_TOUCH_BASE_ADDR 0x43C30000
#define BOP_1_SERVO_BASE_ADDR 0x43C70000
#define BOP_2_TOUCH_BASE_ADDR 0x43C50000
#define BOP_2_SERVO_BASE_ADDR 0x43C60000

#define AXI_TIMER_0_ADDR	0x42800000
#define AXI_TIMER_1_ADDR	0x42810000


char * bitstream_control_0[1] = {"control_0_slot_0.bit"};
char * bitstream_inverse_0[1] = {"inverse_0_slot_1.bit"};
char * bitstream_control_1[1] = {"control_1_slot_0.bit"};
char * bitstream_inverse_1[1] = {"inverse_1_slot_1.bit"};
char * bitstream_control_2[1] = {"control_2_slot_0.bit"};
char * bitstream_inverse_2[1] = {"inverse_2_slot_1.bit"};




volatile struct recobop_info rb_info[3];

t_hdmi video_info;


inline double radians(double deg) {
	return deg * (M_PI / 180.0);
}

static void exit_signal(int sig) 
{
	reconos_cleanup();
	printf("[recobop] aborted\n");
	exit(0);
}


int main(int argc, char **argv) {
	int i;

	reconos_init();
	reconos_app_init();

	if(memory_init() < 0)
	{
		printf("Error while allocating memory \n");
		return -1;
	}

	cycle_timer_init(&cycle_timer, 20, cycle_timer_cmd_mutex, cycle_timer_cmd_cond);


	if(hdmi_output_init(&(video_info.hdmi_output), "/dev/fb0") != 0)
	{
		printf("HDMI Output: Init error \n");
	}

	if(hdmi_input_init(&(video_info.hdmi_input), "/dev/video0", video_cmd) != 0)
	{
		printf("HDMI Output: Init error \n");
	}

	a9timer = a9timer_init();


	rb_info[0].timerregister = &(a9timer->TMR_CNT_REG_L);
	rb_info[0].rc_flag_control = 0UL;
	rb_info[0].rc_flag_inverse = 0UL;
	rb_info[0].pServo = (uint32_t*)axi_servo_init(BOP_0_SERVO_BASE_ADDR);
	rb_info[0].pTouch = (uint32_t*)axi_touch_init( BOP_0_TOUCH_BASE_ADDR);
	rb_info[0].demo_nr = 0UL;
	rb_info[0].stackaddr_control = (uint32_t*)malloc(50 * sizeof(uint32_t));
	rb_info[0].threadid_control = 0;
	rb_info[0].stackaddr_inverse = (uint32_t*)malloc(50 * sizeof(uint32_t));
	rb_info[0].threadid_inverse = 3;
	rb_info[0].rc_flag_touch = 0UL;
	rb_info[0].rc_flag_servo = 0UL;
	
	rb_info[1].timerregister = &(a9timer->TMR_CNT_REG_L);
	rb_info[1].rc_flag_control = 0UL;
	rb_info[1].rc_flag_inverse = 0UL;
	rb_info[1].pServo = (uint32_t*)axi_servo_init(BOP_1_SERVO_BASE_ADDR);
	rb_info[1].pTouch = (uint32_t*)axi_touch_init(BOP_1_TOUCH_BASE_ADDR);
	rb_info[1].demo_nr = 1UL;
	rb_info[1].stackaddr_control = (uint32_t*)malloc(50 * sizeof(uint32_t));
	rb_info[1].threadid_control = 1;
	rb_info[1].stackaddr_inverse = (uint32_t*)malloc(50 * sizeof(uint32_t));
	rb_info[1].threadid_inverse = 4;
	rb_info[1].rc_flag_touch = 0UL;
	rb_info[1].rc_flag_servo = 0UL;

	rb_info[2].timerregister = &(a9timer->TMR_CNT_REG_L);
	rb_info[2].rc_flag_control = 0UL;
	rb_info[2].rc_flag_inverse = 0UL;
	rb_info[2].pServo = (uint32_t*)axi_servo_init(BOP_2_SERVO_BASE_ADDR);
	rb_info[2].pTouch = (uint32_t*)axi_touch_init(BOP_2_TOUCH_BASE_ADDR);
	rb_info[2].demo_nr = 2UL;
	rb_info[2].stackaddr_control = (uint32_t*)malloc(50 * sizeof(uint32_t));
	rb_info[2].threadid_control = 2;
	rb_info[2].stackaddr_inverse = (uint32_t*)malloc(50 * sizeof(uint32_t));
	rb_info[2].threadid_inverse = 5;
	rb_info[2].rc_flag_touch = 0UL;
	rb_info[2].rc_flag_servo = 0UL;


	signal(SIGINT, exit_signal);
	signal(SIGTERM, exit_signal);
	signal(SIGABRT, exit_signal);	
	
	for(i = 0; i < 3; i++)
	{
		printf("Init Data on %x \n", (uint32_t)&(rb_info[i]));
		printf("Servo %x, Touch %x \n", (uint32_t)rb_info[i].pServo, (uint32_t)rb_info[i].pTouch);
		printf("Init Servo data: 0: %x, 1: %x \n",((uint32_t*)rb_info[i].pServo)[0] & 0x0fff,((uint32_t*)rb_info[i].pServo)[1] & 0x0fff);
		printf("Init Touch data: 0: %x, 1: %x \n",((uint32_t*)rb_info[i].pTouch)[0] & 0x0fff,((uint32_t*)rb_info[i].pTouch)[1] & 0x0fff);
	
		rb_info[i].thread_p[0] = reconos_thread_create_swt_touch    ((void *)&(rb_info[2]), 74);	
		rb_info[i].thread_p[1] = reconos_thread_create_swt_control  ((void *)&(rb_info[2]), 73);
		rb_info[i].thread_p[2] = reconos_thread_create_swt_inverse  ((void *)&(rb_info[2]), 72);
		rb_info[i].thread_p[3] = reconos_thread_create_swt_servo    ((void *)&(rb_info[2]), 71);
	}

	video_info.thread_p = reconos_thread_create_hwt_color2bw((void*)(&(video_info.rc_flag)));


	while(1) 
	{
		sleep(100);
	}

	reconos_cleanup();
	reconos_app_cleanup();
}
