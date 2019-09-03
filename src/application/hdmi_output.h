/********************************************************************          
* hdmi_input.h           -- hdmi userspace output driver            *
*                                                                   *
*                                                                   *  
* Author(s):  Christian Lienen                                      *   
*                                                                   *   
********************************************************************/

#ifndef HDMI_OUTPUT_H
#define HDMI_OUTPUT_H

#include <stdint.h>

typedef struct
{
    uint32_t* image;    //important that image is the first element
    int fd_fb;
    uint32_t width;
    uint32_t height;
}t_hdmi_output;



uint32_t hdmi_output_init(t_hdmi_output * hdmi_output, char * device  );

#endif