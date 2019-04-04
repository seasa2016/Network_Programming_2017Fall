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
#include <sys/stat.h>  

struct command;
struct temp_command;



using std::vector;

void singal_server(int);
bool push(vector<command> &data,temp_command &arr,bool &exit,int number,char*);
bool parsing(vector<command> &data,char *arr,int);
int perform(char *text,char** argu);
void welcome(int);
bool str_echo(int);
void broadcast(char *arr);
