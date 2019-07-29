#ifndef CYCLE_TIMER_H
#define CYCLE_TIMER_H

#include <pthread.h>
#include <stdint.h>


typedef struct 
{
    pthread_t           timer_thread;
    uint64_t            period;
    pthread_mutex_t*    mutex;
    pthread_cond_t*     cond;
}t_cycle_timer;


void cycle_timer_init(t_cycle_timer * cycle_timer, uint64_t period, pthread_mutex_t*    mutex, pthread_cond_t*     cond );
void cycle_timer_wait(t_cycle_timer * cycle_timer);

#endif