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

typedef struct 
{
	uint32_t channel;
	uint32_t mode;
	FILE * fd;
	char * filename;
	t_axi_timer * timer;
	t_diff_measurement * diff_measurement;
	uint32_t lasttimervalue;
	
	
	uint64_t samplecnt; 

	double scale;
	char * unit;

	pthread_t thread;
    uint32_t bStart;
	uint32_t bStop;

}t_log;

#define LOG_CHANNEL_0 		0 
#define LOG_CHANNEL_1 		1
#define LOG_CHANNEL_2		2
#define LOG_CHANNEL_3		3

#define LOG_MODE_STDOUT 	1
#define LOG_MODE_FILE		2 

#define LOG_MODE_SINGLE 	4
#define LOG_MODE_DIFFERENCE 8


uint32_t    log_check	(t_log * log);
void        log_deinit	(t_log * log);
void        log_checkthread (t_log * log);
void 		log_init (t_log * log, t_axi_timer* timer, t_diff_measurement* diff_timer, uint32_t channel, uint32_t mode, char * filename, double scale, char * unit);

#endif