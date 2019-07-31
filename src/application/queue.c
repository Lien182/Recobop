#include "queue.h"
#include <stdio.h>
#include <sys/mman.h>
#include <limits.h>
#include <string.h>



int queue_init(t_queue * queue, uint32_t size)
{
        queue->buf = (uint32_t*)malloc(size * sizeof(uint32_t));
        if(queue->buf == NULL)
                return -1;
        queue->head = 0;
        queue->tail = 0;
        queue->size = size;
        memset(queue->buf, 0, queue->size * sizeof(uint32_t));
        return 0;
}


void queue_enqueue(t_queue * queue, uint32_t newval)
{
        queue->buf[queue->tail] = newval;
        if(queue->tail == queue->size-1)
                queue->tail = 0;
        else
                queue->tail++;        
}

uint32_t queue_dequeue(t_queue * queue)
{
        uint32_t tmp = queue->buf[queue->head];
        if(queue->head == queue->size-1)
                queue->head = 0;
        else
                queue->head++;
        return tmp;
        
}

uint32_t queue_isempty(t_queue * queue)
{
        if(queue->head == queue->tail)
                return 1;
        else
                return 0;        
}