#include "reconf_server.h"
#include <errno.h>
#include "reconos.h"
#include "reconos_calls.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>


#include "a9timer.h"
#include "recobop.h"

int32_t reconf_server_buffer_bitstream(char* filename, t_bitstream * bitstream)
{

	FILE *bitfile;

	bitfile = fopen(filename, "rb");
	if(!bitfile){
		printf("[RECONF SERVER] Error opening bitfile %s\n",filename);
		return -1;
	}

	fseek(bitfile,0L,SEEK_END);
	bitstream->size=ftell(bitfile);
	rewind(bitfile);

	bitstream->data = (char *)malloc(bitstream->size*sizeof(char));
	if(!(bitstream->data)){
		printf("[RECONF SERVER] Error allocating memory for bitstream %s\n",filename);
		return -1;
	}
	fread(bitstream->data, sizeof(char), bitstream->size, bitfile);
	fclose(bitfile);

        printf("[RECONF SERVER] Loaded bitfile %s [%d bytes] into RAM \n", filename, bitstream->size);
        return 0;
}

int reconf_server_reconfigure(t_bitstream bitstream, uint32_t threadid, uint32_t partial){
	/* construct path of bitfile */


	int fd_partial = open("/sys/devices/soc0/amba/f8007000.devcfg/is_partial_bitstream", O_RDWR);
	if(fd_partial < 0){
		printf("[RECONF SERVER] Failed to open xdevcfg attribute 'is_partial_bitstream' when configuring threadid %d\n",threadid);
		return -1;
	}

	char partial_flag[2];
	if(!partial) {
		strcpy(partial_flag,"0");
	}
	else {
		strcpy(partial_flag,"1");
	}
	write(fd_partial, partial_flag, 2);
	close(fd_partial);

	fd_partial = open("/dev/xdevcfg", O_RDWR);
	if(fd_partial < 0){
		printf("[RECONF SERVER] Failed to open xdevcfg device when configuring threadid %d\n",threadid);
		return -1;
	}
	printf("[RECONF SERVER] Opened xdevcfg. Configuring with %u bytes\n",bitstream.size);
	write(fd_partial, bitstream.data, bitstream.size);
	int fd_finish_flag = open("/sys/devices/soc0/amba/f8007000.devcfg/prog_done", O_RDWR);
	char finish_flag = '0';

	/* wait until reconfiguration is finished */
	while(finish_flag != '1'){
		read(fd_finish_flag,&finish_flag,1);
	}
	printf("[RECONF SERVER] Reconfiguration with bitfile threadid %d finished\n", threadid);
	close(fd_partial);
	close(fd_finish_flag);

	return 0;

}

void* reconf_server_thread(void * arg)
{
	uint32_t tStartRequest, tStartReconfiguration, tEndReconfiguration, tEndRequest;
	t_reconf_server * reconf_server = (t_reconf_server*)arg;

	struct hwslot *act_hwslot;
    act_hwslot = reconf_server->rt->hwslot;

    struct sockaddr_in servaddr, cliaddr; 
    uint8_t buf[11];  
	int32_t request; 

    // Creating socket file descriptor 
    if ( (reconf_server->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("[RECONF SERVER] Can not create a socket \n"); 
        exit(EXIT_FAILURE); 
    } 
	else
	{
		perror("[RECONF SERVER] create socket \n");
	}
	
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    servaddr.sin_family    		= AF_INET;  
    servaddr.sin_addr.s_addr 	= INADDR_ANY; 
    servaddr.sin_port = htons(reconf_server->port); 
      
    if ( bind(reconf_server->sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("[RECONF SERVER] binding failed\n"); 
        exit(EXIT_FAILURE); 
    } 
      
    int len, n; 

	while(1)
	{
		printf("[RECONF SERVER] wait for new reconfig request\n");
		n = recvfrom(reconf_server->sockfd, (char *)buf, 10, MSG_WAITALL, ( struct sockaddr *) &servaddr, &len); 
		tStartRequest = a9timer_getvalue(a9timer);
    	buf[n] = '\0'; 
    	printf("[RECONF SERVER] new reconf request %s\n", buf);

		request = atoi(buf);

		switch(request)
		{
			case RECONF_REQUEST_RGB2GRAY: 
				printf("[RECONF SERVER] RECONF_REQUEST_RGB2GRAY\n");
				*(reconf_server->rc_flag) = 1;
				mbox_get(reconf_server->reconf_mb);
				reconos_thread_suspend_block(reconf_server->rt);
				*(reconf_server->rc_flag) = 0;
				tStartReconfiguration = a9timer_getvalue(a9timer);
				reconf_server_reconfigure(reconf_server->bitstreams[0], 0, 1);
				tEndReconfiguration = a9timer_getvalue(a9timer);
				reconos_thread_resume(reconf_server->rt,((int*)act_hwslot)[0]); 
				tEndRequest = a9timer_getvalue(a9timer);
				break;

			case RECONF_REQUEST_SOBEL:
				printf("[RECONF SERVER] RECONF_REQUEST_SOBEL\n"); 
				*(reconf_server->rc_flag) = 1;
				mbox_get(reconf_server->reconf_mb);
				reconos_thread_suspend_block(reconf_server->rt);
				*(reconf_server->rc_flag) = 0;
				reconfigure(reconf_server->bitstreams[1], 0, 1);
				reconos_thread_resume(reconf_server->rt,((int*)act_hwslot)[0]); 
				break;

			default: 
				printf("[RECONF SERVER] invalid reconfiguration request! \n "); 
				break;
		}

		printf("[RECONF SERVER]  Kernel switch took %3.4f ms\n", (double)(tEndReconfiguration - tStartReconfiguration)* 0.00000003);
		printf("[RECONF SERVER]  Request took %3.4f ms\n", (double)(tEndRequest - tStartRequest)* 0.00000003);
	}

	close(reconf_server->sockfd); 
    return NULL; 

}



uint32_t reconf_server_init(t_reconf_server * reconf_server, uint16_t port, char ** bitstreams, uint32_t nbitstreams, uint32_t * rc_flag, struct reconos_thread *rt, struct mbox * recon_mb)
{
	int i = 0;
	reconf_server->port = port;
	reconf_server->shutdown = 0;
	reconf_server->nbitstreams;
	reconf_server->rt = rt;
	reconf_server->rc_flag = rc_flag;
	reconf_server->reconf_mb = recon_mb;

	for(i = 0; i < nbitstreams; i++)
	{
		reconf_server_buffer_bitstream(bitstreams[i], &(reconf_server->bitstreams[i]));
	}

    if(pthread_create(&(reconf_server->thread), 0, &reconf_server_thread, (void*)reconf_server) != 0)
	{
		printf("[RECONF SERVER] Error during check thread starting \n");
	}


    return 0;
}


