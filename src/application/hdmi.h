/********************************************************************          
* hdmi.c           -- hdmi structure for hdmi input output          *
*                                                                   *
*                                                                   *  
* Author(s):  Christian Lienen                                      *   
*                                                                   *   
********************************************************************/

#ifndef HDMI_H
#define HDMI_H

#include <stdint.h>
#include "hdmi_input.h"
#include "hdmi_output.h"

typedef struct 
{
	t_hdmi_input	hdmi_input;
	//Positon in the struct is important for the memory access by the video hardware threads
	uint32_t		rc_flag;	
	t_hdmi_output 	hdmi_output;
	struct reconos_thread *thread_p;
}
t_hdmi;


#endif