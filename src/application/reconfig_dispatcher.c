#include "reconfig_dispatcher.h"
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



int reconfigure(char* filename,unsigned int partial){
	/* construct path of bitfile */

	FILE *bitfile;
	unsigned int size;
	char *bitstream;

	bitfile = fopen(filename, "rb");
	if(!bitfile){
		printf("Error opening bitfile %s\n",filename);
		return -1;
	}

	fseek(bitfile,0L,SEEK_END);
	size=ftell(bitfile);
	rewind(bitfile);

	bitstream = (char *)malloc(size*sizeof(char));
	if(!bitstream){
		printf("Error allocating memory for bitstream %s\n",filename);
		return -1;
	}
	fread(bitstream, sizeof(char), size, bitfile);
	fclose(bitfile);

	int fd_partial = open("/sys/devices/soc0/amba/f8007000.devcfg/is_partial_bitstream", O_RDWR);
	if(fd_partial < 0){
		printf("Failed to open xdevcfg attribute 'is_partial_bitstream' when configuring %s\n",filename);
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
		printf("Failed to open xdevcfg device when configuring %s\n",filename);
		return -1;
	}
	printf("Opened xdevcfg. Configuring with %u bytes\n",size);
	write(fd_partial, bitstream, size);
	int fd_finish_flag = open("/sys/devices/soc0/amba/f8007000.devcfg/prog_done", O_RDWR);
	char finish_flag = '0';

	/* wait until reconfiguration is finished */
	while(finish_flag != '1'){
		read(fd_finish_flag,&finish_flag,1);
	}
	printf("Reconfiguration with bitfile %s finished\n",filename);
	close(fd_partial);
	close(fd_finish_flag);

	return 0;

}


void * reconfig_dispatcher_thread(void* arg)
{
    t_reconfig_dispatcher * reconfig_dispatcher = (t_reconfig_dispatcher*)arg; 
    uint32_t req = 0; // 16bit slot id (high bits) && 16 bit thread id
    uint16_t slotid, threadid;

    while(1)
    {
        req = mbox_get(reconfig_dispatcher->mb_reconfig_request);
        
        threadid = (uint16_t)(req & 0xffff);
        slotid =   (uint16_t)((req >> 16) & 0xffff);
        
        /* DO RECONFIGURATION */

        if(threadid >= MAX_THREADS)
        {
                printf("[RECONFIG_DISPATCHER] Fatal Error: wrong req \n");
                break;
        }
                
        reconfigure(bitstream_file_list[req], 1);


        mbox_put(reconfig_dispatcher->mb_reconfig_response, req);
    }

}


void reconfig_dispatcher_init(t_reconfig_dispatcher * reconfig_dispatcher)
{
    struct sched_param param;
    pthread_attr_t attr;
    int ret;

    /* Lock memory */
    if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
            printf("mlockall failed: %m\n");
            exit(-2);
    }

    /* Initialize pthread attributes (default values) */
    ret = pthread_attr_init(&attr);
    if (ret) {
            printf("init pthread attributes failed\n");
            return;
    }

    /* Set a specific stack size  */
    ret = pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
    if (ret) {
        printf("pthread setstacksize failed\n");
        return;
    }

    /* Set scheduler policy and priority of pthread */
    ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    if (ret) {
            printf("pthread setschedpolicy failed\n");
            return;
    }
    param.sched_priority = 75;
    ret = pthread_attr_setschedparam(&attr, &param);
    if (ret) {
            printf("pthread setschedparam failed\n");
            return;
    }
    /* Use scheduling parameters of attr */
    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (ret) {
            printf("pthread setinheritsched failed\n");
            return;
    }
    
    pthread_create(&reconfig_dispatcher->reconfig_dispatcher_thread, &attr, &reconfig_dispatcher_thread, (void*)reconfig_dispatcher);



	
}
