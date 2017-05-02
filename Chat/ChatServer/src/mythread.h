#ifndef _THREAD_H_
#define _THREAD_H_

#define TYPE_WEL 0
#define TYPE_UPDATE 12  //更新数据包
#define TYPE_UPLOAD 13  //从本地上传文件到云端
#define TYPE_DOWNLOAD 14 //从云端下载文件到本地
// 登陆数据包
#define	TYPE_LOGIN 1   
// 注册数据包
#define	TYPE_REG  2	
// 聊天数据包
#define	TYPE_CHAT  3  
// 心跳数据包 
#define	TYPE_HEART 4 
// 命令数据包
#define	TYPE_CMD 5 

#define	TYPE_OK 6
#define	TYPE_ERR 7

#define	TYPE_EXITOK 8
#define	TYPE_EXITERR 9
#define TYPE_USREXIT 10
#define TYPE_CHECK 11


//数据文件保存路径宏

#define CLIENT_DATA_PATH 	"client_data"
#define LOG_PATH 			"log.txt"
#define PROFILE_PATH 		"profile"
#define FILENAME 			"client_update.tar"


//协议结构体
#pragma pack(1)
typedef struct message
{
	int msg_type; //协议头
	char name[20]; 
	char pwd[20];
	char buf[1024]; //数据包内容
}MSG;

//客户成员链表结构体
typedef struct client
{
	char name[20];
	char pwd[20];
	struct client *next;
}CLIENT;

//在线客户端链表结构体
typedef struct clientol
{
	int lisfd;
	char name[20];
	struct clientol *next;
}COL;

//心跳线程处理结构体
typedef struct threadname
{
	int lisfd;
	int pipefd;
	pthread_t thread;
}TN;

//会话链表
typedef struct connect_info
{
	char ip[20];
	char name[20];
	char reason[20];
	char date[32];
	struct connect_info *next;
}INFO;

#pragma pack(4)

extern char PRE_PATH[56];

//云端下载函数
extern int cloud_download(int lisfd,char *filename,char *foldername);

//从本地上传到云端
extern int cloud_upload(int lisfd,char *filename,char *foldername);

//检测更新文件是否存在
extern int update_check(int lisfd);

//服务器命令操作函数
extern void *cmd_thread(void *argc);

//服务器可接受命令帮助文件
extern void print_help();

//服务器查看在线人数函数
extern int list_onlineusr();

//服务器关闭释放链表函数
extern int server_shutdown();

//会话链表函数
extern int list_info();

//每个客户端产生一个子线程的运行函数
extern void *main_thread(void *argc);

//服务器端注册函数
extern int client_reg(int lisfd,MSG msg,CLIENT *client_head);

//服务器端登录处理函数
extern int client_login(int lisfd,MSG msg,CLIENT *client_head);

//服务其处理shell命令函数
extern int client_shell(int lisfd,MSG msg,char *name);

//查看在线人数处理函数
extern int client_check(int lisfd,COL *clientol_head);

//聊天函数
extern int client_chat(int lisfd,COL *clientol_head,MSG msg);

//心跳处理函数
extern int client_heart(int lisfd,MSG msg,pthread_t heartthread,char *name,int fd);

//心跳异常处理线程运行函数
extern void *heartthread(void *argc);

//根据套接字获取ip
extern int getip(int lisfd,char *str);

//数据包解码读取
extern int myread(struct message *msg,int lisfd);

//数据包封装发送
extern int mywrite(int type,char *name,char *pwd,char *buf,int lisfd);

//服务器登陆函数
extern int server_login();

//服务器socket初始化
extern int server_sock_int(int port);

//密码隐藏
extern int new_getpass(char *dest, int maxlen);

//配置文件加载
extern int load_profile(char *ip,int *port);

//用户数据初始化
extern int usr_data_init();

//管理员信息加载函数
extern int load_admin(char *name,char *pwd);

#endif
