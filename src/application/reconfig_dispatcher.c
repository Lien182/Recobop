/********************************************************************          
* reconfig_dispatcher.c -- functions for context switching          *
*                          reconfiguration of ReconOS slots         *   
*			   parts are overtaken from the ReconOS     *
*			   reconfiguration example		    *
*                                                                   *   
* Author(s):  Christian Lienen                                      *   
*                                                                   *   
********************************************************************/  

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

#include "a9timer.h"
#include "recobop.h"


int reconfig_dispatcher_reconfigure(t_bitstream bitstream, uint32_t threadid, uint32_t partial)
{
#if RECONFIG_VERBOSE_DEBUG == 1
        uint32_t tStart, tEnd;
        tStart = a9timer_getvalue(a9timer);
#endif
        /* construct path of bitfile */
        int fd_partial = open("/sys/devices/soc0/amba/f8007000.devcfg/is_partial_bitstream", O_RDWR);
        if(fd_partial < 0){
                printf("[Reconfig Dispatcher] Failed to open xdevcfg attribute 'is_partial_bitstream' when configuring threadid %d\n",threadid);
                return -1;
        }

        char partial_flag[2];
        if(!partial) 
        {
                strcpy(partial_flag,"0");
        }
        else 
        {
                strcpy(partial_flag,"1");
        }
        write(fd_partial, partial_flag, 2);
        close(fd_partial);

        fd_partial = open("/dev/xdevcfg", O_RDWR);
        if(fd_partial < 0){
                printf("[Reconfig Dispatcher] Failed to open xdevcfg device when configuring threadid %d\n",threadid);
                return -1;
        }
#if RECONFIG_VERBOSE_DEBUG == 1
        printf("[Reconfig Dispatcher] Opened xdevcfg. Configuring with %u bytes\n",bitstream.size);
#endif
        write(fd_partial, bitstream.data, bitstream.size);
        int fd_finish_flag = open("/sys/devices/soc0/amba/f8007000.devcfg/prog_done", O_RDWR);
        char finish_flag = '0';

        /* wait until reconfiguration is finished */
        while(finish_flag != '1'){
                read(fd_finish_flag,&finish_flag,1);
        }
#if RECONFIG_VERBOSE_DEBUG == 1
        printf("[Reconfig Dispatcher] Reconfiguration with bitfile threadid %d finished\n", threadid);
#endif
        close(fd_partial);
        close(fd_finish_flag);


        
#if RECONFIG_VERBOSE_DEBUG == 1
        tEnd = a9timer_getvalue(a9timer);
        printf("[Reconfig Dispatcher] Reconfiguration took %3.4f ms\n", (tEnd - tStart)* 0.00000333);
#endif
        return 0;
}

void * reconfig_dispatcher_thread(void* arg)
{ 
    t_reconfig_slot * reconfig_slot = (t_reconfig_slot*)arg; 
    uint32_t threadid,newthreadid = 0;
    uint32_t slotid = reconfig_slot->slotid;

    struct hwslot *act_hwslot;
    act_hwslot = reconfig_slot->recon_thread->hwslot;
    uint32_t tStart, tEnd;

#if RECONFIG_VERBOSE_DEBUG == 1
    printf("[Reconfig Dispatcher] The dispatcher says hello \n");
#endif 

    while(1)
    {
        threadid = mbox_get(reconfig_slot->mb_reconfig_request);
        tStart = a9timer_getvalue(a9timer);
      
        if(threadid >= MAX_THREADS)
        {        
               printf("[Reconfig Dispatcher] Fatal Error: wrong reqested threadid\n");
               break;
        }
        else
        {       
                reconfig_slot->threads[newthreadid].state = TASKSTATE_WAITING;
        }
        
   
        newthreadid = reconfig_queue_dequeue(reconfig_slot->reconfig_queue, slotid);

#if RECONFIG_VERBOSE_DEBUG == 1
        //change init data
        printf("[Reconfig Dispatcher] New Init Data: %08x for thread %x \n", reconfig_slot->threads[newthreadid].init_data, newthreadid);
#endif        

        reconos_thread_suspend_block(reconfig_slot->recon_thread); 
        //hwslot_setreset(reconfig_slot->recon_thread->hwslot, 0);
        pthread_mutex_lock(reconfig_slot->reconfiguration_mutex);  
        reconfig_slot->recon_thread->init_data = reconfig_slot->threads[newthreadid].init_data;
#if RECONFIG_VERBOSE_DEBUG == 1     
        printf("[Reconfig Dispatcher] Suspend reconos block %d\n", ((int*)act_hwslot)[0]);
#endif
        if(reconfig_slot->breconfigurable == 1)
        {
           reconfig_dispatcher_reconfigure(reconfig_slot->threads[newthreadid].bitstreams[slotid], newthreadid, 1);     
        }
       
        reconos_thread_resume(reconfig_slot->recon_thread,((int*)act_hwslot)[0]); 
        reconfig_slot->threads[newthreadid].state = TASKSTATE_RUNNING;
        pthread_mutex_unlock(reconfig_slot->reconfiguration_mutex);  
        tEnd = a9timer_getvalue(a9timer);
        printf("[Reconfig Dispatcher] Context switch on slot %d took %3.4f ms\n", slotid,(tEnd - tStart)* 0.000003);

        
    }
  
    return NULL;

}


//Starts the dispatcher for the given slot
void reconfig_dispatcher_init(t_reconfig_slot * reconfig_slot )
{
    struct sched_param param;
    pthread_attr_t attr;
    int ret;
    
    int prio = 0;

    if(prio != 0)
    {
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
        param.sched_priority = prio;
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

        pthread_create(&(reconfig_slot->reconfig_dispatcher_thread), &attr, &reconfig_dispatcher_thread, (void*)reconfig_slot);
    }
    else
    {
        pthread_create(&(reconfig_slot->reconfig_dispatcher_thread), NULL, &reconfig_dispatcher_thread, (void*)reconfig_slot);    
    }
    
    
         
    
}
