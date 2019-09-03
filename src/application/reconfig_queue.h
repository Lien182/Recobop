/********************************************************************          
* reconfig_queue.c   -- reconfiguration queue mainly implements     *
*                       an array of queueus for the scheduler       *
*                                                                   *  
* Author(s):  Christian Lienen                                      *   
*                                                                   *   
********************************************************************/

#ifndef RECONFIG_QUEUE_H
#define RECONFIG_QUEUE_H

#include <stdint.h>
#include "queue.h"
#include "reconfig_settings.h"


typedef struct 
{
     t_queue slotqueue[MAX_THREADS]; 

}t_reconfig_queue;


void      reconfig_queue_init(t_reconfig_queue * reconfig_queue );

void      reconfig_queue_enqueue(t_reconfig_queue * reconfig_queue, uint32_t slotid , uint32_t threadid);

uint32_t  reconfig_queue_dequeue(t_reconfig_queue * reconfig_queue, uint32_t slotid);

uint32_t  reconfig_queue_isempty(t_reconfig_queue * reconfig_queue);
#endif