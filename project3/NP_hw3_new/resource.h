//{{NO_DEPENDENCIES}}
// Microsoft Visual C++ generated include file.
// Used by NP_hw3.rc
//
#define ID_MAIN                         9
#define ID_LISTEN                     1001
#define ID_EXIT							1002
#define IDC_EDIT1						1003
#define IDC_RESULT                 1004
#define WM_SOCKET_NOTIFY (WM_USER + 1)
#define CGI_SOCKET_NOTIFY (WM_USER + 2)

#define user_num 5

#define UNDEFINE -1
#define UNLINK 0
#define CONNECTING 1
#define READING 2
#define WRITING 3

#include <windows.h>
#include <vector>
#include "cgi.h"
#include "NP_hw3.h"
using namespace std;

struct user_type {
	bool able;
	int type;
	char host[256];
	int port;
	char file[256];

	SOCKET fd;
	HANDLE  file_fd;

	int needwrite;
	int rev_size;
	char rev_buf[32384];
	char rev[32384];
	char sent_buf[32384];
	char output_buf[32384] = "\0";
};

struct reply {
	sockaddr_in server_addr, clild_addr;
	SOCKET parent_fd;
	int count;
	struct user_type user[5];
};



// Next default values for new objects
// 
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        102
#define _APS_NEXT_COMMAND_VALUE         40001
#define _APS_NEXT_CONTROL_VALUE         1005
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif
