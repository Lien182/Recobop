/********************************************************************          
* axi_timer.h            -- user space driver for the diff_timer    *
*                        	ip-core			                        *
*                                                                   *  
* Author(s):  Christian Lienen                                      *   
*                                                                   *   
********************************************************************/

#ifndef DIFFERENCE_MEASUREMENT_H
#define DIFFERENCE_MEASUREMENT_H

#include <stdint.h>

typedef struct
{
	volatile uint32_t	CONFIG;
	volatile uint32_t 	CNT_REG;
	volatile uint32_t	CAP0;
	volatile uint32_t 	CAP1;
	volatile uint32_t	CAP2;
	volatile uint32_t	CAP3;

}t_diff_measurement;


t_diff_measurement * diff_measurement_timer_init( uint32_t base_addr);

#endif