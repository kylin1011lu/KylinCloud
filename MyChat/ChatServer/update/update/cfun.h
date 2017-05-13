#ifndef _CFUN_H_
#define _CFUN_H_

#define TYPE_WEL 0
// 登陆数据包
#define	TYPE_LOGIN 1   
// 注册数据包
#define	TYPE_REG  2	
// 消息数据包
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
#define TYPE_UPDATE 12
#define TYPE_UPLOAD 13  //从本地上传文件到云端
#define TYPE_DOWNLOAD 14 //从云端下载文件到本地

#define VERSION	"ver 1.0.1"


//协议结构体
#pragma pack(1)
typedef struct message
{
	int msg_type; //协议头
	char name[20]; 
	char pwd[20];
	char buf[1024]; //数据包内容
}MSG;

#pragma pack(4)

//从云端下载文件函数
extern int download_file(int sockfd,char *filename);

//从本地上传文件函数
extern int upload_file(int sockfd,char *filename);

//c/s 直接文件发送函数
extern int file_send(int fd,char *filename,int type);

//c/s 直接文件接收函数
extern int file_rev(int fd);

//进行文件更新
extern int do_update(int sockfd);

//读取来自服务器器消息线程函数
extern void *threadread(void *argc);

//客户端菜单函数
extern void do_service(int sockfd);

//客户端登录函数
extern int client_login(int sockfd);

//用户功能菜单函数
extern void user_menu(int sockfd,char *name);

//shell命名处理函数
extern int shell_cmd(int sockfd);

//发送shell命令函数
extern int send_shell(int sockfd,char *str);

//聊天菜单函数
extern int client_chatmenu(int sockfd,char *name);

//聊天函数
extern int client_chat(int sockfd,char *name);

//聊天设置函数
extern int client_chatset();

//聊天命令分解函数
extern int talk(char *talkcmd,int sockfd,char *name);

//查看当前在线用户
extern int onlineusr_check(int sockfd);

//客户端注册函数
extern int client_reg(int sockfd);

//数据包解码读取
extern int myread(struct message *msg,int sockfd);

//数据包封装发送
extern int mywrite(int type,char *name,char *pwd,char *buf,int sockfd);

//密码不回显
extern int new_getpass(char *dest, int maxlen);

#endif
