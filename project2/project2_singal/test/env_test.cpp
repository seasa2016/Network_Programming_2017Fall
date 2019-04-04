/* getenv example: getting path */
#include <stdio.h>      /* printf */
#include <stdlib.h>     /* getenv */
#include <unistd.h>

int main ()
{
	char* pPath;
	//setenv("PATH",".",1);

	pPath = getenv ("PATH");
	if (pPath!=NULL)
		printf ("The current path is: \n%s\n",pPath);
	
	return 0;
}
