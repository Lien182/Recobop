/********************************************************************          
* log.h           -- log unit; it can check all used time sources   *
*                    for differences in the capture values and      *
*					 and writes the resulting time in stdout or 	*
*					 in a file*										*
*                                                                   *  
* Author(s):  Christian Lienen                                      *   
*                                                                   *   
********************************************************************/
#ifndef LOG_H
#define LOG_H


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



#include "axi_timer.h"
#include "difference_measurement.h"
#include "a9timer.h"

typedef struct 
{
	uint32_t sleeptime;

	uint32_t channel;
	uint32_t mode;
	FILE * fd;
	char * filename;

	uint32_t lasttimervalue;
	uint64_t samplecnt; 
	double scale;
	char * unit;
	pthread_t thread;
    uint32_t bStart;
	uint32_t bStop;

	//Different time source
	t_a9timer * a9timer;
	t_a9timer_capture a9timer_capture;
	t_axi_timer * timer;
	t_diff_measurement * diff_measurement;


}t_log;

#define LOG_CHANNEL_0 		0 
#define LOG_CHANNEL_1 		1
#define LOG_CHANNEL_2		2
#define LOG_CHANNEL_3		3

#define LOG_MODE_STDOUT 	1
#define LOG_MODE_FILE		2 

#define LOG_MODE_AXI_CHANNEL 			0x00000044
#define LOG_MODE_DIFFERENCE_UNIT 		0x00000088
#define LOG_MODE_A9TIMER_CHANNEL		0x00000014
#define LOG_MODE_A9TIMER_DIFFERENCE 	0x00000028




uint32_t    log_check	(t_log * log);
void        log_deinit	(t_log * log);
void * 		log_checkthread(void * arg);
void 		log_init (t_log * log, void * measurement_unit, uint32_t channel, uint32_t mode, char * filename, double scale, char * unit, uint32_t sleeptime);

#endif