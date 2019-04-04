#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/uio.h>
#include<fcntl.h>

#define user_num 5

#define UNLINK 0
#define CONNECTING 1
#define READING 2
#define WRITING 3

struct user_type{
	int type;
	char host[256];
	int port;
	char file[256];
	int fd;
	FILE * file_fd;
	int needwrite;
	int rev_size;
    char rev[32384];
	char sent_buf[32384];
	char output_buf[32384]="\0";
}user[user_num];

struct client_type{	
	int count;
    int nfds=getdtablesize();
    fd_set rfds; /* readable file descriptoclient.rs*/
    fd_set wfds; /* writable file descriptoclient.rs*/
    fd_set rs; /* active file descriptoclient.rs*/
    fd_set ws; /* active file descriptoclient.rs*/

    struct timeval tt;
}client;


int readline(int fd,char *ptr,int maxlen);

void start_html()
{
	printf("Content-type:text/html\r\n\r\n");
	printf("<html>\n");
	printf("<head>\n");
	printf("<title>Network Programming Homework 3</title>\n");
	printf("</head>\n");
	printf("<body bgcolor=#336699>\n");
	printf("<font face=\"Courier New\" size=2 color=#FFFF99>\n");
	printf("<table width=\"800\" border=\"1\">\n");
	printf("<tr>\n");
	printf("<tr>\n");
	//deal with the link
	for(int i=0;i<user_num;i++)
	    printf("<td>%s</td>",user[i].host);
	printf("\n");

	printf("<tr>\n");
	for(int i=0;i<user_num;i++)
	    printf("<td valign=\"top\" id=\"%c%d\"></td>",'m',i);
	printf("\n");

	printf("</table>\n"); 
	printf("</font>\n");
	printf("</body>\n");
	printf("</html>\n");
	fflush(stdout);
}
void connect_user(int i)
{
	struct sockaddr_in client_sin;

	int SERVER_PORT;

	struct hostent *he; 

	user[i].file_fd = fopen (user[i].file,"r");
	if ( user[i].file_fd == NULL)
	{
		printf("<script>document.all['m%d'].innerHTML += \"Error : '%s' doesn't exist<br>\"</script>\n",i,user[i].file);
		fclose (user[i].file_fd);
		return ;
	}
	if((he=gethostbyname(user[i].host) ) == NULL)
	{
		printf("<script>document.all['m%d'].innerHTML += \"Error : client doesn't exist<br>\"</script>\n",i);
		return ;
	}	
	SERVER_PORT = (u_short)user[i].port;

	bzero(&client_sin,sizeof(client_sin));
	client_sin.sin_family = AF_INET;
	client_sin.sin_addr = *((struct in_addr *)he->h_addr); 
	client_sin.sin_port = htons(SERVER_PORT);


	user[i].fd = socket(AF_INET,SOCK_STREAM,0);

	int flag = fcntl(user[i].fd, F_GETFL, 0);
	fcntl(user[i].fd, F_SETFL, flag | O_NONBLOCK);

	if(connect(user[i].fd,(struct sockaddr *)&client_sin,sizeof(client_sin)) < 0)
	{
		if (errno != EINPROGRESS) 
		{
			printf("<script>document.all['m%d'].innerHTML += \"error at connect<br>\"</script>\n",i);
			return ;
		}       
	} 

    //sleep(1);
	client.nfds = client.nfds > user[i].fd ? client.nfds : user[i].fd ;
	// fd_set
	FD_SET(user[i].fd, &client.rs);
	FD_SET(user[i].fd, &client.ws);  
	
	client.count++;
	user[i].type =  CONNECTING ;
}

int main ()
{
	char arr[256],*ptr;
    int error,n;
	char temp[16192];

	strcpy(arr , getenv( "QUERY_STRING" ) );

    close(STDIN_FILENO);
    ptr=strtok(arr,"1");
	for(int i=0;i<user_num;i++)
	{
		user[i].type = UNLINK;
		ptr=strtok(NULL,"&");
		ptr=strchr(ptr,'=');
		strcpy(user[i].host,ptr+1);

		ptr=strtok(NULL,"&");
		ptr=strchr(ptr,'=');
		user[i].port=atoi(ptr+1);

		ptr=strtok(NULL,"&");
		ptr=strchr(ptr,'=');
		strcpy(user[i].file,ptr+1);
        user[i].needwrite=0;
		strcpy(user[i].rev,"");
		strcpy(user[i].sent_buf,"");
		strcpy(user[i].output_buf,"");
	}
	start_html();
	//deal with link
	
	FD_ZERO(&client.rfds); 
	FD_ZERO(&client.wfds); 
	FD_ZERO(&client.rs); 
	FD_ZERO(&client.ws);
    client.count=0;
	client.tt.tv_sec=1;
    client.nfds=0;
    
	for(int i=0;i<user_num;i++)
		if(user[i].host[0] != '\0')
			connect_user(i);
   
	
    client.nfds=client.nfds+1;
    while(client.count)
	{ 
		
        memcpy(&client.rfds, &client.rs, sizeof(client.rfds)); 
        memcpy(&client.wfds, &client.ws, sizeof(client.wfds));
        //printf("before n%d r%d w%d\n",client.nfds,client.rfds,client.wfds);
        //printf("ori n%d r%d w%d\n",client.nfds,client.rs,client.ws);
        
        if ( select(client.nfds, &client.rfds, &client.wfds, (fd_set*)NULL, &client.tt) < 0 ) 
        {
			//printf("%d %s\n",errno,strerror(errno));
			
			//exit(1);
		}

        //printf("n%d r%d w%d\n",client.nfds,FD_ISSET( user[0].fd , & client.rfds ),FD_ISSET( user[0].fd , & client.wfds ));
		for(int i=0 ; i<user_num ; i++)
		{
			if(user[i].type == UNLINK)
				continue;

            
			if ( user[i].type == CONNECTING && ( FD_ISSET( user[i].fd , & client.rfds) || FD_ISSET( user[i].fd , & client.wfds )))
			{
                n=sizeof(error);
				if (getsockopt( user[i].fd , SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&n) < 0 ||error != 0) 
				{
					printf("<script>document.all['m%d'].innerHTML += \"error at connect2<br>\"</script>\n",i);
					//strcat(user[i].output_buf,temp);
					//return (-1);
				}
				user[i].type = READING;
				FD_CLR(user[i].fd, &client.ws);
				strcpy(temp,"");
                //printf("user %d\n",i);
            }
			else if( user[i].type == WRITING && FD_ISSET(user[i].fd, &client.wfds) ) 
			{
				if( user[i].needwrite==0)
				{
					if ( fgets ( user[i].sent_buf , sizeof(user[i].sent_buf) , user[i].file_fd ) == NULL )
					{

					}
					/////////////////////
					
					ptr = strchr( user[i].sent_buf , '\r');
					if( ptr )
					{
						*(ptr+1) = '\n';
						*(ptr+2) = '\0';
					}
					else
					{
						ptr = strchr( user[i].sent_buf , '\n');
						if( ptr )
						{
							*(ptr) = '\r';
							*(ptr+1) = '\n';
							*(ptr+2) = '\0';
						}
						else
						{
							int size = strlen(user[i].sent_buf);
							user[i].sent_buf[size] = '\r';
							user[i].sent_buf[size+1] = '\n';
							user[i].sent_buf[size+2] = '\0';
						}
					}
				
					user[i].rev_size = user[i].needwrite = strlen(user[i].sent_buf);
					
					
					sprintf(temp,"<script>document.all['m%d'].innerHTML += \"<b>",i);
					strcat(user[i].output_buf,temp);
					for( int ini=0; user[i].sent_buf[ini] ; ini++)
					{
						switch( user[i].sent_buf[ini] )
						{
							case '\r':
							break;
							case '\n':
								strcat(user[i].output_buf,"<br>");
							break;
							case '\"':
								strcat(user[i].output_buf,"&quot;");
							break;
							case '\'':
								strcat(user[i].output_buf,"&apos;");
							break;
							case '<':
								strcat(user[i].output_buf,"&lt;");
							break;
							case '>':
								strcat(user[i].output_buf,"&gt;");
							break;
							case '&':
								strcat(user[i].output_buf,"&amp;");
							break;
							case ' ':
								strcat(user[i].output_buf,"&nbsp;");
							break;
							default:
								sprintf(temp,"%c",user[i].sent_buf[ini]);
								strcat(user[i].output_buf,temp);
						}
					}	
					sprintf(temp,"</b>\"</script>\n");
					strcat(user[i].output_buf,temp);
				}

				n = write( user[i].fd , user[i].sent_buf + user[i].rev_size - user[i].needwrite , user[i].needwrite); 
				user[i].needwrite -= n;

				if (n <= 0 || user[i].needwrite <= 0) 
				{
					
					fflush(stdout);

					// write finished
					//FD_CLR(csockA, &client.ws);
					user[i].type = READING;
					FD_SET( user[i].fd, &client.rs);
				}
			}
			else if( user[i].type == READING && FD_ISSET(user[i].fd, &client.rfds) )
			{
                //printf("--%d %d ok here\n",client.count,user[i].type);
				bool key=false;

				fflush(stdout);
				n = read( user[i].fd , user[i].rev , 4096);
				user[i].rev[n]='\0';
				
				sprintf(temp,"<script>document.all['m%d'].innerHTML += \"",i);
				strcat(user[i].output_buf,temp);
				
				for( int ini=0; user[i].rev[ini] ; ini++)
				{
					switch( user[i].rev[ini] )
					{
						case '\r':
						break;
						case '\n':
							strcat(user[i].output_buf,"<br>");
						break;
						case '\"':
							strcat(user[i].output_buf,"&quot;");
						break;
						case '\'':
							strcat(user[i].output_buf,"&apos;");
						break;
						case '<':
							strcat(user[i].output_buf,"&lt;");
						break;
						case '>':
							strcat(user[i].output_buf,"&gt;");
						break;
						case '&':
							strcat(user[i].output_buf,"&amp;");
						break;
						case ' ':
							strcat(user[i].output_buf,"&nbsp;");
						break;
						default:
							sprintf(temp,"%c",user[i].rev[ini]);
							strcat(user[i].output_buf,temp);
					}
				}	
				
				strcat(user[i].output_buf,"\"</script>\n");
				
				/*********************************************************/
				for(int ss=0;user[i].rev[ss+1];ss++)
					if(user[i].rev[ss]=='%' && user[i].rev[ss+1]==' ')
						key=true;

				if(n == 0)    //the oppsite close the socket
				{
					FD_CLR(user[i].fd, &client.rs);
					FD_CLR(user[i].fd, &client.ws);
					
					user[i].type = UNLINK ;
					printf("%s",user[i].output_buf);
					fflush(stdout);
					close(user[i].fd);

					client.count--;
                    //printf("exit test %d\n",client.count);
				}
				else
				{
					n=strlen(user[i].rev);
					if( key )
					{
						// read finished

						FD_CLR( user[i].fd, &client.rs);
						user[i].type = WRITING ;
						FD_SET( user[i].fd, &client.ws);

						
						printf("%s",user[i].output_buf);
						fflush(stdout);
						strcpy(user[i].output_buf,"");
					}
					else if(user[i].rev[n]=='\n' || user[i].rev[n-1]=='\n')
					{
						printf("%s",user[i].output_buf);
						fflush(stdout);
						strcpy(user[i].output_buf,"");
					}
					user[i].rev[0] = '\0';
				}
			}
		}
	}
	
	return 0;
}

int readline(int fd,char *ptr,int maxlen)
{
	int n,rc;
	char c;
	*ptr = 0;
	for(n=1;n<maxlen;n++)
	{
		if((rc=read(fd,&c,1)) == 1)
		{
			*ptr++ = c;	
			if(c==' '&& *(ptr-2) =='%'){break; }
			if(c=='\n')  break;
		}
		else if(rc==0)
		{
			if(n==1)     return(0);
			else         break;
		}
		else
			return(-1);
	}
	return(n);
}
