#include "resource.h"

struct {
	char ext[16];
	char filetype[16];
} extensions[] = {
	{ "gif", "image/gif" },
	{ "jpg", "image/jpeg" },
	{ "jpeg","image/jpeg" },
	{ "png", "image/png" },
	{ "zip", "image/zip" },
	{ "gz",  "image/gz" },
	{ "tar", "image/tar" },
	{ "htm", "text/html" },
	{ "html","text/html" },
	{ "exe","text/plain" },
	{ "cgi","cgi==" },
	{ 0,0 } };

struct user_type {
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
}user[user_num];

struct client_type {
	int count;
	SOCKET fd;
	HWND hwnd;
	HWND hwndEdit;
}client;

list<SOCKET> Socks;

DWORD g_BytesTransferred = 0;
SOCKET globalfd;

//=================================================================
//	Global Variables
//=================================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return DialogBox(hInstance, MAKEINTRESOURCE(ID_MAIN), NULL, MainDlgProc);
}

void handle_socket(HWND hwnd, SOCKET fd, struct sockaddr_in &serv_addr, struct sockaddr_in &cli_addr)
{
	int j, file_fd, buflen, len;
	long i, ret;
	char *fstr, *ptr, *ptr1, *next_token = NULL;
	char buffer[BUFSIZE + 1];
	char arr[BUFSIZE + 1];
	char temp[64];
	char file_name[64];

	//read the browser message
	ret = recv(fd, buffer, BUFSIZE, 0);

	//read error
	if (ret == 0 || ret == -1)
	{
		exit(3);
	}
	buffer[ret] = '\0';

	static HWND hwndEdit;
	hwndEdit = GetDlgItem(hwnd, IDC_RESULT);

	EditPrintf(hwndEdit, buffer);

	//block return to upper
	for (j = 0; buffer[j + 1]; j++)
		if (buffer[j] == '.'&&buffer[j + 1] == '.')
		{
			printf("can't get back to upper\n");
			exit(3);
		}

	strcpy_s(arr, buffer);

	ptr = strtok_s(arr, " \n\r", &next_token);
	_putenv_s("REQUEST_METHOD", ptr);
	if ((strcmp(ptr, "GET") && strcmp(ptr, "get")))
	{
		printf("only deal with get mothod\n");
		exit(1);
	}
	ptr = strtok_s(NULL, " \n\r", &next_token);
	_putenv_s("REQUEST_URI", ptr);
	ptr1 = strchr(ptr, '?');
	if (ptr1 != NULL)
	{
		*ptr1 = '\0';
		ptr1++;
		_putenv_s("SCRIPT_NAME", ptr);
		_putenv_s("QUERY_STRING", ptr1);
	}
	else
	{
		_putenv_s("SCRIPT_NAME", ptr);
		_putenv_s("QUERY_STRING", "");
	}
	strcpy_s(file_name, ptr);

	ptr = strtok_s(NULL, " \n\r", &next_token);
	_putenv_s("SERVER_PROTOCOL", ptr);

	_putenv_s("REMOTE_ADDR", inet_ntoa(cli_addr.sin_addr));
	sprintf(temp, "%u", htons(cli_addr.sin_port));
	_putenv_s("REMOTE_PORT", temp);
	_putenv_s("SERVER_ADDR", inet_ntoa(serv_addr.sin_addr));
	sprintf(temp, "%u", htons(serv_addr.sin_port));
	_putenv_s("SERVER_PORT", temp);
	ptr = strtok_s(NULL, "\n\r", &next_token);

	ptr = strtok_s(NULL, ":", &next_token);
	ptr = strtok_s(NULL, "\n", &next_token);
	_putenv_s("HTTP_USER_AGENT", ptr + 1);
	ptr = strtok_s(NULL, ":", &next_token);
	ptr = strtok_s(NULL, "\n", &next_token);
	_putenv_s("HTTP_ACCEPT", ptr + 1);
	ptr = strtok_s(NULL, ":", &next_token);
	ptr = strtok_s(NULL, "\n", &next_token);
	_putenv_s("HTTP_ACCEPT_LANGUAGE", ptr);
	ptr = strtok_s(NULL, ":", &next_token);
	ptr = strtok_s(NULL, "\n", &next_token);
	_putenv_s("HTTP_ACCEPT_ENCODING", ptr);
	ptr = strtok_s(NULL, ":", &next_token);
	ptr = strtok_s(NULL, "\n", &next_token);
	_putenv_s("HTTP_CONNECTION", ptr);
	ptr = strtok_s(NULL, ":", &next_token);
	_putenv_s("HTTP_UPGRADE_INSECURE_REQUESTS", ptr);
	_putenv_s("REDIRECT_STATUS", "200");
	_putenv_s("GATEWAY_INTERFACE", "CGI/1.1");
	_putenv_s("CONTENT_LENGTH", "0");
	_putenv_s("PATH", ".");

	//find the file format
	buflen = strlen(file_name);
	fstr = (char *)0;

	for (i = 0; extensions[i].ext; i++)
	{
		len = strlen(extensions[i].ext);
		if (!strncmp(&file_name[buflen - len], extensions[i].ext, len))
		{
			fstr = extensions[i].filetype;
			break;
		}
	}

	if (extensions[i].ext == 0)
	{
		fstr = extensions[i - 1].filetype;
	}

	//deal with the hw3.cgi
	if (strcmp(extensions[i].ext, "cgi") == 0)
	{
		sprintf(buffer, "HTTP/1.1 200 OK\r\n");
		send(fd, buffer, strlen(buffer), 1);

		char as[5];
		sprintf(as, "%d", fd);
		EditPrintf(hwndEdit, as);

		hw3(hwnd, fd);
		return;
	}
	else
	{
		sprintf(buffer, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", fstr);
		send(fd, buffer, strlen(buffer), 1);



		DWORD  dwBytesRead = 0;
		OVERLAPPED ol = { 0 };

		HANDLE hFile;
		hFile = CreateFile(file_name + 1, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			EditPrintf(hwndEdit, TEXT("Terminal failure: Unable to open file\n"));
			return;
		}
		else
		{
			while (1)
			{

				ReadFile(hFile, buffer, 1024, &dwBytesRead, NULL);

				// This is the section of code that assumes the file is ANSI text. 
				// Modify this block for other data types if needed.

				if (dwBytesRead > 0 && dwBytesRead <= BUFSIZE)
				{
					buffer[dwBytesRead] = '\0'; // NULL character
					send(fd, buffer, strlen(buffer), 1);
				}
				else if (dwBytesRead == 0)
				{
					break;
				}
			}

		}
		closesocket(fd);
	}
	return;
}


BOOL CALLBACK MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	WSADATA wsaData;

	static HWND hwndEdit;
	static SOCKET msock, ssock;
	static struct sockaddr_in serv_addr, cli_addr;
	int err;
	int count;
	char as[20];

	switch (Message)
	{
	case WM_INITDIALOG:
		hwndEdit = GetDlgItem(hwnd, IDC_RESULT);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_LISTEN:
			WSAStartup(MAKEWORD(2, 0), &wsaData);

			//create master socket
			msock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			msock;

			if (msock == INVALID_SOCKET) {
				EditPrintf(hwndEdit, TEXT("=== Error: create socket error ===\r\n"));
				WSACleanup();
				return TRUE;
			}

			err = WSAAsyncSelect(msock, hwnd, WM_SOCKET_NOTIFY, FD_ACCEPT | FD_CLOSE);

			if (err == SOCKET_ERROR) {
				EditPrintf(hwndEdit, TEXT("=== Error: select error ===\r\n"));
				closesocket(msock);
				WSACleanup();
				return TRUE;
			}

			//fill the address info about server
			serv_addr.sin_family = AF_INET;
			serv_addr.sin_port = htons(SERVER_PORT);
			serv_addr.sin_addr.s_addr = INADDR_ANY;

			//bind socket
			err = bind(msock, (LPSOCKADDR)&serv_addr, sizeof(struct sockaddr));

			if (err == SOCKET_ERROR) {
				EditPrintf(hwndEdit, TEXT("=== Error: binding error ===\r\n"));
				WSACleanup();
				return FALSE;
			}

			err = listen(msock, 2);

			if (err == SOCKET_ERROR) {
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
		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_ACCEPT:
			ssock = accept(msock, NULL, NULL);
			EditPrintf(hwndEdit, TEXT("=== Accept one new client(%d), List size ===\r\n"), ssock);


			Socks.push_back(ssock);

			sprintf(as, "%d %d\n", ssock, msock);
			EditPrintf(hwndEdit, as);

			//
			(ssock, "hello\n", 7, 0);
			err = WSAAsyncSelect(ssock, hwnd, WM_SOCKET_NOTIFY, FD_CLOSE | FD_READ | FD_WRITE);

			if (err == SOCKET_ERROR) {
				EditPrintf(hwndEdit, TEXT("=== Error: select error ===\r\n"));
				closesocket(msock);
				WSACleanup();
				return TRUE;
			}

			break;
		case FD_READ:
			//Write your code for read event here.
			handle_socket(hwnd, wParam, serv_addr, cli_addr);
			break;
		case FD_WRITE:
			//Write your code for write event here
			EditPrintf(hwndEdit, TEXT("test\r\n"));
			break;
		case FD_CLOSE:
			EditPrintf(hwndEdit, TEXT("close at this time\r\n"));
			break;
		};
		break;

	case CGI_NOTIFY:
		//EditPrintf(hwndEdit, TEXT("test cgi\r\n"));
		cgi(hwnd, Message, wParam, lParam, ssock);
		break;
	default:
		return FALSE;
	};

	return TRUE;
}

int EditPrintf(HWND hwndEdit, TCHAR * szFormat, ...)
{
	TCHAR   szBuffer[2048];
	va_list pArgList;

	va_start(pArgList, szFormat);
	wvsprintf(szBuffer, szFormat, pArgList);
	va_end(pArgList);

	SendMessage(hwndEdit, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
	SendMessage(hwndEdit, EM_REPLACESEL, FALSE, (LPARAM)szBuffer);
	SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);
	return SendMessage(hwndEdit, EM_GETLINECOUNT, 0, 0);
}


void start_html()
{
	char arr[1024] = "\0";
	char tt[1024] = "\0";
	strcat(arr, "Content-type:text/html\r\n\r\n");
	strcat(arr, "<html>\r\n");
	strcat(arr, "<head>\r\n");
	strcat(arr, "<title>Network Programming Homework 3</title>\r\n");
	strcat(arr, "</head>\r\n");
	strcat(arr, "<body bgcolor=#336699>\r\n");
	strcat(arr, "<font face=\"Courier New\" size=2 color=#FFFF99>\r\n");
	strcat(arr, "<table width=\"800\" border=\"1\">\r\n");
	strcat(arr, "<tr>\r\n");
	//deal with the link
	for (int i = 0; i < user_num; i++)
	{
		sprintf(tt, "<td>%s</td>", user[i].host);
		strcat(arr, tt);
	}
	strcat(arr, "\r\n");
	strcat(arr, "<tr>\r\n");
	strcat(arr, "<tr>\r\n");
	for (int i = 0; i < user_num; i++)
	{
		sprintf(tt, "<td valign=\"top\" id=\"%c%d\"></td>", 'm', i);
		strcat(arr, tt);
	}
	strcat(arr, "\r\n");
	strcat(arr, "</table>\r\n");
	strcat(arr, "</font>\r\n");
	strcat(arr, "</body>\r\n");
	strcat(arr, "</html>\r\n");
	send(client.fd, arr, strlen(arr), 1);
	EditPrintf(client.hwndEdit, arr);

	//EditPrintf(client.hwndEdit , arr);
}
void connect_user(int i)
{
	struct sockaddr_in client_sin;

	int server_port;
	int err;
	struct hostent *he;
	char arr[1024] = "\0";

	user[i].file_fd = CreateFile(user[i].file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (user[i].file_fd == INVALID_HANDLE_VALUE)
	{
		sprintf(arr, "<script>document.all['m%d'].innerHTML += \"Error : '%s' doesn't exist<br>\"</script>\r\n", i, user[i].file);
		send(client.fd, arr, strlen(arr), 1);
		return;
	}
	if ((he = gethostbyname(user[i].host)) == NULL)
	{
		sprintf(arr, "<script>document.all['m%d'].innerHTML += \"Error : client doesn't exist<br>\"</script>\r\n", i);
		send(client.fd, arr, strlen(arr), 1);
		return;
	}
	server_port = (u_short)user[i].port;

	memset(&client_sin, sizeof(client_sin), 0);
	client_sin.sin_family = AF_INET;
	client_sin.sin_addr = *((struct in_addr *)he->h_addr);
	client_sin.sin_port = htons(server_port);

	user[i].fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);


	WSAAsyncSelect(user[i].fd, client.hwnd, CGI_NOTIFY, FD_CONNECT);

	int n;

	if ((n = connect(user[i].fd, (struct sockaddr *)&client_sin, sizeof(client_sin))) < 0)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			sprintf(arr, "<script>document.all['m%d'].innerHTML += \"error at connect<br>\"</script>\r\n", i);
			send(client.fd, arr, strlen(arr), 1);
			return;
		}
	}

	client.count++;
	user[i].type = READING;

}

int hw3(HWND hwnd, SOCKET fd)
{
	char arr[256], *ptr;
	int error, n;
	char temp[16192];
	client.hwndEdit = GetDlgItem(hwnd, IDC_RESULT);

	client.fd = fd;
	client.hwnd = hwnd;

	strcpy(arr, getenv("QUERY_STRING"));

	//strcpy(arr, "h1=nplinux2.cs.nctu.edu.tw&p1=11400&f1=t1.txt&h2=&p2=&f2=&h3=&p3=&f3=&h4=&p4=&f4=&h5=&p5=&f5=");
	//strcpy(arr,"h1=nplinux2.cs.nctu.edu.tw&p1=11400&f1=t1.txt&h2=nplinux4.cs.nctu.edu.tw&p2=11400&f2=t1.txt&h3=nplinux2.cs.nctu.edu.tw&p3=11400&f3=t1.txt&h4=&p4=&f4=&h5=&p5=&f5=");
	//strcpy(arr,"h1=nplinux4.cs.nctu.edu.tw&p1=12400&f1=t1.txt&h2=nplinux4.cs.nctu.edu.tw&p2=12400&f2=t2.txt&h3=&p3=&f3=&h4=&p4=&f4=&h5=&p5=&f5=");
	//strcpy(arr,"h1=nplinux2.cs.nctu.edu.tw&p1=11400&f1=t1.txt&h2=nplinux2.cs.nctu.edu.tw&p2=11400&f2=t1.txt&h3=&p3=&f3=&h4=&p4=&f4=&h5=&p5=&f5=");
	ptr = strtok(arr, "1");
	for (int i = 0; i < user_num; i++)
	{
		user[i].type = UNLINK;
		user[i].fd = -1;
		ptr = strtok(NULL, "&");
		ptr = strchr(ptr, '=');
		strcpy(user[i].host, ptr + 1);

		ptr = strtok(NULL, "&");
		ptr = strchr(ptr, '=');
		user[i].port = atoi(ptr + 1);

		ptr = strtok(NULL, "&");
		ptr = strchr(ptr, '=');
		strcpy(user[i].file, ptr + 1);
		user[i].needwrite = 0;
		strcpy(user[i].output_buf, "");
	}
	start_html();
	//deal with link

	client.count = 0;

	for (int i = 0; i < user_num; i++)
		if (user[i].host[0] != '\0')
			connect_user(i);

	return 0;
}

int readline(HANDLE  fd, char *ptr, int maxlen)
{
	int n, rc;
	char c, temp;
	DWORD  dwBytesRead = 0;
	*ptr = 0;
	for (n = 1; n < maxlen; n++)
	{
		rc = ReadFile(fd, &c, 1, &dwBytesRead, NULL);
		if (rc == 1)
		{
			*ptr++ = c;
			if (c == ' '&& *(ptr - 2) == '%') { break; }
			if (c == '\n')  break;
		}
		else if (rc == 0)
		{
			if (n == 1)     return(0);
			else         break;
		}
		else
			return(-1);
	}


	return(n);
}


void cgi(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam, SOCKET globalfd)
{
	int i;
	int error, n, len;
	char temp[1024];
	bool key = false;


	for (i = 0; user[i].fd != wParam && i < client.count; i++);

	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_CONNECT:

		//EditPrintf(client.hwndEdit, TEXT("test1\n"));
		WSAAsyncSelect(user[i].fd, hwnd, CGI_NOTIFY, FD_READ | FD_CLOSE);
		break;
	case FD_WRITE:
		//EditPrintf(client.hwndEdit, TEXT("test2\n"));
		//Write your code for read event here.
		int ini;
		if (user[i].needwrite == 0)
		{
			len = readline(user[i].file_fd, user[i].sent_buf, 16192);
			if (len < 0) exit(1);

			user[i].sent_buf[len - 1] = 13;
			user[i].sent_buf[len] = 10;
			user[i].sent_buf[len + 1] = 0;

			user[i].rev_size = user[i].needwrite = len + 1;

			sprintf(temp, "<script>document.all['m%d'].innerHTML += \"<b>", i);
			strcat(user[i].output_buf, temp);
			//EditPrintf(client.hwndEdit, TEXT("\n\n"));
			for (ini = 0; ini < len + 1; ini++)
			{
				//EditPrintf(client.hwndEdit, user[i].output_buf);
				switch (user[i].sent_buf[ini])
				{
				case '\r':
					break;
				case '\n':
					strcat(user[i].output_buf, "<br>");
					break;
				case '\"':
					strcat(user[i].output_buf, "&quot;");
					break;
				case '\'':
					strcat(user[i].output_buf, "&apos;");
					break;
				case '<':
					strcat(user[i].output_buf, "&lt;");
					break;
				case '>':
					strcat(user[i].output_buf, "&gt;");
					break;
				case '&':
					strcat(user[i].output_buf, "&amp;");
					break;
				case ' ':
					strcat(user[i].output_buf, "&nbsp;");
					break;
				default:
					sprintf(temp, "%c\0", user[i].sent_buf[ini]);
					strcat(user[i].output_buf, temp);
					break;
				}
			}
			strcat(user[i].output_buf, "</b>\"</script>\n");
		}

		n = send(user[i].fd, user[i].sent_buf + user[i].rev_size - user[i].needwrite, user[i].needwrite, 1);
		user[i].needwrite -= n;

		if (n <= 0 || user[i].needwrite <= 0)
		{
			fflush(stdout);
			// write finished
			user[i].type = READING;
			//FD_SET(user[i].fd, &client.rs);

			WSAAsyncSelect(user[i].fd, hwnd, CGI_NOTIFY, FD_READ | FD_CLOSE);
		}
		break;
	case FD_READ:
		//EditPrintf(client.hwndEdit, TEXT("test3\n"));
		//Write your code for write event here
		key = false;

		n = recv(user[i].fd, user[i].rev, 4096, 0);


		if (n == 0)    //the oppsite close the socket
		{

			user[i].type = UNLINK;
			send(client.fd, user[i].output_buf, strlen(user[i].output_buf), 1);
			EditPrintf(client.hwndEdit, user[i].output_buf);

			//printf("exit test %d\n",client.count);
		}
		else if (n > 0)
		{
			user[i].rev[n] = '\0';
			sprintf(temp, "<script>document.all['m%d'].innerHTML += \"", i);
			strcat(user[i].output_buf, temp);

			for (int ini = 0; user[i].rev[ini]; ini++)
			{
				switch (user[i].rev[ini])
				{
				case '\r':
					break;
				case '\n':
					strcat(user[i].output_buf, "<br>");
					break;
				case '\"':
					strcat(user[i].output_buf, "&quot;");
					break;
				case '\'':
					strcat(user[i].output_buf, "&apos;");
					break;
				case '<':
					strcat(user[i].output_buf, "&lt;");
					break;
				case '>':
					strcat(user[i].output_buf, "&gt;");
					break;
				case '&':
					strcat(user[i].output_buf, "&amp;");
					break;
				case ' ':
					strcat(user[i].output_buf, "&nbsp;");
					break;
				default:
					sprintf(temp, "%c", user[i].rev[ini]);
					strcat(user[i].output_buf, temp);
				}
			}
			strcat(user[i].output_buf, "\"</script>\r\n");

			/*********************************************************/
			for (int ss = 0; user[i].rev[ss + 1]; ss++)
				if (user[i].rev[ss] == '%' && user[i].rev[ss + 1] == ' ')
					key = true;

			n = strlen(user[i].rev);
			if (key)
			{
				// read finished

				user[i].type = WRITING;


				send(globalfd, user[i].output_buf, strlen(user[i].output_buf), 1);
				EditPrintf(client.hwndEdit, user[i].output_buf);

				strcpy(user[i].output_buf, "");
				WSAAsyncSelect(user[i].fd, hwnd, CGI_NOTIFY, FD_WRITE);
			}
			else if (user[i].rev[n] == '\n' || user[i].rev[n - 1] == '\n')
			{
				send(globalfd, user[i].output_buf, strlen(user[i].output_buf), 1);
				EditPrintf(client.hwndEdit, user[i].output_buf);

				strcpy(user[i].output_buf, "");
			}
			user[i].rev[0] = '\0';
		}
		break;
	case FD_CLOSE:

		sprintf(temp, "test close\n");
		EditPrintf(client.hwndEdit, temp);

		send(globalfd, user[i].output_buf, strlen(user[i].output_buf), 1);
		EditPrintf(client.hwndEdit, user[i].output_buf);
		strcpy(user[i].output_buf, "");


		//closesocket(user[i].fd);
		//user[i].fd=-1;
		break;
	}
}



