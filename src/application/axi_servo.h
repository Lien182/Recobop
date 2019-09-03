/********************************************************************          
* axi_servo.h            -- user space driver for the axi servo     *
*                        	ip-core			                        *
*                                                                   *  
* Author(s):  Christian Lienen                                      *   
*                                                                   *   
********************************************************************/

#ifndef AXI_SERVO_H
#define AXI_SERVO_H

#include <stdint.h>

typedef struct
{
	volatile uint32_t	SERVO_0;
	volatile uint32_t 	SERVO_1;
	volatile uint32_t	SERVO_2;
	volatile uint32_t 	SERVO_3;
	volatile uint32_t	SERVO_4;
	volatile uint32_t	SERVO_5;

}t_axi_servo;


t_axi_servo * axi_servo_init( uint32_t base_addr);

#endif