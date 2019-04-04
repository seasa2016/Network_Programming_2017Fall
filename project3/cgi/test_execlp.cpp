#include<stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(void)
{
    setenv("PATH",".",1);
    printf("%d",execlp("hello.cgi","hello.cgi",NULL));


    
}