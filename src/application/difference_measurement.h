#ifndef DIFFERENCE_MEASUREMENT_H
#define DIFFERENCE_MEASUREMENT_H

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


typedef struct
{
	volatile uint32_t	CONFIG;
	volatile uint32_t 	CNT_REG;
	volatile uint32_t	CAP0;
	volatile uint32_t 	CAP1;
	volatile uint32_t	CAP2;
	volatile uint32_t	CAP3;

}t_diff_measurement;



t_diff_measurement * diff_measurement_timer_init( uint32_t base_addr    );
void diff_measurement_timer_start(t_diff_measurement * diff_measurement );
void diff_measurement_timer_stop(t_diff_measurement * diff_measurement  );

#endif