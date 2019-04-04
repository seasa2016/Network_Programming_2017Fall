#include"np1_service.h"


int main(int argc,char* argv[])
{
	int sockfd,port;
	struct sockaddr_in serv_addr;

	if(argc==2)
	{
		sscanf(argv[1],"%d",&port);
	}
	else 
		port=11400;
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		fprintf(stderr,"server:can't open stream socket\n");
		exit(-1);
	}	
	/*
		bind the local address
	*/

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
	setenv("PATH","bin:.",1);

	singal_server(sockfd);

	return 0;
}
