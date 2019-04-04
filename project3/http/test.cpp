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

#define user_num 5

 
struct user_type{
  char host[256];
  int port;
  char file[256];
  int fd;
}user[user_num];


fd_set readfds;
char gSc = 0;

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
int connect_user(int i)
{
	struct sockaddr_in client_sin;
	
  char msg_buf[30000];
	int len;
	int SERVER_PORT;
	FILE *fp; 
	struct hostent *he; 
	
  gSc = 0;
	
  fp = fopen(user[i].file , "r");
  if (fp == NULL) 
  {
    printf("<script>document.all['m0'].innerHTML += \"Error : '%s' doesn't exist<br>\";</script>\n",user[i].file);
    return 0;
  }
	if((he=gethostbyname(user[i].host) ) == NULL)
	{
		printf("<script>document.all['m0'].innerHTML += \"Usage : client <server ip> <port> <testfile><br>\";</script>\n");
		return 0;
	}	
	SERVER_PORT = (u_short)user[i].port;

	user[i].fd = socket(AF_INET,SOCK_STREAM,0);
	
	bzero(&client_sin,sizeof(client_sin));
	
  client_sin.sin_family = AF_INET;
	client_sin.sin_addr = *((struct in_addr *)he->h_addr); 
	client_sin.sin_port = htons(SERVER_PORT);
	
  if(connect(user[i].fd,(struct sockaddr *)&client_sin,sizeof(client_sin)) == -1)
	{
		perror("");
		exit(1);
	}
}

int main ()
{
  
  char arr[256],*ptr;
  
  strcpy(arr , getenv( "QUERY_STRING" ) );
  
  //h1=a&p1=123&f1=c&h2=&p2=&f2=&h3=&p3=&f3=&h4=&p4=&f4=&h5=&p5=&f5=
  ptr=strtok(arr,"1");
  for(int i=0;i<user_num;i++)
  {
    ptr=strtok(NULL,"&");
    ptr=strchr(ptr,'=');
    strcpy(user[i].host,ptr+1);
    
    ptr=strtok(NULL,"&");
    ptr=strchr(ptr,'=');
    user[i].port=atoi(ptr+1);
    
    ptr=strtok(NULL,"&");
    ptr=strchr(ptr,'=');
    strcpy(user[i].file,ptr+1);
  }
  start_html();

  //deal with link
  for(int i=0;i<user_num;i++)
    connect_user(i);




  /*
  for(int i=0;i<50;i++)
  {
    printf("<script>document.all['m0'].innerHTML += \"****************************************<br>\";</script>\n");
    sleep(1);
  }*/
  return 0;
}
