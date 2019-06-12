#ifndef HDMI_INPUT_H
#define HDMI_INPUT_H

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
#include <string.h>
#include <sys/ioctl.h>


#include <linux/videodev2.h>

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