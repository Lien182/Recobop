#ifndef RECONFIG_SCHEDULER_H
#define RECONFIG_SCHEDULER_H

#include <pthread.h>
#include <stdint.h>
#include "reconfig.h"
#include "reconfig_queue.h"
#include "reconfig_dispatcher.h"

#define SLOTMASK_SLOT_0 0x00000001
#define SLOTMASK_SLOT_1 0x00000002

#define TASKSTATE_RUNNING     0x00000001
#define TASKSTATE_STOP        0x00000002
#define TASKSTATE_WAITING     0x00000003


typedef struct
{
    uint32_t threadid;
    uint32_t state;
    uint32_t slotmask;
    void *   context;
}t_reconfig_task;



typedef struct 
{
     t_reconfig_queue         * reconfig_queue;
     t_reconfig_dispatcher    * reconfig_dispatcher;
     t_reconfig_task          taskset[MAX_THREADS];
     uint32_t                 taskset_size;     
}t_reconfig_scheduler;



#endif