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

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>


char* bitstream_file_list[4] =
{
     "slot_0_control_thread.bit", "slot_1_control_thread.bit", "slot_0_inverse_thread.bit", "slot_1_inverse_thread.bit"
};



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


int reconfig_dispatcher_buffer_bitstream(t_reconfig_dispatcher * reconfig_dispatcher, char* filename, t_bitstream * bitstream)
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


void * reconfig_dispatcher_thread(void* arg)
{
    t_reconfig_scheduler  * reconfig_scheduler  = (t_reconfig_scheduler*)arg;    
    t_reconfig_dispatcher * reconfig_dispatcher = &(reconfig_scheduler->reconfig_dispatcher); 
    uint32_t req = 0; // 16bit slot id (high bits) && 16 bit thread id
    uint32_t slotid, threadid,newthreadid;
    

    while(1)
    {
        req = mbox_get(reconfig_dispatcher->mb_reconfig_request);
        
        threadid = (req & 0xffff);
        slotid =   (req >> 16) & 0xffff;
        
        /* DO RECONFIGURATION */
        

        if(threadid >= MAX_THREADS)
        {
                printf("[Reconfig Dispatcher] Fatal Error: wrong reqested threadid\n");
                break;
        }

        newthreadid = reconfig_queue_dequeue( &(reconfig_scheduler->reconfig_queue), slotid);
        reconfigure((reconfig_dispatcher->bitstreams[threadid]), threadid, 1);
        reconfig_scheduler_dispatcher_feedback(reconfig_scheduler, slotid, threadid, newthreadid);

        //mbox_put(reconfig_dispatcher->mb_reconfig_response, req);
    }

}


void reconfig_dispatcher_init(t_reconfig_dispatcher * reconfig_dispatcher)
{
    struct sched_param param;
    pthread_attr_t attr;
    int ret;
    int i;

    //READ BITSTREAMS INTO RAM
    for(i = 0; i < MAX_THREADS; i++)
    {
            reconfig_dispatcher_buffer_bitstream(reconfig_dispatcher, bitstream_file_list[i],&(reconfig_dispatcher->bitstreams[i]) );
    }



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
