/*
* file: TCPClient.h
* brief: Created by kylin
*/

#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdint.h>
#include <google/protobuf/message_lite.h>

#define MAX_DOMAIN_SIZE							64
#define MAX_NETWORK_PACKAGE_SIZE                65535

typedef ::google::protobuf::MessageLite  Message;

//协议结构体
#pragma pack(1)
typedef struct message_head
{
	uint16_t msgid;
	uint32_t userid;
	uint32_t size;

}MY_MSG_HEAD;

struct network_address
{
	uint32_t				                ip;
	char									domain[MAX_DOMAIN_SIZE];
	uint16_t				                port;
};

#pragma pack(4)

#endif