#include"np1_service.h"


int main(int argc,char* argv[])
{
	int sockfd,newsockfd,clilen,childpid,status;
	struct sockaddr_in cli_addr,serv_addr;
	int port;

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
	
	if(argc == 2)
	{
		sscanf(argv[1],"%d",&port);
		serv_addr.sin_port = htons(port);
	}
	else
		serv_addr.sin_port = htons(11400);
		
	if(bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)
	{
		fprintf(stderr,"server:can't bind local address\n");
		exit(-1);
	}
	listen(sockfd,0);
	
	
//	printf("%s\n",getenv("PWD"));
	setenv("PATH","bin:.",1);
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
			//replace stdin stdout stderr with 
			dup2(newsockfd,STDIN_FILENO);
			dup2(newsockfd,STDOUT_FILENO);
			dup2(newsockfd,STDERR_FILENO);
			close(newsockfd);
			
			str_echo();
			exit(0);
		}
		close(newsockfd);
		waitpid(childpid, &status, WNOHANG);
	}

	return 0;
}
