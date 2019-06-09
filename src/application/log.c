#include "log.h"


uint32_t log_check(t_log * log)
{
	uint32_t cnt;

	if(log->channel == LOG_CHANNEL_0)
		cnt = log->timer->TLR0;
	else
		cnt = log->timer->TLR1;
	
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
	uint32_t cnt_0, cnt_1;

	cnt_0 = log->diff_measurement->CAP0;

	if(log->channel == LOG_CHANNEL_1)
		cnt_1 = log->diff_measurement->CAP1;
	else if(log->channel == LOG_CHANNEL_2)
		cnt_1 = log->diff_measurement->CAP2;
	else if(log->channel == LOG_CHANNEL_3)
		cnt_1 = log->diff_measurement->CAP3;
	



	if(log->bStart) // Difference makes no sense for the first sample
	{
		log->bStart = 0UL;
		log->lasttimervalue = cnt_1;
		log->samplecnt = 1;
		return 1;
	}

	if( cnt_1 != log->lasttimervalue )// && (cnt_1 > cnt_0 ))
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
	fclose(log->fd);
}

void log_checkthread(t_log * log)
{
	while(log->bStop != 1)
	{
		if(log->mode & LOG_MODE_SINGLE)
			log_check(log);
		else if(log->mode & LOG_MODE_DIFFERENCE)
			log_check_difference(log);
		
		
		usleep(1);
	}

}

void log_init(t_log * log, t_axi_timer* timer, t_diff_measurement* diff_timer, uint32_t channel, uint32_t mode, char * filename, double scale, char * unit)
{
	if(mode & LOG_MODE_SINGLE)
		log->timer = timer;
	else if(mode & LOG_MODE_DIFFERENCE)
		log->diff_measurement = diff_timer;

	log->fd = fopen(filename, "w+");
	if(log->fd == NULL)
	{
		printf("LOG_INIT: Open %s failed \n", filename);
	}

	log->bStart = 1UL;
	log->bStop = 0UL;
	log->lasttimervalue = 0;
	log->scale = scale;
	log->samplecnt = 0;
	log->channel = channel;
	log->mode = mode;
	log->unit = unit;
	log->filename = filename;

	if(pthread_create(&(log->thread), 0, &log_checkthread, log) != 0)
	{
		printf("LOG: Error during check thread starting \n");
	}

	return;
}
