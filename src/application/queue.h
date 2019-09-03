/********************************************************************          
* queue.h           -- queue structure for the reconf server        *
*                                                                   *
*                                                                   *  
* Author(s):  Christian Lienen                                      *   
*                                                                   *   
********************************************************************/
#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>
#include <semaphore.h>


typedef struct 
{
     uint32_t *          buf;
     uint32_t            tail;
     uint32_t            head;
     uint32_t            size;
     sem_t               sem_write;
     sem_t               sem_read;
     pthread_mutex_t     queue_mutex;
} t_queue;



int       queue_init(t_queue * queue, uint32_t size);
void      queue_enqueue(t_queue * queue, uint32_t newval);
uint32_t  queue_dequeue(t_queue * queue);
uint32_t  queue_isempty(t_queue * queue);

#endif