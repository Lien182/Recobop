/********************************************************************          
* axi_touch.h            -- user space driver for the axi touch     *
*                        	ip-core			                        *
*                                                                   *  
* Author(s):  Christian Lienen                                      *   
*                                                                   *   
********************************************************************/

#ifndef AXI_TOUCH_H
#define AXI_TOUCH_H

#include <stdint.h>

typedef struct
{
	volatile uint32_t	POS_X;
	volatile uint32_t 	POS_Y;

}t_axi_touch;


t_axi_touch * axi_touch_init( uint32_t base_addr);

#endif