
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<iostream>
#include <sys/wait.h> 
using namespace std;

#define BUFSIZE 8096

struct {
char ext[16];
char filetype[16];
} extensions [] = {
{"gif", "image/gif" },
{"jpg", "image/jpeg"},
{"jpeg","image/jpeg"},
{"png", "image/png" },
{"zip", "image/zip" },
{"gz",  "image/gz"  },
{"tar", "image/tar" },
{"htm", "text/html" },
{"html","text/html" },
{"exe","text/plain" },
{"cgi","cgi=="},
{0,0} };

const string ENV[ 24 ] = {
   "COMSPEC", 
   "DOCUMENT_ROOT", 
   "GATEWAY_INTERFACE",   
   "HTTP_ACCEPT", 
   "HTTP_ACCEPT_ENCODING",             
   "HTTP_ACCEPT_LANGUAGE", 
   "HTTP_CONNECTION",         
   "HTTP_HOST", 
   "HTTP_USER_AGENT", 
   "PATH",            
   "QUERY_STRING", 
   "REMOTE_ADDR", 
   "REMOTE_PORT",      
   "REQUEST_METHOD", 
   "REQUEST_URI", 
   "SCRIPT_FILENAME",
   "SCRIPT_NAME", 
   "SERVER_ADDR", 
   "SERVER_ADMIN",      
   "SERVER_NAME",
   "SERVER_PORT",
   "SERVER_PROTOCOL",     
   "SERVER_SIGNATURE",
   "SERVER_SOFTWARE"};   


void handle_socket(int fd,struct sockaddr_in &serv_addr,struct sockaddr_in &cli_addr)
{
    int j, file_fd, buflen, len;
    long i, ret;
    char *fstr,*ptr,*ptr1;
    char buffer[BUFSIZE+1];
    char arr[BUFSIZE+1];
    char temp[64];
    char file_name[64];
    bool key=false;

    //read the browser message
    ret = read(fd,buffer,BUFSIZE);
    
    //read error
    if (ret==0||ret==-1) 
    {
        exit(3);
    }
    buffer[ret] = 0;
    //printf("new open\n");

    //block return to upper
    for ( j=0 ; buffer[j+1] ; j++)
        if (buffer[j]=='.'&&buffer[j+1]=='.')
        {
            printf("can't get back to upper\n");
            exit(3);
        }    
    /*
    for(i=0;i<24;i++)
        printf("%s : %s\n",ENV[i].c_str(),getenv(ENV[i].c_str()));
    return ;
    */
    strcpy(arr,buffer);

    ptr=strtok(arr," \n\r");
    setenv("REQUEST_METHOD",ptr,1);
    if( (strcmp(ptr,"GET") && strcmp(ptr,"get")))
    {
        printf("only deal with get mothod\n");
        exit(1);
    }
    ptr=strtok(NULL," \n\r");
    setenv("REQUEST_URI",ptr,1);
    ptr1=strchr(ptr,'?');
    if(ptr1!=NULL)
    {
        *ptr1='\0';
        ptr1++;
        //printf("query%s\n",ptr1);
        setenv("SCRIPT_NAME",ptr,1);
        setenv("QUERY_STRING",ptr1,1);
    }
    else
    {
        setenv("SCRIPT_NAME",ptr,1);
        setenv("QUERY_STRING","",1);
    }
    strcpy(file_name,ptr);

    ptr=strtok(NULL," \n\r");
    setenv("SERVER_PROTOCOL",ptr,1);
    
    setenv("REMOTE_ADDR",inet_ntoa(cli_addr.sin_addr),1);
    sprintf(temp,"%u",htons(cli_addr.sin_port));
    setenv("REMOTE_PORT",temp,1);
    setenv("SERVER_ADDR",inet_ntoa(serv_addr.sin_addr),1);
    sprintf(temp,"%u",htons(serv_addr.sin_port));
    setenv("SERVER_PORT",temp,1);
    ptr=strtok(NULL,"\n\r");
    
    ptr=strtok(NULL,":");
    ptr=strtok(NULL,"\n");
    setenv("HTTP_USER_AGENT",ptr+1,1);
    ptr=strtok(NULL,":");
    ptr=strtok(NULL,"\n");
    setenv("HTTP_ACCEPT",ptr+1,1);
    ptr=strtok(NULL,":");
    ptr=strtok(NULL,"\n");
    setenv("HTTP_ACCEPT_LANGUAGE",ptr,1);
    ptr=strtok(NULL,":");
    ptr=strtok(NULL,"\n");
    setenv("HTTP_ACCEPT_ENCODING",ptr,1);
    ptr=strtok(NULL,":");
    ptr=strtok(NULL,"\n");
    setenv("HTTP_CONNECTION",ptr,1);
    ptr=strtok(NULL,":");
    setenv("HTTP_UPGRADE_INSECURE_REQUESTS",ptr,1);
    setenv("REDIRECT_STATUS","200",1);
    setenv("GATEWAY_INTERFACE","CGI/1.1",1);
    setenv("CONTENT_LENGTH","0",1);
    setenv("PATH",".",1);
    
    if(strncmp(getenv("REMOTE_ADDR"),"140.113.167.",12)==0)
    {
        key=true;
    }
    if(key)
    {
        //printf("file:%s\n",file_name);
        //find the file format
        buflen = strlen(file_name);
        fstr = (char *)0;

        for(i=0;extensions[i].ext;i++) 
        {
            len = strlen(extensions[i].ext);
            if(!strncmp(&file_name[buflen-len], extensions[i].ext, len)) 
            {
                fstr = extensions[i].filetype;
                break;
            }
        }
        //printf("this file:%s %s\n",extensions[i].ext,extensions[i].filetype);

        if(extensions[i].ext == 0) 
        {
            fstr = extensions[i-1].filetype;
        }
        if( strcmp(extensions[i].ext,"cgi")==0)
        {
            dup2(fd,STDIN_FILENO);
            dup2(fd,STDOUT_FILENO);
            dup2(fd,STDERR_FILENO);
            printf("HTTP/1.0 200 OK\r\n");
            fflush(stdout);
            execlp(file_name+1,file_name+1,NULL);
        }
        else if((file_fd=open(file_name+1,O_RDONLY))==-1)
            write(fd, "Failed to open file\n", 20);


        sprintf(buffer,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", fstr);
        write(fd,buffer,strlen(buffer));


        while ((ret=read(file_fd, buffer, BUFSIZE))>0) 
        {
            write(fd,buffer,ret);
        }
    }
    else
    {
        sprintf(buffer,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", "text/html");
        write(fd,buffer,strlen(buffer));
        sprintf(buffer,"connection denied\n%s\n",getenv("REMOTE_ADDR"));
        write(fd,buffer,strlen(buffer));
    }
    exit(1);
}

int main(int argc, char **argv)
{
    int pid, listenfd, socketfd;
    size_t length;
    struct sockaddr_in cli_addr;
    struct sockaddr_in serv_addr;
    int status,port;

    if(argc==2)
        sscanf(argv[1],"%d",&port);
    else
        port=11400;

    if ((listenfd=socket(AF_INET, SOCK_STREAM,0))<0)
        exit(3);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    //binding
    if (bind(listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)
    {
        printf("bind error\n");
        exit(3);
    }
    //start listening
    if (listen(listenfd,0)<0)
    {
        printf("listen error\n");
        exit(3);
    }

    while(1) 
    {
        length = sizeof(cli_addr);
        
        if ((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, (socklen_t *)&length))<0)
        {
            printf("accept error\n");
            exit(3);
        }

    
        if ((pid = fork()) < 0) {
            printf("can't fork the process\n");
            exit(3);
        } 
        else 
        {
            if (pid == 0) //child
            {  
                close(listenfd);
                handle_socket(socketfd,serv_addr,cli_addr);
            } 
            else        //parent
            { 
                close(socketfd);
		        waitpid(pid, &status, WNOHANG);
            }
        }
    }
}