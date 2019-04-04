#include <stdio.h>    
#include <stdlib.h>    
#include <unistd.h>    
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()    
{    
	//int pfds[2];    
   
	int ftp;

	ftp=open("abc.txt",O_WRONLY);


	dup2(ftp,1);
	close(ftp);
	printf("%d\n",ftp);
	printf("test");
}
