#ifndef _LINK_H_
#define _LINK_H_

#include "mythread.h"

//c/s 直接文件发送函数
extern int file_send(int fd,char *true_filename,char *filename,int type);

//c/s 直接文件接收函数
extern int file_rev(int fd,char *filename);

//用户链表加载函数
extern int load_client(CLIENT *client_head);

//用户链表保存函数
extern int save_client(CLIENT *client_head);

//用户链表尾部插入函数
extern int client_link_after(CLIENT *client_head,CLIENT *newclient);

//用户链表释放
extern int free_client(CLIENT *client_head);

//会话链表释放
extern int free_info(INFO *info_head);

//在线用户链表释放
extern int free_clientol(COL *clientol_head);

//在线用户链表添加函数
extern int clientol_link_after(COL *clientol_head,char *name,int lisfd);

//用户登录检测是否已登录
extern int clientol_link_checkname(COL *clientol_head,char *name);

//用户下线在线链表删除函数
extern int clientol_link_delete(COL *clientol_head,char *name);

//会话链表添加
extern int connect_info_after(INFO *info_head,char *name,int lisfd,int reason);

//日志保存文件
extern int log_write(char *logbuf);

//数据包转化
extern int log_dis(MSG msg,int lisfd,char *name);

#endif
