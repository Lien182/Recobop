/********************************************************************          
* queue.c           -- queue structure for the reconf server        *
*                                                                   *
*                                                                   *  
* Author(s):  Christian Lienen                                      *   
*                                                                   *   
********************************************************************/

#include "queue.h"
#include <stdio.h>
#include <sys/mman.h>
#include <limits.h>
#include <string.h>
#include <pthread.h>

int queue_init(t_queue * queue, uint32_t size)
{
        queue->buf = (uint32_t*)malloc(size * sizeof(uint32_t));
        if(queue->buf == NULL)
                return -1;
        queue->head = 0;
        queue->tail = 0;
        queue->size = size;
        memset(queue->buf, 0, queue->size * sizeof(uint32_t));

        sem_init(&(queue->sem_read),0 , 0);
        sem_init(&(queue->sem_write), 0, size);
        pthread_mutex_init(&queue->queue_mutex, NULL);

        return 0;
}


void queue_enqueue(t_queue * queue, uint32_t newval)
{
        sem_wait(&(queue->sem_write));        
        pthread_mutex_lock(&queue->queue_mutex);


        queue->buf[queue->tail] = newval;
        if(queue->tail == queue->size-1)
                queue->tail = 0;
        else
                queue->tail++;        

        pthread_mutex_unlock(&queue->queue_mutex);
        sem_post(&(queue->sem_read));
}

uint32_t queue_dequeue(t_queue * queue)
{
        sem_wait(&(queue->sem_read));
        pthread_mutex_lock(&queue->queue_mutex);
        
        uint32_t tmp = queue->buf[queue->head];
        if(queue->head == queue->size-1)
                queue->head = 0;
        else
                queue->head++;
        
        pthread_mutex_unlock(&queue->queue_mutex);
        sem_post(&(queue->sem_write));
        return tmp;

}

uint32_t queue_isempty(t_queue * queue)
{
        if(queue->head == queue->tail)
                return 1;
        else
                return 0;        
}