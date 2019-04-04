#include"np1_service.h"
#define MAXLINE 11000
#define fifo_root "./../tmp/fifo_%d_%d"
#define message_size 2048
using std::vector;


struct command{
	int fd[2];
	command(void){		
		fd[0]=-1;
		fd[1]=-1;
	}
};

struct temp_command{
	char arr[257];
	unsigned int shift; 
};

struct m_send{
	key_t type=9964;
	int mid_type;
	char *arr_type;

	key_t to=9965;
	int mid_to;
	char *arr_to;
	
	key_t message=9966;
	int mid_message;
	char *arr_message;

	key_t who=9967;
	int mid_who;
	char *arr_who;

	key_t able=9968;
	int mid_able;
	char *arr_able;

	key_t cl=9969;
	int mid_cl;
	char *arr_cl;

	bool checker;
}message_send;

struct user_data{
	bool use;
	int fd;
	int pid;
	char name[32];
	char addr[32];
}user[40];
int ssockfd;

void multi(int sockfd)
{
	ssockfd=sockfd;
	int newsockfd,clilen,childpid,status;
	struct sockaddr_in cli_addr;
	int i;
	char arr[256];

	//enable the share memory
	if((message_send.mid_type =shmget(message_send.type, message_size, IPC_CREAT | 0666)) < 0){
        perror("type shmget\n");exit(-1);
    }
	if((message_send.arr_type = (char* )shmat(message_send.mid_type, NULL, 0)) ==(char* ) -1){
        perror("type shmat\n"); exit(-1);
    }
	if((message_send.mid_to =shmget(message_send.to, message_size, IPC_CREAT | 0666)) < 0){
        perror("to shmget\n");  exit(-1);
    }
	if((message_send.arr_to = (char* )shmat(message_send.mid_to, NULL, 0)) ==(char* ) -1){
        perror("to shmat\n"); exit(-1);
    }
	if((message_send.mid_message =shmget(message_send.message, message_size, IPC_CREAT | 0666)) < 0){
        perror("to shmget\n");  exit(-1);
    }
	if((message_send.arr_message = (char* )shmat(message_send.mid_message, NULL, 0)) ==(char* ) -1){
        perror("to shmat\n"); exit(-1);
    }
	if((message_send.mid_who =shmget(message_send.who, message_size, IPC_CREAT | 0666)) < 0){
        perror("who shmget\n");exit(-1);
    }
	if((message_send.arr_who = (char* )shmat(message_send.mid_who, NULL, 0)) ==(char* ) -1){
        perror("who shmat\n"); exit(-1);
    }
	if((message_send.mid_able =shmget(message_send.able, message_size, IPC_CREAT | 0666)) < 0){
        perror("able shmget\n");exit(-1);
    }
	if((message_send.arr_able = (char* )shmat(message_send.mid_able, NULL, 0)) ==(char* ) -1){
        perror("able shmat\n"); exit(-1);
    }
	if((message_send.mid_cl =shmget(message_send.cl, message_size, IPC_CREAT | 0666)) < 0){
        perror("cl shmget\n");exit(-1);
    }
	if((message_send.arr_cl = (char* )shmat(message_send.mid_cl, NULL, 0)) ==(char* ) -1){
        perror("cl shmat\n"); exit(-1);
    }
	message_send.checker=true;
	/*********************************************************************/


	signal(SIGUSR1,check);
	signal(SIGUSR2,check2);

	for(i=0;i<40;i++)
	{
		user[i].use=false;
		user[i].fd=-1;
		user[i].name[0]='\0';
		user[i].addr[0]='\0';
		user[i].pid=-1;
		message_send.arr_able[i]='0';
	}

	for(;;)
	{
		clilen	=sizeof(cli_addr);
		newsockfd	=accept(sockfd,(struct sockaddr*)&cli_addr,(socklen_t*)&clilen);
		fflush(stdout);

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
			//replace stdin stdout stderr with newsockfd
			dup2(newsockfd,STDIN_FILENO);
			dup2(newsockfd,STDOUT_FILENO);
			dup2(newsockfd,STDERR_FILENO);
			close(newsockfd);
			for(i=0;i<40;i++)
				if(user[i].use)
					close(user[i].fd);
			for(i=0;user[i].use;i++);
			message_send.arr_able[i]='1';
			
			str_echo(i);
			exit(0);
		}
		dup2(newsockfd,STDOUT_FILENO);
		dup2(newsockfd,STDERR_FILENO);

		for(i=0;user[i].use;i++);
		
		user[i].use=true;
		user[i].fd=newsockfd;
		user[i].pid=childpid;
		sprintf(user[i].addr,"CGILAB/511");
		//sprintf(user[i].addr,"%s/%d",inet_ntoa(cli_addr.sin_addr),htons(cli_addr.sin_port));
		sprintf(user[i].name,"(no name)");

		
		printf("****************************************\n");
		printf("** Welcome to the information server. **\n");
		printf("****************************************\n");
		
		sprintf(arr,"*** User \'%s\' entered from %s. ***\n",user[i].name,user[i].addr);
		fflush(stdout);

		broadcast(arr);

		printf("%c ",'%');
		fflush(stdout);

		waitpid(childpid, &status, WNOHANG);
		dup2(ssockfd,STDOUT_FILENO);
		dup2(ssockfd,STDERR_FILENO);
	}

	
    if((shmdt(message_send.arr_message)) < 0){
        perror("shmdt");exit(1);
    }
    if((shmdt(message_send.arr_to)) < 0){
        perror("shmdt");exit(1);
    }
    if((shmdt(message_send.arr_type)) < 0){
        perror("shmdt");exit(1);
    }
    if((shmdt(message_send.arr_who)) < 0){
        perror("shmdt");exit(1);
    }
    if((shmdt(message_send.arr_able)) < 0){
        perror("shmdt");exit(1);
    }
    if((shmdt(message_send.arr_cl)) < 0){
        perror("shmdt");exit(1);
    }
    if(shmctl(message_send.mid_message, IPC_RMID, NULL)<0){
        fprintf(stderr,"share fail\n");exit(1);
	}
    if(shmctl(message_send.mid_to, IPC_RMID, NULL)<0){
        fprintf(stderr,"share fail\n");exit(1);
	}
    if(shmctl(message_send.mid_type, IPC_RMID, NULL)<0){
        fprintf(stderr,"share fail\n");exit(1);
	}
    if(shmctl(message_send.mid_who, IPC_RMID, NULL)<0){
        fprintf(stderr,"share fail\n");exit(1);
	}
    if(shmctl(message_send.mid_able, IPC_RMID, NULL)<0){
        fprintf(stderr,"share fail\n");exit(1);
	}
    if(shmctl(message_send.mid_cl, IPC_RMID, NULL)<0){
        fprintf(stderr,"share fail\n");exit(1);
	}
}

void check2(int a)
{
	message_send.checker=false;
}

void check(int b)
{
	//first check type

    int type;
	
	sscanf(message_send.arr_type," %d",&type);

	switch(type)
	{
		case 1:		//name
			name();
		break;
		case 2:		//who
			who();
		break;
		case 3:		//yell
			yell();
		break;
		case 4:		//tell
			tell();
		break;
		case 5:		//tell
			dell();
		break;
		case 6:
			ccl();
		break;
		case 7:
			bro_fifo_out();
		break;
		case 8:
			bro_fifo_in();
		break;
	}
}
void bro_fifo_in()
{
	char arr[1024];
	int pid,i,to;

	sscanf(message_send.arr_who," %d",&pid);
	sscanf(message_send.arr_to," %d",&to);
	for(i=0;i<40;i++)
		if(user[i].use)
			if(user[i].pid==pid)
				break;

	sprintf(arr,"*** %s (#%d) just piped '%s' to %s (#%d) ***\n",user[i].name,i+1,message_send.arr_message,user[to-1].name,to);
	fflush(stdout);
	broadcast(arr);

	kill(pid,SIGUSR2);
}

void bro_fifo_out()
{
	char arr[1024];
	int pid,i,to;

	sscanf(message_send.arr_who," %d",&pid);
	sscanf(message_send.arr_to," %d",&to);
	for(i=0;i<40;i++)
		if(user[i].use)
			if(user[i].pid==pid)
				break;

	sprintf(arr,"*** %s (#%d) just received from %s (#%d) by '%s' ***\n",user[i].name,i+1,user[to-1].name,to,message_send.arr_message);
	fflush(stdout);
	broadcast(arr);
	kill(pid,SIGUSR2);
}

void ccl()
{
}

void name()
{
	char arr[256],comm[256];
	int pid,i,number;

	sscanf(message_send.arr_message," %s",comm);
	sscanf(message_send.arr_who," %d",&pid);

	for(number=0;number<40;number++)
		if(user[number].use)
			if(user[number].pid==pid)
				break;
	
	for(i=0;i<40;i++)
		if(user[i].use)
			if(strcmp(comm,user[i].name)==0)
			{
				sprintf(arr,"*** User '%s' already exists. ***\n",user[i].name);

				write(user[number].fd,arr,strlen(arr));
				kill(pid,SIGUSR2);
				return ;
			}


	strcpy(user[number].name,comm);
	//sprintf(arr,"*** User from %s is named '%s'. ***\n","CGILAB/511",user[number].name);
	sprintf(arr,"*** User from %s is named '%s'. ***\n",user[number].addr,user[number].name);
	fflush(stdout);
	broadcast(arr);
	kill(pid,SIGUSR2);
}
void who()
{
	int i,number,pid;
	sscanf(message_send.arr_who," %d",&pid);

	for(i=0;i<40;i++)
		if(user[i].use)
			if(user[i].pid==pid)
				break;
	number=i;
	dup2(user[i].fd,STDOUT_FILENO);
	dup2(user[i].fd,STDERR_FILENO);

	printf("<ID>\t<nickname>\t<IP/port>\t<indicate me>\n");
	for(i=0;i<40;i++)
		if(user[i].use)
		{
			//printf("%d\t%s\t%s",i+1,user[i].name,"CGILAB/511");
			printf("%d\t%s\t%s",i+1,user[i].name,user[i].addr);
			if(number==i)
				printf("\t<-me\n");
			else
				printf("\n");
		}
	fflush(stdout);
	kill(pid,SIGUSR2);
}
void yell()
{
	char arr[1024];
	int pid,i;

	//sscanf(message_send.arr_message," %s",comm);
	sscanf(message_send.arr_who," %d",&pid);
	for(i=0;i<40;i++)
		if(user[i].use)
			if(user[i].pid==pid)
				break;

	sprintf(arr,"*** %s yelled ***: %s\n",user[i].name,message_send.arr_message);
	fflush(stdout);
	broadcast(arr);
	kill(pid,SIGUSR2);
}
void tell()
{
	char arr[1024];
	int pid,i,shift;

	//sscanf(message_send.arr_message," %s",comm);
	sscanf(message_send.arr_who," %d",&pid);
	sscanf(message_send.arr_to," %d",&shift);

	for(i=0;i<40;i++)
		if(user[i].use)
			if(user[i].pid==pid)
				break;

	if(user[shift-1].use)
	{			
		sprintf(arr,"*** %s told you ***: %s\n",user[i].name,message_send.arr_message);
		fflush(stdout);
		write(user[shift-1].fd,arr,strlen(arr));
	}
	else
	{
		sprintf(arr,"*** Error: user #%d does not exist yet. ***\n",shift);
		fflush(stdout);
		write(user[i].fd,arr,strlen(arr));	
	}
	kill(pid,SIGUSR2);
}
void dell()
{
	char arr[256];
	int pid,i,j;

	sscanf(message_send.arr_who," %d",&pid);
	for(i=0;i<40;i++)
		if(user[i].use)
			if(user[i].pid==pid)
				break;

	fflush(stdout);
	fflush(stderr);

	sprintf(arr,"*** User '%s' left. ***\n",user[i].name);
	broadcast(arr);


	user[i].use=false;
	message_send.arr_able[i]='0';
			
	char fifo_name[128];
	for(j=0;j<40;j++)
	{
		sprintf(fifo_name,"./../tmp/fifo_%d_%d",j+1,i+1);
		//sprintf(fifo_name,"./../../tmp/fifo_%d_%d",j+1,i+1);
		unlink(fifo_name);
		sprintf(fifo_name,"./../tmp/fifo_%d_%d",i+1,j+1);
		//sprintf(fifo_name,"./../../tmp/fifo_%d_%d",i+1,j+1);
		unlink(fifo_name);
	} 
	close(user[i].fd);
	dup2(ssockfd,STDIN_FILENO);
	dup2(ssockfd,STDOUT_FILENO);
	dup2(ssockfd,STDERR_FILENO);
}
void broadcast(char *arr)
{
	for(int i=0;i<40;i++)
		if(user[i].use)
		{
			write(user[i].fd,arr,strlen(arr));
		}
}

void str_echo(int number)
{
	int n;
	char line[MAXLINE],buffer[MAXLINE];
	vector<command> data;

	char *lline;
	while(1)
	{
		line[0]='\0';
		
		while((n=read(0,buffer,MAXLINE)))
		{
			buffer[n]='\0';
			strcat(line,buffer);
			
			if(buffer[n-1]==10)
				break;
		}
		lline=strtok(line,"\r\n");
		
		if(parsing(data,lline,number)==false)	
		{
			fflush(stdout);
			fflush(stderr);
			//close
			
			sprintf(message_send.arr_type,"%d",5);
			sprintf(message_send.arr_who,"%d",getpid());
			kill(getppid(),SIGUSR1);
			
			/***************************************************/
			return ;
		}
		fprintf(stdout,"%c ",'%');
		fflush(stdout);
	}
}

bool push(vector<command> &data,temp_command &message,bool &eexit,int number ,char *comm)
{
	char *ptr,*ptr1,*ptr2;
	bool key_output=false;
	bool key_input=false;
	bool key_fifo=false;
	char temp[MAXLINE];
	char err[50];
	int file,shift;
	int fifo_in,fifo_out;
	char fifo_name[128];
	int shift_fifo_in,shift_fifo_to;
	char com[64];
	char name[64];
	char *app[10];

	strcpy(temp,message.arr);


	/************************************************************/
	ptr=strtok(temp," ");
	strcpy(com,ptr);
	int i=1;
	app[0]=com;
	if(strcmp("exit",ptr)==0)
	{
		eexit=true;
		return true;
	}
	else if(strcmp("setenv",ptr)==0)
	{
		ptr1=strtok(NULL," ");
		ptr2=strtok(NULL," ");
		setenv(ptr1,ptr2,1);
		
		return true;
	}
	else if(strcmp("printenv",ptr)==0)
	{
		ptr1=strtok(NULL," ");
		
		printf("%s=%s\n",ptr1,getenv(ptr1));
		fflush(stdout);
	
		return true;
	}
	//[who], [tell], [yell], [name]
	else if(strcmp("who",ptr)==0)
	{
		sprintf(message_send.arr_type,"%d",2);
		sprintf(message_send.arr_who,"%d",getpid());
		
		kill(getppid(),SIGUSR1);
		
		while(message_send.checker);
		message_send.checker=true;	

		return true;
	}
	else if(strcmp("tell",ptr)==0)
	{
		//*** (sender's name) told you ***: (message)
		//% tell 3 Hello World.
		ptr1=strtok(NULL," ");
		shift=atoi(ptr1);
		ptr1=strtok(NULL,"");

		sprintf(message_send.arr_type,"%d",4);
		sprintf(message_send.arr_message,"%s",ptr1);
		sprintf(message_send.arr_who,"%d",getpid());
		sprintf(message_send.arr_to,"%d",shift);

		kill(getppid(),SIGUSR1);
		
		while(message_send.checker);
		message_send.checker=true;
		return true;
	}
	else if(strcmp("yell",ptr)==0)
	{
		ptr1=strtok(NULL,"");
	
		sprintf(message_send.arr_type,"%d",3);
		sprintf(message_send.arr_message,"%s",ptr1);
		sprintf(message_send.arr_who,"%d",getpid());
		kill(getppid(),SIGUSR1);
		
		while(message_send.checker);
		message_send.checker=true;
		return true;
	}
	else if(strcmp("name",ptr)==0)
	{
		ptr1=strtok(NULL," ");
		
		sprintf(message_send.arr_type,"%d",1);
		sprintf(message_send.arr_message,"%s",ptr1);
		sprintf(message_send.arr_who,"%d",getpid());
		kill(getppid(),SIGUSR1);
		
		while(message_send.checker);
		message_send.checker=true;

		return true;
	}
	else		//normal
	{
		ptr1=strtok(NULL,"");
		if(ptr1)
		{
			strcpy(temp,ptr1);

			//deal with argument
			ptr = strchr(temp,'>');
			if(ptr)		//deal with pipe to others  or output to file
			{
				ptr++;
				strcpy(err,"open file error");
				if(*ptr==' ')		//output to file
				{
					key_output=true;
					sscanf(ptr," %s",name);

					file = open(name, O_RDWR | O_CREAT, 0666);
					if (file == -1)
					{
						fprintf(stderr,"%s\n",err);
						fflush(stderr);
					}
				}
				else
				{
					key_fifo=true;
					sscanf(ptr," %d",&shift_fifo_to);
					/////////check for fifo different for two server

					if(message_send.arr_able[shift_fifo_to-1]=='0')
					{
						fprintf(stderr,"*** Error: user #%d does not exist yet. ***\n",shift_fifo_to);
						fflush(stderr);
						return false;
					}
					sprintf(fifo_name , fifo_root , number+1,shift_fifo_to);
					fflush(stdout);
					fifo_out = open(fifo_name, O_RDONLY | O_NONBLOCK);
					if(fifo_out>=0)
					{  
						fprintf(stderr,"*** Error: the pipe #%d->#%d already exists. ***\n",number+1,shift_fifo_to); 
						fflush(stderr); 
						return false;
					}  
					fifo_out = open(fifo_name, O_WRONLY | O_CREAT, 0666);
					if (fifo_out == -1)  
					{  
						fprintf(stderr, "fifo open failure!\n");  
						return false;
					}  
					
					/***********************************************/
				}
			}
			
			//deal with pipe input
			ptr = strchr(temp,'<');
			if(ptr)
			{
				key_input=true;
				sscanf(ptr+1,"%d",&shift_fifo_in);
				/////////check for pipe different for two server

				sprintf(fifo_name , fifo_root , shift_fifo_in,number+1);
				fflush(stdout);
				fifo_in = open(fifo_name,  O_RDONLY |O_NONBLOCK);
				if (fifo_in < 0)  
				{  
					printf("*** Error: the pipe #%d->#%d does not exist yet. ***\n",shift_fifo_in,number+1);  
					fflush(stdout);
					return false;
				}

				
				/***********************************/

				//receive fifo	
				sprintf(message_send.arr_type,"%d",7);
				sprintf(message_send.arr_message,"%s",comm);
				sprintf(message_send.arr_who,"%d",getpid());
				sprintf(message_send.arr_to,"%d",shift_fifo_in);
				kill(getppid(),SIGUSR1);
				while(message_send.checker);
				message_send.checker=true;
				/*********************************************/
			}
			
			i=1;
			ptr=strtok(temp," ");
			do
			{
				if(*ptr == '>')
				{
					if(key_output)
						ptr=strtok(NULL," ");
					continue;
				}
				else if(*ptr == '<')
					continue;

				app[i]=ptr;
				i++;
			}while((ptr=strtok(NULL," ")));
		}
		app[i]=NULL;
		/*************************************************************/
		
		//deal with input
		int infd[2],outfd[2];			//get the pipe send to it	
		if(data.size())
		{
			infd[0]=data[0].fd[0];
			infd[1]=data[0].fd[1];
		}
		else
		{
			infd[0]=-1;
			infd[1]=-1;
		}
		/*************************************************************/

		//deal with output
		outfd[0]=-1;
		outfd[1]=-1;
		if(message.shift>0)
		{
			if((message.shift)>=data.size())	//ouput to a pipe
			{
				command temp;
				temp.fd[0]=-1;
				temp.fd[1]=-1;
				while((message.shift)>=data.size())
					data.push_back(temp);
				
			}
	
			outfd[0]=data[message.shift].fd[0];
			outfd[1]=data[message.shift].fd[1];

			if(outfd[0]==-1 && outfd[1]==-1)
			{
				pipe(outfd);
				data[message.shift].fd[0]=outfd[0];
				data[message.shift].fd[1]=outfd[1];
			}
		}
		else
		{
			outfd[0]=-1;
			outfd[1]=-1;
		}
		/*************************************************/

		int status;
		pid_t childpid;
		switch ((childpid = fork()))
		{
			case -1:
				strcpy(err,"fork error");
				fprintf(stderr,"%s\n",err);
				fflush(stderr);
				break;
			case 0:		//child

				//file or pipe(output)
				if(key_fifo==true)
				{
					//printf("able \n");
					//fflush(stdout);

					dup2(fifo_out,STDOUT_FILENO);
					close(fifo_out);
					if(outfd[1]!=-1)
					{
						close(outfd[1]);
					}
					if(outfd[0]!=-1)
					{
						close(outfd[0]);
					}
				}
				else if(key_output==true)
				{
					dup2(file,STDOUT_FILENO);
					close(file);
					if(outfd[1]!=-1)
					{
						close(outfd[1]);
					}
					if(outfd[0]!=-1)
					{
						close(outfd[0]);
					}
				}
				else if(outfd[1]!=-1)
				{
					dup2(outfd[1],STDOUT_FILENO);
					close(outfd[0]);
					close(outfd[1]);
				}
				/***********************************************/
				
				//file or pipe(input)
				if(key_input)
				{
					dup2(fifo_in,STDIN_FILENO);
					close(fifo_in);
					if(infd[0]!=-1)
						close(infd[0]);
					
					if(infd[1]!=-1)
						close(infd[1]);
				}
				else if(infd[0]!=-1)
				{
					dup2(infd[0],STDIN_FILENO);
					close(infd[0]);
					close(infd[1]);
				}
				/************************************************/

				perform(com,app);

				exit(-1);
				break;
			default:	//remember this time we need to wait
				if(infd[1]!=-1)
				{
					close(infd[1]);
					data[0].fd[1]=-1;
				}
				wait(&status);
				if(status!=0)
					return false;

				if(key_fifo==true)		//output pipe
				{
					sprintf(message_send.arr_type,"%d",8);
					sprintf(message_send.arr_message,"%s",comm);
					sprintf(message_send.arr_who,"%d",getpid());
					sprintf(message_send.arr_to,"%d",shift_fifo_to);
					kill(getppid(),SIGUSR1);
					close(fifo_out);

					while(message_send.checker);
					message_send.checker=true;
				}
				if(key_input)
				{
					close(fifo_in);
    				unlink(fifo_name);  
				}

				if(infd[0]!=-1)
				{
					close(infd[0]);
					data[0].fd[0]=-1;
				}
				if(key_output==true)
					close(file);
		}
		return true;
	}
}


bool parsing(vector<command>&data,char *arr,int number)
{
	char temp[MAXLINE];
	strcpy(temp,arr);
	char *ptr,*ptr1,*num;
	unsigned int shift=0;
	temp_command tcommand;

	
	vector<temp_command> buffer;
	
	//fist deal with the string
	ptr1=strtok(temp,"|");
	while(1)
	{
		shift=0;
		ptr=ptr1;
		ptr1=strtok(NULL,"|");
		
		if(ptr1==NULL)
		{
			break;
		}
		else if('0'<=*ptr1 && *ptr1<='9')//number pipe
		{
			//cut the number part
			num=ptr1;
			while(*ptr1 && '0'<=*ptr1 && *ptr1<='9')
				ptr1++;
			
			if(*ptr1=='\0')
			{
				shift=atoi(num);
				break;
			}
			else
			{
				*ptr1='\0';
				shift=atoi(num);
				ptr1++;
			}
		}
		else //origin pipe
		{
			shift=1;
		}
		strcpy(tcommand.arr,ptr);
		tcommand.shift=shift;
		buffer.push_back(tcommand);
	}
	strcpy(tcommand.arr,ptr);
	tcommand.shift=shift;
	buffer.push_back(tcommand);
	

	bool eexit=false;
	if(buffer.size()==1)
	{
		push(data,buffer[0],eexit,number,arr);
		
		if(data.size())
		{
			if(data[0].fd[0]!=-1)
				close(data[0].fd[0]);
			if(data[0].fd[1]!=-1)
				close(data[0].fd[1]);

			data.erase(data.begin());
		}

		buffer.erase(buffer.begin());
		if(eexit==true)
			return false;
	}
	else
	{
		unsigned int n_size=buffer.size();
		for(unsigned int i=0;i<n_size;i++)	
		{
			if(push(data,buffer[0],eexit,number,arr))
			{
				if(data.size())
				{
					if(data[0].fd[0]!=-1)
						close(data[0].fd[0]);
					if(data[0].fd[1]!=-1)
						close(data[0].fd[1]);

					data.erase(data.begin());
				}
				buffer.erase(buffer.begin());
			}
			else
			{
				break;
			}
			if(eexit==true)
				return false;
		}
	}
	return true;
}

int perform(char *text,char **argu)
{
	int rt;
	rt=execvp(text,argu);

	fprintf(stderr,"Unknown command: [%s].\n",text);	
	fflush(stderr);
	exit(rt);
}
