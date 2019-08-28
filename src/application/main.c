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

#include "reconfig.h"


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

volatile t_video_info video_info;


inline double radians(double deg) {
	return deg * (M_PI / 180.0);
}

static void exit_signal(int sig) 
{
	reconos_cleanup();
	printf("[recobop] aborted\n");
	exit(0);
}



uint32_t * touch_init(int fd, uint32_t base_addr)
{
	void * touch;

	touch = mmap(0, 0x10000, PROT_READ| PROT_WRITE, MAP_SHARED | MAP_POPULATE, fd, base_addr);
	if (touch == MAP_FAILED) {
		printf("ERROR: Could not map memory\n");
		return NULL;
	}

	return (uint32_t*)touch;
}

uint32_t * servo_init(int fd, uint32_t base_addr)
{
	void * servo;

	servo = mmap(0, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, fd, base_addr);
	if (servo == MAP_FAILED) {
		printf("ERROR: Could not map memory\n");
		return NULL;
	}

	return (uint32_t*)servo;
}


int main(int argc, char **argv) {
	int a, i;
	int x, y;
	uint32_t s, c, pos;
	uint32_t m = 0xffffffff;
	int pos_x, pos_y;
	char line[128];
	struct reconos_thread *rt;

	struct hwslot * hw;

	uint32_t control_stack[20];

	t_axi_timer * axi_timer_0;
	t_axi_timer * axi_timer_1;
	t_axi_timer * axi_timer_demonstrator_0;
	t_diff_measurement * diff_timer_mailbox_0;
	t_log log_demonstrator_0;
	t_log log_mailbox_0;
	t_log log_a9timertest;
	t_log log_hw_inverse;
	t_log log_hw_control;
	t_log log_touch_ctrl;

	int32_t touch_x_old , touch_x;

	int x_pos, y_pos;
	uint32_t cyclecnt = 0;

	uint32_t data;

	float alpha, beta;

	t_reconfig_scheduler reconfig_scheduler;

	reconos_init();
	reconos_app_init();

	if(memory_init() < 0)
	{
		printf("Error while allocating memory \n");
		return -1;
	}

	cycle_timer_init(&cycle_timer, 20, cycle_timer_cmd_mutex, cycle_timer_cmd_cond);

	//Init Measurement HW
	a9timer = a9timer_init();
	diff_timer_mailbox_0 = diff_timer_init(0x43C80000);
	axi_timer_0 = axi_timer_init(AXI_TIMER_0_ADDR);
	axi_timer_1 = axi_timer_init(AXI_TIMER_1_ADDR);

/*
	
	axi_timer_start(axi_timer_0, TIMER_AXI_START_CHANNEL_0);
	
	axi_timer_demonstrator_0 = axi_timer_init(0x42810000);
	log_init(&log_demonstrator_0, (void*)axi_timer_demonstrator_0, LOG_CHANNEL_1, LOG_MODE_STDOUT | LOG_MODE_FILE | LOG_MODE_AXI_CHANNEL, "demonstrator_0.csv", 0.00001, "ms", 1000);
	axi_timer_start(axi_timer_demonstrator_0, TIMER_AXI_START_CHANNEL_1);

	

	log_init(&log_mailbox_0, (void*)diff_timer_mailbox_0, LOG_CHANNEL_0, LOG_MODE_STDOUT | LOG_MODE_FILE | LOG_MODE_DIFFERENCE_UNIT, "mailbox_0.csv", 0.00001, "ms", 1000);
	
	log_init(&log_a9timertest, (void*)a9timer, LOG_CHANNEL_0, LOG_MODE_A9TIMER_CHANNEL | LOG_MODE_FILE | LOG_MODE_STDOUT, "a9test.csv", 0.00000333, "ms", 1000);
*/	
	//log_init(&log_sw_control, (void*)a9timer, LOG_CHANNEL_0, LOG_MODE_A9TIMER_CHANNEL | LOG_MODE_FILE | LOG_MODE_STDOUT, "sw_control.csv", 0.00000333, "ms", 100);
	//log_init(&log_sw_inverse, (void*)a9timer, LOG_CHANNEL_0, LOG_MODE_A9TIMER_CHANNEL | LOG_MODE_FILE | LOG_MODE_STDOUT, "sw_inverse.csv", 0.00000333, "ms", 100);
	
	//log_init(&log_touch_ctrl, (void*)a9timer, LOG_CHANNEL_0, LOG_MODE_A9TIMER_CHANNEL | LOG_MODE_FILE | LOG_MODE_STDOUT, "log_touch_ctrl.csv", 0.00000333, "ms", 10);
	

	//log_init(&log_hw_inverse, (void*)diff_timer_mailbox_0, LOG_CHANNEL_0, LOG_MODE_STDOUT | LOG_MODE_FILE | LOG_MODE_DIFFERENCE_UNIT, "hw_inverse.csv", 0.00001, "ms", 100);
	//log_init(&log_hw_control, (void*)diff_timer_mailbox_0, LOG_CHANNEL_1, LOG_MODE_STDOUT | LOG_MODE_FILE | LOG_MODE_DIFFERENCE_UNIT, "hw_control.csv", 0.00001, "ms", 100);
	//log_init(&log_sw_control, (void*)a9timer, LOG_CHANNEL_0, LOG_MODE_A9TIMER_DIFFERENCE | LOG_MODE_FILE | LOG_MODE_STDOUT, "sw_control.csv", 0.00000333, "ms", 1000);
	//log_init(&log_sw_video, (void*)a9timer, LOG_CHANNEL_0, LOG_MODE_STDOUT | LOG_MODE_FILE | LOG_MODE_A9TIMER_DIFFERENCE, 	"sw_video.csv"	, 0.00000333, "ms", 100);


	
	
	if(hdmi_output_init(&(video_info.hdmi_output), "/dev/fb0") != 0)
	{
		printf("HDMI Output: Init error \n");
	}

	if(hdmi_input_init(&(video_info.hdmi_input), "/dev/video0", video_cmd) != 0)
	{
		printf("HDMI Output: Init error \n");
	}
	

	

	axi_timer_start(axi_timer_1, TIMER_AXI_START_CHANNEL_0, TIMER_AXI_MODE_GENERATE, 3000000);

	rb_info[0].timerregister = &(axi_timer_0->TCR0);
	rb_info[0].rc_flag_control = 0UL;
	rb_info[0].rc_flag_inverse = 0UL;
	rb_info[0].pServo = (uint32_t*)servo_init(memfd, BOP_0_SERVO_BASE_ADDR);
	rb_info[0].pTouch = (uint32_t*)touch_init(memfd, BOP_0_TOUCH_BASE_ADDR);
	rb_info[0].demo_nr = 0UL;
	rb_info[2].stackaddr_control = (uint32_t)malloc(50 * sizeof(uint32_t));
	rb_info[2].threadid_control = 0;
	rb_info[2].stackaddr_inverse = (uint32_t)malloc(50 * sizeof(uint32_t));
	rb_info[2].threadid_inverse = 3;
	
	rb_info[1].timerregister = &(axi_timer_0->TCR0);
	rb_info[1].rc_flag_control = 0UL;
	rb_info[1].rc_flag_inverse = 0UL;
	rb_info[1].pServo = (uint32_t*)servo_init(memfd, BOP_1_SERVO_BASE_ADDR);
	rb_info[1].pTouch = (uint32_t*)touch_init(memfd, BOP_1_TOUCH_BASE_ADDR);
	rb_info[1].demo_nr = 1UL;
	rb_info[1].stackaddr_control = (uint32_t)malloc(50 * sizeof(uint32_t));
	rb_info[1].threadid_control = 1;
	rb_info[1].stackaddr_inverse = (uint32_t)malloc(50 * sizeof(uint32_t));
	rb_info[1].threadid_inverse = 4;

	rb_info[2].timerregister = &(axi_timer_0->TCR0);
	rb_info[2].rc_flag_control = 0UL;
	rb_info[2].rc_flag_inverse = 0UL;
	rb_info[2].pServo = (uint32_t*)servo_init(memfd, BOP_2_SERVO_BASE_ADDR);
	rb_info[2].pTouch = (uint32_t*)touch_init(memfd, BOP_2_TOUCH_BASE_ADDR);
	rb_info[2].demo_nr = 2UL;
	rb_info[2].stackaddr_control = (uint32_t)malloc(50 * sizeof(uint32_t));
	rb_info[2].threadid_control = 2;
	rb_info[2].stackaddr_inverse = (uint32_t)malloc(50 * sizeof(uint32_t));
	rb_info[2].threadid_inverse = 5;

	
	for(i = 0; i < 25; i++)
	{
		rb_info[2].stackaddr_control[i] = i<<19 ;
	}


	for(i = 0; i < 3; i++)
	{
		printf("Init data: %x \n", &(rb_info[i]));
		printf("Servo %x, Touch %x \n", rb_info[i].pServo, rb_info[i].pTouch);
		printf("Init Servo data: 0: %x, 1: %x \n",((uint32_t*)rb_info[i].pServo)[0] & 0x0fff,((uint32_t*)rb_info[i].pServo)[1] & 0x0fff);
		printf("Init Touch data: 0: %x, 1: %x \n",((uint32_t*)rb_info[i].pTouch)[0] & 0x0fff,((uint32_t*)rb_info[i].pTouch)[1] & 0x0fff);
	}


	//Scheduler Init
	//reconfig_scheduler_init(&reconfig_scheduler, &cycle_timer);

	signal(SIGINT, exit_signal);
	signal(SIGTERM, exit_signal);
	signal(SIGABRT, exit_signal);

	printf("Initializing Info\n");
	
	/*
	for(i = 0; i < 3; i++)
	{
		printf("Init Data on %x \n", (void *)&(rb_info[i]));		
		rb_info[i].thread_p[1] = reconos_thread_create_swt_servo  ((void *)&(rb_info[i]), 70);
		rb_info[i].thread_p[4] = reconos_thread_create_swt_touch  ((void *)&(rb_info[i]), 70);

	}

	//rb_info[i].thread_p[2] = reconos_thread_create_swt_control((void *)&(rb_info[i]),70);
	rb_info[0].thread_p[2] = reconos_thread_create_hwt_control((void *)&(rb_info[i]));
	rb_info[0].thread_p[3] = reconos_thread_create_hwt_inverse((void *)&(rb_info[i]));

	*/

	rb_info[0].thread_p[1] = reconos_thread_create_swt_control  ((void *)&(rb_info[2]), 70);
	rb_info[0].thread_p[4] = reconos_thread_create_swt_inverse  ((void *)&(rb_info[2]), 70);


	video_info.thread_p = reconos_thread_create_hwt_color2bw((uint32_t)(&(video_info.rc_flag)));



	for(i = 0; i < 3; i++)
	{
		printf("Test of nr: %d \n", *((uint32_t*)(&rb_info[i])+8/4));
	}



#if 0
	printf("Testing control algorithm ...\n");
	for (a = 0; a < 10000000; a+=1) {
		x = cos(a * M_PI / 180) * 1000;
		y = sin(a * M_PI / 180) * 1000;
		
		printf("Mailbox 0\n");
		mbox_put(touch_0_pos, 0 | (x & 0xfff) << 12 | (y & 0xfff) << 0);
	//	printf("Mailbox 1\n");
	//	mbox_put(touch_1_pos, 0 | (x & 0xfff) << 12 | (y & 0xfff) << 0);
	//	printf("Mailbox 2\n");
	//	mbox_put(touch_2_pos, 0 | (x & 0xfff) << 12 | (y & 0xfff) << 0);
		
		//printf("0: Servo data: 0: %x, 1: %x \n",((uint32_t*)rb_info[0].pServo)[0] & 0x0fff,((uint32_t*)rb_info[0].pServo)[1] & 0x0fff);
		
		
		//mbox_put(touch_pos, 1000000);
		usleep(10000);
	//	a9timer_caputure(a9timer, &(log_a9timertest.a9timer_capture), A9TIMER_CAPTURE_SINGLE);

	//	printf("%x %x %x %x %x \n",diff_timer_mailbox_0->CNT_REG, diff_timer_mailbox_0->CAP0, diff_timer_mailbox_0->CAP1, diff_timer_mailbox_0->CAP2, diff_timer_mailbox_0->CAP3);
	}
#endif



	while(1) {

	
		x_pos = ((int32_t*)rb_info[0].pTouch)[0] & 0x0fff;
	
		if(x_pos & 0x0800)
			x_pos |= 0xfffff000;
		y_pos = ((int32_t*)rb_info[0].pTouch)[1] & 0x0fff;
		if(y_pos & 0x0800)
			y_pos |= 0xfffff000;

		cyclecnt = ((int32_t*)rb_info[2].pTouch)[0] >> 12;		
		touch_x = ((uint32_t*)rb_info[2].pTouch)[0];

		printf("main AXI: X: %x, Y: %x ;\n", x_pos, y_pos); 
/*
		//if(touch_x_old != touch_x)
			//a9timer_caputure(a9timer, &(log_touch_ctrl.a9timer_capture), A9TIMER_CAPTURE_START); 


		touch_x_old = touch_x;
		
		//printf("2 Touch data: 0: %x, 1: %x \n",((uint32_t*)rb_info[1].pTouch)[0] & 0x0fff,((uint32_t*)rb_info[1].pTouch)[1] & 0x0fff);
		//printf("3 Touch data: 0: %x, 1: %x \n",((uint32_t*)rb_info[2].pTouch)[0] & 0x0fff,((uint32_t*)rb_info[2].pTouch)[1] & 0x0fff);
		//sleep(1000000000);
		
		//printf("CNT REG 0: %x, CNT REG 1: %x \n", axi_timer_1->TCR0, axi_timer_1->TLR0);

		//pos = mbox_get(touch_2_pos);

		data = mbox_get(inverse_2_cmd); 

		alpha = fitofl((data >> 17) & 0x3fff, 14, 6);
		beta  = fitofl((data >> 3)  & 0x3fff, 14, 6);

		printf("leg: %d, a: %3.6f, b: %3.6f \n",data & 7, alpha, beta);
*/	

		sleep(1);


	}

	reconos_cleanup();
	reconos_app_cleanup();
}
