/********************************************************************          
* hdmi_input.h           -- hdmi userspace input driver based on    *
*                        	v4l examples                            *
*                                                                   *  
* Author(s):  Christian Lienen                                      *   
*                                                                   *   
********************************************************************/
#ifndef HDMI_INPUT_H
#define HDMI_INPUT_H

#include <stdint.h>
#include <pthread.h>

typedef struct
{
    int fd;
    uint32_t width;
    uint32_t height;
    uint32_t* image;
    struct mbox *mb;
    pthread_t thread;
}t_hdmi_input;



uint32_t hdmi_input_init(t_hdmi_input * hdmi_input, char * device , struct mbox *buffer_pointer );

#endif