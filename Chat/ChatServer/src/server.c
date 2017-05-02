#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include "mythread.h"
#include "link.h"
#include "dis.h"

//读写锁
pthread_rwlock_t rwlock;  
pthread_rwlock_t rwlock2;
pthread_rwlock_t log_rwlock;  

pthread_key_t heart_key; //心跳处理key
//互斥锁
pthread_mutex_t mutex;
pthread_mutex_t info_mutex;

//共享资源
CLIENT *client_head;
COL *clientol_head;
INFO *info_head;
int heart_flag; //是否显示心跳信息开关

//信号处理函数
void sighub_handle(int signo)
{
	if(signo == SIGHUP)
	{

	}
}

//启动路径
char PRE_PATH[56] = {0};

int main(int argc,char **argv)
{

	if(signal(SIGHUP,sighub_handle) == SIG_ERR)
	{
		printf("signal failure!\n");
		exit(-1);
	}

	char buf[50] = {0};
	getcwd(buf,sizeof(buf));
	puts(buf);

	puts(argv[0]);

	if (strstr(argv[0],"bins") != NULL)
	{
		strcpy(PRE_PATH,"bins/");
	}

	int sockfd = -1;
	char ip[20] = {0}; //无效
	ushort port = 0;

	//加载服务器设置文件
	load_profile(ip,&port);
	//初始化数据链表
	usr_data_init();
	//服务器管理员登录
	server_login();
	//服务器sock初始化
	sockfd = server_sock_int(port);

	pthread_t server_thread;	
	//服务器命令操作线程
	if((pthread_create(&server_thread,NULL,cmd_thread,NULL)) != 0)
	{
		printf("pthread create error!\n");
	}

	//实现获取当前时间.
	time_t  ticks;
	struct tm *t;
	char date[32]="";
	ticks = time(NULL);
	t= localtime(&ticks);


	//读写锁互斥锁初始化
	pthread_rwlock_init(&rwlock,NULL);
	pthread_rwlock_init(&rwlock2,NULL);
	pthread_rwlock_init(&log_rwlock,NULL);  
	pthread_mutex_init(&mutex,NULL);
	pthread_mutex_init(&info_mutex,NULL);

	//分离属性设置
	int err = -1;
	pthread_t thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	/*accept接受网络连接,获得已连接套接字*/
	char logbuf[120] = {0};
	char str[20]={0};
	int lisfd = -1;
	socklen_t len = 0;
	struct sockaddr_in client;
	memset(&client,0,sizeof(client));
	while(1)
	{
		printf("等待客户连接...\n");
		if((lisfd = accept(sockfd,(struct sockaddr *)&client,&len)) < 0)
		{
			perror("accept failure!\n");
			exit(4);
		}
		strftime(date,sizeof(date),"%Y-%m-%d_%H:%M:%S",t); 
		//获取客户端相关信息
		memset(str,0,sizeof(str));
		inet_ntop(AF_INET,&client.sin_addr.s_addr,str,sizeof(str));
		printf("客户端ip(端口):%s(%d)连接成功\n",str,ntohs(client.sin_port));
		printf("客户端套接字:%d(%s)\n",lisfd,date);
		memset(logbuf,0,sizeof(logbuf));
		sprintf(logbuf,"客户端(%s)连接成功",str);
		log_write(logbuf);

		//每个客户连接到服务器产生一个子进程，并将获取的连接套接字传入到进程函数中
		if((err = pthread_create(&thread,&attr,main_thread,(void *)&lisfd)) != 0)
		{
			printf("pthread create error!\n");
		}
		pthread_attr_destroy(&attr);
			
	}
	//读写锁互斥锁销毁
	pthread_rwlock_destroy(&rwlock);
	pthread_rwlock_destroy(&rwlock2);
	pthread_rwlock_destroy(&log_rwlock); 
	pthread_mutex_destroy(&mutex);
	pthread_mutex_destroy(&info_mutex);
	return 0;
}

/*
*配置文件ip port加载函数
*入参 ip_加载ip 加载port
*成功返回0 出错返回-1
*/
 int load_profile(char *ip,int *port)
{
	assert(ip != NULL);
	
	FILE *fp=NULL;
	char prepath[128] = {0};
	strcpy(prepath,PRE_PATH);
 	if((fp=fopen(strcat(prepath,PROFILE_PATH),"rt")) == NULL)
	{
		perror("profile file open error!\n");
		exit(-1);
	}

	char buf_ip[100] = {0};
	char buf_port[100] = {0};

	fgets(buf_ip,sizeof(buf_ip),fp);
	fgets(buf_port,sizeof(buf_port),fp);

	char *b = ":";

	strtok(buf_ip,b);
	ip = strtok(NULL,b);

	strtok(buf_port,b);
	char * iport=strtok(NULL,b);

	ip = strtok(ip,"\"");	

	*port = atoi(iport);
	printf("\033[2J""\033[1;1H""配置文件加载中....\n");
	printf("ip加载成功%s",ip);
	printf("port加载成功%d\n",*port);

	fclose(fp);

	return 0;
}

/*
*用户数据初始化
*成功返回0 出错返回-1
*/
int usr_data_init()
{
	//创建用户头节点并加载用户链表
	client_head = (CLIENT *)calloc(1,sizeof(CLIENT));
	if(NULL == client_head)
	{
		perror("client head calloc error!\n");
		return -1;
	}
	client_head->next = NULL;
	if(load_client(client_head) == 0)
		printf("用户链表加载成功...\n");

	//创建在线用户头节点
	clientol_head = (COL *)calloc(1,sizeof(COL));
	if(NULL == clientol_head)
	{
		perror("client online head calloc error!\n");
		return -1;
	}
	clientol_head->next = NULL;

	//创建会话连接头节点
	info_head = (INFO *)calloc(1,sizeof(INFO));
	if(NULL == info_head)
	{
		perror("info head calloc error!\n");
		return -1;
	}
	info_head->next = NULL;

	return 0;
}

/*
*服务器登陆函数
*成功返回0,失败返回-1
*/
int server_login()
{
	char name[20];
	char pwd[20];
	int i = 0;
	//load_admin(name,pwd);
	printf("管理员信息加载成功...\n");
	printf(RED"\n\t\t\t\t★★★★★麒麟云系统★★★★★\n"NONE);
	printf("  \t\t\t\t★★★★★ ver1.0.0 ★★★★★\n\n\n");

	do
	{
		memset(name,0,sizeof(name));
		memset(pwd,0,sizeof(pwd));
		printf("请输入管理员名字:");
		fgets(name,sizeof(name),stdin);
		name[strlen(name)-1]='\0';
		printf("请输入密码:");
		new_getpass(pwd,20);

		if((strcmp(name,"admin") == 0) && (strcmp(pwd,"123456") == 0))
		{
			printf("登陆成功!\n");
			return 0;
		}
		else
			printf(RED"帐号或密码错误,请重新输入!\n"NONE);

		i++;
		if(i == 3)
		{
			printf(RED"帐号密码错误三次,系统关闭\n"NONE);
			exit(-1);
		}
		
	}while(1);

	return 0;	
}

/*
*服务器socket初始化
*返回服务器套接字
*/
int  server_sock_int(int port)
{
	/*socket创建监听套接字*/
	int sockfd = -1;
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0)
	{
		perror("socket failure!\n");
		exit(1);
	}

	/*设置套接字选项*/
	int opt = 1;
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	/*bing进行ip和端口绑定*/

	struct sockaddr_in server;
	memset(&server,0,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = INADDR_ANY;
	//inet_pton(AF_INET,"127.0.0.1",&ser.sin_addr.s_addr);
	
	if(bind(sockfd,(struct sockaddr *)&server,sizeof(server)) != 0)
	{
		perror("bind failure!\n");
		exit(2);
	}

	/*listen进行监听*/
	if(listen(sockfd,10) != 0)
	{
		perror("listen failure!\n");
		exit(3);
	}
	return sockfd;
}

/*
*参数dest是目标字符串, maxlen是最大长度，
*如果输入超过了最大长度，则密码将会被截断
*成功返回0，否则返回－1
*/
int new_getpass(char *dest, int maxlen)
{	
	assert(dest != NULL);

	struct termios oldflags, newflags;
	int len;

	//设置终端为不回显模式
	tcgetattr(fileno(stdin), &oldflags);
	newflags = oldflags;
	newflags.c_lflag &= ~ECHO;
	newflags.c_lflag |= ECHONL;
	if (tcsetattr(fileno(stdin), TCSANOW, &newflags) != 0)
	{
		perror("tcsetattr");
		return -1;
	}

	//获取来自键盘的输入
	fgets(dest, maxlen, stdin);
	len = strlen(dest);
	if( len > maxlen-1 )
	len = maxlen - 1;
	dest[len-1] = 0;

	//恢复原来的终端设置
	if (tcsetattr(fileno(stdin), TCSANOW, &oldflags) != 0)
	{
		perror("tcsetattr");
		return -1;
	}
	return 0;
}

