#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <fcntl.h>  
#include <limits.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <unistd.h>
#include <ctype.h>
#define SERVER_FIFO_NAME "/tmp/server_fifo"  
#define CLIENT_FIFO_NAME "/tmp/client_fifo"  
  
#define BUFFER_SIZE PIPE_BUF  
#define MESSAGE_SIZE 20  
#define NAME_SIZE 256  
  
typedef struct message  
{  
    pid_t client_pid;  
    char data[MESSAGE_SIZE + 1];  
}message;  
 
int main()  
{  
    int server_fifo_fd;  
    int client_fifo_fd;  
  
    int res;  
    char client_fifo_name[NAME_SIZE];  
  
    char arr[256];
  
    char *p;  
  
    
    server_fifo_fd = open(SERVER_FIFO_NAME, O_RDONLY | O_NONBLOCK);  
    if (server_fifo_fd < 0)  
    {  
        fprintf(stderr, "Sorry, server fifo open failure!\n");  
        exit(EXIT_FAILURE);  
    }  

    while((res=read(server_fifo_fd,arr,256)))
    {
        printf("%d %s\n",res,arr);
    }


    close(server_fifo_fd);  
    printf("%d",unlink(SERVER_FIFO_NAME));  
    exit(EXIT_SUCCESS);  
}  