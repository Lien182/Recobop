#include "reconfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <limits.h>
#include "reconos_app.h"

void * reconfig_scheduler_schedule(void * arg)
{
	t_reconfig_scheduler * reconfig_scheduler = (t_reconfig_scheduler*)arg;

	while(1)
	{
		cycle_timer_wait(reconfig_scheduler->cycle_timer);
		printf("[Reconfig Scheduler] ################### NEW CYCLE ###################\n");
		
		if(0 == reconfig_queue_isempty(&(reconfig_scheduler->reconfig_queue)))
		{
			printf("[Reconfig Scheduler] ____________________ QUEUE NOT EMPTY ____________________\n");
		}
		printf("[Reconfig Scheduler] Fill the queue \n");


		reconfig_queue_enqueue(&(reconfig_scheduler->reconfig_queue), 0, 0);
		reconfig_queue_enqueue(&(reconfig_scheduler->reconfig_queue), 0, 1);
		reconfig_queue_enqueue(&(reconfig_scheduler->reconfig_queue), 0, 2);
		reconfig_queue_enqueue(&(reconfig_scheduler->reconfig_queue), 1, 3);
		reconfig_queue_enqueue(&(reconfig_scheduler->reconfig_queue), 1, 4);
		reconfig_queue_enqueue(&(reconfig_scheduler->reconfig_queue), 1, 5);
		
	}


}

void reconfig_scheduler_init(t_reconfig_scheduler * reconfig_scheduler, t_cycle_timer * cycle_timer)
{
    struct sched_param param;
    pthread_attr_t attr;
    int ret;
	int i,j;


	for(j = 0; j < MAX_THREADS; j++)
	{

		for(i = 0; i < MAX_SLOTS; i++)
		{
			reconfig_scheduler->taskset[j].bitstreams[i].data = NULL;
			reconfig_scheduler->taskset[j].bitstreams[i].size = 0;
			
		}

		reconfig_scheduler->taskset[j].init_data = 0;
		reconfig_scheduler->taskset[j].slotmask = 0;
		reconfig_scheduler->taskset[j].threadid = 0;
		
		
	}

	reconfig_scheduler->taskset_size = 0;
	reconfig_scheduler->slotset_size = 0;
	reconfig_scheduler->cycle_timer = cycle_timer;

	pthread_mutex_init(&(reconfig_scheduler->reconfiguration_mutex),NULL);

	//Initializatuon of the queue
	reconfig_queue_init(&(reconfig_scheduler->reconfig_queue));
	
    /* Lock memory */
    if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
            printf("[Reconfig Dispatcher] mlockall failed: %m\n");
            exit(-2);
    }

    /* Initialize pthread attributes (default values) */
    ret = pthread_attr_init(&attr);
    if (ret) {
            printf("[Reconfig Dispatcher] init pthread attributes failed\n");
            return;
    }

    /* Set a specific stack size  */
    ret = pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
    if (ret) {
        printf("[Reconfig Dispatcher] pthread setstacksize failed\n");
        return;
    }

    /* Set scheduler policy and priority of pthread */
    ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    if (ret) {
            printf("[Reconfig Dispatcher] pthread setschedpolicy failed\n");
            return;
    }
    param.sched_priority = 75;
    ret = pthread_attr_setschedparam(&attr, &param);
    if (ret) {
            printf("[Reconfig Dispatcher] pthread setschedparam failed\n");
            return;
    }
    /* Use scheduling parameters of attr */
    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (ret) {
            printf("[Reconfig Dispatcher] pthread setinheritsched failed\n");
            return;
    }

    pthread_create(&(reconfig_scheduler->reconfig_scheduler_thread), &attr, &reconfig_scheduler_schedule, (void*)reconfig_scheduler);   

}

int32_t reconfig_scheduler_buffer_bitstream(char* filename, t_bitstream * bitstream)
{

	FILE *bitfile;

	bitfile = fopen(filename, "rb");
	if(!bitfile){
		printf("[Reconfig Scheduler] Error opening bitfile %s\n",filename);
		return -1;
	}

	fseek(bitfile,0L,SEEK_END);
	bitstream->size=ftell(bitfile);
	rewind(bitfile);

	bitstream->data = (char *)malloc(bitstream->size*sizeof(char));
	if(!(bitstream->data)){
		printf("[Reconfig Scheduler] Error allocating memory for bitstream %s\n",filename);
		return -1;
	}
	fread(bitstream->data, sizeof(char), bitstream->size, bitfile);
	fclose(bitfile);

        printf("[Reconfig Scheduler] Loaded bitfile %s [%d bytes] into RAM \n", filename, bitstream->size);
        return 0;
}

int32_t reconfig_scheduler_free_bitstream(t_bitstream bitstream)
{
	if(bitstream.data != NULL)
		free(bitstream.data);
}




int32_t reconfig_scheduler_register_new_slot(t_reconfig_scheduler * reconfig_scheduler, void * init_data,  struct mbox *mb_reconfig_request, char * initial_bitstream)
{
	uint32_t slotid;
	t_bitstream init_bitstream;

	if(reconfig_scheduler->slotset_size < MAX_SLOTS)
	{
		printf("[Reconfig Scheduler] Register new slot \n");

		//new slot id
		slotid =  reconfig_scheduler->slotset_size;
		
		//create new threadid
		reconfig_scheduler->slotset[slotid].slotid = slotid;
		reconfig_scheduler->slotset[slotid].threads = reconfig_scheduler->taskset;
		reconfig_scheduler->slotset[slotid].reconfig_queue = &reconfig_scheduler->reconfig_queue;
		reconfig_scheduler->slotset[slotid].mb_reconfig_request = mb_reconfig_request;
		reconfig_scheduler->slotset[slotid].reconfiguration_mutex = &(reconfig_scheduler->reconfiguration_mutex);
		
	
		printf("[Reconfig Scheduler] Buffer bitstream %s \n", initial_bitstream);
		reconfig_scheduler_buffer_bitstream(initial_bitstream, &init_bitstream);
		reconfig_dispatcher_reconfigure(init_bitstream, 0xff, 1);
#ifdef RECONF
		reconfig_scheduler->slotset[slotid].recon_thread = reconos_thread_create_hwt_reconf(init_data);	
#else
	#warning No reconf slot defined
#endif
		//if(slotid == 1)
		reconfig_dispatcher_init(&(reconfig_scheduler->slotset[slotid]));

		//Start the new thread
		
		
		//reconos_thread_resume(reconfig_scheduler->slotset[slotid].recon_thread, slotid + 6);

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
		printf("[Reconfig Scheduler] Register new task \n");
		//create new threadid
		threadid = reconfig_scheduler->taskset_size;
		reconfig_scheduler->taskset[threadid].threadid 	= threadid;
		reconfig_scheduler->taskset[threadid].state 	= TASKSTATE_WAITING;
		reconfig_scheduler->taskset[threadid].slotmask 	= slotmask;
		reconfig_scheduler->taskset[threadid].init_data = initdata;
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
				printf("[Reconfig Scheduler] threadid %d: loaded bitfile %s into RAM\n", reconfig_scheduler->taskset[threadid].threadid , bitstream[j]);
				j++;
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