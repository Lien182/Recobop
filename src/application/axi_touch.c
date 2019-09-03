/********************************************************************          
* axi_touch.c            -- user space driver for the axi touch     *
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

#include "axi_touch.h"
#include "memory.h"

t_axi_touch * axi_touch_init( uint32_t base_addr)
{
	t_axi_touch * axi_touch;

	axi_touch = mmap(0, 0x10000, PROT_READ| PROT_WRITE, MAP_SHARED | MAP_POPULATE, memfd, base_addr);
	if (axi_touch == MAP_FAILED) {
		printf("[AXI Touch] ERROR: Could not map memory\n");
		return NULL;
	}

	return axi_touch;
}