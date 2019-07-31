#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>


typedef struct 
{
     uint32_t * buf;
     uint32_t tail;
     uint32_t head;
     uint32_t size;
} t_queue;



int       queue_init(t_queue * queue, uint32_t size);
void      queue_enqueue(t_queue * queue, uint32_t newval);
uint32_t  queue_dequeue(t_queue * queue);
uint32_t  queue_isempty(t_queue * queue);

#endif