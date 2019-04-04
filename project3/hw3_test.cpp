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
	int file_fd;
	int needwrite;
	int rev_size;
    char rev_buf[32384];
    char rev[32384];
	char sent_buf[32384];
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

	char msg_buf[30000];
	int len;
	int SERVER_PORT;

	struct hostent *he; 

	user[i].file_fd = open(user[i].file , O_RDONLY |O_NONBLOCK);
	if (user[i].file_fd == -1) 
	{
		printf("<script>document.all['m%d'].innerHTML += \"Error : '%s' doesn't exist<br>\"</script>\n",i,user[i].file);
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
	int fd;
    int error,n;

	//strcpy(arr , getenv( "QUERY_STRING" ) );

    close(STDIN_FILENO);
    strcpy(arr,"h1=nplinux3.cs.nctu.edu.tw&p1=11400&f1=test2.txt&h2=&p2=&f2=&h3=&p3=&f3=&h4=&p4=&f4=&h5=&p5=&f5=");
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
    /*
    printf("n%d r%d w%d\n",client.nfds,client.rfds,client.wfds);
	*/
	for(int i=0;i<user_num;i++)
		if(user[i].host[0] != '\0')
			connect_user(i);
    /*
    printf("n%d r%d w%d\n",client.nfds,client.rs,client.ws);
    
    printf("--");
    for(int i=0;i<user_num;i++)
        printf("- %d fd:%d -",user[i].type,user[i].fd);
    printf("\n");
    */
	int len;
	
    client.nfds=client.nfds+1;
    while(client.count)
	{ 
		
        memcpy(&client.rfds, &client.rs, sizeof(client.rfds)); 
        memcpy(&client.wfds, &client.ws, sizeof(client.wfds));
        //printf("before n%d r%d w%d\n",client.nfds,client.rfds,client.wfds);
        //printf("ori n%d r%d w%d\n",client.nfds,client.rs,client.ws);
        
        if ( select(client.nfds, &client.rfds, &client.wfds, (fd_set*)NULL, &client.tt) < 0 ) 
        {
			printf("%d %s\n",errno,strerror(errno));
			exit(1);
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
					return (-1);
				}
				user[i].type = READING;
				FD_CLR(user[i].fd, &client.ws);
                //printf("user %d\n",i);
            }
			else if( user[i].type == WRITING && FD_ISSET(user[i].fd, &client.wfds) ) 
			{
				if( user[i].needwrite==0)
				{
					len = readline( user[i].file_fd , user[i].sent_buf , sizeof(user[i].sent_buf));
					if(len < 0) exit(1);

					user[i].sent_buf[len-1] = 13;
					user[i].sent_buf[len] = 10;
					user[i].sent_buf[len+1] = 0;
				
					user[i].rev_size = user[i].needwrite=len+1;
				}

				n = write( user[i].fd , user[i].sent_buf + user[i].rev_size - user[i].needwrite , user[i].needwrite); 
				user[i].needwrite -= n;

				if (n <= 0 || user[i].needwrite <= 0) 
				{
					strtok(user[i].sent_buf,"\r\n");
					printf("<script>document.all['m%d'].innerHTML += \"%s<br>\"</script>\n",i,user[i].sent_buf);
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
				fflush(stdout);
				n = read( user[i].fd , user[i].rev_buf , 4096);
				user[i].rev_buf[n]='\0';
				strcat(user[i].rev,user[i].rev_buf);
				
				//printf("%s",user[i].rev);
				//fflush(stdout);
				
				if(n == 0)    //the oppsite close the socket
				{
					FD_CLR(user[i].fd, &client.rs);
					FD_CLR(user[i].fd, &client.ws);
					
					user[i].type = UNLINK ;
					close(user[i].fd);

					client.count--;
                    //printf("exit test %d\n",client.count);
				}
				else
				{
					n=strlen(user[i].rev);
					if( user[i].rev[0]=='%' && user[i].rev[1]==' ')
					{
						// read finished
						printf("<script>document.all['m%d'].innerHTML += \"%s\"</script>\n",i,user[i].rev);
						fflush(stdout);
						FD_CLR( user[i].fd, &client.rs);
						user[i].type = WRITING ;
						FD_SET( user[i].fd, &client.ws);
					}
					else if(user[i].rev[n]=='\n' || user[i].rev[n-1]=='\n')
					{
						printf("<script>document.all['m%d'].innerHTML += \"",i);
                        
						int ini=0;
                        char *pp;
						char arr[32384];
						strcpy(arr,user[i].rev);

						
                        pp=strtok(user[i].rev,"\r\n\"\'&<> ");
						if(pp)
							do
							{
								for( ; ini < pp-user[i].rev ; ini++)
								{
									switch( arr[ini] )
									{
										case '\r':
										break;
										case '\n':
											printf("<br>");
										break;
										case '\"':
											printf("&quot;");
										break;
										case '\'':
											printf("&apos;");
										break;
										case '<':
											printf("&lt;");
										break;
										case '>':
											printf("&gt;");
										break;
										case '&':
											printf("&amp;");
										break;
										case ' ':
											printf("&nbsp;");
										break;
									}
								}
								printf("%s",pp);
							}while(pp=strtok(NULL,"\r\n"));
						
						for( ; arr[ini] ; ini++)
						{
							switch( arr[ini] )
							{
								case '\r':
								break;
								case '\n':
									printf("<br>");
								break;
								case '\"':
									printf("&quot;");
								break;
								case '\'':
									printf("&apos;");
								break;
								case '<':
									printf("&lt;");
								break;
								case '>':
									printf("&gt;");
								break;
								case '&':
									printf("&amp;");
								break;
								case ' ':
									printf("&nbsp;");
								break;
							}
						}
                        printf("\"</script>\n");
                        
                        //user[i].rev
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
