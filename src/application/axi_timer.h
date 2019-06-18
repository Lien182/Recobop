#ifndef AXI_TIMER_H
#define AXI_TIMER_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <signal.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>


//All offsets in DWORDS
#define TIMER_AXI_REG_OFFSET_TCSR0	0x00000000
#define TIMER_AXI_REG_OFFSET_TLR0	0x00000001
#define TIMER_AXI_REG_OFFSET_TCR0	0x00000002
#define TIMER_AXI_REG_OFFSET_TCSR1	0x00000004
#define TIMER_AXI_REG_OFFSET_TLR1	0x00000005
#define TIMER_AXI_REG_OFFSET_TCR1	0x00000006

#define TIMER_AXI_START_CHANNEL_0	0x00000001
#define TIMER_AXI_START_CHANNEL_1	0x00000002

#define TIMER_AXI_MODE_CAPTURE		0x00000000
#define TIMER_AXI_MODE_GENERATE		0x00000001

typedef struct
{
	volatile uint32_t	TCSR0;
	volatile uint32_t 	TLR0;
	volatile uint32_t	TCR0;
	volatile uint32_t 	reserved_1;
	volatile uint32_t	TCSR1;
	volatile uint32_t	TLR1;
	volatile uint32_t	TCR1;
	volatile uint32_t	reserved_2;

}t_axi_timer;



t_axi_timer * axi_timer_init( uint32_t base_addr);
void axi_timer_start(t_axi_timer * axi_timer,uint32_t channel, uint32_t mode, uint32_t interval );
void axi_timer_stop(t_axi_timer * axi_timer, uint32_t channel);


#endif