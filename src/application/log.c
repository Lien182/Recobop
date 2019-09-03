/********************************************************************          
* log.c           -- log unit; it can check all used time sources   *
*                    for differences in the capture values and      *
*					 and writes the resulting time in stdout or 	*
*					 in a file*										*
*                                                                   *  
* Author(s):  Christian Lienen                                      *   
*                                                                   *   
********************************************************************/

#include "log.h"



#define LOG_INTERNAL_MODE_SINGLE		0x00000004
#define LOG_INTERNAL_MODE_DIFFERENCE	0x00000008



uint32_t log_check(t_log * log)
{
	uint32_t cnt;

	if((log->mode & LOG_MODE_AXI_CHANNEL) == LOG_MODE_AXI_CHANNEL)
	{
		if(log->channel == LOG_CHANNEL_0)
			cnt = log->timer->TLR0;
		else
			cnt = log->timer->TLR1;
	}
	else
	{
		cnt = log->a9timer_capture.tStart;
		
	}
	
	if(log->bStart) // Difference makes no sense for the first sample
	{
		
		log->bStart = 0UL;
		log->lasttimervalue = cnt;
		log->samplecnt = 1;
		return 1;
	}

	if( cnt != log->lasttimervalue )
	{
		if(log->mode & LOG_MODE_FILE)
			fprintf(log->fd, "%08lld,\t %6.6f %s\n",log->samplecnt, (double)(cnt - log->lasttimervalue)*log->scale, log->unit);
		
		if(log->mode & LOG_MODE_STDOUT)
			printf("%s, %08lld;\t %6.6f %s;\n",log->filename, log->samplecnt, (double)(cnt - log->lasttimervalue)*log->scale, log->unit);
		
		log->lasttimervalue = cnt;
		log->samplecnt++;

		return 1;
	}
	return 0;
}

uint32_t log_check_difference(t_log * log)
{
	uint32_t cnt_0 = 0, cnt_1 = 0;

	if((log->mode & LOG_MODE_DIFFERENCE_UNIT) == LOG_MODE_DIFFERENCE_UNIT)
	{
		
		if(log->channel == LOG_CHANNEL_0)
		{
			cnt_0 = log->diff_measurement->CAP0;
			cnt_1 = log->diff_measurement->CAP1;
		}
		else if(log->channel == LOG_CHANNEL_1)
		{
			cnt_0 = log->diff_measurement->CAP2;
			cnt_1 = log->diff_measurement->CAP3;
		}
			
	}
	else
	{
		cnt_0 = log->a9timer_capture.tStart;
		cnt_1 = log->a9timer_capture.tStop;
	}
	
	if(log->bStart) // Difference makes no sense for the first sample
	{
		log->bStart = 0UL;
		log->lasttimervalue = cnt_1;
		log->samplecnt = 1;
		return 1;
	}

	if( cnt_1 != log->lasttimervalue && (cnt_1 > cnt_0 ))
	{
		if(log->mode & LOG_MODE_FILE)
			fprintf(log->fd, "%08lld,\t %6.6f %s\n",log->samplecnt, (double)(cnt_1 - cnt_0)*log->scale, log->unit);
		
		if(log->mode & LOG_MODE_STDOUT)
			printf("%s, %08lld;\t %6.6f %s;\n",log->filename, log->samplecnt, (double)(cnt_1 - cnt_0)*log->scale, log->unit);
		
		log->lasttimervalue = cnt_1;
		log->samplecnt++;

		return 1;
	}
	return 0;
}


void log_deinit(t_log * log)
{
	log->bStop = 1;
	printf("LOG: Wait for exiting thread.. \n");
	pthread_join(log->thread, NULL);
	fclose(log->fd);
}

void * log_checkthread(void * arg)
{
	t_log * log = (t_log*)arg;


	while(log->bStop != 1)
	{
		if(log->mode & LOG_INTERNAL_MODE_SINGLE)
			log_check(log);
		else if(log->mode & LOG_INTERNAL_MODE_DIFFERENCE)
			log_check_difference(log);

		usleep(log->sleeptime);
	}

	return NULL;
}

void log_init(t_log * log, void * measurement_unit, uint32_t channel, uint32_t mode, char * filename, double scale, char * unit, uint32_t sleeptime)
{
	if((mode & LOG_MODE_AXI_CHANNEL) == LOG_MODE_AXI_CHANNEL)
	{
		log->timer = (t_axi_timer*)measurement_unit;
		printf("Log: LOG_MODE_AXI_CHANNEL \n");
	}	
	else if((mode & LOG_MODE_DIFFERENCE_UNIT) == LOG_MODE_DIFFERENCE_UNIT)
	{
		log->diff_measurement = (t_diff_measurement*)measurement_unit;
		printf("Log: LOG_MODE_DIFFERENCE_UNIT \n");
	}	
	else if((mode & LOG_MODE_A9TIMER_CHANNEL) == LOG_MODE_A9TIMER_CHANNEL)
	{
		log->a9timer = (t_a9timer*)measurement_unit;
		printf("Log: LOG_MODE_A9TIMER_CHANNEL \n");
	}		
	else if((mode & LOG_MODE_A9TIMER_DIFFERENCE) == LOG_MODE_A9TIMER_DIFFERENCE)
	{
		log->a9timer = (t_a9timer*)measurement_unit;
		printf("Log: LOG_MODE_A9TIMER_DIFFERENCE \n");
	}
		

	log->fd = fopen(filename, "w+");
	if(log->fd == NULL)
	{
		printf("LOG_INIT: Open %s failed \n", filename);
	}

	log->sleeptime = sleeptime;
	log->bStart = 1UL;
	log->bStop = 0UL;
	log->lasttimervalue = 0;
	log->scale = scale;
	log->samplecnt = 0;
	log->channel = channel;
	log->mode = mode;
	log->unit = unit;
	log->filename = filename;
	log->a9timer_capture.tStart = 0;
	log->a9timer_capture.tStop = 0;

	if(pthread_create(&(log->thread), 0, &log_checkthread, log) != 0)
	{
		printf("LOG: Error during check thread starting \n");
	}

	return;
}

