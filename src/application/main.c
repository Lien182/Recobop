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

#define TOUCH_BASE_ADDR 0x43C10000
#define SERVO_BASE_ADDR 0x43C00000

volatile struct recobop_info rb_info;

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
	uint32_t * touch;

	touch = (uint32_t *)mmap(0, 0x10000, PROT_READ, MAP_SHARED, *fd, base_addr);
	if (touch == MAP_FAILED) {
		printf("ERROR: Could not map memory\n");
		return NULL;
	}

	return touch;
}

uint32_t * servo_init(int * fd, uint32_t base_addr)
{
	uint32_t * servo;

	servo = (uint32_t *)mmap(0, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, base_addr);
	if (servo == MAP_FAILED) {
		printf("ERROR: Could not map memory\n");
		return NULL;
	}

	return servo;
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
	printf("Hello World\n");
	printf("int is %d and float is %d\n", sizeof(int), sizeof(float));

	reconos_init();
	reconos_app_init();

	if(memory_init(&fd) < 0)
	{
		printf("Error while allocating memory \n");
		return -1;
	}
	rb_info.pServo = servo_init(&fd, SERVO_BASE_ADDR);
	rb_info.pTouch = touch_init(&fd, TOUCH_BASE_ADDR);

	printf("pServo: %x, pTouch: %x \n", rb_info.pServo, rb_info.pTouch);

	signal(SIGINT, exit_signal);
	signal(SIGTERM, exit_signal);
	signal(SIGABRT, exit_signal);

	printf("Initializing Info\n");
	rb_info.ctrl_touch_wait = 1000000;

	//rb_info.thread_p[0] = reconos_thread_create_hwt_performance((void *)&rb_info);
	rb_info.thread_p[1] = reconos_thread_create_hwt_servo((void *)&rb_info);
	rb_info.thread_p[2] = reconos_thread_create_swt_control((void *)&rb_info);
	rb_info.thread_p[3] = reconos_thread_create_hwt_inverse((void *)&rb_info);
	//rb_info.thread_p[4] = reconos_thread_create_hwt_inverse((void *)&rb_info);
	//rb_info.thread_p[5] = reconos_thread_create_hwt_inverse((void *)&rb_info);
	//rb_info.thread_p[6] = reconos_thread_create_hwt_inverse((void *)&rb_info);
	//rb_info.thread_p[7] = reconos_thread_create_hwt_inverse((void *)&rb_info);
	rb_info.thread_p[8] = reconos_thread_create_hwt_touch((void *)&rb_info);
	//rb_info.thread_p[9] = reconos_thread_create_swt_power((void *)&rb_info);
	//rb_info.thread_p[10] = reconos_thread_create_swt_saw((void *)&rb_info);
	//rb_info.thread_p[11] = reconos_thread_create_swt_web((void *)&rb_info);
	//rb_info.thread_p[12] = reconos_thread_create_hwt_vga((void *)&rb_info);

	

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

#if 0
	printf("Testing control algorithm ...\n");
	for (a = 0; a < 10000000; a+=1) {
		x = cos(a * M_PI / 180) * 2000;
		y = sin(a * M_PI / 180) * 2000;
		printf("%d,%d\n", x, y);
		mbox_put(touch_pos, 0 | (x & 0xfff) << 12 | (y & 0xfff) << 0);
		mbox_put(touch_pos, 1000000);
		usleep(10000);
	}
#endif

	while(1) {
		printf("Touch data: 0: %x, 1: %x \n",((uint32_t*)rb_info.pTouch)[0],((uint32_t*)rb_info.pTouch)[1]);
		//sleep(1000000000);
		sleep(1);
	}

	reconos_cleanup();
	reconos_app_cleanup();
}
