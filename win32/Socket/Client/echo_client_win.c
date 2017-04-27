#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

#define BUF_SIZE 1024
void Echo_ErrorHandling(char* message);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hSocket;
	SOCKADDR_IN servAddr;

	int strLen;
	char message[BUF_SIZE];

	if (argc != 3)
	{
		printf("Usage:%s <ip> <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		Echo_ErrorHandling("WSAStartup() error!");

	hSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hSocket == INVALID_SOCKET)
	{
		Echo_ErrorHandling("socket() error");
	}

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(argv[1]);
	servAddr.sin_port = htons(atoi(argv[2]));

	if (connect(hSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
	{
		Echo_ErrorHandling("connect() error");
	}

	while (1)
	{
		fputs("Please input message:(Q to quit)", stdout);
		fgets(message, BUF_SIZE, stdin);

		if (!strcmp(message,"q\n") || !strcmp(message,"Q\n"))
		{
			break;
		}

		send(hSocket, message, strlen(message), 0);

		strLen = recv(hSocket, message, BUF_SIZE - 1, 0);
		if (strLen == -1)
		{
			Echo_ErrorHandling("recv() error");
		}
		message[strLen] = '\0';
		printf("Message from server:%s\n", message);
	}



	closesocket(hSocket);

	WSACleanup();
	return 0;

}

void Echo_ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}