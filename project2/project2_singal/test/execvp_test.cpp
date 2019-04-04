#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<string.h>
int main(void)
{
	printf("entering main process---\n");
	int ret;
	
	char *arr[5];
	arr[0]=new char[20];
	arr[1]=new char[20];
	arr[2]=new char[20];
	arr[3]=new char[20];

	strcpy(arr[0],"cat");
	strcpy(arr[1],"exe_test.cpp");
	arr[2]=NULL;

	ret = execv("/bin/cat",arr);
	if(ret == -1)
		perror("execl error");
	printf("exiting main process ----\n");
	return 0;
}
