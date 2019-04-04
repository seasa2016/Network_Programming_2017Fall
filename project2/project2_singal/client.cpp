#include"np1_service.h"
#define MAXLINE 512

int main(int argc,char* argv[])
{
	int sockfd;
	struct sockaddr_in serv_addr;


	serv_addr.sin_family	=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port	=htons(9487);

	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
		fprintf(stderr,"error1\n");
	if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)
		fprintf(stderr,"error2\n");
	
	int n;
	char sendline[MAXLINE],recvline[MAXLINE];

	strcpy(sendline,"\n");
	write(sockfd,sendline,strlen(sendline));
	
	readline(sockfd,recvline,MAXLINE);
//	recvline[n]=0;
	printf("%s\n",recvline);

	
	
	while(fgets(sendline,MAXLINE,stdin)!=NULL)
	{
		n=strlen(sendline);
		if(write(sockfd,sendline,n)!=n)
			fprintf(stderr,"written error");

		n=readline(sockfd,recvline,MAXLINE);
		if(n<0)
			fprintf(stderr,"readline error");
		recvline[n]='\0';
		fputs(recvline,stdout);
	}
	


	close(sockfd);
	return 0;
}
