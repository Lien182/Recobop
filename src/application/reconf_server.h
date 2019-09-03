/********************************************************************          
* reconf_server.h -- 	remote reconfiguration server               *
*                                                                   *  
* Author(s):  Christian Lienen                                      *   
*                                                                   *   
********************************************************************/  

#ifndef RECONF_SERVER_H
#define RECONF_SERVER_H

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

#include "reconfig.h"

#define RECONF_REQUEST_RGB2GRAY 100
#define RECONF_REQUEST_SOBEL    200

typedef struct
{
    t_bitstream bitstreams[2];
    uint32_t    nbitstreams;
    uint32_t    shutdown;
    pthread_t   thread;
    uint16_t    port;
    int         sockfd; 
    uint32_t*   rc_flag;
    struct mbox * reconf_mb;
    struct reconos_thread*  rt;
}
t_reconf_server;


uint32_t reconf_server_init(t_reconf_server * reconf_server, uint16_t port, char ** bitstreams, uint32_t nbitstreams, uint32_t * rc_flag, struct reconos_thread *rt, struct mbox * recon_mb);

#endif