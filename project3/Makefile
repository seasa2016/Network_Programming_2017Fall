HELL = /usr/bin/bash        #宣告command所使用的shell環境為bash             

all:
	g++ -o server -std=c++11 -Wall -g http_server.cpp
	g++ -o hw3.cgi -std=c++11 -Wall -g hw3.cpp
server:
	g++ -o server -std=c++11 -Wall -g http_server.cpp
cgi:
	g++ -o hw3.cgi hw3.cpp -std=c++11 -Wall -g

clean:	
	@rm -rf *.o
