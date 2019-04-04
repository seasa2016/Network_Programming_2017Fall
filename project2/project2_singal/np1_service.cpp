#include"np1_service.h"
#define MAXLINE 11000
#define fifo_root "./../tmp/fifo_%d_%d"

using std::vector;
extern int errno;

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

struct user_data{
	bool use;
	int fd;
	int count;
	char name[32];
	char addr[32];
	char env[64];
	vector<command> data;
	int fifo[40];
};
user_data user[40];


void singal_server(int sockfd)
{
	int newsockfd,clilen;
	struct sockaddr_in cli_addr;

	for(int i=0;i<40;i++)
	{
		user[i].count=0;
		user[i].use=false;
		user[i].fd=-1;
		user[i].addr[0]='\0';
		user[i].data.clear();
		for(int j=0;j<40;j++)
			user[i].fifo[j]=-1;
	}

    int i,j;
    int fd,nfds;
    fd_set rfds;
    fd_set afds;

    nfds=getdtablesize();
    FD_ZERO(&afds);
    FD_SET(sockfd,&afds);

    while(1)
    {
        memcpy(&rfds,&afds,sizeof(rfds));

        if(select(nfds,&rfds,(fd_set*)0,(fd_set*)0,(struct timeval*) 0)<0)
        {
            exit(1);
            fprintf(stderr,"select error:%s\n",strerror(errno));
        }
        if(FD_ISSET(sockfd,&rfds))
        {
			clilen	=sizeof(cli_addr);
			newsockfd	=accept(sockfd,(struct sockaddr*)&cli_addr,(socklen_t*)&clilen);

            if(newsockfd<0)
            {
                exit(1);
                fprintf(stderr,"accept error:%s\n",strerror(errno));
            }
            FD_SET(newsockfd,&afds);
			//set the information in to array

			for(i=0;user[i].use;i++);

			//fprintf(stderr,"new fd%d\n",newsockfd);
			//fflush(stderr);
			user[i].count=0;
			user[i].use=true;
			user[i].fd=newsockfd;
			sprintf(user[i].addr,"CGILAB/511");
			//sprintf(user[i].addr,"%s/%d",inet_ntoa(cli_addr.sin_addr),htons(cli_addr.sin_port));
			sprintf(user[i].env,"bin:.");
			sprintf(user[i].name,"(no name)");
			user[i].data.clear();

			dup2(newsockfd,STDIN_FILENO);
			dup2(newsockfd,STDOUT_FILENO);
			dup2(newsockfd,STDERR_FILENO);

			welcome(i);
			/********************************************************/
        }
		else
		{
			for(fd=0;fd<nfds;fd++){
				if(fd!=sockfd && FD_ISSET(fd,&rfds))
				{
					//some one input
					//replace stdin stdout stderr with 
					dup2(fd,STDIN_FILENO);
					dup2(fd,STDOUT_FILENO);
					dup2(fd,STDERR_FILENO);
					
					for(i=0;;i++)
						if(user[i].use)
							if(fd==user[i].fd)
								break;
					
					setenv("PATH",user[i].env,1);

					if(str_echo(i)==false)
					{
						char arr[64];
						sprintf(arr,"*** User '%s' left. ***\n",user[i].name);
						broadcast(arr);

						close(fd);
						user[i].use=false;
						
						dup2(sockfd,STDIN_FILENO);
						dup2(sockfd,STDOUT_FILENO);
						dup2(sockfd,STDERR_FILENO);
					
							
						FD_CLR(fd,&afds);
						
						
						int n=user[i].data.size();
						for(j=0;j<n;j++)
						{
							if(user[i].data[j].fd[0]!=-1)
								close(user[i].data[j].fd[0]);
							if(user[i].data[j].fd[1]!=-1)
								close(user[i].data[j].fd[1]);
						} 
						user[i].data.clear();
						
						char fifo_name[128];
						for(j=0;j<40;j++)
						{
							if(user[j].fifo[i]!=-1)
							{
								sprintf(fifo_name,fifo_root,j+1,i+1);
								close(user[j].fifo[i]);
								user[j].fifo[i]=-1;
								unlink(fifo_name);
							}
							if(user[i].fifo[j]!=-1)
							{
								sprintf(fifo_name,fifo_root,i+1,j+1);
								close(user[i].fifo[j]);
								user[i].fifo[j]=-1;
								unlink(fifo_name);
							}
						} 

					}
					/*****************************/
				}
				fflush(stderr);
			}
		}
    }
}
void broadcast(char *arr)
{
	for(int i=0;i<40;i++)
		if(user[i].use)
		{
			write(user[i].fd,arr,strlen(arr));
		}
}

void welcome(int num)
{
	char arr[128];
	printf("****************************************\n");
	printf("** Welcome to the information server. **\n");
	printf("****************************************\n");
	//sprintf(arr,"*** User \'%s\' entered from %s. ***\n",user[num].name,"CGILAB/511");
	sprintf(arr,"*** User \'%s\' entered from %s. ***\n",user[num].name,user[num].addr);
	fflush(stdout);

	broadcast(arr);
	
	printf("%c ",'%');
	fflush(stdout);
}

bool str_echo(int num)
{
	int n;
	char line[MAXLINE],buffer[MAXLINE];
	

	char *lline;
	
	//fprintf(stderr,"why\n");
	//fflush(stderr);

	line[0]='\0';
	while((n=read(0,buffer,MAXLINE)))
	{
		//fprintf(stderr,"%d %d--\n",buffer[n-1],buffer[n-2]);
		//fflush(stderr);


		buffer[n]='\0';
		strcat(line,buffer);
		
		if(buffer[n-1]==10)
			break;
	}
	lline=strtok(line,"\r\n");

	//fprintf(stderr,"%d %s\n",strlen(lline),lline);
	//fflush(stderr);

	if(parsing(user[num].data,lline,num)==false)	
	{
		fflush(stdout);
		return false;
	}
	printf("%c ",'%');
	fflush(stdout);
	return true;
}

bool push(vector<command> &data,temp_command &message,bool &eexit,int number,char* comm)
{
	char *ptr,*ptr1,*ptr2;
	bool key_output=false;
	bool key_input=false;
	bool key_fifo=false;
	char temp[MAXLINE];
	char err[64];
	int file,shift;
	char arr[256];
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
		
		strcpy(user[number].env,ptr2); 
		
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
		printf("<ID>\t<nickname>\t<IP/port>\t<indicate me>\n");
		for(int i=0;i<40;i++)
			if(user[i].use)
			{
				//printf("%d\t%s\t%s",i+1,user[i].name,"CGILAB/511");
				printf("%d\t%s\t%s",i+1,user[i].name,user[i].addr);
				if(number==i)
					printf("\t<-me\n");
				else
					printf("\n");
			}
		return true;
	}
	else if(strcmp("tell",ptr)==0)
	{
		//*** (sender's name) told you ***: (message)
		//% tell 3 Hello World.
		ptr1=strtok(NULL," ");
		shift=atoi(ptr1);

		if(user[shift-1].use)
		{			
			ptr1=strtok(NULL,"");
			sprintf(arr,"*** %s told you ***: %s\n",user[number].name,ptr1);
			fflush(stdout);
			write(user[shift-1].fd,arr,strlen(arr));
		}
		else
		{
			printf("*** Error: user #%d does not exist yet. ***\n",shift);
			fflush(stdout);
		}
		return true;
	}
	else if(strcmp("yell",ptr)==0)
	{
		ptr1=strtok(NULL,"");
		sprintf(arr,"*** %s yelled ***: %s\n",user[number].name,ptr1);
		fflush(stdout);
		broadcast(arr);
		return true;
	}
	else if(strcmp("name",ptr)==0)
	{
		ptr1=strtok(NULL," ");
		for(int i=0;i<40;i++)
			if(user[i].use)
				if(strcmp(ptr1,user[i].name)==0)
				{
					printf("*** User '%s' already exists. ***\n",user[i].name);
					return true;
				}
		if(user[number].count==3)
		{
			printf("error over 3\n");
			return true;
		}
		user[number].count++;
		strcpy(user[number].name,ptr1);
		//sprintf(arr,"*** User from %s is named '%s'. ***\n","CGILAB/511",user[number].name);
		sprintf(arr,"*** User from %s is named '%s'. ***\n",user[number].addr,user[number].name);
		fflush(stdout);
		broadcast(arr);

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

					if(user[shift_fifo_to-1].use==false)
					{
						fprintf(stderr,"*** Error: user #%d does not exist yet. ***\n",shift_fifo_to);
						fflush(stderr);
						return false;
					}
					sprintf(fifo_name , fifo_root , number+1,shift_fifo_to);
					fflush(stdout);
					if (mkfifo(fifo_name, 0666) == -1)  
					{  
						fprintf(stderr,"*** Error: the pipe #%d->#%d already exists. ***\n",number+1,shift_fifo_to); 
						fflush(stderr); 
						return false;
					}  
					fifo_out = open(fifo_name, O_RDWR | O_NONBLOCK);

					if (fifo_out == -1)  
					{  
						fprintf(stderr, "fifo open failure!/n");  
						exit(-1);  
					}  
					user[number].fifo[shift_fifo_to-1]=fifo_out;
				}
			}

			//deal with pipe input
			ptr = strchr(temp,'<');
			if(ptr)
			{
				key_input=true;
				sscanf(ptr+1," %d",&shift_fifo_in);
				/////////check for pipe different for two server

				sprintf(fifo_name , fifo_root , shift_fifo_in,number+1);
				fflush(stdout);
				
				fifo_in = open(fifo_name,  O_RDONLY |O_NONBLOCK);
				
				if (fifo_in == -1)  
				{  
					printf("*** Error: the pipe #%d->#%d does not exist yet. ***\n",shift_fifo_in,number+1);  
					fflush(stdout);
					return false;
				}
				close(user[shift_fifo_in-1].fifo[number]);
				user[shift_fifo_in-1].fifo[number]=-1;
				
				sprintf(arr,"*** %s (#%d) just received from %s (#%d) by '%s' ***\n",user[number].name,number+1,user[shift_fifo_in-1].name,shift_fifo_in,comm);
				fflush(stdout);
				broadcast(arr);
				//////////////////////////////
			}
			
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

				if(key_fifo==true)
				{
					sprintf(arr,"*** %s (#%d) just piped '%s' to %s (#%d) ***\n",user[number].name,number+1,comm,user[shift_fifo_to-1].name,shift_fifo_to);
					fflush(stdout);
					broadcast(arr);
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
		if(eexit==true)
			return false;
		
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
