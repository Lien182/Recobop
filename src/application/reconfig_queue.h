#ifndef RECONFIG_QUEUE_H
#define RECONFIG_QUEUE_H

#include <pthread.h>
#include <stdint.h>
#include "queue.h"
#include "reconfig.h"


typedef struct 
{
     pthread_mutex_t queue_mutex;
     t_queue slotqueue[MAX_THREADS]; 

}t_reconfig_queue;


void      reconfig_queue_init(t_reconfig_queue * reconfig_queue );

void      reconfig_queue_enqueue(t_reconfig_queue * reconfig_queue, uint32_t slotid , uint32_t threadid);

uint32_t  reconfig_queue_dequeue(t_reconfig_queue * reconfig_queue, uint32_t slotid);
#endif