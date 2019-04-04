#include"np1_service.h"
#define MAXLINE 11000
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


void str_echo()
{
	int n;
	char line[MAXLINE],buffer[MAXLINE];
	char prompt[3]="% ";
	vector<command> data;
	strcpy(line,"****************************************\r\n");
	strcat(line,"** Welcome to the information server. **\r\n");
	strcat(line,"****************************************\r\n");

	n=strlen(line);
	if(write(1,line,n)!=n)
	{
		strcpy(line,"str_echo:written error\r\n");

		write(1,line,strlen(line));
		exit(-1);
	}
	fflush(stdout);
	//write(1,prompt,3);
	fprintf(stdout,"%s",prompt);
	fflush(stdout);
	char *lline;
	while(1)
	{
		line[0]='\0';
		while((n=read(0,buffer,MAXLINE)))
		{
			buffer[n]='\0';
			strcat(line,buffer);
			//fprintf(stderr,"N:%d QQ-%s-",n,line);
			fflush(stderr);
			
			if(buffer[n-1]==10 || buffer[n-2]==10)
				break;
		}
		//fprintf(stderr,"test-%s-",line);
		fflush(stdout);
		fflush(stderr);

		lline=strtok(line,"\r\n");
		
		if(parsing(data,lline)==false)	
		{
			fflush(stdout);
			fflush(stderr);
			return ;
		}
		

		printf("%s",prompt);
		fflush(stdout);
		fflush(stderr);
	}
}
bool push(vector<command> &data,temp_command &message,bool &eexit)
{
	char *ptr,*ptr1,*ptr2;
	bool key=false;
	char temp[MAXLINE];
	char err[50];
	int file;
	strcpy(temp,message.arr);
	
	//printf("%d %s\n",message.shift,temp);
	//fflush(stdout);

	ptr=strtok(temp,">");
	ptr1=strtok(NULL," ");
	
	key=false;
	if(ptr1 != NULL)							//output to file
	{
		strcpy(err,"open file error");
		
		key=true;
		file = open(ptr1, O_RDWR | O_CREAT, 0666);
		if (file == -1)
		{
			fprintf(stderr,"%s\r\n",err);
			fflush(stderr);
		}
	}
	
	ptr=strtok(ptr," ");
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
		if(key)
		{
			char dd[100];
			sprintf(dd,"%s=%s\r\n",ptr1,getenv(ptr1));
			write(file,dd,strlen(dd));
		}
		else
		{
			fprintf(stdout,"%s=%s\r\n",ptr1,getenv(ptr1));
			fflush(stdout);
		}
		return true;
	}
	else		//normal
	{
		char *app[10];
		int i=1;
		app[0]=ptr;
		ptr1=strtok(NULL," ");
		do
		{
			app[i]=ptr1;
			i++;
		}while((ptr1=strtok(NULL," ")));
		app[i]=NULL;


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
	
		int status;
		pid_t childpid;
		switch ((childpid = fork()))
		{
			case -1:
				strcpy(err,"fork error");
				fprintf(stderr,"%s\r\n",err);
				fflush(stderr);
				break;
			case 0:		//child
				if(key==true)
				{
					dup2(file,STDOUT_FILENO);
					close(file);
					if(outfd[1]!=-1 || outfd[0]!=-1)
					{
						close(outfd[1]);
						close(outfd[0]);
					}
				}
				else if(outfd[1]!=-1)
				{
					dup2(outfd[1],STDOUT_FILENO);
					close(outfd[0]);
					close(outfd[1]);
				}
				if(infd[0]!=-1)
				{
					dup2(infd[0],STDIN_FILENO);
					close(infd[0]);
					close(infd[1]);
				}
				perform(ptr,app);
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
				if(infd[0]!=-1)
				{
					close(infd[0]);
					data[0].fd[0]=-1;
				}
				if(key==true)
					close(file);
		}
		return true;
	}
}


bool parsing(vector<command>&data,char *arr)
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
	

	bool exit=false;
	if(buffer.size()==1)
	{
		push(data,buffer[0],exit);
		
		if(data.size())
		{
			if(data[0].fd[0]!=-1)
				close(data[0].fd[0]);
			if(data[0].fd[1]!=-1)
				close(data[0].fd[1]);

			data.erase(data.begin());
		}

		buffer.erase(buffer.begin());
		if(exit==true)
			return false;
	}
	else
	{
		unsigned int n_size=buffer.size();
		for(unsigned int i=0;i<n_size;i++)	
		{
			if(push(data,buffer[0],exit))
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
			if(exit==true)
				return false;
		}
	}
	return true;
}

int perform(char *text,char **argu)
{
	//printf("--%d--\n",argu);
	//fflush(stdout);
	
	int rt;
	rt=execvp(text,argu);

	fprintf(stderr,"Unknown command: [%s].\r\n",text);	
	fflush(stderr);
	exit(rt);
}
