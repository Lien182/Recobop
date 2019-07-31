#include "reconfig_scheduler.h"
#include <stdio.h>
#include <sys/mman.h>
#include <limits.h>


void reconfig_scheduler_init(t_reconfig_scheduler * reconfig_scheduler, t_reconfig_queue * reconfig_queue)
{
	reconfig_scheduler->taskset_size = 0;
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


uint32_t reconfig_scheduler_register_new_task(t_reconfig_scheduler * reconfig_scheduler, uint32_t slotmask, uint32_t contextsize)
{
	if(reconfig_scheduler->taskset_size < MAX_THREADS)
	{
		//create new threadid
		reconfig_scheduler->taskset[reconfig_scheduler->taskset_size].threadid 	= reconfig_scheduler->taskset_size;
		reconfig_scheduler->taskset[reconfig_scheduler->taskset_size].state 	= TASKSTATE_WAITING;
		reconfig_scheduler->taskset[reconfig_scheduler->taskset_size].slotmask 	= slotmask;
		
		//malloc context new buffer
		reconfig_scheduler->taskset[reconfig_scheduler->taskset_size].context 	= malloc(contextsize);
		
		if(reconfig_scheduler->taskset[reconfig_scheduler->taskset_size].context == NULL)
			return 0;
		
		reconfig_scheduler->taskset_size++;

		return reconfig_scheduler->taskset[reconfig_scheduler->taskset_size].threadid;
	}
	else
	{
		return  0;
	}
	
}

uint32_t reconfig_scheduler_dispatcher_feedback(t_reconfig_scheduler * reconfig_scheduler, uint32_t slot, uint32_t oldthreadid, uint32_t newthreadid)
{
	reconfig_scheduler->taskset[newthreadid].state = TASKSTATE_RUNNING;
	reconfig_scheduler->taskset[oldthreadid].state = TASKSTATE_WAITING;
}