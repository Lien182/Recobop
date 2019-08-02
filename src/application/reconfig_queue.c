#include "reconfig_queue.h"
#include <stdio.h>
#include <sys/mman.h>
#include <limits.h>


void reconfig_queue_init(t_reconfig_queue * reconfig_queue )
{
        int i;
        for(i = 0;  i < MAX_SLOTS; i++)
                queue_init(&(reconfig_queue->slotqueue)[i], MAX_THREADS);

        
	
}

void reconfig_queue_enqueue(t_reconfig_queue * reconfig_queue, uint32_t slotid , uint32_t threadid)
{
        queue_enqueue(&reconfig_queue->slotqueue[slotid], threadid);
}

uint32_t reconfig_queue_dequeue(t_reconfig_queue * reconfig_queue, uint32_t slotid)
{
        uint32_t ret;
        ret = queue_dequeue(&reconfig_queue->slotqueue[slotid]);
        return ret;
}