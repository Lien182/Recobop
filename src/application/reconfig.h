#ifndef RECONFIG_H
#define RECONFIG_H

//DEFINES

#include <pthread.h>
#include <stdint.h>
#include "reconfig_settings.h"
#include "reconfig_queue.h"


#define MAX_SLOTS   2
#define MAX_THREADS 3


#define SLOTMASK_SLOT_0 0x00000001
#define SLOTMASK_SLOT_1 0x00000002

#define TASKSTATE_RUNNING     0x00000001
#define TASKSTATE_STOP        0x00000002
#define TASKSTATE_WAITING     0x00000003

typedef struct 
{
     char * data;
     size_t size;
}
t_bitstream;


typedef struct
{

    uint32_t    threadid;
    uint32_t    state;
    uint32_t    slotmask;
    void *      context;
    void *      init_data;
    t_bitstream bitstreams[MAX_SLOTS];

}t_reconfig_task;


typedef struct 
{
    uint32_t            slotid;
    pthread_t           reconfig_dispatcher_thread;
    struct mbox         *mb_reconfig_request;     
    t_reconfig_task *   threads;
    t_reconfig_queue *  reconfig_queue;

}t_reconfig_dispatcher;

typedef struct 
{
    uint32_t                 slotid;
    void **                  rtinit_data;
    t_reconfig_dispatcher    reconfig_dispatcher;
}t_reconfig_slot;



typedef struct 
{
     t_reconfig_queue         reconfig_queue;     
     t_reconfig_task          taskset[MAX_THREADS];
     t_reconfig_slot          slotset[MAX_SLOTS];
     uint32_t                 taskset_size;  
     uint32_t                 slotset_size;   
}t_reconfig_scheduler;




void       reconfig_scheduler_init(t_reconfig_scheduler * reconfig_scheduler);

void       reconfig_scheduler_schedule(t_reconfig_scheduler * reconfig_scheduler);

int32_t    reconfig_scheduler_register_new_slot(t_reconfig_scheduler * reconfig_scheduler, void ** rtinit_data, struct mbox *mb_reconfig_request);

int32_t    reconfig_scheduler_register_new_task(t_reconfig_scheduler * reconfig_scheduler, uint32_t slotmask, uint32_t contextsize, void * initdata, char ** bitstream);

int32_t    reconfig_scheduler_dispatcher_feedback(t_reconfig_scheduler * reconfig_scheduler, uint32_t slot, uint32_t oldthreadid, uint32_t newthreadid);



int32_t    reconfig_scheduler_buffer_bitstream(char* filename, t_bitstream * bitstream);

void       reconfig_dispatcher_init(t_reconfig_dispatcher * reconfig_dispatcher );

#endif
