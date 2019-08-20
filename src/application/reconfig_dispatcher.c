#include "reconfig.h"
#include <stdio.h>
#include <sys/mman.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "mbox.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

int reconfigure(t_bitstream bitstream, uint32_t threadid, uint32_t partial){
	/* construct path of bitfile */


	int fd_partial = open("/sys/devices/soc0/amba/f8007000.devcfg/is_partial_bitstream", O_RDWR);
	if(fd_partial < 0){
		printf("[Reconfig Dispatcher] Failed to open xdevcfg attribute 'is_partial_bitstream' when configuring threadid %d\n",threadid);
		return -1;
	}

	char partial_flag[2];
	if(!partial) {
		strcpy(partial_flag,"0");
	}
	else {
		strcpy(partial_flag,"1");
	}
	write(fd_partial, partial_flag, 2);
	close(fd_partial);

	fd_partial = open("/dev/xdevcfg", O_RDWR);
	if(fd_partial < 0){
		printf("[Reconfig Dispatcher] Failed to open xdevcfg device when configuring threadid %d\n",threadid);
		return -1;
	}
	printf("[Reconfig Dispatcher] Opened xdevcfg. Configuring with %u bytes\n",bitstream.size);
	write(fd_partial, bitstream.data, bitstream.size);
	int fd_finish_flag = open("/sys/devices/soc0/amba/f8007000.devcfg/prog_done", O_RDWR);
	char finish_flag = '0';

	/* wait until reconfiguration is finished */
	while(finish_flag != '1'){
		read(fd_finish_flag,&finish_flag,1);
	}
	printf("[Reconfig Dispatcher] Reconfiguration with bitfile threadid %d finished\n", threadid);
	close(fd_partial);
	close(fd_finish_flag);

	return 0;

}




void * reconfig_dispatcher_thread(void* arg)
{ 
    t_reconfig_slot * reconfig_slot = (t_reconfig_slot*)arg; 
    uint32_t threadid,newthreadid;
    
    uint32_t slotid = reconfig_slot->slotid;

    while(1)
    {
  
        threadid = mbox_get(reconfig_slot->reconfig_dispatcher.mb_reconfig_request);
  
        /* DO RECONFIGURATION */
        if(threadid >= MAX_THREADS)
        {
                printf("[Reconfig Dispatcher] Fatal Error: wrong reqested threadid\n");
                break;
        }

        newthreadid = reconfig_queue_dequeue( reconfig_slot->reconfig_dispatcher.reconfig_queue, slotid);

        //change init data
        *(reconfig_slot->rtinit_data) = reconfig_slot->reconfig_dispatcher.threads[newthreadid].init_data;
        reconfigure(reconfig_slot->reconfig_dispatcher.threads[threadid].bitstreams[slotid], threadid, 1);
    }

    return NULL;

}


void reconfig_dispatcher_init(t_reconfig_dispatcher * reconfig_dispatcher)
{
    struct sched_param param;
    pthread_attr_t attr;
    int ret;


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
    
    pthread_create(&reconfig_dispatcher->reconfig_dispatcher_thread, &attr, &reconfig_dispatcher_thread, (void*)reconfig_dispatcher);      
    
    



	
}
