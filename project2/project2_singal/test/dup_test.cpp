#define TESTSTR "Hello dup2\n"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include <fcntl.h>

int main(void) {
	int fd3;
	fd3 = open("testdup2.dat", 0666);
	dup2(fd3,STDOUT_FILENO);
	//test if we can still use stdout as output when we first change it
	close(fd3);

	int fd[2];
	pid_t child;

	pipe(fd);
	switch((child=fork()))
	{
		case 0:
			dup2(fd[1],STDOUT_FILENO);
			close(fd[1]);
			close(fd[0]);
			execlp("ls","ls","-al",NULL);
			exit(1);
			break;
		default:
			dup2(fd[0],STDIN_FILENO);
			close(fd[1]);
			close(fd[0]);
			execlp("cat","cat",NULL);
			exit(1);
			break;
	}
	
	printf("pid:%d\n",child);
	printf(TESTSTR);
	return 0;
	
}
