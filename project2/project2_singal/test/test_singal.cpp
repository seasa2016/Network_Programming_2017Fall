#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include<vector>
#include<fcntl.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include"sockop.h"

#define QLEN 5
#define BUFSIZE 4096

extern int errno;

int main(int argc,char *argv[])
{
    struct sockaddr_in fsin,cli_addr;
    int msock;
    fd_set rfds;
    fd_set afds;
    int alen;
    int fd,nfds;
    char temp[1024];
    int index;

    msock=passivesock("9487","tcp",QLEN);

    nfds=getdtablesize();
    FD_ZERO(&afds);
    FD_SET(msock,&afds);

    while(1)
    {
        memcpy(&rfds,&afds,sizeof(rfds));

        if(select(nfds,&rfds,(fd_set*)0,(fd_set*)0,(struct timeval*) 0)<0)
        {
            exit(1);
            fprintf(stderr,"select error:%s\n",strerror(errno));
        }
        if(FD_ISSET(msock,&rfds))
        {
            int ssock;
            alen=sizeof(fsin);
            ssock=accept(msock,(struct sockaddr*)&fsin,(socklen_t*)&cli_addr);

            if(ssock<0)
            {
                exit(1);
                fprintf(stderr,"accept error:%s\n",strerror(errno));
            }
            FD_SET(ssock,&afds);
        }
        for(fd=0;fd<nfds;fd++){
            if(fd!=msock && FD_ISSET(fd,&rfds))
            {
                index=read(fd,temp,1024);
                write(fd,temp,index);
            }
        }
    }
}