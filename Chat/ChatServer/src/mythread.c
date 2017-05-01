#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <assert.h>
#include <sys/wait.h>
#include <setjmp.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "mythread.h"
#include "link.h"
#include "dis.h"

extern pthread_rwlock_t rwlock;
extern CLIENT *client_head;
extern COL *clientol_head;
extern INFO *info_head;
extern pthread_key_t heart_key;
extern int heart_flag;


void sig_handle(int signo)
{
	if(SIGUSR1 == signo)
	{
		printf("a client thread exit!\n");
		pthread_exit(NULL);
	}
}

/*
*功能:心跳异常处理线程函数
*入参:父线程的ID
*返回无
*/

void *heart(void *argc)
{
	TN *tn = (TN *)argc;
	//pthread_key_create(&heart_key,NULL);//创建key
	//printf("%p\n",&y);
	//将Y的值设置在key中
	//pthread_setspecific(heart_key,(void*)&tn->i);

	//取出key中的变量地址
	//int *i = (int*)pthread_getspecific(heart_key);

	int i = 0;
	long flag;
	ssize_t size = -2;
	flag = fcntl(tn->pipefd,F_GETFL,0);
	flag = flag | O_NONBLOCK;
	fcntl(tn->pipefd,F_SETFL,flag);
	while(1)
	{
		if((size = read(tn->pipefd,&i,sizeof(int))) != -1)
			i = 0;
		
		if(i == 20)
		{
			//向父线程发送信号
			pthread_kill(tn->thread,SIGUSR1);
			close(tn->lisfd);
			//结束线程自己
			pthread_exit(NULL);
		}
		else
		{
			sleep(1);
			//printf("%d\n",i);
			i++;
		}
	}

	return (void *)0;
}


/*
*功能:客户端对应子线程的运行函数，提供功能选项
*入参：客户端对应的已连接套接字 返回值0
*/
void *main_thread(void *argc)
{
	if(signal(SIGUSR1,sig_handle) == SIG_ERR)
	{
		perror("signal failure!\n");
		exit(-1);
	}

	int lisfd =*((int*)argc);

	MSG msg;
	int ret = 0;
	char name[20] ={0};

	char ip[20] = {0};
	getip(lisfd,ip);

	int fd[2] = {0};
	if(pipe(fd) < 0)
	{
		perror("pipe error!\n");
	}	

	ret = update_check(lisfd);
	
	if(ret == -1)
		mywrite(TYPE_WEL," "," "," ",lisfd);

	pthread_t dad_thread = pthread_self(); //获取当前线程ID传入子线程中
	TN th_na;
	memset(&th_na,0,sizeof(TN));
	th_na.thread = dad_thread;
	th_na.lisfd = lisfd;
	th_na.pipefd = fd[0];
	//心跳异常处理线程
	int err = -1;
	pthread_t heartthread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	if((err = pthread_create(&heartthread,&attr,heart,(void *)&th_na)) != 0)
	{
		printf("pthread create error!\n");
	}
	pthread_attr_destroy(&attr);
	
	do
	{
		memset(&msg,0,sizeof(msg));
		myread(&msg,lisfd);
		//非心跳包写入日志
		if(msg.msg_type != TYPE_HEART)
			log_dis(msg,lisfd,name);
		//printf("msgtype>>%d\n",msg.msg_type);
		switch(msg.msg_type)
		{
			case TYPE_LOGIN://处理登入业务
				ret = client_login(lisfd,msg,client_head);
				if(ret == 0)
					strcpy(name,msg.name);
				break;
			case TYPE_REG://处理注册业务
				client_reg(lisfd,msg,client_head);
				save_client(client_head);
				break;
			case TYPE_CMD://shell命令处理
				client_shell(lisfd,msg,name);
				break;
			case TYPE_HEART://心跳
				client_heart(lisfd,msg,heartthread,name,fd[1]);
				break;
			case TYPE_CHECK://在线人数
				client_check(lisfd,clientol_head);
				break;
			case TYPE_CHAT://聊天
				client_chat(lisfd,clientol_head,msg);
				break;
			case TYPE_DOWNLOAD: //云端下载
				cloud_download(lisfd,msg.buf,name);
				break;
			case TYPE_UPLOAD: //云端下载
				cloud_upload(lisfd,msg.buf,name);
				break;
			case TYPE_USREXIT:
				printf("%s用户(%s)下线了\n",msg.name,ip);
				clientol_link_delete(clientol_head,name); //下线后在线链表删除此用户
				connect_info_after(info_head,name,lisfd,1); 	
				break;
			case 0:  //客户端意外断线，read返回0
			case TYPE_EXITERR: //客户端异常退出，Ctrl+c Ctrl+z
				connect_info_after(info_head,name,lisfd,2); 
				close(lisfd);
				printf("客户端(%s)异常退出\n",ip);
				clientol_link_delete(clientol_head,name);
				pthread_cancel(heartthread);	
				pthread_exit(NULL);				
			case TYPE_EXITOK://正常exit退出
				close(lisfd);
				//save_client(client_head);				
				printf("客户端(%s)正常退出\n",ip);
				pthread_cancel(heartthread);	
				pthread_exit(NULL);	
			default:
				printf("数据包类型无法辨别！\n");
				break;
		}
	}while(1);


	pthread_exit(NULL);
}

/*
*从本地上传文件到云端函数
*lisfd_已连接套接字 filename_文件名指针 foldername *用户云端文件夹
*成功返回0 出错返回-1
*/
int cloud_upload(int lisfd,char *filename,char *foldername)
{
	assert(lisfd >= 0);
	assert(filename != NULL);
	assert(foldername != NULL);

	//生成服务器用的文件名 含路径
	char true_filename[50] = {0};
	sprintf(true_filename,"./%s/%s",foldername,filename);

	if(file_rev(lisfd,true_filename) < 0)
	{
		printf("%s文件从%s客户端上传失败!\n",filename,foldername);
		return -1;
	}

	printf("%s上传%s文件到云端!\n",foldername,filename);
	return 0;
	
}

/*
*从云端下载文件函数
*lisfd_已连接套接字 filename_文件名指针
*成功返回0 出错返回-1
*/
int cloud_download(int lisfd,char *filename,char *foldername)
{
	assert(lisfd >= 0);
	assert(filename != NULL);
	assert(foldername != NULL);

	//生成真正的完整的文件名
	char true_filename[50] = {0};
	sprintf(true_filename,"./%s/%s",foldername,filename);

	printf("%s\n",true_filename);
	
	//检测文件是否存在
	int ret = access(true_filename,F_OK | R_OK);
	if(ret < 0)
	{
		mywrite(TYPE_ERR," "," ","文件不存在！",lisfd);
		return -1;
	}

	if(file_send(lisfd,true_filename,filename,TYPE_DOWNLOAD) == 0)
		printf("%s从云端成功下载文件%s!\n",foldername,filename);
	else
	{
		printf("文件下载失败！\n");
		return -1;
	}
	
	return 0;
}

/*
*检测更新文件是否存在，存在则发送更新
*入参无
*成功返回0 出错返回-1
*/
int update_check(int lisfd)
{
	assert(lisfd >= 0);
	
	char filename[50] = FILENAME;
	MSG msg;
	memset(&msg,0,sizeof(MSG));

	//检测更新文件是否存在
	int ret = access("client_update.tar",F_OK | R_OK);

	//printf("ret>>%d\n",ret);
	if(ret == -1)
		return -1;

	//发送更新提示
	ssize_t size = mywrite(TYPE_UPDATE," "," ","ver 1.0.1",lisfd);
	printf("size>>%d\n",(int)size);

	myread(&msg,lisfd);

	if(msg.msg_type == TYPE_ERR)
		return -2;
	
	if(msg.msg_type == TYPE_OK)
	{
		if(file_send(lisfd,filename,filename,TYPE_UPDATE) == 0)
			printf("更新包发送成功!\n");
		else
			printf("更新包发送失败！\n");
	}

	return 0;	
}

/*
*心跳处理函数
*入参 lisfd_已连接套接字
*出错返回-1
*/
int client_heart(int lisfd,MSG msg,pthread_t heartthread,char *name,int fd)
{
	assert(lisfd >= 0);

	char ip[20] = {0};
	getip(lisfd,ip);
	
	if(heart_flag == 1)
		printf("%s from %s(%s)\n",msg.buf,name,ip);
	int i =0;
	write(fd,&i,sizeof(int));
	//pthread_kill(heartthread,SIGUSR2);

	return 0;
}

/*
*功能:聊天处理函数
*入参 lisfd_已连接套接字 clientol_head_在线用户链表头，msg_聊天信息包
*成功返回 0，出错返回-1
*/
int client_chat(int lisfd,COL *clientol_head,MSG msg)
{
	assert(lisfd >= 0);
	assert(clientol_head != NULL);

	printf("%s to %s:%s\n",msg.name,msg.pwd,msg.buf);

	clientol_head = clientol_head->next;
	while(clientol_head != NULL)
	{
		if(strcmp(clientol_head->name,msg.pwd) == 0)
		{
			mywrite(TYPE_CHAT,msg.name,clientol_head->name,msg.buf,clientol_head->lisfd);
			//printf("chat size>>%d\n",size);
			break;
		}
		clientol_head = clientol_head->next;
	}
	if(clientol_head == NULL)
	{
		mywrite(TYPE_CHAT," SERVER"," ","当前用户不在线！",lisfd);
		return -1;
	}
	
	return 0;	
}
/*
*功能:在线人数查看函数
*入参 lisfd_已连接套接字 clientol_head_在线用户链表头
*成功返回 0，出错返回-1
*/
int client_check(int lisfd,COL *clientol_head)
{
	assert(lisfd >= 0);
	assert(clientol_head != NULL);
	
	pthread_rwlock_rdlock(&rwlock);
	
	char buf[1024];
	memset(buf,0,sizeof(1024));
	char name[21];
	
	clientol_head = clientol_head->next;
	while(clientol_head != NULL)
	{
		memset(name,0,sizeof(name));
		sprintf(name,"%s\n",clientol_head->name);
		strcat(buf,name);
		if(strlen(buf) > 1004)
			break;
		clientol_head = clientol_head->next;
	}

	mywrite(TYPE_CHECK," "," ",buf,lisfd);
	pthread_rwlock_unlock(&rwlock);

	return 0;	
}

/*
*shell命令处理函数
*lisfd_已连接套接字，msg_shell数据包，client_head_用户链表
*成功返回0，出错返回-1
*/
int client_shell(int lisfd,MSG msg,char *name)
{	
	assert(lisfd >= 0);

	ssize_t size = -1;	

	int fd[2]={0};
	if(pipe(fd)<0)
	{
		perror("pipe error!\n");
		return -1;
	}
	pid_t pid= -1;	
	pid = fork();
	if(pid < 0)
	{
		printf("fork error!\n");
		return -1;
	}
	else if(pid == 0)
	{
		close(fd[0]);
		char ip[20] = {0};
		getip(lisfd,ip);		
		printf("客户端(%s)请求命令:%s\n",ip,msg.buf);
		if(strncmp(msg.buf,"pwd",sizeof(char)*3) == 0)
		{
			write(fd[1],"/\n",sizeof(char)*2);
			exit(0);
		}
		char cmd[40] = {0};

		dup2(fd[1],STDOUT_FILENO);
		dup2(fd[1],STDERR_FILENO);
		sprintf(cmd,"cd %s;%s",name,msg.buf);

		if(execl("/bin/bash",msg.buf,"-c",cmd,NULL)<0)
		{
			perror("exec error!\n");
			exit(-1);
		}
		exit(0);
	}
	else
	{
		char buf[1020];
		close(fd[1]);
		int flag = 0;
		int status = -1;
		waitpid(pid,&status,0);
		while(1)
		{
			memset(&buf,0,sizeof(buf));
			size=read(fd[0],buf,sizeof(buf));
			if(size == 0)
			{	
				if(flag == 0)
				{
					mywrite(TYPE_OK," "," "," ",lisfd);
				}
				if(flag == 1)
				{
					mywrite(TYPE_ERR," "," "," ",lisfd);
				}
				break;
			}
			mywrite(TYPE_CMD," "," ",buf,lisfd);			
			flag = 1;
		}
			
	}
	
	
	return 0;
}

/*
*功能:服务器端登录处理函数
*入参:lisfd_已连接套接字，msg_登录数据包，client_head_用户链表
*成功返回0，出错返回-1
*/
int client_login(int lisfd,MSG msg,CLIENT *client_head)
{
	assert(lisfd >= 0);
	assert(client_head != NULL);

	pthread_rwlock_wrlock(&rwlock);
	int ret = -1;
	ret = clientol_link_checkname(clientol_head,msg.name);
	if(ret == -1)
	{
		mywrite(TYPE_ERR," "," ","用户已经登录！",lisfd);
		pthread_rwlock_unlock(&rwlock);
		return -1;
		
	}
	client_head = client_head->next;
	while(client_head != NULL)
	{
		if(strcmp(client_head->name,msg.name) == 0 && strcmp(client_head->pwd,msg.pwd) == 0)
		{
			//发送登录确认包
			if(mywrite(TYPE_OK," "," "," ",lisfd) < 0)
			{
				perror("login confirm data send error!\n");
				pthread_rwlock_unlock(&rwlock);
				return -1;
			}
			//登录用户添加到在线用户链表中
			clientol_link_after(clientol_head,msg.name,lisfd);
			//会话链表
			connect_info_after(info_head,msg.name,lisfd,0);
			break;
		}
		client_head = client_head->next;
	}

	//用户名或密码错误,发送失败信息
	if(client_head == NULL)
	{
		if(mywrite(TYPE_ERR," "," ","用户名或者密码错误！",lisfd) < 0)
		{
			perror("login confirm data send error!\n");
			pthread_rwlock_unlock(&rwlock);
			return -1;
		}
		pthread_rwlock_unlock(&rwlock);
		return -1;
	}
	char ip[20] = {0};
	getip(lisfd,ip);
	printf("%s用户(%s)上线了\n",msg.name,ip);
	pthread_rwlock_unlock(&rwlock);
	return 0;	
}

/*
*功能:服务器端注册函数,注册成功向客户端发送确认，失败返回原因
*入参:lisfd_已连接套接字 msg_注册数据包,cleint_head 用户链表头
*成功返回0，出错返回-1
*/
int client_reg(int lisfd,MSG msg,CLIENT *client_head)
{

	assert(lisfd >= 0);
	assert(client_head != NULL);
	pthread_rwlock_wrlock(&rwlock);

	CLIENT *head = client_head->next;

	//判断是否重名
	while(head != NULL)
	{
		if(strcmp(msg.name,head->name) == 0)
		{
			mywrite(TYPE_ERR," "," "," ",lisfd);
			pthread_rwlock_unlock(&rwlock);
			return -1;
		}			
		head = head ->next;		
	}
	
	//无重名的情况下添加到链表中并发送注册确认信息


	CLIENT *newclient = (CLIENT *)calloc(1,sizeof(CLIENT));
	if(newclient == NULL)
	{
		perror("newclient calloc error!\n");
		pthread_rwlock_unlock(&rwlock);
		return -1;
	}
	newclient->next = NULL;
	strcpy(newclient->name,msg.name);
	strcpy(newclient->pwd,msg.pwd);
	if(client_link_after(client_head,newclient) != 0)
	{
		perror("register error!\n");
		pthread_rwlock_unlock(&rwlock);
		return -1;
	}
	
	char ip[20] = {0};
	getip(lisfd,ip);
	printf("客户端(%s)注册了账户%s\n",ip,msg.name);
	mkdir(msg.name,0755); //创建一个同名文件夹,存放用户数据

	mywrite(TYPE_OK," "," "," ",lisfd);
	pthread_rwlock_unlock(&rwlock);
	
	return 0;	
}

/*
*根据套接字获取ip函数
*入参 lisfd_套接字,ip_字符串存放地址
*成功返回0,出错返回-1
*/

int getip(int lisfd,char *str)
{
	assert(lisfd >= 0);

	struct sockaddr_in clientaddr;
	memset(&clientaddr,0,sizeof(clientaddr));
	socklen_t len = sizeof(clientaddr);
	if(getpeername(lisfd,(struct sockaddr *)&clientaddr,&len) < 0)
	{
		perror("get perr name error!\n");
		return -1;
	}
	inet_ntop(AF_INET,&clientaddr.sin_addr.s_addr,str,sizeof(char)*20);
	
	return 0;
}

/*
*功能,读取数据包
*入参：msg_数据包结构体指针，lisfd_已连接套接字
*返回值：读取成功返回读取的字节数，失败返回-1
*/
int myread(struct message *msg,int lisfd)
{
	if(NULL == msg)
	{
		printf("msg NULL!\n");
		return -1;
	}
	ssize_t size;
	size = read(lisfd,msg,sizeof(MSG));

	return size;
}

/*
*功能:服务器命令操作函数
*入参无
*/
void *cmd_thread(void *argc)
{
	char cmd[100];
	int flag =0;

	while(1)
	{
		flag = 0;
		memset(cmd,0,sizeof(cmd));
		printf("$:");
		fgets(cmd,sizeof(cmd),stdin);
		if(strcmp(cmd,"\n") == 0)
			continue;
		cmd[strlen(cmd) - 1] = '\0';

		if(cmd[0] == '$')
		{
			char str[100] = {0};
			strcpy(str,(cmd + 1));
			system(str);
			continue;
		}
		
		char *d = " ";
		char *p = NULL;
		p = strtok(cmd,d);
		if(p == NULL)
			continue;
		
		if(strcmp(p,"help") == 0)
			print_help();
		else if(strcmp(p,"clear") == 0)
			system("clear");
		else if(strcmp(p,"l") == 0)
		{
			p = strtok(NULL,d);
			if(p == NULL)
			{
				printf("无效命令，请使用help命令查看可用命令！\n");
				continue;
			}
			if(strcmp(p,"-i") == 0)
				list_info();
			else if(strcmp(p,"-u") == 0)
				list_onlineusr();
			else if(strcmp(p,"-g") == 0)
				system("cat log.txt");
			else
				printf("无效命令，请使用help命令查看可用命令！\n");			
		}
		else if(strcmp(p,"h") == 0)
		{
			p = strtok(NULL,d);
			if(p == NULL)
				heart_flag = 0;
			else if(strcmp(p,"-on") == 0)
				heart_flag = 1;
			else
				printf("无效命令，请使用help命令查看可用命令！\n");			
		}
		else if(strcmp(p,"b") == 0)
		{
			p = strtok(NULL,d);
			if(p == NULL)
			{
				printf("请确认输入了要禁用的用户名！\n");
				continue;
			}
			COL *head = clientol_head->next;
			while(head != NULL)
			{
				if(strcmp((p+1),head->name) == 0)
				{
					close(head->lisfd);
					clientol_link_delete(clientol_head,head->name);
					flag = 1;
					printf("操作成功！\n");
					break;
				}
				head = head->next;
			}

			if(flag == 0)
			printf("%s用户不在线！\n",(p+1));
		}
		else if(strcmp(p,"clean") == 0)
		{
			system("rm log.txt");
			system("touch log.txt");
		}
		else if(strcmp(p,"shutdown") == 0)
			server_shutdown();
		else			
			printf("无效命令，请使用help命令查看可用命令！\n");		
		
	}
	return 0;
}

/*
*关闭服务器，释放所有列表
*成功返回0，出错返回-1
*/

int server_shutdown()
{
	free_client(client_head);
	free_clientol(clientol_head);
	free_info(info_head);

	client_head = NULL;
	clientol_head = NULL;
	info_head = NULL;

	printf("客户链表释放成功...\n");
	printf("在线用户链表释放成功...\n");
	printf("会话链表释放成功...\n");
	
	printf("服务器关闭成功！\n");

	exit(0);
}

/*
*服务器查看会话链表
*参数无 
*成功返回0 出错返回-1
*/
int list_info()
{
	INFO *head = info_head->next;
	if(head == NULL)
	{
		printf("会话链表为空！\n");
		return 0;
	}
	while(head != NULL)
	{
		printf("%s\t%s\t%s\t%s\n",head->name,head->ip,head->date,head->reason);
		head = head->next;
	}
	return 0;
}

/*
*服务器查看在线人数函数
*参数无 
*成功返回0 出错返回-1
*/
int list_onlineusr()
{
	COL *head = clientol_head->next;
	if(head == NULL)
	{
		printf("当前无用户在线！\n");
		return 0;
	}
	while(head != NULL)
	{
		printf("%s\n",head->name);
		head = head->next;
	}
	return 0;
}

/*
*功能:服务器命令帮助文件
*
*/
void print_help()
{
	system("cat server_help.txt");
}

/*
*功能,数据包封装发送
*入参：type_协议头，name_客户端姓名，pwd_客户端密码，buf_数据包内容，lisfd_已连接套接字
*返回值：成功返回发送的字节数，失败返回-1
*/
int mywrite(int type,char *name,char *pwd,char *buf,int lisfd)
{
	if(NULL == name || NULL == pwd || NULL == buf)
	{
		printf("data NULL!\n");
		return -1;
	}
	MSG msg;
	memset(&msg,0,sizeof(msg));
	msg.msg_type = type;
	strcpy(msg.name,name);
	strcpy(msg.pwd,pwd);
	strcpy(msg.buf,buf);

	ssize_t size;

	size = write(lisfd,&msg,sizeof(MSG));
	
	return size;
}
