#include "hdmi_output.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>


uint32_t hdmi_output_init(t_hdmi_output * hdmi_output, char * device  )
{
    int i = 0;
   	int bitspp = 0;

   	hdmi_output->fd_fb = open(device, O_RDWR);
   	if(hdmi_output->fd_fb <= 0)
   	{
		printf("[HMDI output] Can not open the framebuffer \n");
		return -1;
	}

	struct fb_var_screeninfo screeninfo;
	ioctl(hdmi_output->fd_fb, FBIOGET_VSCREENINFO, &screeninfo);

	bitspp = screeninfo.bits_per_pixel;
	if(bitspp != 32) {
		printf("[HMDI output] Wrong color resolution\n");
		close(hdmi_output->fd_fb);
		return -2; 
	}

	hdmi_output->width  = screeninfo.xres;
	hdmi_output->height = screeninfo.yres;

	hdmi_output->image = (unsigned int*) mmap(0, hdmi_output->width * hdmi_output->height * sizeof(uint32_t), PROT_READ | PROT_WRITE, MAP_SHARED, hdmi_output->fd_fb, 0);

	for(i = 0; i < hdmi_output->height * hdmi_output->width; i++)
			hdmi_output->image[i] = 0;
	
	return 0;
}
