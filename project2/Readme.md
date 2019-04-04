Project #2: remote working ground (rwg)

In this homework, you are asked to design chat-like systems, called
remote working systems (server only).  In this system, users can
meet with/work with/talk to/make friends.  Basically, this system 
supports all functions, as you did in the first project.  In addition, 
all clients can see what all on-line users are working.  

Command Summary: 
* All commands in project 1.  

* who: show all the online users. E.g., 
  <sockd> <nickname>      <IP/port>                    <indicate me>
  3       Iam3            140.113.215.62/1201          <- me
  4       (No Name)       140.113.215.63/1013
  5       student5        140.113.215.64/1302
  
* tell <sockd> <message>: send only to <user> the message <message>. E.g.,  
        % tell 5 Hello.           # from client #3 (Iam3)
  The user "student5" will receive the following. 
        *** Iam3 told you ***:  Hello.
        
* yell <message>: broadcast the message <message> to all users. E.g., 
        % yell Hi...
  All users will receive the following. 
        *** student1 yelled ***:  Hi...
        
* name <name>: set my name to <name> and broacast to all users. E.g., 
        % name student5
  All users will receive the following. 
        *** User from 140.113.215.62/1201 is named 'student5'. ***

* Whenever a client comes in, broadcast as follows. 
        *** User '(no name)' entered from 140.113.215.63/1013. ***
  Whenever a client leaves, broadcast as follows. 
        *** User 'student5' left. ***
        
* student7 (#7) pipes a command into student3 (#3) via a pipe #7->#3.
        % cat test.html >3
  All clients see the following message.
        *** student7 (#7) just piped 'cat test.html >3' to student3 (#3) *** 
  If the pipe #7->#3 exists already, show the following error message. 
        *** Error: the pipe #7->#3 already exists. *** 
  The client student3 can use the following to receive from the pipe #7->#3.
        % cat <7                         
  All clients see the following message.
        *** student3 (#3) just received from student7 (#7) by 'cat <7' *** 
  If the pipe #7->#3 does not exists, show the following error message. 
        *** Error: the pipe #7->#3 does not exist yet. *** 

The following is a scenario of using the system.

csh> telnet myserver.nctu.edu.tw 7001 # the server port number 
**************************************************************
** Welcome to the information server, myserver.nctu.edu.tw. **
**************************************************************
** You are in the directory, /home/studentA/rwg/.
** This directory will be under "/", in this system.  
** This directory includes the following executable programs. 
** 
**      bin/
**      test.html       (test file)
**
** The directory bin/ includes: 
**      cat
**      ls
**      removetag               (this is a simple filtering-sgml program, you write.)
** 
% # All commands in Project 1 must still work!!!
% who                           # All IDs must be distinct between 1 ~ 30.
3       (no name)       140.113.215.62/1201          <- me
% name �����`
*** User from 140.113.215.62/1201 is named '�����`'. ***
% ls
bin/            test.html   test1.txt   test2.txt
*** User '(no name)' entered from 140.113.215.63/1013. ***
% who 
3       �����`          140.113.215.62/1201          <- me
4       (no name)       140.113.215.63/1013
*** User from 140.113.215.63/1013 is named '�毫'. ***
% who 
3       �����`          140.113.215.62/1201          <- me
4       �毫            140.113.215.63/1013
*** User '(no name)' entered from 140.113.215.64/1302. ***
% who
3       �����`          140.113.215.62/1201          <- me
4       �毫            140.113.215.63/1013
5       (no name)       140.113.215.64/1302
% yell �H�Ѩ�Project #2��򰵡H�бЧڡI
*** (no name) yelled ***:  ��աA�ڤ]�B���_�C  :-(
*** �毫 yelled ***:  �ڽ޵s�I�V�e�����աI
% tell 4 �����I�گ��{�u�A�Чi�D�ڰաI
*** �毫 told you ***:  �n���I�J�Ӭݰ��p����ɮװe���A�a�I
*** �毫 (#4) just piped 'cat test.html >3' to �����` (#3) *** 
*** �毫 told you ***:  ���A�i�H��'cat <4'��L���q�X�ӬݡI
% cat <5     #���p�ߥ���
*** Error: the pipe #5->#3 does not exist yet. *** 
% cat <4
<!test.html>
<TITLE>Test<TITLE>
<BODY>This is a <b>test</b> program
for rwg.
</BODY>
*** �����` (#3) just received from �毫 (#4) by 'cat <4' *** 
% tell 4 ���I�]���C�I�n�γ�I
*** �毫 told you ***:  �����٥i�H��{�����G�e���A��I
*** �毫 (#4) just piped 'removetag0 test.html >3' to �����` (#3) ***  
*** �毫 told you ***:  ���A�٥i�H�ε{������I������'number <4'
% number <4
  1 Error: illegal tag "!test.html"
  2 
  3 Test 
  4 This is a test program
  5 for ras.
  6 
*** �����` (#3) just received from �毫 (#4) by 'number <4' *** 
% tell 4 ���I�A�n����I���ߡI
*** �毫 told you ***:  ���Ȯ�I�����ӴN�O�㯫��
*** User '�毫' left. ***
% exit
csh> 



Now, let us see what happened to the second user.  


csh> telnet myserver.nctu.edu.tw 7001 # the server port number 
**************************************************************
** Welcome to the information server, myserver.nctu.edu.tw. **
**************************************************************
** You are in the directory, /home/studentA/rwg/.
** This directory will be under "/", in this system.  
** This directory includes the following executable programs. 
** 
**      bin/
**      test.html       (test file)
**
** The directory bin/ includes: 
**      cat
**      ls
**      removetag               (this is a simple filtering-sgml program, you write.)
** 
% # All commands in Project 1 must still work!!!
% name �毫
*** User from 140.113.215.63/1013 is named '�毫'. ***
*** User '(no name)' entered from 140.113.215.64/1302. ***
% who
3       �����`          140.113.215.62/1201
4       �毫            140.113.215.63/1013          <- me
5       (no name)       140.113.215.64/1302
*** �����` yelled �H�Ѩ�Project #2��򰵡H�бЧڡI
*** (no name) yelled ***:  ��աA�ڤ]�B���_�C  :-(
% yell �ڽ޵s�I�V�e�����աI
*** �����` told you ***:  �����I�گ��{�u�A�Чi�D�ڰաI
% tell 3 �n���I�J�Ӭݰ��p����ɮװe���A�a�I
% cat test.html >3
*** �毫 (#4) just piped 'cat test.html >3' to �����` (#3) *** 
% tell 3 ���A�i�H��'cat <4'��L���q�X�ӬݡI
*** �����` (#3) just received from �毫 (#4) by 'cat <4' *** 
*** �����` told you ***:  ���I�]���C�I�n�γ�I
% tell 3 �����٥i�H��{�����G�e���A��I
% removetag0 test.html >3
*** �毫 (#4) just piped 'removetag0 test.html >3' to �����` (#3) ***  
% tell 3 ���A�٥i�H�ε{������I������'number <4'
*** �����` (#3) just received from �毫 (#4) by 'number <4' *** 
*** �����` told you ***:  ���I�A�n����I���ߡI
% tell 3 ���Ȯ�I�����ӴN�O�㯫��
% exit
csh> 



Requirements:

* Write two different server programs. 
  (1) Use the single-process concurrent paradigm.
  (2) Use the concurrent connection-oriented paradigm with shared memory.  

* For each client, all behaviors required by project 1 are still required
  in this project.  

Hints: 

* You can assume that the # of users is <= 30. 

* ">3" or "<4" has no space between them.  So, you can distinct them 
  from "> filename" easily.  

* For the second program (2), 
  * One chat buffer has at most 10 unread messages, each of which has 
    at most 1024 bytes. 
  * For each pair of clients (A, B), one chat buffer is allocated for A 
    to send messages to B. If the chat buffer is full (10 messages), 
    A needs to wait for B to consume messages.  
  * For each pipe for ">3", use FIFO instead of pipe. 
  * If a message has more than 1024 bytes, simply truncate it to 1024 bytes. 
  * If you use BBS-like method, you can use signal "SIGUSR1" or "SIGUSR2" to help.  
  * For "who", the master process maintain an id for each forked process. 
    Since there are no more than 30 processes, id <= 30 and let the id be <sockd>. 

If you find some commands confusing or not workable, please let us know. 

Due date: 11/19 (Monday)


