HELL = /usr/bin/bash        #宣告command所使用的shell環境為bash             

all:
	g++ -o server -std=c++11 -Wall -g server_multi.cpp multi.cpp
	g++ -o hw4.cgi -std=c++11 -Wall -g hw4.cpp
	
cgi:
	g++ -o hw4.cgi hw4.cpp -std=c++11 -Wall -g

server: server_multi.o multi.o
	g++ -o $@ $^ -std=c++11 -Wall -g

server_multi.o: server_multi.cpp
	g++ $< -c -std=c++11 -Wall -g

multi.o: multi.cpp
	g++ $< -c -std=c++11 -Wall -g

clean:	
	@rm -rf *.o
