#include "axi_timer.h"
#include "memory.h"

t_axi_timer * axi_timer_init( uint32_t base_addr)
{
	t_axi_timer * axi_timer;

	axi_timer = (t_axi_timer *)mmap(0, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, base_addr);
	if (axi_timer == MAP_FAILED) {
		printf("Timer Init: Could not map memory\n");
		return NULL;
	}

	return axi_timer;
}

void axi_timer_start(t_axi_timer * axi_timer,uint32_t channel, uint32_t mode, uint32_t interval )
{
	if(channel & TIMER_AXI_START_CHANNEL_0)
	{
		if(mode == TIMER_AXI_MODE_CAPTURE)
		{
			//Enable timer0 and external capture trigger timer 0
			axi_timer->TCSR0 = (1<<8) | (1<<7) | (1<<3) | (1<<4) | (1<<0);
		}
		else if(mode == TIMER_AXI_MODE_GENERATE)
		{
			axi_timer->TLR0 = -interval + 2;
			axi_timer->TCSR0 = (1<<2) | (1<<4) | (1<<7);
		}

	}

	if(channel & TIMER_AXI_START_CHANNEL_1)
	{
		if(mode == TIMER_AXI_MODE_CAPTURE)
		{
			//Enable timer1 and external capture trigger timer 1
			axi_timer->TCSR1 = (1<<8) | (1<<7) | (1<<3) | (1<<4) | (1<<0);
		}

	}

}

void axi_timer_stop(t_axi_timer * axi_timer, uint32_t channel)
{
	if(channel & TIMER_AXI_START_CHANNEL_0)
		axi_timer->TCSR0 &= ~(1<<7);
	if(channel & TIMER_AXI_START_CHANNEL_1)
		axi_timer->TCSR1 &= ~(1<<7);
	
}

