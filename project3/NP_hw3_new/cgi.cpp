#include "resource.h"
#define BUFSIZE 8096
#define user_num 5

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
	{ "html", "text/html" },
	{ "exe","text/plain" },
	{ "cgi","cgi==" },
	{ 0,0 } };





void handle_socket(  HWND hwnd , SOCKET fd , struct reply &user_information )
{
	int j, file_fd, buflen, len;
	long i, ret;
	char *fstr, *ptr, *ptr1, *next_token = NULL;
	char buffer[BUFSIZE + 1];
	char arr[BUFSIZE + 1];
	char temp[64];
	char file_name[64];

	//read the browser message
	ret = recv(  fd, buffer, BUFSIZE, 0 );

	//read error
	if (  ret == 0 || ret == -1 )
	{
		exit(  3 );
	}
	buffer[ret] = '\0';

	static HWND hwndEdit;
	hwndEdit = GetDlgItem(  hwnd, IDC_RESULT );
	EditPrintf(  hwndEdit, buffer );

	//block return to upper
	for (  j = 0; buffer[j + 1]; j++ )
		if (  buffer[j] == '.'&&buffer[j + 1] == '.' )
		{
			printf(  "can't get back to upper\n" );
			exit(  3 );
		}

	strcpy_s(  arr, buffer );

	ptr = strtok_s(  arr, " \n\r", &next_token );
	_putenv_s(  "REQUEST_METHOD", ptr );
	if (  (  strcmp(  ptr, "GET" ) && strcmp(  ptr, "get" ) ) )
	{
		printf(  "only deal with get mothod\n" );
		exit(  1 );
	}
	ptr = strtok_s(  NULL, " \n\r", &next_token );
	_putenv_s(  "REQUEST_URI", ptr );
	ptr1 = strchr(  ptr, '?' );
	if (  ptr1 != NULL )
	{
		*ptr1 = '\0';
		ptr1++;
		_putenv_s(  "SCRIPT_NAME", ptr );
		_putenv_s(  "QUERY_STRING", ptr1 );
	}
	else
	{
		_putenv_s(  "SCRIPT_NAME", ptr );
		_putenv_s(  "QUERY_STRING", "" );
	}
	strcpy_s(  file_name, ptr );

	ptr = strtok_s(  NULL, " \n\r", &next_token );
	_putenv_s(  "SERVER_PROTOCOL", ptr );

	_putenv_s(  "REMOTE_ADDR", inet_ntoa(   user_information.clild_addr.sin_addr  )  );
	sprintf(  temp, "%u", htons(   user_information.clild_addr.sin_port  )  );
	_putenv_s(  "REMOTE_PORT", temp );
	_putenv_s(  "SERVER_ADDR", inet_ntoa( user_information.server_addr.sin_addr ) );
	sprintf(  temp, "%u", htons(user_information.server_addr.sin_port ) );
	_putenv_s(  "SERVER_PORT", temp );
	ptr = strtok_s(  NULL, "\n\r", &next_token );

	ptr = strtok_s(  NULL, ":", &next_token );
	ptr = strtok_s(  NULL, "\n", &next_token );
	_putenv_s(  "HTTP_USER_AGENT", ptr + 1 );
	ptr = strtok_s(  NULL, ":", &next_token );
	ptr = strtok_s(  NULL, "\n", &next_token );
	_putenv_s(  "HTTP_ACCEPT", ptr + 1 );
	ptr = strtok_s(  NULL, ":", &next_token );
	ptr = strtok_s(  NULL, "\n", &next_token );
	_putenv_s(  "HTTP_ACCEPT_LANGUAGE", ptr );
	ptr = strtok_s(  NULL, ":", &next_token );
	ptr = strtok_s(  NULL, "\n", &next_token );
	_putenv_s(  "HTTP_ACCEPT_ENCODING", ptr );
	ptr = strtok_s(  NULL, ":", &next_token );
	ptr = strtok_s(  NULL, "\n", &next_token );
	_putenv_s(  "HTTP_CONNECTION", ptr );
	ptr = strtok_s(  NULL, ":", &next_token );
	_putenv_s(  "HTTP_UPGRADE_INSECURE_REQUESTS", ptr );
	_putenv_s(  "REDIRECT_STATUS", "200" );
	_putenv_s(  "GATEWAY_INTERFACE", "CGI/1.1" );
	_putenv_s(  "CONTENT_LENGTH", "0" );
	_putenv_s(  "PATH", "." );

	//find the file format
	buflen = strlen(  file_name );
	fstr = (  char * )0;

	for (  i = 0; extensions[i].ext; i++ )
	{
		len = strlen(  extensions[i].ext );
		if (  !strncmp(  &file_name[buflen - len], extensions[i].ext, len ) )
		{
			fstr = extensions[i].filetype;
			break;
		}
	}

	if (  extensions[i].ext == 0 )
	{
		fstr = extensions[i - 1].filetype;
	}

	//deal with the hw3.cgi
	if (  strcmp(  extensions[i].ext, "cgi" ) == 0 )
	{
		sprintf(  buffer, "HTTP/1.1 200 OK\r\n" );
		send(  fd, buffer, strlen(  buffer ), 1 );

		/*char as[5];
		sprintf(  as, "%d", fd );
		EditPrintf(  hwndEdit, as );*/

		hw3(  hwnd, user_information);
		return;
	}
	else if (  strcmp(  extensions[i].ext, "htm" ) == 0 )
	{
		sprintf(  buffer, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", fstr );
		send(  fd, buffer, strlen(  buffer ), 1 );



		DWORD  dwBytesRead = 0;
		OVERLAPPED ol = { 0 };

		HANDLE hFile;
		hFile = CreateFile(  file_name + 1, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if (  hFile == INVALID_HANDLE_VALUE )
		{
			EditPrintf(  hwndEdit, TEXT(  "Terminal failure: Unable to open file\n" ) );
			return;
		}
		else
		{
			while (  1 )
			{

				ReadFile(  hFile, buffer, 1024, &dwBytesRead, NULL );
				if (  dwBytesRead > 0 && dwBytesRead <= BUFSIZE )
				{
					buffer[dwBytesRead] = '\0'; // NULL character
					send(  fd, buffer, strlen(  buffer ), 1 );
				}
				else if (  dwBytesRead == 0 )
				{
					break;
				}
			}

		}
		closesocket(  fd );
	}
	else 		closesocket(  fd );

	return;
}

int hw3(  HWND hwnd, struct reply &user_information)
{
	char arr[256], *ptr;
	int error, n;
	char temp[16192];
	HWND hwndEdit;
	hwndEdit = GetDlgItem(  hwnd, IDC_RESULT );


	//strcpy(  arr, "h1=nplinux2.cs.nctu.edu.tw&p1=11400&f1=t1.txt&h2=&p2=&f2=&h3=&p3=&f3=&h4=&p4=&f4=&h5=&p5=&f5=" );
	strcpy(  arr, getenv(  "QUERY_STRING" ) );

	ptr = strtok(  arr, "1" );
	for (  int i = 0; i < user_num; i++ )
	{
		user_information.user[i].able = false;
		user_information.user[i].type = UNDEFINE;
		user_information.user[i].fd = -1;
		ptr = strtok(  NULL, "&" );
		ptr = strchr(  ptr, '=' );
		strcpy(user_information.user[i].host, ptr + 1 );

		ptr = strtok(  NULL, "&" );
		ptr = strchr(  ptr, '=' );
		user_information.user[i].port = atoi(  ptr + 1 );

		ptr = strtok(  NULL, "&" );
		ptr = strchr(  ptr, '=' );
		strcpy( user_information.user[i].file, ptr + 1 );
		user_information.user[i].needwrite = 0;
		strcpy( user_information.user[i].output_buf, "" );
	}
	start_html( hwndEdit , user_information );
	//deal with link

	user_information.count = 0;

	for (  int i = 0; i < user_num; i++ )
		if ( user_information.user[i].host[0] != '\0' )
			connect_user( hwnd , user_information , i );

	return 0;
}

int readline(  HANDLE  fd, char *ptr, int maxlen )
{
	int n, rc;
	char c, temp;
	DWORD  dwBytesRead = 0;
	*ptr = 0;
	for (  n = 1; n < maxlen; n++ )
	{
		rc = ReadFile(  fd, &c, 1, &dwBytesRead, NULL );
		if (  rc == 1 )
		{
			*ptr++ = c;
			if (  c == ' '&& *(  ptr - 2 ) == '%' ) { break; }
			if (  c == '\n' )  break;
		}
		else if (  rc == 0 )
		{
			if (  n == 1 )     return(  0 );
			else         break;
		}
		else
			return(  -1 );
	}


	return(  n );
}

void connect_user(  HWND hwnd , struct reply &user_information , int user_index )
{
	struct sockaddr_in client_sin;
	HWND hwndEdit;
	hwndEdit = GetDlgItem(hwnd, IDC_RESULT);

	int server_port;
	int err;
	struct hostent *he;
	char arr[1024] = "\0";

	user_information.user[user_index].file_fd = CreateFile(user_information.user[user_index].file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if (user_information.user[user_index].file_fd == INVALID_HANDLE_VALUE )
	{
		sprintf(  arr, "<script>document.all['m%d'].innerHTML += \"Error : '%s' doesn't exist<br>\"</script>\r\n", user_index, user_information.user[user_index].file );
		send( user_information.parent_fd , arr, strlen(  arr ), 1 );
		return;
	}
	if (  (  he = gethostbyname(user_information.user[user_index].host ) ) == NULL )
	{
		sprintf(  arr, "<script>document.all['m%d'].innerHTML += \"Error : client doesn't exist<br>\"</script>\r\n", user_index);
		send( user_information.parent_fd , arr, strlen(  arr ), 1 );
		return;
	}
	server_port = (  u_short )user_information.user[user_index].port;

	memset(  &client_sin, sizeof(  client_sin ), 0 );
	client_sin.sin_family = AF_INET;
	client_sin.sin_addr = *(  (  struct in_addr * )he->h_addr );
	client_sin.sin_port = htons(  server_port );

	user_information.user[user_index].fd = socket(  AF_INET, SOCK_STREAM, IPPROTO_TCP );


	WSAAsyncSelect(user_information.user[user_index].fd , hwnd, CGI_SOCKET_NOTIFY, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE);

	int n;

	if (  (  n = connect(user_information.user[user_index].fd, (  struct sockaddr * )&client_sin, sizeof(  client_sin ) ) ) < 0 )
	{
		if (  WSAGetLastError(   ) != WSAEWOULDBLOCK )
		{
			sprintf(  arr, "<script>document.all['m%d'].innerHTML += \"error at connect<br>\"</script>\r\n", user_index);
			send( user_information.parent_fd , arr, strlen(  arr ), 1 );
			return;
		}
	}

	char text[256];
	sprintf(text, "cgi socket at index:%d with value:%d \n",  user_index , user_information.user[user_index].fd);
	EditPrintf(hwndEdit, TEXT(text));

	user_information.user[user_index].type = UNLINK;
	user_information.count++;
}

void start_html(HWND hwnd , struct reply &user_information )
{
	HWND hwndEdit;
	hwndEdit = GetDlgItem(hwnd, IDC_RESULT);
	char arr[1024] = "\0";
	char tt[1024] = "\0";
	strcat(  arr, "Content-type:text/html\r\n\r\n" );
	strcat(  arr, "<html>\r\n" );
	strcat(  arr, "<head>\r\n" );
	strcat(  arr, "<title>Network Programming Homework 3</title>\r\n" );
	strcat(  arr, "</head>\r\n" );
	strcat(  arr, "<body bgcolor=#336699>\r\n" );
	strcat(  arr, "<font face=\"Courier New\" size=2 color=#FFFF99>\r\n" );
	strcat(  arr, "<table width=\"800\" border=\"1\">\r\n" );
	strcat(  arr, "<tr>\r\n" );
	//deal with the link
	for (  int i = 0; i < user_num; i++ )
	{
		sprintf(  tt, "<td>%s</td>", user_information .user[i].host );
		strcat(  arr, tt );
	}
	strcat(  arr, "\r\n" );
	strcat(  arr, "<tr>\r\n" );
	strcat(  arr, "<tr>\r\n" );
	for (  int i = 0; i < user_num; i++ )
	{
		sprintf(  tt, "<td valign=\"top\" id=\"%c%d\"></td>", 'm', i );
		strcat(  arr, tt );
	}
	strcat(  arr, "\r\n" );
	strcat(  arr, "</table>\r\n" );
	strcat(  arr, "</font>\r\n" );
	strcat(  arr, "</body>\r\n" );
	strcat(  arr, "</html>\r\n" );
	send( user_information.parent_fd , arr, strlen(  arr ), 1 );
	EditPrintf(  hwndEdit, arr );
}

void cgi_write(HWND hwnd, reply & user_information , int user_index)
{
	int len , ini , n , iError;
	char temp[2048];
	HWND hwndEdit;
	hwndEdit = GetDlgItem(hwnd, IDC_RESULT);

	if ( user_information.user[ user_index ].needwrite == 0)
	{
		len = readline(user_information.user[ user_index ].file_fd, user_information.user[ user_index ].sent_buf, 16192);
		if (len < 0) return ;

		user_information.user[ user_index ].sent_buf[len - 1] = 13;
		user_information.user[ user_index ].sent_buf[len] = 10;
		user_information.user[ user_index ].sent_buf[len + 1] = 0;

		user_information.user[ user_index ].rev_size = user_information.user[user_index].needwrite = strlen( user_information.user[user_index].sent_buf );

		sprintf(temp, "<script>document.all['m%d'].innerHTML += \"<b>", user_index);
		strcat( user_information.user[user_index].output_buf , temp);
		//EditPrintf(client.hwndEdit, TEXT("\n\n"));
		for (ini = 0; ini < len + 1; ini++)
		{
			//EditPrintf(client.hwndEdit, user[i].output_buf);
			switch ( user_information.user[user_index].sent_buf[ini] )
			{
			case '\r':
				break;
			case '\n':
				strcat( user_information.user[user_index].output_buf , "<br>");
				break;
			case '\"':
				strcat(user_information.user[user_index].output_buf, "&quot;");
				break;
			case '\'':
				strcat(user_information.user[user_index].output_buf, "&apos;");
				break;
			case '<':
				strcat(user_information.user[user_index].output_buf, "&lt;");
				break;
			case '>':
				strcat(user_information.user[user_index].output_buf, "&gt;");
				break;
			case '&':
				strcat(user_information.user[user_index].output_buf, "&amp;");
				break;
			case ' ':
				strcat(user_information.user[user_index].output_buf, "&nbsp;");
				break;
			default:
				sprintf(temp, "%c\0", user_information.user[user_index].sent_buf[ini]);
				strcat(user_information.user[user_index].output_buf, temp);
				break;
			}
		}
		strcat(user_information.user[user_index].output_buf, "</b>\"</script>\n");
	}

	EditPrintf(hwndEdit, TEXT("read file\n"));
	n = send(user_information.user[user_index].fd, user_information.user[user_index].sent_buf + user_information.user[user_index].rev_size - user_information.user[user_index].needwrite, user_information.user[user_index].needwrite, 1);
	user_information.user[user_index].needwrite -= n;

	char sdsd[16];
	sprintf(sdsd, "%d\n", n);
	EditPrintf( hwndEdit, sdsd);
	if (n == SOCKET_ERROR)
	{
		iError = WSAGetLastError();
		if (iError == WSAEWOULDBLOCK)
		{
			//sleep for awile
			Sleep(1000);
		}
	}
	else if (n == 0 || user_information.user[user_index].needwrite <= 0)
	{
		// write finished
		user_information.user[user_index].type = READING;
	}
}

void cgi_read(HWND hwnd, reply & user_information, int user_index)
{
	bool key = false;
	int n;
	HWND hwndEdit;
	hwndEdit = GetDlgItem(hwnd, IDC_RESULT);
	char temp[2048];

	n = recv(user_information.user[user_index].fd, user_information.user[user_index].rev, 4096, 0);
	if (n == 0)    //the oppsite close the socket
	{

		user_information.user[user_index].type = UNDEFINE;
		user_information.user[user_index].able = false;

		send(user_information.user[user_index].fd, user_information.user[user_index].output_buf, strlen(user_information.user[user_index].output_buf), 1);
		EditPrintf(hwndEdit, user_information.user[user_index].output_buf);

		EditPrintf(hwndEdit, TEXT("cgi_close\r\n"));
		//printf("exit test %d\n",client.count);
	}
	else if (n > 0)
	{
		user_information.user[user_index].rev[n] = '\0';
		sprintf(temp, "<script>document.all['m%d'].innerHTML += \"", user_index);
		strcat(user_information.user[user_index].output_buf, temp);

		for (int ini = 0; user_information.user[user_index].rev[ini]; ini++)
		{
			switch (user_information.user[user_index].rev[ini])
			{
			case '\r':
				break;
			case '\n':
				strcat(user_information.user[user_index].output_buf, "<br>");
				break;
			case '\"':
				strcat(user_information.user[user_index].output_buf, "&quot;");
				break;
			case '\'':
				strcat(user_information.user[user_index].output_buf, "&apos;");
				break;
			case '<':
				strcat(user_information.user[user_index].output_buf, "&lt;");
				break;
			case '>':
				strcat(user_information.user[user_index].output_buf, "&gt;");
				break;
			case '&':
				strcat(user_information.user[user_index].output_buf, "&amp;");
				break;
			case ' ':
				strcat(user_information.user[user_index].output_buf, "&nbsp;");
				break;
			default:
				sprintf(temp, "%c", user_information.user[user_index].rev[ini]);
				strcat(user_information.user[user_index].output_buf, temp);
			}
		}
		strcat(user_information.user[user_index].output_buf, "\"</script>\r\n");

		/*********************************************************/
		for (int ss = 0; user_information.user[user_index].rev[ss + 1]; ss++)
			if (user_information.user[user_index].rev[ss] == '%' && user_information.user[user_index].rev[ss + 1] == ' ')
				key = true;

		n = strlen(user_information.user[user_index].rev);
		if (key)
		{
			// read finished

			user_information.user[user_index].type = WRITING;
			
			int ss = send(user_information.parent_fd , user_information.user[user_index].output_buf, strlen(user_information.user[user_index].output_buf) , 1);
			
			/*
			EditPrintf( hwndEdit, user_information.user[user_index].output_buf);
			sprintf(user_information.user[user_index].output_buf, "%d %d\n", ss, strlen(user_information.user[user_index].output_buf));
			EditPrintf( hwndEdit, user_information.user[user_index].output_buf);
			*/
			
			EditPrintf(hwndEdit, TEXT("change to writing \r\n"));
			

			//clean up the buffer
			strcpy(user_information.user[user_index].output_buf, "");

			EditPrintf(hwndEdit, TEXT("cgi_writing \r\n"));
			cgi_write( hwnd, user_information, user_index);
		}
		else if (user_information.user[user_index].rev[n] == '\n' || user_information.user[user_index].rev[n - 1] == '\n')
		{
			int ss = send(user_information.parent_fd, user_information.user[user_index].output_buf, strlen(user_information.user[user_index].output_buf) , 1);
			
			/*
			EditPrintf(hwndEdit, user_information.user[user_index].output_buf);
			sprintf(user_information.user[user_index].output_buf, "%d %d\n", ss, strlen(user_information.user[user_index].output_buf));
			EditPrintf(hwndEdit, user_information.user[user_index].output_buf);
			*/

			//clean up the buffer
			strcpy(user_information.user[user_index].output_buf, "");
		}
		//clean up the buffer
		user_information.user[user_index].rev[0] = '\0';
	}
}

void send_data(SOCKET fd)
{

}
