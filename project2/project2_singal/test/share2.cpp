#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define BUFSZ 2048

int main(int argc, char *argv[])
{
    int shmid;
    int ret;
    key_t key;
    char *shmadd;
    key =2012;
    char arr[2048];

    if((shmid = shmget(1, BUFSZ, IPC_CREAT | 0666)) < 0)
    {
        perror("shmget");
        exit(-1);
    }
    
    if((shmadd = (char* )shmat(shmid, NULL, 0)) ==(char* ) -1)
    {
        perror("shmat");
        exit(-1);
    }
    //sprintf(shmadd,"acs\n");
    sscanf(shmadd," %s",arr);

    printf("%s",arr);
    printf("%s",shmadd);
    
    printf("copy data from shared-memory\n");
    

    ret = shmdt(shmadd);
    if(ret < 0)
    {
        perror("shmdt");
        exit(1);
    }
    else
    {
        printf("deleted shared-memory\n");
    }

    return 0;
}