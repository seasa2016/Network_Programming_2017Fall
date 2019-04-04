#pragma once
#include <windows.h>

int hw3(HWND hwnd, struct reply&);
void handle_socket(HWND hwnd, SOCKET fd, struct reply&);
int readline(HANDLE  fd, char *ptr, int maxlen);
void connect_user(HWND hwnd , struct reply & , int i);
void start_html( HWND , struct reply &);
void cgi_read(HWND, struct reply &,int);
void cgi_write(HWND, struct reply &, int);
void send_data(SOCKET,);
