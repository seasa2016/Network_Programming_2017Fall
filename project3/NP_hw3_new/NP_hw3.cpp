#include "resource.h"

#define SERVER_PORT 11400


//=================================================================
//	Global Variables
//=================================================================

vector<reply> user_data;



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,	LPSTR lpCmdLine, int nCmdShow)
{
	
	return DialogBox(hInstance, MAKEINTRESOURCE(ID_MAIN), NULL, MainDlgProc);
}

BOOL CALLBACK MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	WSADATA wsaData;

	static reply user_in;
	static HWND hwndEdit;
	static SOCKET msock, ssock;
	static struct sockaddr_in sa, cli_addr;

	int err , addrlen=sizeof(cli_addr) , i , j ;
	bool key = false;

	switch(Message) 
	{
		case WM_INITDIALOG:
			hwndEdit = GetDlgItem(hwnd, IDC_RESULT);
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case ID_LISTEN:

					WSAStartup(MAKEWORD(2, 0), &wsaData);

					//create master socket
					msock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

					if( msock == INVALID_SOCKET ) {
						EditPrintf(hwndEdit, TEXT("=== Error: create socket error ===\r\n"));
						WSACleanup();
						return TRUE;
					}

					err = WSAAsyncSelect(msock, hwnd, WM_SOCKET_NOTIFY, FD_ACCEPT | FD_CLOSE | FD_WRITE | FD_READ);

					if ( err == SOCKET_ERROR ) {
						EditPrintf(hwndEdit, TEXT("=== Error: select error ===\r\n"));
						closesocket(msock);
						WSACleanup();
						return TRUE;
					}

					//fill the address info about server
					sa.sin_family		= AF_INET;
					sa.sin_port			= htons(SERVER_PORT);
					sa.sin_addr.s_addr	= INADDR_ANY;

					//bind socket
					err = bind(msock, (LPSOCKADDR)&sa, sizeof(struct sockaddr));

					if( err == SOCKET_ERROR ) {
						EditPrintf(hwndEdit, TEXT("=== Error: binding error ===\r\n"));
						WSACleanup();
						return FALSE;
					}

					err = listen(msock, 0);
		
					if( err == SOCKET_ERROR ) {
						EditPrintf(hwndEdit, TEXT("=== Error: listen error ===\r\n"));
						WSACleanup();
						return FALSE;
					}
					else {
						EditPrintf(hwndEdit, TEXT("=== Server START ===\r\n"));
					}

					break;
				case ID_EXIT:
					EndDialog(hwnd, 0);
					break;
			};
			break;

		case WM_CLOSE:
			EndDialog(hwnd, 0);
			break;

		case WM_SOCKET_NOTIFY:
			switch( WSAGETSELECTEVENT(lParam) )
			{
				case FD_ACCEPT:
					ssock = accept(msock, (sockaddr *)&cli_addr, &addrlen);
					
					user_in.parent_fd = ssock;
					user_in.clild_addr = cli_addr;
					user_in.server_addr = sa;

					user_data.push_back(user_in);

					EditPrintf(hwndEdit, TEXT("=== Accept one new client(%d), List size:%d ===\r\n"), ssock, user_data.size());
					break;
				case FD_READ:
					//Write your code for read event here.
					EditPrintf(hwndEdit, TEXT("fd_read\r\n"));

					for(i=0 ; i < user_data.size() ; i++)
						if(wParam == user_data[i].parent_fd  )
							break;

					char text[256];
					sprintf(text, "cgi list at %d\n", i);
					EditPrintf(hwndEdit, TEXT(text));
					
					handle_socket(hwnd, wParam, user_data[ i ] );
					break;
				case FD_WRITE:
				//Write your code for write event here
					EditPrintf(hwndEdit, TEXT("fd_write\r\n"));
					
					break;
				case FD_CLOSE:
					EditPrintf(hwndEdit, TEXT("close at this time\r\n"));
					for (i = 0; i < user_data.size(); i++)
						if (wParam == user_data[i].parent_fd)
							break;
					user_data.erase(user_data.begin() + i );
					break;
			};
			break;
		case CGI_SOCKET_NOTIFY:
			//find out which user it is
			char text[256];
			for( i=0 ; i < user_data.size() ; i++ )
			{
				for( j=0 ; j <user_num ; j++)
					if( (user_data[i].user[j].able == true  || user_data[i].user[j].type == UNLINK) && user_data[i].user[j].fd == wParam)
					{
						key = true;
						break;
					}
				if(key)
					break;
			}

			sprintf(text, "cgi socket at i:%d j:%d with value:%d \n", i, j, wParam);
			EditPrintf(hwndEdit, TEXT(text));

			if( i == user_data.size() || j == user_num)
			{
				EditPrintf(hwndEdit, TEXT("error with cant finding socket \r\n"));
				break;
			}
			switch (WSAGETSELECTEVENT(lParam))
			{
			case FD_CONNECT:

				user_data[i].user[j].able = true;
				user_data[i].user[j].type = READING;
				break;

			case FD_READ:
				//Write your code for read event here.
				
				if( user_data[i].user[j].type ==READING )
				{
					EditPrintf(hwndEdit, TEXT("cgi_read\r\n"));
					cgi_read( hwnd, user_data[i], j );
				}
				break;
			case FD_WRITE:
				//Write your code for write event here
				
				if (user_data[i].user[j].type == WRITING)
				{
					EditPrintf(hwndEdit, TEXT("cgi_write\r\n"));
					cgi_write( hwnd , user_data[i] , j );
				}
				break;
			case FD_CLOSE:
				EditPrintf(hwndEdit, TEXT("cgi_close\r\n"));

				user_data[i].user[j].able = false;

				send(user_data[i].parent_fd , user_data[i].user[j].output_buf, strlen(user_data[i].user[j].output_buf) + 1, 1);
				EditPrintf(hwndEdit, user_data[i].user[j].output_buf);
				strcpy(user_data[i].user[j].output_buf, "");
				
				EditPrintf(hwndEdit, "close cgi\n");

				user_data[i].count--;
				closesocket(user_data[i].user[j].fd);
				if(user_data[i].count==0)
					closesocket(user_data[i].parent_fd);

				break;
			};
			break;
		default:
			return FALSE;


	};

	return TRUE;
}

int EditPrintf (HWND hwndEdit, TCHAR * szFormat, ...)
{
     TCHAR   szBuffer [1024] ;
     va_list pArgList ;

     va_start (pArgList, szFormat) ;
     wvsprintf (szBuffer, szFormat, pArgList) ;
     va_end (pArgList) ;

     SendMessage (hwndEdit, EM_SETSEL, (WPARAM) -1, (LPARAM) -1) ;
     SendMessage (hwndEdit, EM_REPLACESEL, FALSE, (LPARAM) szBuffer) ;
     SendMessage (hwndEdit, EM_SCROLLCARET, 0, 0) ;
	 return SendMessage(hwndEdit, EM_GETLINECOUNT, 0, 0); 
}