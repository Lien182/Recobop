/********************************************************************          
* axi_servo.c            -- user space driver for the axi servo     *
*                        	ip-core			                        *
*                                                                   *  
* Author(s):  Christian Lienen                                      *   
*                                                                   *   
********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "axi_servo.h"
#include "memory.h"

t_axi_servo * axi_servo_init( uint32_t base_addr)
{
	t_axi_servo * axi_servo;

	axi_servo = (t_axi_servo *)mmap(0, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, memfd, base_addr);
	if (axi_servo == MAP_FAILED) {
		printf("[AXI Servo] Init could not map memory\n");
		return NULL;
	}

	return axi_servo;
}
