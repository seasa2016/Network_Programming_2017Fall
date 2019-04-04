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

struct command;
struct temp_command;



using std::vector;
void str_echo();
bool push(vector<command> &data,temp_command &arr,bool &exit);
bool parsing(vector<command> &data,char *arr);
int perform(char *text,char**argu);
