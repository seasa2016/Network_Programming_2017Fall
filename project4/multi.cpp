#include"multi.h"

void handler(int sockfd)
{
	unsigned char buffer[512];
    int n;
    printf("a connect\n");
    n = read(sockfd, buffer, 512);
    buffer[n]=0;
    
    unsigned char VN = buffer[0] ;
    unsigned char CD = buffer[1] ;
    unsigned short DST_PORT = (unsigned int)buffer[2] << 8 | (unsigned int)buffer[3] ;
    unsigned int DST_IP = ((unsigned int)buffer[7] << 24) | ((unsigned int)buffer[6] << 16) | ((unsigned int)buffer[5] << 8) | ((unsigned int)buffer[4]);
    char dst_ip[64];
    char* USER_ID = (char*)buffer + 8 ;
    
    sprintf(dst_ip,"%u.%u.%u.%u",buffer[4],buffer[5],buffer[6],buffer[7]);

    if( VN != 0x4 )   //only accept VN==4
        exit(1);
    /*if( DST_IP < 256)
    {
        printf("receive domain name message\n");
        n = read(sockfd, buffer, 8);
    }*/
    //check firewall
    FILE *firewall_fd;
    char pre[36];
    struct fire firewall_check;
    char *ptr;
    bool firewall_checker = false;

    if ((firewall_fd = fopen("socks.conf", "r")) == NULL)
    {
        perror("Cannot open output file\n"); 
        return ;
    }
    
    while(fscanf(firewall_fd," %s %c %s",pre,&firewall_check.type,firewall_check.id))
    {
    
        ptr = strchr(firewall_check.id,'*');
        *ptr = 0;
        if( CD == 0x01 && firewall_check.type == 'c' )     //connect mode
        {
            //printf("-%s-\n",firewall_check.id);
            if(strncmp(dst_ip, firewall_check.id, strlen(firewall_check.id)) == 0)
            {
                firewall_checker=true;
                break;
            }    
        }   
        else if( CD == 0x02 && firewall_check.type == 'b')
        {
            if(strncmp(dst_ip, firewall_check.id, strlen(firewall_check.id)) == 0)
            {
                firewall_checker=true;
                break;
            }    
        } 
        if(feof(firewall_fd))
            break;
    }

    //**********************//
    buffer[0] = 0;
    buffer[1] = 90;
    
    printf("\n\n---------------------------------\n");
    printf("<S_IP>\t:%s\n",getenv("REMOTE_ADDR"));
    printf("<S_PORT>\t:%s\n",getenv("REMOTE_PORT"));
    printf("<D_IP>\t:%u.%u.%u.%u\n",buffer[4],buffer[5],buffer[6],buffer[7]);
    printf("<D_PORT>\t:%u\n",DST_PORT);
    
    if(firewall_checker == false)   //deny the connect
    {
        buffer[1] = 91;
        if( CD == 0x01)  printf("<Command>\t:connect\n");
        else if( CD == 0x02)  printf("<Command>\t:bind\n");
        else printf("why this mode\n");
        printf("<Reply>\t:Reject by firewall\n");
        write(sockfd, buffer, 8);
    }
    else if( CD == 0x01 )     //connect mode
    {
        printf("<Command>\t:connect\n");
        connecting(sockfd, buffer, VN, CD, DST_PORT, DST_IP );
    }
    else if( CD == 0x02 )    //bind mode
    {
        printf("<Command>\t:bind\n");
        binding(sockfd, buffer, VN, CD, DST_PORT, DST_IP );
    }
    else
    {
        printf("why go here\n");
    }
}
void connecting(int sockfd, unsigned char *buffer, unsigned char VN, unsigned char CD, unsigned short DST_PORT, unsigned int DST_IP )
{
    struct sockaddr_in server_sin;
	int server_fd, nfds=0, n;
    fd_set rfds;/* readable file descriptoclient.rs*/
    fd_set rs;  /* active file descriptoclient.rs*/
    unsigned char text[256];
    bool key;

    bzero(&server_sin,sizeof(server_sin));
	server_sin.sin_family = AF_INET;
	server_sin.sin_addr.s_addr = DST_IP; 
	server_sin.sin_port = htons(DST_PORT);

	server_fd = socket(AF_INET,SOCK_STREAM,0);
	if(connect(server_fd,(struct sockaddr *)&server_sin,sizeof(server_sin)) < 0)
	{
        buffer[1] = 91;
        
        printf("<Reply>\t:Reject no this ip server\n");
        write(sockfd, buffer, 8);
        return ;
	} 
    //reply connect success
    printf("<Reply>\t:Accept\n");
    write(sockfd, buffer, 8);


	FD_ZERO(&rs);
	FD_SET(server_fd, &rs);
	FD_SET(sockfd, &rs); 
    nfds= ( server_fd>sockfd ? server_fd : sockfd ) + 1;
    
    while(1)
	{ 
        memcpy(&rfds, &rs, sizeof( rfds ) ); 
        key=false;

        if ( select(nfds, &rfds, (fd_set*)NULL, (fd_set*)NULL, NULL) < 0 ) 
        {
			printf("%d %s\n",errno,strerror(errno));			
			exit(1);
		}

        for(int fd=0 ; fd < nfds ; fd++)
		{
            if(FD_ISSET( fd, &rfds) ) //read went server or browser
			{
                n = read(fd,text,256);
                if(n == 0)
                {
                    if(fd == server_fd)
                        printf("dissconnet by server\n");
                    else if(fd == sockfd)
                        printf("dissconnet by client\n");
                    key=true;
                    break;
                }
                else if(fd == server_fd)
                    write(sockfd, text, n);
                else if(fd == sockfd)
                    write(server_fd, text, n);
			}
		}
        if(key)
            break;
	}
}
void binding(int sockfd, unsigned char *buffer, unsigned char VN, unsigned char CD, unsigned short DST_PORT, unsigned int DST_IP )
{
	struct sockaddr_in serv_addr,cli_addr;
	int port,ftp_fd,client_fd,n;
	unsigned char mess[8];

    if((ftp_fd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		fprintf(stderr,"server:can't open stream socket\n");
		exit(-1);
	}	

    bzero((char*)&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family	=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    do
    {
        port = rand() % 65536;
        serv_addr.sin_port	=htons(port);
	}while((bind(ftp_fd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0));
	
    mess[0] = 0;
    mess[1] = 90;
    //port
    mess[2] = port / 256;
    mess[3] = port % 256;
    //dest ip
    mess[4] = 0;
    mess[5] = 0;
    mess[6] = 0;
    mess[7] = 0;
    
    if(listen(ftp_fd,0) <0) //fail to connect
    {
        mess[1] = 91;
        
        printf("<Reply>\t:Reject listen fail\n");
        write(sockfd, mess, 8);
        return ;
    }

    write(sockfd, mess, 8);
	n = sizeof(cli_addr);
	client_fd = accept( ftp_fd , (struct sockaddr *)&cli_addr, (socklen_t *)&n);
	
	if(client_fd < 0)
	{
        buffer[1] = 91;
        
        printf("<Reply>\t:Reject accept fail\n");
        write(sockfd, buffer, 8);
        return ;
	}
    
	write(sockfd, buffer , 8);

    printf("<Reply>\t:Accept\n");

    int nfds=0;
    fd_set rfds;/* readable file descriptoclient.rs*/
	fd_set rs;  /* active file descriptoclient.rs*/
	bool key;
	unsigned char text[256];

	FD_ZERO(&rs);
	FD_SET(client_fd, &rs);
	FD_SET(sockfd, &rs); 
	nfds= ( client_fd > sockfd ? client_fd : sockfd ) + 1;

	printf("nfd:%d\n",nfds);
    while(1)
	{ 
        memcpy(&rfds, &rs, sizeof( rfds ) ); 
        key=false;

        if ( select(nfds, &rfds, (fd_set*)NULL, (fd_set*)NULL, NULL) < 0 ) 
        {
			printf("%d %s\n",errno,strerror(errno));			
			exit(1);
		}

        for(int fd=0 ; fd < nfds ; fd++)
		{
            if(FD_ISSET( fd, &rfds) ) //read went server or browser
			{
                n = read(fd,text,256);
                if(n == 0)
                {
                    if(fd == client_fd)
                        printf("dissconnet by ftp\n");
                    else if(fd == sockfd)
                        printf("dissconnet by client\n");
                    key=true;
                    break;
                }
                else if(fd == client_fd)
                    write( sockfd, text, n );
                else if(fd == sockfd)
                    write( client_fd, text, n );
			}
		}
        if(key)
            break;
	}
}
