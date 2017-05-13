#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "link.h"
#include "mythread.h"

extern pthread_rwlock_t rwlock2;
extern pthread_mutex_t mutex;
extern pthread_mutex_t info_mutex;
extern pthread_rwlock_t log_rwlock;  

/*
*c/s 直接文件发送函数
*入参 fd_接收方套接字 文件名含路径
*成功返回0，失败返回-1
*/
int file_send(int fd,char *true_filename,char *filename,int type)
{
	assert(fd >= 0);
	assert(filename != NULL);

	MSG msg;
	memset(&msg,0,sizeof(MSG));
	int count;
	strcpy(msg.buf,filename);
	//printf("%s\n",msg.buf);
	msg.msg_type = type;
	//count = send(fd,&msg,sizeof(MSG),0);
	count = write(fd,&msg,sizeof(MSG));
	if(count <0)
	{
		perror("error!\n");
		return -1;
	}

	FILE *fp = fopen(true_filename,"rb");
	
	memset(&msg,0,sizeof(MSG));
	int length = 0;
	while((length = fread(msg.buf,sizeof(char),1024,fp))>0)
	{
		sprintf(msg.name,"%d",length);
		printf("file length:%d\n",length);
		msg.msg_type = type;
		//printf("%d\n",type);
		if(write(fd,&msg,sizeof(MSG)) < 0)
		{
			perror("send error!\n");
			return -1;
		}
		memset(&msg,0,sizeof(MSG));
	}

	//write(fd,&msg,sizeof(MSG));		
	
	memset(&msg,0,sizeof(MSG));
	msg.msg_type = TYPE_OK;
	if(write(fd,&msg,sizeof(MSG)) < 0)
	{
		perror("send error!\n");
		return -1;
	}
	fclose(fp);
	printf("文件发送完毕!\n");
	return 0;
}

/*
*c/s 直接文件接收函数
*入参 fd_发送方套接字 
*成功返回0，失败返回-1
*/
int file_rev(int fd,char *filename)
{
	assert(fd >= 0);

	printf("准备接收文件%s\n",filename);

	FILE *fp = fopen(filename,"wb+");
	char buff[1024];
	memset(buff,0,sizeof(buff));
	//DATA data;
	//memset(&data,0,sizeof(DATA));
	int length = 0;
	while(1)
	{
		//length = read(fd,&msg,sizeof(MSG));
		length = recv(fd,buff,1024,0);
		if(length < 0)
		{
			perror("error!\n");
			return -1;
		}

		if(strncmp(buff,"finished",8) == 0)
			break;
		printf("length>>%d\n",length);
		//int buflen = atoi(msg.name);
		//printf("name,buflen>>%d\n",buflen);
		int writelen = fwrite(buff,sizeof(char),length,fp);
		if(writelen < length)
		{
			perror("write!\n");
			return -1;
		}
		if(length < 1024)
			break;
		memset(buff,0,sizeof(buff));
	}

	printf("文件接收完毕!\n");
	fclose(fp);
	return 0;
}

/*
*功能:会话链表添加
*入参:info_head_头节点 name_用户名字 lisfd_已连接套接字
*成功返回0，出错返回-1
*/
int connect_info_after(INFO *info_head,char *name,int lisfd,int reason)
{
	assert(info_head != NULL);
	assert(name != NULL);
	assert(lisfd >= 0);

	//printf("reason>>%d\n",reason);
	pthread_mutex_lock(&info_mutex);
	INFO *newinfo = (INFO *)calloc(1,sizeof(INFO));
	if(NULL == newinfo)
	{
		perror("newinfo calloc error!\n");
		pthread_mutex_unlock(&info_mutex);
		return -1;
	}
	newinfo->next = NULL;
	strcpy(newinfo->name,name);
	char ip[20] = {0};
	getip(lisfd,ip);
	strcpy(newinfo->ip,ip);
	//实现获取当前时间.
	time_t  ticks;
	struct tm *t;
	char date[32]="";
	ticks = time(NULL);
	t= localtime(&ticks);
	strftime(date,sizeof(date),"%Y-%m-%d_%H:%M:%S",t);
	strcpy(newinfo->date,date);

	if(reason == 0)
		strcpy(newinfo->reason,"正常上线");
	else if(reason == 1)
		strcpy(newinfo->reason,"正常下线");
	else if(reason == 2)
		strcpy(newinfo->reason,"异常下线");

	while(info_head->next != NULL)
	{
		info_head = info_head->next;		
	}


	info_head->next = newinfo;
	pthread_mutex_unlock(&info_mutex);
	return 0;
}


/*
*功能:用户下线在线链表删除
*入参 clientol_head_在线用户链表头节点 name_下线用户名字
*删除成功返回0，出错返回-1
*/
int clientol_link_delete(COL *clientol_head,char *name)
{
	assert(clientol_head != NULL);
	if(clientol_head ->next == NULL)
		return 0;
	assert(name != NULL);
	pthread_mutex_lock(&mutex);
	COL *pre = clientol_head;
	clientol_head = clientol_head->next;

	while(clientol_head != NULL)
	{
		if(strcmp(clientol_head->name,name) == 0)
		{
			break;
		}
		pre=clientol_head;
		clientol_head = clientol_head->next;
	}
	if(clientol_head == NULL)
		return -1;

	printf("%s delete success!\n",name);
	COL *temp = pre->next;
	pre->next = temp->next;
	free(temp);
	pthread_mutex_unlock(&mutex);

	return 0;
}

/*
*功能:检测当前登录用户是否已经在线
*入参 clientol_head_在线用户链表头节点 name_登录用户名字
*没有登录返回0，已经登录返回-1
*/
int clientol_link_checkname(COL *clientol_head,char *name)
{
	assert(clientol_head != NULL);
	assert(name != NULL);

	clientol_head = clientol_head->next;
	while(clientol_head != NULL)
	{
		if(strcmp(clientol_head->name,name) == 0)
			return -1;
		clientol_head = clientol_head->next;
	}

	return 0;
}

/*
*功能:在线用户链表添加函数
*入参 clientol_head_在线用户链表头节点 name_在线用户名字
*成功返回0，出错返回-1
*/
int clientol_link_after(COL *clientol_head,char *name,int lisfd)
{
	assert(clientol_head != NULL);
	assert(name != NULL);

	pthread_mutex_lock(&mutex);
	COL *newol = (COL *)calloc(1,sizeof(COL));
	if(NULL == newol)
	{
		perror("newol calloc error!\n");
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	newol->next = NULL;
	strcpy(newol->name,name);
	newol->lisfd = lisfd;

	while(clientol_head->next != NULL)
	{
		clientol_head = clientol_head->next;
	}


	clientol_head->next = newol;
	pthread_mutex_unlock(&mutex);

	return 0;
}


/*
*客户信息链表加载函数
*入参:client_head 客户链表头节点
*成功返回0，出错返回-1
*/

int load_client(CLIENT *client_head)
{
	assert(client_head != NULL);
	
	char prepath[128] = {0};
	strcpy(prepath,PRE_PATH);

	FILE *fp=NULL;
	if((fp=fopen(strcat(prepath,CLIENT_DATA_PATH),"rt")) == NULL)
	{
		perror("client file open error!\n");
		return -1;
	}
	
	int i = -1;
	while(1)
	{
		CLIENT *newclient = (CLIENT *)calloc(1,sizeof(CLIENT));
		if(newclient == NULL)
		{
			perror("newclient calloc error!\n");
			fclose(fp);
			return -1;
		}
		newclient->next = NULL;

		i = fscanf(fp,"%s\t%s\n",newclient->name,newclient->pwd);
		if(i != 2)
		{	
			free(newclient);
			break;
		}
		
		client_link_after(client_head,newclient);
	}

	fclose(fp);

	return 0;
}

/*
*客户信息链表保存函数
*入参:client_head 客户链表头节点
*成功返回0，出错返回-1
*/

int save_client(CLIENT *client_head)
{
	assert(client_head != NULL);
	pthread_rwlock_wrlock(&rwlock2);
	char prepath[128] = {0};
	strcpy(prepath,PRE_PATH);

	FILE *fp=NULL;
	if((fp=fopen(strcat(prepath,CLIENT_DATA_PATH),"wt"))==NULL)
	{
		perror("client file open error!\n");
		pthread_rwlock_wrlock(&rwlock2);
		return -1;
	}

	client_head = client_head->next;
	while(client_head != NULL)
	{
		fprintf(fp,"%s\t%s\n",client_head->name,client_head->pwd);
		client_head = client_head->next;
	}

	fclose(fp);
	pthread_rwlock_unlock(&rwlock2);

	return 0;
}

/*
*客户链表尾部插入函数
*入参：client_head_链表头节点，newclient_链表新节点
*成功返回0，出错返回-1
*/
int client_link_after(CLIENT *client_head,CLIENT *newclient)
{
	assert(client_head != NULL);
	assert(newclient != NULL);

	while(client_head->next != NULL)
	{
		client_head = client_head->next;
	}


	client_head->next = newclient;
	newclient->next = NULL;
	
	return 0;
}

/*
*数据包日志处理函数
*msg_数据结构体
*成功返回0 出错返回-1
*/

int log_dis(MSG msg,int lisfd,char *name)
{
	char logbuff[120];
	char ip[20] = {0};
	getip(lisfd,ip);
	memset(logbuff,0,sizeof(logbuff));

	switch(msg.msg_type)
	{
		case TYPE_LOGIN://处理登入业务
			sprintf(logbuff,"%s(%s)请求登陆",msg.name,ip);
			break;
		case TYPE_REG://处理注册业务
			sprintf(logbuff,"%s(%s)请求注册",msg.name,ip);
			break;
		case TYPE_CMD://shell命令处理
			sprintf(logbuff,"%s(%s)请求执行%s命令",name,ip,msg.buf);
			break;
		case TYPE_CHECK://在线人数
			sprintf(logbuff,"%s(%s)请求查看在线人数",name,ip);
			break;
		case TYPE_DOWNLOAD: //云端下载
			sprintf(logbuff,"%s请求从云端下载文件%s\n",name,msg.buf);
			break;
		case TYPE_UPLOAD: //本地上传
			sprintf(logbuff,"%s请求从本地上传文件%s到云端\n",name,msg.buf);
			break;
		case TYPE_CHAT://聊天
			sprintf(logbuff,"%s(%s)请求发送消息(%s)给(%s)",msg.name,ip,msg.buf,msg.pwd);
			break;
		case TYPE_USREXIT:
			sprintf(logbuff,"%s(%s)正常下线",name,ip);
			break;
		case 0:  //客户端意外断线，read返回0
		case TYPE_EXITERR: //客户端异常退出，Ctrl+c Ctrl+z
			sprintf(logbuff,"%s(%s)异常断线",name,ip);
			break;				
		case TYPE_EXITOK://正常exit退出
			sprintf(logbuff,"客户端%s正常退出",ip);
			break;
	}
	log_write(logbuff);
	return 0;
}

/*
*日志写入函数
*入参 logbuf_日志内容
*成功返回0,出错-1
*/
int log_write(char *logbuf)
{
	assert(logbuf != NULL);
	
	//pthread_rwlock_wrlock(&log_rwlock);
	char prepath[128] = {0};
	strcpy(prepath,PRE_PATH);

	FILE *fp=NULL;
	if((fp=fopen(strcat(prepath,LOG_PATH),"at")) == NULL)
	{
		perror("log file open error!\n");
		//pthread_rwlock_unlock(&log_rwlock);
		return -1;
	}

	//实现获取当前时间.
	time_t  ticks;
	struct tm *t;
	char date[32]="";
	ticks = time(NULL);
	t= localtime(&ticks);
	strftime(date,sizeof(date),"%Y-%m-%d_%H:%M:%S",t); 

	
	fprintf(fp,"(%s)%s\n",date,logbuf);

	fclose(fp);
	//pthread_rwlock_wrlock(&log_rwlock);
	return 0;	
}

/*
* 客户链表释放函数
*入参：client_head_链表头
*成功返回0，出错返回-1
*/

int free_client(CLIENT *client_head)
{
	assert(client_head != NULL);

	CLIENT *temp = NULL;
	client_head = client_head->next;
	while(client_head != NULL)
	{
		temp = client_head;
		client_head = client_head->next;
		free(temp);

	}

	return 0;
}

/*
* 在线客户链表释放函数
*入参：clientol_head_链表头
*成功返回0，出错返回-1
*/

int free_clientol(COL *clientol_head)
{
	assert(clientol_head != NULL);

	COL *temp = NULL;
	clientol_head = clientol_head->next;
	while(clientol_head != NULL)
	{
		temp = clientol_head;
		clientol_head = clientol_head->next;
		free(temp);

	}

	return 0;
}

/*
* 会话链表释放函数
*入参：client_head_链表头
*成功返回0，出错返回-1
*/

int free_info(INFO *info_head)
{
	assert(info_head != NULL);

	INFO *temp = NULL;
	info_head = info_head->next;
	while(info_head != NULL)
	{
		temp = info_head;
		info_head = info_head->next;
		free(temp);

	}

	return 0;
}

