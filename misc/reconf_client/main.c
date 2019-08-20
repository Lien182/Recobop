#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

// Driver code 
int main(int argc, char **argv) 
{ 
    int sockfd; 
    int32_t request;
    int32_t port;
    char buf[10]; 
    struct sockaddr_in     servaddr; 
  
    if( argc > 2)
	{
        port    = atoi(argv[1]);
		request = atoi(argv[2]);
		printf("[RECONFIG CLIENT] Set Request to %d \n", request);
	}
    else
    {
        printf("[RECONFIG CLIENT] Please choose a new filter for the demonstrator! \n");
        printf("[RECONFIG CLIENT] RECONF_REQUEST_RGB2GRAY  100\n");
        printf("[RECONFIG CLIENT] RECONF_REQUEST_SOBEL     200\n");
        printf("[RECONFIG CLIENT] example: reconfig_client <port> <request>\n");
        exit(0);
    }

    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("[RECONFIG CLIENT] socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(port); 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
      
    int n, len; 
      
    sprintf(buf,"%d",request);
    printf("[RECONFIG CLIENT] Send Request %s \n", buf);
    sendto(sockfd, (const char *)buf, strlen(buf), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 

    close(sockfd); 
    return 0; 
} 

