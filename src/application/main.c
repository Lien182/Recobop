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

#define MAX_ANGLE 60
#define SLEEP 20000

#define BOP_0_TOUCH_BASE_ADDR 0x43C10000
#define BOP_0_SERVO_BASE_ADDR 0x43C00000
#define BOP_1_TOUCH_BASE_ADDR 0x43C30000
#define BOP_1_SERVO_BASE_ADDR 0x43C70000
#define BOP_2_TOUCH_BASE_ADDR 0x43C50000
#define BOP_2_SERVO_BASE_ADDR 0x43C60000

#define AXI_TIMER_0_ADDR	0x42800000


//All offsets in DWORDS
#define TIMER_AXI_REG_OFFSET_TCSR0	0x00000000
#define TIMER_AXI_REG_OFFSET_TLR0	0x00000001
#define TIMER_AXI_REG_OFFSET_TCR0	0x00000002
#define TIMER_AXI_REG_OFFSET_TCSR1	0x00000004
#define TIMER_AXI_REG_OFFSET_TLR1	0x00000005
#define TIMER_AXI_REG_OFFSET_TCR1	0x00000006

#define TIMER_AXI_START_CHANNEL_0	0x00000001
#define TIMER_AXI_START_CHANNEL_1	0x00000002


typedef struct
{
	volatile uint32_t	TCSR0;
	volatile uint32_t 	TLR0;
	volatile uint32_t	TCR0;
	volatile uint32_t 	reserved_1;
	volatile uint32_t	TCSR1;
	volatile uint32_t	TLR1;
	volatile uint32_t	TCR1;
	volatile uint32_t	reserved_2;

}t_axi_timer;

volatile struct recobop_info rb_info[3];

inline double radians(double deg) {
	return deg * (M_PI / 180.0);
}

static void exit_signal(int sig) {
	reconos_cleanup();

	printf("[recobop] aborted\n");

	exit(0);
}


int memory_init(int * fd)
{
	*fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (*fd < 0) {
		printf("ERROR: Could not open /dev/mem\n");
		close(*fd);
		return -1;
	}
	return 0;
}

void memory_deinit(int * fd)
{
	close(*fd);
}


uint32_t * touch_init(int * fd, uint32_t base_addr)
{
	void * touch;

	touch = mmap(0, 0x10000, PROT_READ, MAP_SHARED, *fd, base_addr);
	if (touch == MAP_FAILED) {
		printf("ERROR: Could not map memory\n");
		return NULL;
	}

	return (uint32_t*)touch;
}

uint32_t * servo_init(int * fd, uint32_t base_addr)
{
	void * servo;

	servo = mmap(0, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, base_addr);
	if (servo == MAP_FAILED) {
		printf("ERROR: Could not map memory\n");
		return NULL;
	}

	return (uint32_t*)servo;
}

t_axi_timer * axi_timer_init(int * fd, uint32_t base_addr)
{
	t_axi_timer * axi_timer;

	axi_timer = (t_axi_timer *)mmap(0, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, base_addr);
	if (axi_timer == MAP_FAILED) {
		printf("Timer Init: Could not map memory\n");
		return NULL;
	}

	return axi_timer;
}

void axi_timer_start(t_axi_timer * axi_timer,uint32_t channel )
{
	if(channel & TIMER_AXI_START_CHANNEL_0)
	{
		//Enable timer0 and external capture trigger timer 0
		axi_timer->TCSR0 |= (1<<8) | (1<<7) | (1<<3) | (1<<4) | (1<<0);
	}

	if(channel & TIMER_AXI_START_CHANNEL_1)
	{
		//Enable timer1 and external capture trigger timer 1
		axi_timer->TCSR1 |= (1<<8) | (1<<7) | (1<<3) | (1<<4) | (1<<0);
	}

}

void axi_timer_stop(t_axi_timer * axi_timer, uint32_t channel)
{
	if(channel & TIMER_AXI_START_CHANNEL_0)
		axi_timer->TCSR0 &= ~(1<<7);
	if(channel & TIMER_AXI_START_CHANNEL_1)
		axi_timer->TCSR1 &= ~(1<<7);
	
}

typedef struct 
{
	uint32_t channel;
	uint32_t mode;
	int fd;
	char * filename;
	t_axi_timer * timer;
	uint32_t lasttimervalue;
	uint32_t bstart;
	
	uint64_t samplecnt; 

	double scale;
	char * unit;

}t_log;

#define LOG_CHANNEL_0 0 
#define LOG_CHANNEL_1 1

#define LOG_MODE_STDOUT 1
#define LOG_MODE_FILE	2 



uint32_t log_check(t_log * log)
{
	uint32_t cnt;

	if(log->channel == LOG_CHANNEL_0)
		cnt = log->timer->TLR0;
	else
		cnt = log->timer->TLR1;
	
	if(log->bstart) // Difference makes no sense for the first sample
	{
		log->bstart = 0UL;
		log->lasttimervalue = cnt;
		log->samplecnt = 1;
		return 1;
	}

	if( cnt != log->lasttimervalue )
	{
		if(log->mode & LOG_MODE_FILE)
			fprintf(&(log->fd), "%08lld;\t %6.6f %s;\n",log->samplecnt, (double)(cnt - log->lasttimervalue)*log->scale, log->unit);
		
		if(log->mode & LOG_MODE_STDOUT)
			printf("%s, %08lld;\t %6.6f %s;\n",log->filename, log->samplecnt, (double)(cnt - log->lasttimervalue)*log->scale, log->unit);
		
		log->lasttimervalue = cnt;
		log->samplecnt++;

		return 1;
	}
	return 0;
}

void log_deinit(t_log * log)
{
	close(&(log->fd));
}

void log_init(t_log * log, t_axi_timer* timer, uint32_t channel, uint32_t mode, char * filename, double scale, char * unit)
{
	log->timer = timer;


	log->fd = open(filename, "O_RDWR");
	if(log->fd == NULL)
	{
		printf("LOG_INIT: Open %s failed \n", filename);
	}

	log->bstart = 1UL;
	log->lasttimervalue = 0;
	log->scale = scale;
	log->samplecnt = 0;
	log->channel = channel;
	log->mode = mode;
	log->unit = unit;
	log->filename = filename;

	log_check(log);

	return;
}


int main(int argc, char **argv) {
	int fd;
	int a, i;
	int x, y;
	uint32_t s, c, pos;
	uint32_t m = 0xffffffff;
	int pos_x, pos_y;
	char line[128];
	struct reconos_thread *rt;


	t_axi_timer * axi_timer_0;


	printf("Hello World\n");
	printf("int is %d and float is %d\n", sizeof(int), sizeof(float));

	reconos_init();
	reconos_app_init();

	if(memory_init(&fd) < 0)
	{
		printf("Error while allocating memory \n");
		return -1;
	}

	axi_timer_0 = axi_timer_init(&fd, 0x42800000);
	axi_timer_start(axi_timer_0, TIMER_AXI_START_CHANNEL_0);

	rb_info[0].pServo = (uint32_t)servo_init(&fd, BOP_0_SERVO_BASE_ADDR);
	rb_info[0].pTouch = (uint32_t)touch_init(&fd, BOP_0_TOUCH_BASE_ADDR);
	rb_info[0].demo_nr = 0;
	rb_info[1].pServo = (uint32_t)servo_init(&fd, BOP_1_SERVO_BASE_ADDR);
	rb_info[1].pTouch = (uint32_t)touch_init(&fd, BOP_1_TOUCH_BASE_ADDR);
	rb_info[1].demo_nr = 1;
	rb_info[2].pServo = (uint32_t)servo_init(&fd, BOP_2_SERVO_BASE_ADDR);
	rb_info[2].pTouch = (uint32_t)touch_init(&fd, BOP_2_TOUCH_BASE_ADDR);
	rb_info[2].demo_nr = 2;
	rb_info[0].timerregister = &(axi_timer_0->TCR0);
	rb_info[1].timerregister = &(axi_timer_0->TCR0);
	rb_info[2].timerregister = &(axi_timer_0->TCR0);


	for(i = 0; i < 3; i++)
	{
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
		rb_info[i].ctrl_touch_wait = 1000000;
		//rb_info[i].thread_p[1] = reconos_thread_create_hwt_servo(  (void *)&(rb_info[i]));
		rb_info[i].thread_p[2] = reconos_thread_create_swt_control((void *)&(rb_info[i]));
		rb_info[i].thread_p[3] = reconos_thread_create_swt_inverse((void *)&(rb_info[i]));
		//rb_info[i].thread_p[4] = reconos_thread_create_hwt_touch(  (void *)&rb_info[i]);

	}

	
	
	

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
	printf("  Control: %f\n", control_sum / sum);
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
	}
#endif

	while(1) {
		printf("Touch data: 0: %x, 1: %x \n",((uint32_t*)rb_info[0].pTouch)[0] & 0x0fff,((uint32_t*)rb_info[0].pTouch)[1] & 0x0fff);
		//sleep(1000000000);
		sleep(1);
	}

	reconos_cleanup();
	reconos_app_cleanup();
}
