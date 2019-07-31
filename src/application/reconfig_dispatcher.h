#ifndef RECONFIG_DISPATCHER_H
#define RECONFIG_DISPATCHER_H

#include <pthread.h>
#include <stdint.h>
#include "mbox.h"
#include "reconfig.h"

typedef struct 
{
     pthread_t reconfig_dispatcher_thread;
     struct mbox *mb_reconfig_request;
     struct mbox *mb_reconfig_response;     

     //uint32_t running_task_set[MAX_SLOTS];

}t_reconfig_dispatcher;


void reconfig_dispatcher_init(t_reconfig_dispatcher * reconfig_dispatcher );

#endif