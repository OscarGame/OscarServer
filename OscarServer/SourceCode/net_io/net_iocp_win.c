#include<stdio.h>
#include<string.h>
#include <stdlib.h>

#include <WinSock2.h>
#include <mswsock.h>

#include <windows.h>


#define my_malloc malloc
#define my_free free
#define my_realloc realloc

#define MAX_PKG_SIZE ((1<<16) - 1)
#define MAX_RECV_SIZE 2047

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "odbccp32.lib")

#include "../server_gateway.h"

static LPFN_ACCEPTEX lpfnAcceptEx;
static LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs;

struct io_package {
	WSAOVERLAPPED overlapped;
	WSABUF wsabuffer;

	int opt; // 标记一下我们当前的请求的类型;
	int accpet_sock;
	int recved; // 收到的字节数;
	unsigned char* long_pkg;
	int max_pkg_len;
	unsigned char pkg[MAX_RECV_SIZE + 1];
};

void start_server(char* ip, int port, int socket_type, int protocal_type)
{
	init_server_gateway();
	init_session_manager(socket_type, protocal_type);

	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data);

	// 新建一个完成端口;
	SOCKET l_sock = INVALID_SOCKET;
	HANDLE iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (iocp == NULL) {
		goto failed;
	}

	// 创建一个线程
	// CreateThread(NULL, 0, ServerWorkThread, (LPVOID)iocp, 0, 0);
	// end

	l_sock = socket(AF_INET,SOCK_STREAM,0);
	if (l_sock == INVALID_SOCKET) {
		goto failed;
	}

	struct sockaddr_in s_address;
	memset(&s_address, 0, sizeof(s_address));
	s_address.sin_family = AF_INET;
	s_address.sin_addr.s_addr = inet_addr(ip);
	s_address.sin_port = htons(port);

	if (bind(l_sock,(struct sockaddr*)&s_address,sizeof(s_address)) != 0 )
	{
		goto failed;

	}

	if (listen(l_sock, SOMAXCONN) != 0) {
		goto failed;
	}

	DWORD dwBytes = 0;
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	WSAIoctl(l_sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidAcceptEx, sizeof(guidAcceptEx), &lpfnAcceptEx, sizeof(lpfnAcceptEx),
		&dwBytes, NULL, NULL);

	dwBytes = 0;
	GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
	if (0 != WSAIoctl(l_sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidGetAcceptExSockaddrs,
		sizeof(guidGetAcceptExSockaddrs),
		&lpfnGetAcceptExSockaddrs,
		sizeof(lpfnGetAcceptExSockaddrs),
		&dwBytes, NULL, NULL))
	{
	}


	CreateIoCompletionPort((HANDLE)l_sock,iocp,(DWORD)0,0);

	struct io_package* pkg = my_malloc(sizeof(struct io_package));


failed:
	if (iocp != NULL) {
		CloseHandle(iocp);
	}

	if (l_sock != INVALID_SOCKET) {
		closesocket(l_sock);
	}

	exit_session_manager();
	exit_server_gateway();

	WSACleanup();

}