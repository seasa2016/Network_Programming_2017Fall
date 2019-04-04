#include"multi.h"

int main(int argc,char *argv[])
{
    int sockfd,port,status;
	struct sockaddr_in serv_addr,cli_addr;
    int newsockfd,clilen,childpid;
	char temp[64];

    //printf("argc:%d\n",argc);
    if(argc!=2)
    {
        fprintf(stderr,"error argument\n");
        exit(-1);
    }
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		fprintf(stderr,"server:can't open stream socket\n");
		exit(-1);
	}	
	/*
		bind the local address
	*/
	sscanf(argv[1],"%d",&port);
    
    bzero((char*)&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family	=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port	=htons(port);

	if(bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)
	{
		fprintf(stderr,"server:can't bind local address\n");
		exit(-1);
	}
	listen(sockfd,0);
	setenv("SERVER_ADDR",inet_ntoa(serv_addr.sin_addr),1);
	sprintf(temp,"%u",htons(serv_addr.sin_port));
	setenv("SERVER_PORT",temp,1);
	for(;;)
	{
		clilen	=sizeof(cli_addr);
		newsockfd	=accept(sockfd,(struct sockaddr*)&cli_addr,(socklen_t*)&clilen);

		if(newsockfd<0)
		{
			fprintf(stderr,"server:accept error\n");
			exit(-1);
		}
		if((childpid=fork())<0)
		{
			fprintf(stderr,"server:fork error\n");
		}
		else if(childpid==0)	//in child process
		{
			close(sockfd);
			
			setenv("REMOTE_ADDR",inet_ntoa(cli_addr.sin_addr),1);
			sprintf(temp,"%u",htons(cli_addr.sin_port));
			setenv("REMOTE_PORT",temp,1);
			handler(newsockfd);
			exit(0);
		}
		close(newsockfd);
		waitpid(childpid, &status, WNOHANG);
	}
	return 0;
}