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
#include <netdb.h>
#include <assert.h>
#include "cfun.h"
#include "dis.h"

int sockfd = -1;
pthread_t thread;
int fd[2];
//int reconnect_flag = 0;//与服务断开后重连开关
int alarmflag = 0; //定时器开关 用户一旦登录成功开始发送心跳 下线后不再发送心跳
//jmp_buf jmpbuffer;

#define PROFILE_PATH 		"profile"
//启动路径
char PRE_PATH[56] = {0};

void sig_handle(int signo);
void signal_register();
 int load_profile(char *ip,ushort *port);

int main(int argc,char **argv)
{
	printf("\033[2J""\033[1;1H");
	printf(RED"\n\t\t\t    ★★★★★麒麟云系统★★★★★\n"NONE);
	printf("  \t\t\t    ★★★★★ "VERSION" ★★★★★\n\n\n");

	char ip[56] = {0}; //服务器域名
	ushort port = 0;

	if (strstr(argv[0],"bin") != NULL)
	{
		strcpy(PRE_PATH,"bin/");
	}

	//信号注册
	signal_register();
	//加载服务器设置文件
	load_profile(ip,&port);

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
	server.sin_port = htons(port);
	inet_pton(AF_INET,ip,&server.sin_addr.s_addr);
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

void signal_register()
{
	/*注册异常下线信号*/
	//SIGINT:由Interrupt Key产生，通常是CTRL+C或者DELETE。发送给所有ForeGround Group的进程
	if(signal(SIGINT,sig_handle) == SIG_ERR)
	{
		perror("signal failure!\n");
		exit(-1);
	}
	//SIGTSTP:中止进程。无法处理和忽略。
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
}

//信号处理函数
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

/*
*配置文件ip port加载函数
*入参 ip_加载ip 加载port
*成功返回0 出错返回-1
*/
 int load_profile(char *ip,ushort *port)
{
	assert(ip != NULL);
	
	FILE *fp=NULL;
	char prepath[128] = {0};
	strcpy(prepath,PRE_PATH);
 	if((fp=fopen(strcat(prepath,PROFILE_PATH),"rt")) == NULL)
	{
		perror("profile file open error!\n");
		return -1;
	}

	char buf_domain[100] = {0};
	char buf_port[100] = {0};

	char *domain = NULL;

	fgets(buf_domain,sizeof(buf_domain),fp);
	fgets(buf_port,sizeof(buf_port),fp);

	int i = 2;
	char *b = " ";
	domain = strtok(buf_domain,b);
	while(i--)
	{
		domain=strtok(NULL,b);
	}
	char *iport = strtok(buf_port,b);
	i = 2;
	while(i--)
	{
		iport=strtok(NULL,b);
	}
	puts(domain);
	struct hostent *host;
	host = gethostbyname(domain);
	if(!host)
	{
		perror("domain parse error!\n");
		return -1;
	}

	// printf("Official name:%s\n",host->h_name);
	// for(i=0;host->h_aliases[i];i++)
	// 	printf("Aliases %d:%s\n",i+1,host->h_aliases[i]);

	// printf("Address type:%s\n",(host->h_addrtype==AF_INET)?"AF_INET":"AF_INET6");
	// for(i=0;host->h_addr_list[i];i++)
	// 	printf("IP addr%d:%s\n",i+1,inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));

	if (host->h_addr_list == NULL)
	{
		perror("ip parse error!\n");
		return -1;
	}

	strcpy(ip,inet_ntoa(*(struct in_addr*)host->h_addr_list[0]));

	*port = (ushort)atoi(iport);
	printf("\033[2J""\033[1;1H""服务器配置文件加载中....\n");
	printf("domain加载成功%s",domain);
	printf("ip加载成功%s\n", ip);
	printf("port加载成功%d\n",*port);

	fclose(fp);

	return 0;
}
