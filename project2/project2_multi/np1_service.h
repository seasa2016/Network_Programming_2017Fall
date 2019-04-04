#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include<vector>
#include<fcntl.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/signalfd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>       
#include <sys/types.h>
#include <sys/stat.h>
struct command;
struct temp_command;



using std::vector;
void ccl();
void check(int);
void check2(int);
void name();
void who();
void yell();
void tell();
void dell();
void bro_fifo_out();
void bro_fifo_in();
void str_echo(int);
void broadcast(char *arr);
void multi(int);
bool push(vector<command> &data,temp_command &arr,bool &exit,int ,char*);
bool parsing(vector<command> &data,char *arr,int);
int perform(char *text,char** argu);
