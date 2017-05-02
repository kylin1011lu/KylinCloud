#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <setjmp.h>
#include "cfun.h"
#include "dis.h"



int sockfd = -1;
pthread_t thread;
int fd[2];
//int reconnect_flag = 0;//与服务断开后重连开关
int alarmflag = 0; //定时器开关 用户一旦登录成功开始发送心跳 下线后不再发送心跳
//jmp_buf jmpbuffer;



void sig_handle(int signo)
{
	if(signo == SIGINT || signo == SIGTSTP)
	{
		mywrite(TYPE_EXITERR," ",","," ",sockfd);
		close(sockfd);
		exit(-1);
	}
	if(signo == SIGALRM)
	{
		char buf[64];
		strcpy(buf,"It is heart");
		mywrite(TYPE_HEART," "," ",buf,sockfd);
		if(alarmflag == 1)
			alarm(3);
	}
}


int main(int argc,char **argv)
{
	printf("\033[2J""\033[1;1H");
	printf(RED"\n\t\t\t    ★★★★★海同云系统★★★★★\n"NONE);
	printf("  \t\t\t    ★★★★★ "VERSION" ★★★★★\n\n\n");

	/*注册异常下线信号*/
	if(signal(SIGINT,sig_handle) == SIG_ERR)
	{
		perror("signal failure!\n");
		exit(-1);
	}
	if(signal(SIGTSTP,sig_handle) == SIG_ERR)
	{
		perror("signal failure!\n");
		exit(-1);
	}

	//心跳包
	if(signal(SIGALRM,sig_handle) == SIG_ERR)
	{
		perror("sigalrm failure!\n");
		exit(-1);
	}


	//int i = 0;
 	//i = setjmp(jmpbuffer);
	//if(i == 0)
	//{
	/*socket创建监听套接字*/

	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0)
	{
		perror("socket failure!\n");
		exit(1);
	}
	/*connect进行服务器连接*/

	struct sockaddr_in server;
	memset(&server,0,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons((short)3000);
	inet_pton(AF_INET,"127.0.0.1",&server.sin_addr.s_addr);
	if(connect(sockfd,(struct sockaddr *)&server,sizeof(server)) != 0)
	{
		perror("connect failure!\n");
		exit(2);
	}


	//主线程与子线程通信管道
	if(pipe(fd)<0)
	{
		perror("pipe error!\n");
	}


	int err = -1;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	if((err = pthread_create(&thread,&attr,threadread,(void *)sockfd)) != 0)
	{
		printf("pthread create error!\n");
	}
	pthread_attr_destroy(&attr);

	do_service(sockfd);

	close(sockfd);
	return 0;
}
