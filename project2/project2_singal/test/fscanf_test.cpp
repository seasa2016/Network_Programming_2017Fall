#include<stdio.h>

int main(void)
{
    char test[50]="  QQ  ssssds";
    char arr[50];
    
    sscanf(test,"%s",arr);
    printf("-%s\n",arr);

    return 0;
}