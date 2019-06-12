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


#define MAX_ANGLE 60
#define SLEEP 20000

#define BOP_0_TOUCH_BASE_ADDR 0x43C10000
#define BOP_0_SERVO_BASE_ADDR 0x43C00000
#define BOP_1_TOUCH_BASE_ADDR 0x43C30000
#define BOP_1_SERVO_BASE_ADDR 0x43C70000
#define BOP_2_TOUCH_BASE_ADDR 0x43C50000
#define BOP_2_SERVO_BASE_ADDR 0x43C60000

#define AXI_TIMER_0_ADDR	0x42800000




volatile struct recobop_info rb_info[3];

volatile t_video_info video_info;


inline double radians(double deg) {
	return deg * (M_PI / 180.0);
}

static void exit_signal(int sig) {

/*
	munmap(rb_info[0].pServo,0x10000);
	munmap(rb_info[1].pServo,0x10000);
	munmap(rb_info[2].pServo,0x10000);
	munmap(rb_info[0].pTouch,0x10000);
	munmap(rb_info[1].pTouch,0x10000);
	munmap(rb_info[2].pTouch,0x10000);

*/
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

	t_axi_timer * axi_timer_0;
	t_axi_timer * axi_timer_demonstrator_0;
	t_diff_measurement * diff_timer_mailbox_0;
	t_log log_demonstrator_0;
	t_log log_mailbox_0;
	t_log log_a9timertest;
	t_log log_hw_inverse;

	


	printf("Hello World\n");
	printf("int is %d and float is %d\n", sizeof(int), sizeof(float));

	reconos_init();
	reconos_app_init();

	if(memory_init() < 0)
	{
		printf("Error while allocating memory \n");
		return -1;
	}

	a9timer = a9timer_init();

	axi_timer_0 = axi_timer_init(0x42800000);
	axi_timer_start(axi_timer_0, TIMER_AXI_START_CHANNEL_0);
	
	axi_timer_demonstrator_0 = axi_timer_init(0x42810000);
	log_init(&log_demonstrator_0, (void*)axi_timer_demonstrator_0, LOG_CHANNEL_1, LOG_MODE_STDOUT | LOG_MODE_FILE | LOG_MODE_AXI_CHANNEL, "demonstrator_0.csv", 0.00001, "ms", 1000);
	axi_timer_start(axi_timer_demonstrator_0, TIMER_AXI_START_CHANNEL_1);

	diff_timer_mailbox_0 = diff_timer_init(0x43C80000);
	log_init(&log_mailbox_0, (void*)diff_timer_mailbox_0, LOG_CHANNEL_0, LOG_MODE_STDOUT | LOG_MODE_FILE | LOG_MODE_DIFFERENCE_UNIT, "mailbox_0.csv", 0.00001, "ms", 1000);
	
	log_init(&log_a9timertest, (void*)a9timer, LOG_CHANNEL_0, LOG_MODE_A9TIMER_CHANNEL | LOG_MODE_FILE | LOG_MODE_STDOUT, "a9test.csv", 0.00000333, "ms", 1000);
	
	log_init(&log_sw_control, (void*)a9timer, LOG_CHANNEL_0, LOG_MODE_A9TIMER_DIFFERENCE | LOG_MODE_FILE | LOG_MODE_STDOUT, "sw_control.csv", 0.00000333, "ms", 1000);
	log_init(&log_sw_inverse, (void*)a9timer, LOG_CHANNEL_0, LOG_MODE_A9TIMER_DIFFERENCE | LOG_MODE_FILE | LOG_MODE_STDOUT, "sw_inverse.csv", 0.00000333, "ms", 1);
	
	log_init(&log_hw_inverse, (void*)diff_timer_mailbox_0, LOG_CHANNEL_1, LOG_MODE_STDOUT | LOG_MODE_FILE | LOG_MODE_DIFFERENCE_UNIT, "hw_inverse.csv", 0.00001, "ms", 1000);


	
	
	if(hdmi_output_init(&(video_info.hdmi_output), "/dev/fb0") != 0)
	{
		printf("HDMI Output: Init error \n");
	}

	if(hdmi_input_init(&(video_info.hdmi_input), "/dev/video0", video_cmd) != 0)
	{
		printf("HDMI Output: Init error \n");
	}
	

	rb_info[0].pServo = (uint32_t*)servo_init(memfd, BOP_0_SERVO_BASE_ADDR);
	rb_info[0].pTouch = (uint32_t*)touch_init(memfd, BOP_0_TOUCH_BASE_ADDR);
	rb_info[0].demo_nr = 0UL;
	rb_info[1].pServo = (uint32_t*)servo_init(memfd, BOP_1_SERVO_BASE_ADDR);
	rb_info[1].pTouch = (uint32_t*)touch_init(memfd, BOP_1_TOUCH_BASE_ADDR);
	rb_info[1].demo_nr = 1UL;
	rb_info[2].pServo = (uint32_t*)servo_init(memfd, BOP_2_SERVO_BASE_ADDR);
	rb_info[2].pTouch = (uint32_t*)touch_init(memfd, BOP_2_TOUCH_BASE_ADDR);
	rb_info[2].demo_nr = 2UL;
	rb_info[0].timerregister = &(axi_timer_0->TCR0);
	rb_info[1].timerregister = &(axi_timer_0->TCR0);
	rb_info[2].timerregister = &(axi_timer_0->TCR0);


	for(i = 0; i < 3; i++)
	{
		printf("Init data: %x \n", &(rb_info[i]));
		printf("Servo %x, Touch %x \n", rb_info[i].pServo, rb_info[i].pTouch);
		printf("Init Servo data: 0: %x, 1: %x \n",((uint32_t*)rb_info[i].pServo)[0] & 0x0fff,((uint32_t*)rb_info[i].pServo)[1] & 0x0fff);
		printf("Init Touch data: 0: %x, 1: %x \n",((uint32_t*)rb_info[i].pTouch)[0] & 0x0fff,((uint32_t*)rb_info[i].pTouch)[1] & 0x0fff);
	}


	signal(SIGINT, exit_signal);
	signal(SIGTERM, exit_signal);
	signal(SIGABRT, exit_signal);

	printf("Initializing Info\n");
	sleep(1);
	for(i = 0; i < 3; i++)
	{
		printf("Init Data on %x \n", (void *)&(rb_info[i]));
		
		rb_info[i].thread_p[1] = reconos_thread_create_hwt_servo(  (void *)&(rb_info[i]));
		rb_info[i].thread_p[2] = reconos_thread_create_swt_control((void *)&(rb_info[i]));
		rb_info[i].thread_p[3] = reconos_thread_create_hwt_inverse((void *)&(rb_info[i]));

		

	
		//rb_info[i].thread_p[4] = reconos_thread_create_hwt_touch(  (void *)&(rb_info[i]));

	}
	video_info.thread_p = reconos_thread_create_swt_video((void*)&video_info);


		
		
#if 0
	int sum = 1000;
	float overhead;
	double touch_sum = 0, control_sum = 0, inverse_sum = 0, overhead_sum = 0, power_sum;
	for (i = 0; i < sum; i++) {
		touch_sum += rbi_perf_touch(&rb_info);
		control_sum += rbi_perf_control(&rb_info);
		inverse_sum += rbi_perf_inverse(&rb_info);
		power_sum += rbi_saw_power(&rb_info);
		while ((overhead = rbi_perf_overhead(&rb_info)) > 10);
		overhead_sum += overhead;
		if (i % 100 == 0)
			printf("%d\n", i);
		usleep(20000);
	}

	printf("Evaluation:\n");
	printf("  Touch: %f\n", touch_sum / sum);
	printf("  Control: %f\n", control_sum / sum);t_diff_measurement * diff_measurement_timer_init( uint32_t base_addr )
	printf("  Inverse: %f\n", inverse_sum / sum);
	printf("  Overhead: %f\n", overhead_sum / sum);
	printf("  Power: %f\n", power_sum / sum);
#endif

#if 0
	while (1) {
		printf("%f\n", rbi_saw_power(&rb_info));
		usleep(50000);
	}
#endif

#if 0
	printf("Resetting platform ...\n");
	for (i = 0; i < 6; i++)
		mbox_put(inverse_cmd, 0 | 0 << 22 | 0 << 12 | 0 << 3 | i << 0);
	sleep(1);
#endif

#if 0
	printf("Testing touch controller ...\n");
	while(1) {
		pos = mbox_get(touch_pos);
		pos_x = (pos >> 12) & 0xfff;
		if (((pos_x >> 11) & 0x1) == 1) {
			pos_x |= m << 12;
		}
		pos_y = (pos >> 0) & 0xfff;
		if (((pos_y >> 11) & 0x1) == 1) {
			pos_y |= m << 12;
		}
		printf("touch position is %d,%d\n", pos_x, pos_y);
		mbox_get(touch_pos);
	}
#endif

	for(i = 0; i < 3; i++)
	{
		printf("Test of nr: %d \n", *((uint32_t*)(&rb_info[i])+48/4));
	}



#if 1
	printf("Testing control algorithm ...\n");
	for (a = 0; a < 10000000; a+=1) {
		x = cos(a * M_PI / 180) * 1000;
		y = sin(a * M_PI / 180) * 1000;
		
		printf("Mailbox 0\n");
		mbox_put(touch_0_pos, 0 | (x & 0xfff) << 12 | (y & 0xfff) << 0);
		printf("Mailbox 1\n");
		mbox_put(touch_1_pos, 0 | (x & 0xfff) << 12 | (y & 0xfff) << 0);
		printf("Mailbox 2\n");
		mbox_put(touch_2_pos, 0 | (x & 0xfff) << 12 | (y & 0xfff) << 0);
		
		//printf("0: Servo data: 0: %x, 1: %x \n",((uint32_t*)rb_info[0].pServo)[0] & 0x0fff,((uint32_t*)rb_info[0].pServo)[1] & 0x0fff);
		
		
		//mbox_put(touch_pos, 1000000);
		usleep(30000);
		a9timer_caputure(a9timer, &(log_a9timertest.a9timer_capture), A9TIMER_CAPTURE_SINGLE);

		printf("%x %x %x %x %x \n",diff_timer_mailbox_0->CNT_REG, diff_timer_mailbox_0->CAP0, diff_timer_mailbox_0->CAP1, diff_timer_mailbox_0->CAP2, diff_timer_mailbox_0->CAP3);
	}
#endif

	while(1) {
		//printf("Touch data: 0: %x, 1: %x \n",((uint32_t*)rb_info[0].pTouch)[0] & 0x0fff,((uint32_t*)rb_info[0].pTouch)[1] & 0x0fff);
		//sleep(1000000000);
		
		printf("CNT REG 0: %x, CNT REG 1: %x \n", diff_timer_mailbox_0->CNT_REG, diff_timer_mailbox_0->CAP0);
		sleep(1);
	}

	reconos_cleanup();
	reconos_app_cleanup();
}
