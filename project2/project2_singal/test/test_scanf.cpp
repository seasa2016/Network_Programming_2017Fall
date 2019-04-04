#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>       
#include <sys/stat.h>
#include <fcntl.h>

int main(void)
{
    int file;
    unlink("QQ.txt");
    file=open("QQ.txt",O_RDONLY , 0666);

    printf("%d\n",file);
}