#ifndef HDMI_OUTPUT_H
#define HDMI_OUTPUT_H

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
#include <sys/ioctl.h>
#include <linux/fb.h>

typedef struct
{
    uint32_t* image;
    int fd_fb;
    uint32_t width;
    uint32_t height;
}t_hdmi_output;



uint32_t hdmi_output_init(t_hdmi_output * hdmi_output, char * device  );

#endif