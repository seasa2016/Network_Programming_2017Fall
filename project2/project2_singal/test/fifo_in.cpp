
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
   
  
    message msg;  
  
    if (mkfifo(SERVER_FIFO_NAME, 0666) == -1)  
    {  
        fprintf(stderr, "Sorry, create server fifo failure!\n");  
        exit(EXIT_FAILURE);  
    }  
    
    server_fifo_fd = open(SERVER_FIFO_NAME, O_RDWR | O_NONBLOCK);  
    if (server_fifo_fd == -1)  
    {  
        fprintf(stderr, "Sorry, open server fifo failure!\n");  
        exit(EXIT_FAILURE);  
    }  
    char arr[1024];

    sprintf(arr, "asdasfqwfq\nsadasvcegqew\nasasveqfqwcaxvas\n");  
    printf("%d %s\n",strlen(arr), arr);  
    write(server_fifo_fd, arr, strlen(arr));  
    
    scanf("%d",&res);
    close(server_fifo_fd);  
  
    exit(EXIT_SUCCESS);  
}  