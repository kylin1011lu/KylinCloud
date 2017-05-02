#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <termios.h>
#include <setjmp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "cfun.h"
#include "dis.h"

extern int fd[2];
extern int alarmflag;
//extern int reconnect_flag;
//extern jmp_buf jmpbuffer;

//聊天信息屏蔽选项 1代表不屏蔽 0代表屏蔽来自其他人的聊天信息
int chatflag = 1; 
int read_thread_flag = 1; //中转线程读关闭开关


/*
*功能:子线程过滤来自服务端消息
*如果是聊天信息直接打印到屏幕上
*非聊天信息写进管道里
*
*/
void *threadread(void *argc)
{
	int sockfd = *((int*)argc);
	MSG msg;
	while(1)
	{

		memset(&msg,0,sizeof(MSG));
		ssize_t size = read(sockfd,&msg,sizeof(MSG));
		//printf("size>>%d\n",size);		
		if(size == 0)
		{
			printf(RED"与服务器连接断开!"NONE);
			fflush(stdout);	
			printf("\n");	
			return NULL;
		}
		if(msg.msg_type == TYPE_CHAT && chatflag == 1)
		{
			printf(GREEN"%s say:%s"NONE,msg.name,msg.buf);
			fflush(stdout);	
			printf("\n");		
		}
		if(msg.msg_type != TYPE_CHAT)
		{
			write(fd[1],&msg,sizeof(MSG));
		}
	}
	

	return NULL;
}

/*
*功能：客户端登录界面
*入参:sockfd 服务端监听套接字
*返回值：无
*/
void do_service(int sockfd)
{	
	assert(sockfd >= 0);

	//检测是否有更新文件，有则执行更新函数
	MSG msg;
	memset(&msg,0,sizeof(msg));
	myread(&msg,sockfd);
	if(msg.msg_type == TYPE_UPDATE && (strcmp(msg.buf,VERSION) > 0))
	{
		do_update(sockfd);
	}
	else if(msg.msg_type == TYPE_UPDATE && (strcmp(msg.buf,VERSION) <= 0))
	{
		mywrite(TYPE_ERR," "," "," ",sockfd);
	}

	int type = -1;

	do
	{
		type = -1;
		
		printf(RED"\n\t\t\t\t★★★★★客户端界面★★★★★\n"NONE);
		printf("\t\t\t\t     1.用户登录       \n");
		printf("\t\t\t\t     2.用户注册       \n");
		printf("\t\t\t\t     0.退出           \n");
		printf("\t\t\t\t★★★★★★★★★★★★★★★★★★★★\n");
		printf("请输入选项>>");
		scanf("%d",&type);
		while(getchar() != '\n');

		switch(type)
		{
			case TYPE_LOGIN://客户登录
				client_login(sockfd);
				break;
			case TYPE_REG://客户注册
				client_reg(sockfd);
				break;
			case 0:
				mywrite(TYPE_EXITOK," "," "," ",sockfd);
				return;
			default:
				printf("输入错误,请重新输入！\n");
				break;
		}
	}while(type != 0);
}

/*
*功能:客户端更新函数
*入参 sockfd 服务器套接字
*成功返回0 出错返回-1
*/
int do_update(int sockfd)
{
	assert(sockfd >= 0);
	
	char ch = '\0';
	printf("检测到更新文件,是否进行更新？[Y/n]");
	scanf("%c",&ch);
	while(getchar() != '\n');

	if(ch != 'Y' && ch != 'y')
	{
		mywrite(TYPE_ERR," "," "," ",sockfd);
		return -1;
	}
	else
	{
		mywrite(TYPE_OK," "," "," ",sockfd);

		if(file_rev(sockfd) == 0)
			printf("更新包接收成功!\n");
		else
			printf("更新包接收失败!\n");

		/*解压以及编译*/
		system("rm client");
		system("tar xvf client_update.tar ./");
		system("make");
		system("make clean");
		printf(GREEN"更新成功,请重启程序！\n"NONE);

		exit(0);
	}

	

	return 0;
}

/*
*客户端登录函数
*入参:sockfd_监听套接字
*成功返回0，出错返回-1
*/
int client_login(sockfd)
{
	assert(sockfd >= 0);

	char name[20]={0};
	char pwd[20] ={0};
	int i =0;
	MSG msg;
	do
	{
		printf("请输入用户名:");
		fgets(name,sizeof(name),stdin);
		name[strlen(name)-1]='\0';
		printf("请输入密码:");
		new_getpass(pwd,20);

		//发送登录数据包
		ssize_t size = mywrite(TYPE_LOGIN,name,pwd," ",sockfd);
		if(size < 0)
		{
			perror("login data send failure!\n");
			return -1;
		}
		//printf("send size>>%d\n",size);


		memset(&msg,0,sizeof(MSG));
		size = myread(&msg,sockfd);
	
		if(msg.msg_type == TYPE_ERR)
		{
			printf("%s\n",msg.buf);
			//return -1;
		}
		if(msg.msg_type == TYPE_OK)
		{
			printf("成功登录!\n");
			break;
		}

		i++;
		if(i == 3)
		{
			printf("用户名或密码错误三次,程序退出!\n");
			raise(SIGINT);
		}
	}while(msg.msg_type != TYPE_OK);
	
	if(msg.msg_type == TYPE_OK)
		user_menu(sockfd,name);

	return 0;	
}

/*
*用户功能菜单函数
*入参 sockfd 监听套接字
*/

void user_menu(int sockfd,char *name)
{
	assert(sockfd >= 0);
	alarmflag = 1;
	alarm(3);//正常登录后开始发送心跳
	int type = -1;
	do
	{
		type = -1;

		printf(RED"\n\t\t\t\t★★★★★用户功能菜单★★★★★\n"NONE);
		printf("\t\t\t\t       1.云系统          \n");
		printf("\t\t\t\t       2.聊天            \n");
		printf("\t\t\t\t       0.返回上一层      \n");
		printf("\t\t\t\t★★★★★★★★★★★★★★★★★★★★★★\n");
		printf("请输入选项>>");
		scanf("%d",&type);
		while(getchar() != '\n');

		switch(type)
		{
			case 1://shell命令处理
				shell_cmd(sockfd);
				break;
			case 2://聊天
				client_chatmenu(sockfd,name);
				break;
			case 0:
				mywrite(TYPE_USREXIT,name," "," ",sockfd);
				alarmflag = 0;
				return;
			default:
				printf("输入错误,请重新输入！\n");
				break;
		}
	}while(type != 0);
}

/*
*功能 聊天界面函数
*入参 sockfd 套接字 name_用户名字
*
*/
int client_chatmenu(int sockfd,char *name)
{
	assert(sockfd >= 0);
	assert(name != NULL);
	
	int type = -1;
	do
	{
		type = -1;
		
		printf(RED"\n\t\t\t\t★★★★★聊天功能菜单★★★★★\n"NONE);
		printf("\t\t\t\t     1.查看当前在线用户     \n");
		printf("\t\t\t\t     2.与在线用户聊天      \n");
		printf("\t\t\t\t     3.聊天设置          \n");
		printf("\t\t\t\t     0.返回上一层        \n");
		printf("\t\t\t\t★★★★★★★★★★★★★★★★★★★★★★\n");
		printf("请输入选项>>");
		scanf("%d",&type);
		while(getchar() != '\n');

		switch(type)
		{
			case 1:
				onlineusr_check(sockfd);
				break;
			case 2://聊天
				client_chat(sockfd,name);
				break;
			case 3://聊天设置
				client_chatset();
				break;
			case 0:
				//mywrite(TYPE_USREXIT,name," "," ",sockfd);
				return 0;
			default:
				printf("输入错误,请重新输入！\n");
				break;
		}
	}while(type != 0);

	return 0;
}

/*
*功能:设置是否接受其他人的聊天信息
*入参 无
*成功返回 0，出错返回-1
*/

int client_chatset()
{
	int type = -1;
	do
	{
		type = -1;
		printf("\n");
		printf(RED"\n\t\t\t\t★★★★★聊天设置★★★★★\n"NONE);
		printf("\t\t\t\t     1.安静模式                \n");
		printf("\t\t\t\t     2.聊天模式                \n");
		printf("\t\t\t\t     0.返回上一层              \n");
		printf("\t\t\t\t★★★★★★★★★★★★★★★★★★\n");
		printf("请输入选项>>");
		scanf("%d",&type);
		while(getchar() != '\n');

		switch(type)
		{
			case 1:
				chatflag = 0;
				printf(RED"安静模式已启用,你将无法接收到其他用户发送给你的消息！\n"NONE);
				break;
			case 2:
				chatflag = 1;
				printf(RED"聊天模式已开启，你将随时接收到来自其他用户的信息！\n"NONE);
				break;
			case 0:
				return 0;
			default:
				printf("输入错误,请重新输入！\n");
				break;
		}
	}while(type != 0);

	return 0;
}
/*
*功能:与其他在线用户一对一聊天
*入参 sockfd 套接字
*成功返回 0，出错返回-1
*/
 int client_chat(int sockfd,char *name)
{
	assert(sockfd >= 0);
	assert(name != NULL);

	//mywrite(TYPE_CHAT,name,"bbb","hello",sockfd);
	char talkcmd[100];

	printf(RED"请按如下命名进行聊天,名字前的 - 不可缺少！名字与内容空一格.直接输入exit将退出聊天窗口\n"NONE);
	printf("talk -name content\n");

	while(1)
	{
		memset(talkcmd,0,sizeof(talkcmd));
		
		fgets(talkcmd,sizeof(talkcmd),stdin);
		talkcmd[strlen(talkcmd)-1] = '\0';
		//printf("%s",talkcmd);
		if(strcmp(talkcmd,"exit") == 0)
			break;
		talk(talkcmd,sockfd,name);	
	}

	return 0;
}

/*
*功能将聊天命令进行分解并发送到服务器
*入参 talkcmd_聊天命令 sockfd 监听套接字 name_发送者名字
*成功返回0，出错返回-1
*/
int talk(char *talkcmd,int sockfd,char *name)
{
	assert(sockfd >= 0);
	assert(talkcmd != NULL);
	assert(name != NULL);

	char towho[20] = {0};
	char content[50] ={0};
	//printf("get tal1\n");
	int i = 0;
	while(i <= 100)
	{
		if(talkcmd[i] == '-')
		{
			i++;
			break;
		}
		i++;
			
	}
		//printf("get tal2\n");
	int j=0;
	while(j < 20 && i < 100 && talkcmd[i] != ' ')
	{
		towho[j] = talkcmd[i];
		i++;
		j++;
	}
	//printf("get tal3\n");
	strncpy(content,(talkcmd+i+1),50);
	//printf("get tal4\n");

	mywrite(TYPE_CHAT,name,towho,content,sockfd);

	return 0;
}

/*
*功能:查看当前在线用户
*入参 sockfd 套接字
*成功返回 0，出错返回-1
*/
int onlineusr_check(int sockfd)
{
	assert(sockfd >= 0);

	mywrite(TYPE_CHECK," "," "," ",sockfd);

	MSG msg;
	memset(&msg,0,sizeof(MSG));
	myread(&msg,sockfd);
	printf("在线用户列表:\n%s",msg.buf);

	return 0;
}

/*
*客户端shell命令处理函数
*入参 sockfd 套接字
*成功返回0，出错返回-1
*/
int shell_cmd(int sockfd)
{
	assert(sockfd >= 0);

	char str[40] = {0};
	char cmd[40] = {0}; 
	while(1)
	{
		//fflush(stdout);
		memset(str,0,sizeof(str));
		memset(cmd,0,sizeof(cmd));
		printf("请输入云系统指令:");
		fgets(cmd,sizeof(cmd),stdin);
		if(strcmp(cmd,"\n") == 0)
			continue;
		cmd[strlen(cmd)-1] = '\0';

		if(strcmp(cmd,"exit") == 0)
			return 0;
		if(strcmp(cmd,"help") == 0)
		{
			system("cat client_help.txt");
			continue;
		}
		if(cmd[0] == '$')
		{
			strcpy(str,(cmd+1));
			system(str);
			continue;
		}
		
		strcpy(str,cmd);		
		char *d = " ";
		char *p = NULL;
		p = strtok(cmd,d);
		if(p == NULL)
			continue;

		if(strcmp(p,"download") == 0)
		{
			p = strtok(NULL,d);
			if(p)
			{
				download_file(sockfd,p);
				continue;
			}
			printf("命令无效，请确认是否输入文件名字！\n");
			continue;
		}
		if(strcmp(p,"upload") == 0)
		{
			p = strtok(NULL,d);
			if(p)
			{
				upload_file(sockfd,p);
				continue;
			}
			printf("命令无效，请确认是否输入文件名字！\n");
			continue;
		}

		send_shell(sockfd,str);
	
	}

	return 0;
}

/*
*从云端下载文件函数
*入参 sockfd_服务器套接字 p_文件名
*成功返回 0 失败返回-1
*/
int download_file(int sockfd,char *filename)
{
	assert(sockfd >= 0);
	assert(filename != NULL);

	//read_thread_flag = 0; //关闭中转线程读取功能

	mywrite(TYPE_DOWNLOAD," "," ",filename,sockfd);
	
	if(file_rev(sockfd) < 0)
	{
		printf("下载失败！\n");
		//read_thread_flag = 1;
		return -1;
	}
	//read_thread_flag = 1;

	return 0;
}

/*
*从云端上传文件函数
*入参 sockfd_服务器套接字 p_文件名
*成功返回 0 失败返回-1
*/
int upload_file(int sockfd,char *filename)
{
	assert(sockfd >= 0);
	assert(filename != NULL);

		//检测文件是否存在
	int ret = access(filename,F_OK | R_OK);
	if(ret < 0)
	{
		printf("%s文件不存在，无法上传!\n",filename);
		return -1;
	}

	mywrite(TYPE_UPLOAD," "," ",filename,sockfd);

	if(file_send(sockfd,filename,TYPE_UPLOAD) == 0)
		printf("%s文件上传成功!\n",filename);
	else
	{
		printf("文件上传失败！\n");
		return -1;
	}

	return 0;
}

/*
*发送shell命令并接收处理结果函数
*入参 str_shell 命令字符串
*成功返回0 出错返回-1
*/

int send_shell(int sockfd,char *str)
{
	assert(str != NULL);

	char success[40] = "命令执行成功!\n";
	
	//发送shell命名包
	ssize_t size = mywrite(TYPE_CMD," "," ",str,sockfd);
	if(size < 0)
	{
		perror("shell cmd send error!\n");
		return -1;
	}

	MSG msg;
	while(1)
	{
		memset(&msg,0,sizeof(MSG));
		myread(&msg,sockfd);
		if(msg.msg_type == TYPE_ERR)
			break;
		if(msg.msg_type == TYPE_OK)
		{
			write(STDOUT_FILENO,success,sizeof(char)*40);	
			break;
		}
		if(msg.msg_type == TYPE_CMD)
			write(STDOUT_FILENO,msg.buf,sizeof(char)*1024);		

	}

	return 0;
}

/*
*客户端注册函数
*入参:sockfd_监听套接字
*成功返回0，出错返回-1
*/
int client_reg(sockfd)
{
	assert(sockfd >= 0);
	
	int i = 0;
	char name[20] = {0};
	char pwd[20] = {0};
	char pwd1[20] = {0};
	printf("请输入注册名:");
	fgets(name,sizeof(name),stdin);
	name[strlen(name) -1] ='\0';
	while(1)
	{
		printf("请输入密码:");
		new_getpass(pwd1,20);
		printf("请再次输入密码:");
		new_getpass(pwd,20);
		if(strcmp(pwd1,pwd) == 0)
			break;
		else
			printf(RED"两次密码不同,请重新输入!\n"NONE);
		i++;
		if(i == 3)
		{
			printf(RED"密码三次输入有误，退出注册！\n"NONE);
			return -1;
		}
	}
	ssize_t size = 0;
	//发送注册数据包
	if((size = mywrite(TYPE_REG,name,pwd," ",sockfd)) < 0)
	{
		perror("reg data send error!\n");
		return -1;
	}
	//printf("send size>>%d\n",size);

	MSG msg;
	memset(&msg,0,sizeof(msg));
	size = myread(&msg,sockfd);
	if(size < 0)
	{
		perror("read error!\n");
		return -1;
	}
	//printf("read size>>%d\n",size);
	if(msg.msg_type == TYPE_OK)
		printf(RED"注册成功！\n"NONE);
	else if(msg.msg_type == TYPE_ERR)
	{
		printf(RED"%s用户名已经被使用，请使用其他用户名进行注册！\n"NONE,name);
		return -1;
	}
	return 0;	
}

/*
*功能,读取数据包
*入参：msg_数据包结构体指针，sockfd_服务器监听套接字
*返回值：读取成功返回读取的字节数，失败返回-1
*/
int myread(struct message *msg,int sockfd)
{
	if(NULL == msg)
	{
		printf("msg NULL!\n");
		return -1;
	}
	ssize_t size;
	size = read(fd[0],msg,sizeof(MSG));

	return size;
}

/*
*功能,数据包封装发送
*入参：type_协议头，name_客户端姓名，pwd_客户端密码，buf_数据包内容，sockfd_服务端监听套接字
*返回值：成功返回发送的字节数，失败返回-1
*/
int mywrite(int type,char *name,char *pwd,char *buf,int sockfd)
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

	size = write(sockfd,&msg,sizeof(MSG));
	
	return size;
}

/*
*c/s 直接文件发送函数
*入参 fd_接收方套接字 文件名含路径
*成功返回0，失败返回-1
*/
int file_send(int fd,char *filename,int type)
{
	assert(fd >= 0);
	assert(filename != NULL);

	//DATA data;
	//memset(&data,0,sizeof(DATA));

	char buff[1024];
	memset(buff,0,sizeof(buff));
	FILE *fp = fopen(filename,"rb");
	
	//memset(&msg,0,sizeof(MSG));
	int length = 0;
	int pre = 0;
	while((length = fread(buff,sizeof(char),1024,fp))>0)
	{
		//sprintf(msg.name,"%d",length);
		printf("file length:%d\n",length);
		//msg.msg_type = type;
		//data.length = length;
		pre = length;
		if(send(fd,buff,length,0) < 0)
		{
			perror("send error!\n");
			return -1;
		}
				
		memset(buff,0,sizeof(buff));
	}
	if(pre == 1024)
	{
		memset(buff,0,sizeof(buff));
		strcpy(buff,"finished");
		length = send(fd,buff,sizeof(buff),0);
		printf("length:%d\n",length);
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
int file_rev(int fd)
{
	assert(fd >= 0);

	MSG msg;
	memset(&msg,0,sizeof(MSG));
	char filename[50];
	memset(filename,0,sizeof(filename));
	int count;
	//count = recv(fd,&msg,sizeof(MSG),0);
	count = myread(&msg,fd);
	if(count < 0)
	{
		perror("error!\n");
		return -1;
	}
	if(msg.msg_type == TYPE_ERR)
	{
		printf("%s\n",msg.buf);
		return -1;
	}
	printf("准备接收文件%s\n",msg.buf);
	strcpy(filename,msg.buf);


	FILE *fp = fopen(filename,"wb+");
	
	memset(&msg,0,sizeof(MSG));
	int length = 0;
	while(1)
	{
		length = myread(&msg,fd);
		printf("length>>%d\n",length);
		//printf("%d\n",msg.msg_type);
		if(length < 0)
		{
			perror("error!\n");
			return -1;
		}
		if(msg.msg_type == TYPE_OK)
		{
			printf("ok\n");
			break;
		}
		int buflen = atoi(msg.name);
		int writelen = fwrite(msg.buf,sizeof(char),buflen,fp);
		if(writelen < buflen)
		{
			perror("write!\n");
			return -1;
		}
		memset(&msg,0,sizeof(MSG));
	}

	printf("文件接收完毕!\n");
	fclose(fp);
	return 0;
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
