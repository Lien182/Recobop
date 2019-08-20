#include "reconfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <limits.h>


void reconfig_scheduler_init(t_reconfig_scheduler * reconfig_scheduler)
{
	reconfig_scheduler->taskset_size = 0;

	//Initializatuon of the queue
	reconfig_queue_init(&(reconfig_scheduler->reconfig_queue));

}

int32_t reconfig_scheduler_buffer_bitstream(char* filename, t_bitstream * bitstream)
{

	FILE *bitfile;

	bitfile = fopen(filename, "rb");
	if(!bitfile){
		printf("[Reconfig Dispatcher] Error opening bitfile %s\n",filename);
		return -1;
	}

	fseek(bitfile,0L,SEEK_END);
	bitstream->size=ftell(bitfile);
	rewind(bitfile);

	bitstream->data = (char *)malloc(bitstream->size*sizeof(char));
	if(!(bitstream->data)){
		printf("[Reconfig Dispatcher] Error allocating memory for bitstream %s\n",filename);
		return -1;
	}
	fread(bitstream->data, sizeof(char), bitstream->size, bitfile);
	fclose(bitfile);

        printf("[Reconfig Dispatcher] Loaded bitfile %s [%d bytes] into RAM \n", filename, bitstream->size);
        return 0;
}


void reconfig_scheduler_schedule(t_reconfig_scheduler * reconfig_scheduler)
{

	//Unflexible example
	reconfig_queue_enqueue(&(reconfig_scheduler->reconfig_queue), 0, 0);
	reconfig_queue_enqueue(&(reconfig_scheduler->reconfig_queue), 0, 1);
	reconfig_queue_enqueue(&(reconfig_scheduler->reconfig_queue), 0, 2);
	reconfig_queue_enqueue(&(reconfig_scheduler->reconfig_queue), 1, 3);
	reconfig_queue_enqueue(&(reconfig_scheduler->reconfig_queue), 1, 4);
	reconfig_queue_enqueue(&(reconfig_scheduler->reconfig_queue), 1, 5);
	
}


int32_t    reconfig_scheduler_register_new_slot(t_reconfig_scheduler * reconfig_scheduler, void ** rtinit_data, struct mbox *mb_reconfig_request)
{
	uint32_t slotid;

	if(reconfig_scheduler->slotset_size < MAX_SLOTS)
	{
		//new slot id
		slotid =  reconfig_scheduler->slotset_size;
		//create new threadid
		reconfig_scheduler->slotset[slotid].slotid 		= slotid;
		reconfig_scheduler->slotset[slotid].rtinit_data = rtinit_data;		
	
		reconfig_scheduler->slotset[slotid].reconfig_dispatcher.slotid = slotid;
		reconfig_scheduler->slotset[slotid].reconfig_dispatcher.threads = reconfig_scheduler->taskset;
		reconfig_scheduler->slotset[slotid].reconfig_dispatcher.reconfig_queue = &reconfig_scheduler->reconfig_queue;
		reconfig_scheduler->slotset[slotid].reconfig_dispatcher.mb_reconfig_request = mb_reconfig_request;
				

		//Initialization of the dispatcher
		reconfig_dispatcher_init(&(reconfig_scheduler->slotset[slotid].reconfig_dispatcher));
		
		reconfig_scheduler->slotset_size++;
		return slotid;
	}
	else
	{
		return  -1;
	}
}

int32_t    reconfig_scheduler_register_new_task(t_reconfig_scheduler * reconfig_scheduler, uint32_t slotmask, uint32_t contextsize, void * initdata, char ** bitstream)
{
	int32_t i, j;

	uint32_t threadid;

	if(reconfig_scheduler->taskset_size < MAX_THREADS)
	{
		//create new threadid
		threadid = reconfig_scheduler->taskset_size;
		reconfig_scheduler->taskset[threadid].threadid 	= threadid;
		reconfig_scheduler->taskset[threadid].state 	= TASKSTATE_WAITING;
		reconfig_scheduler->taskset[threadid].slotmask 	= slotmask;
		
		//malloc context new buffer
		reconfig_scheduler->taskset[threadid].context 	= malloc(contextsize);
		
		if(reconfig_scheduler->taskset[threadid].context == NULL)
			return -2;
		
		j = 0; 
		for(i = 0; i < MAX_SLOTS; i++)
		{
			if(slotmask & 1)
			{
				reconfig_scheduler_buffer_bitstream( bitstream[j], &(reconfig_scheduler->taskset[threadid].bitstreams[i]));
				printf("[reconfig scheduler] threadid %d: loaded bitfile %s into ram \n", reconfig_scheduler->taskset[threadid].threadid , bitstream[j]);
			}
			slotmask >>= 1;
		}

		reconfig_scheduler->taskset_size++;

		return threadid;
	}
	else
	{
		return  -1;
	}
	
}

int32_t reconfig_scheduler_dispatcher_feedback(t_reconfig_scheduler * reconfig_scheduler, uint32_t slot, uint32_t oldthreadid, uint32_t newthreadid)
{
	reconfig_scheduler->taskset[newthreadid].state = TASKSTATE_RUNNING;
	reconfig_scheduler->taskset[oldthreadid].state = TASKSTATE_WAITING;
	return 0;
}