HELL = /usr/bin/bash		#宣告command所使用的shell環境為bash

all:
	g++ -o server -std=c++11 -Wall -g server.cpp np1_service.h np1_service.cpp 

server: server.o np1_service.o
	g++ -o $@ $^ -std=c++11 -Wall -g

np1_service.o: np1_service.cpp
	g++ $< -c -std=c++11 -Wall -g

server.o: server.cpp
	g++ $< -c -std=c++11 -Wall -g

clean:
	@rm -rf *.o
