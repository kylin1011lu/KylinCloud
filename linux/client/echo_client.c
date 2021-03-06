#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024 
void error_handling(char *message);

int main(int argc,char*argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	char message[BUF_SIZE];
	int str_len;

	int idx = 0,read_len = 0;

	if(argc != 3)
	{
		printf("Usage:%s <IP> <Port>\n",argv[0]);
		exit(1);
	}

	sock = socket(PF_INET,SOCK_STREAM,0);
	if(sock == -1)
		error_handling("sock() error");

	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) == -1)
		error_handling("connect() error!");

	//str_len = read(sock,message,sizeof(message)-1);
	//if(str_len == -1)
	//	error_handling("read() error !");

	//while(read_len = read(sock,&message[idx++],1))
	//{
	//	if(read_len == -1)
	//		error_handling("read() error");
	//	str_len+=read_len;
	//}

	//printf("Message form server:%s\n",message);
	//printf("Function read call count:%d\n",str_len);



	while(1)
	{
		fputs("Input message(Q to quit):",stdout);
		fgets(message,BUF_SIZE,stdin);

		if(!strcmp(message,"q\n") || !strcmp(message,"Q\n"))
			break;

		str_len = write(sock,message,strlen(message));
		int rec_len = 0;
		int rec_cnt = 0;
		while(rec_len < str_len)
		{
			rec_cnt = read(sock,message + rec_len,BUF_SIZE-1);
			if(rec_cnt == -1)
				error_handling("read() error");
			rec_len += rec_cnt;
			
		}
		message[str_len]=0;
		printf("Message from server:%s",message);
	}
	close(sock);
	return 0;
}



void error_handling(char *message)
{
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}
