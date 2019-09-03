/********************************************************************          
* a9timer.h             -- 	header for the global a9 timer from the *
*                        	cortex-a9 processor in the Zynq Soc     *
*                                                                   *  
* Author(s):  Christian Lienen                                      *   
*                                                                   *   
********************************************************************/
#ifndef A9TIMER_H
#define A9TIMER_H

#include <stdint.h>

#define A9TIMER_CAPTURE_START   0x00000001
#define A9TIMER_CAPTURE_STOP    0x00000002
#define A9TIMER_CAPTURE_SINGLE  0x00000001 // in single mode, timer value captured in tStart

typedef struct
{
    volatile uint32_t	TMR_CNT_REG_L;
    volatile uint32_t 	TMR_CNT_REG_H;
    volatile uint32_t	TMR_CNTRL_REG;
    volatile uint32_t 	TMR_INT_STATUS_REG;
    volatile uint32_t	TMR_COMPARE_REG_L;
    volatile uint32_t	TMR_COMPARE_REG_H;
    volatile uint32_t	TMR_AUTO_INC_REG;

}t_a9timer;

typedef struct
{
    volatile uint32_t tStart;
    volatile uint32_t tStop;
    
}t_a9timer_capture;



t_a9timer * a9timer_init( void );
uint32_t    a9timer_getvalue(t_a9timer * a9timer );
void        a9timer_caputure(t_a9timer * a9timer, t_a9timer_capture * capture, uint32_t mode );

#endif