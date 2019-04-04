#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<string.h>
int main(int argc ,char *argv[])
{
	printf("entering main process---\n");
	int ret;	
	
	//setenv("PATH",".",1);
		
	strcpy(argv[0],"cat");

	ret = execlp("ls","ls","",NULL);
	if(ret == -1)
		perror("execl error");
	printf("exiting main process ----\n");
	return 0;
}
