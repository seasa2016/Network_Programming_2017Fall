#ifndef multi
#define multi

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h> 
#include<sys/socket.h>
#include<netinet/in.h>
#include<vector>
#include<fcntl.h>
#include<sys/wait.h>
#include<arpa/inet.h>
#include<sys/signalfd.h>
#include<signal.h>
#include<sys/ipc.h>
#include<sys/shm.h>       
#include<sys/types.h>
#include<sys/stat.h>
#include<vector>
#include <sys/time.h>
#include<errno.h>

struct fire{
    char type;
    char id[36];
};


void handler(int);
void connecting(int sockfd, unsigned char*, unsigned char VN, unsigned char CD, unsigned short DST_PORT, unsigned int DST_IP );
void binding(int sockfd, unsigned char* , unsigned char VN, unsigned char CD, unsigned short DST_PORT, unsigned int DST_IP );









#endif