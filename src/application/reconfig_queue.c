#include "reconfig_queue.h"
#include <stdio.h>
#include <sys/mman.h>
#include <limits.h>


void reconfig_queue_init(t_reconfig_queue * reconfig_queue )
{
        int i;
        for(i = 0;  i < MAX_SLOTS; i++)
                queue_init(&(reconfig_queue->slotqueue)[i], MAX_THREADS);

        pthread_mutex_init(&reconfig_queue->queue_mutex, NULL);
	
}

void reconfig_queue_enqueue(t_reconfig_queue * reconfig_queue, uint32_t slotid , uint32_t threadid)
{
        pthread_mutex_lock(&reconfig_queue->queue_mutex);
        /* Do work. */
        queue_enqueue(&reconfig_queue->slotqueue[slotid], threadid);
        pthread_mutex_unlock(&reconfig_queue->queue_mutex);
}

uint32_t reconfig_queue_dequeue(t_reconfig_queue * reconfig_queue, uint32_t slotid)
{
        uint32_t ret;
        pthread_mutex_lock(&reconfig_queue->queue_mutex);
        /* Do work. */
        ret = queue_dequeue(&reconfig_queue->slotqueue[slotid]);
        pthread_mutex_unlock(&reconfig_queue->queue_mutex);
        return ret;
}