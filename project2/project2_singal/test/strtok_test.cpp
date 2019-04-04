#include<stdio.h>
#include<string.h>

int main(void)
{
	char arr[]="1234565432123456";

	char *ptr,*ptr1;

	ptr=strtok(arr,"3");
	while(ptr)
	{


		ptr=strtok(NULL,"3");
	}

}

