#include "cycle_timer.h"
#include <stdio.h>
#include <sys/mman.h>
#include <limits.h>

void * cylce_timer_thread(void* arg)
{
    t_cycle_timer * cycle_timer;

    cycle_timer = (t_cycle_timer*)arg;

    struct timespec tim;
    tim.tv_sec = (cycle_timer->period * 1000000) / 1000000000;
    tim.tv_nsec = (cycle_timer->period * 1000000) % 1000000000;

    printf("Hello from cycletimer %d ns %d s \n",tim.tv_nsec,tim.tv_sec);

    while(1)
    {
        pthread_mutex_lock(cycle_timer->mutex);
        pthread_cond_broadcast(cycle_timer->cond);
        pthread_mutex_unlock(cycle_timer->mutex);

        nanosleep(&tim , NULL);

    }

}


void cycle_timer_init(t_cycle_timer * cycle_timer, uint64_t period, pthread_mutex_t*    mutex, pthread_cond_t*     cond )
{
    //pthread_mutex_init((cycle_timer->mutex), NULL);
    //pthread_cond_init ((cycle_timer->cond), NULL);

    cycle_timer->mutex = mutex;
    cycle_timer->cond = cond;

    cycle_timer->period = period;

    struct sched_param param;
    pthread_attr_t attr;
    int ret;

    /* Lock memory */
    if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
            printf("mlockall failed: %m\n");
            exit(-2);
    }

    /* Initialize pthread attributes (default values) */
    ret = pthread_attr_init(&attr);
    if (ret) {
            printf("init pthread attributes failed\n");
            return;
    }

    /* Set a specific stack size  */
    ret = pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
    if (ret) {
        printf("pthread setstacksize failed\n");
        return;
    }

    /* Set scheduler policy and priority of pthread */
    ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    if (ret) {
            printf("pthread setschedpolicy failed\n");
            return;
    }
    param.sched_priority = 80;
    ret = pthread_attr_setschedparam(&attr, &param);
    if (ret) {
            printf("pthread setschedparam failed\n");
            return;
    }
    /* Use scheduling parameters of attr */
    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (ret) {
            printf("pthread setinheritsched failed\n");
            return;
    }
    


    pthread_create(&cycle_timer->timer_thread, &attr, &cylce_timer_thread, (void*)cycle_timer);
	
}

void cycle_timer_wait(t_cycle_timer * cycle_timer)
{
        pthread_mutex_lock(cycle_timer->mutex);
        pthread_cond_wait(cycle_timer->cond, cycle_timer->mutex);
        pthread_mutex_unlock(cycle_timer->mutex); 
}