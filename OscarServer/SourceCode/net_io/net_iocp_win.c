/*
https://blog.csdn.net/piggyxp/article/details/6922277
https://blog.csdn.net/u010025913/article/details/24467351
*/
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
#define MAX_RECV_SIZE 8//2047

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "odbccp32.lib")

#include "../server_gateway.h"
#include "tcp_session.h"

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

enum {
	IOCP_ACCPET = 0,
	IOCP_RECV,
	IOCP_WRITE,
};


static void post_accept(SOCKET l_sock,HANDLE iocp,struct io_package* pkg)
{
	pkg->wsabuffer.buf = pkg->pkg;
	pkg->wsabuffer.len = MAX_RECV_SIZE;
	pkg->opt = IOCP_ACCPET;

	DWORD dwBytes = 0;
	SOCKET client = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);
	int addr_size = (sizeof(struct sockaddr_in)) + 16;

	pkg->accpet_sock = client;
	printf("accpet client %d\n", client);

	lpfnAcceptEx(l_sock, client, pkg->wsabuffer.buf, 0/*pkg->wsabuffer.len - addr_size* 2*/,
		addr_size, addr_size, &dwBytes, &pkg->overlapped);
}

static void post_recv(SOCKET client_fd,HANDLE iocp)
{
	struct io_package* io_data = my_malloc(sizeof(struct io_package));
	memset(io_data,0,sizeof(struct io_package));

	io_data->opt = IOCP_RECV;
	io_data->wsabuffer.buf = io_data->pkg;
	io_data->wsabuffer.len = MAX_RECV_SIZE;
	io_data->max_pkg_len = MAX_RECV_SIZE;

	DWORD dwRecv = 0;
	DWORD dwFlags = 0;
	int ret = WSARecv(client_fd, &(io_data->wsabuffer),
		1, &dwRecv, &dwFlags,
		&(io_data->overlapped), NULL);
}


static int get_recv_header_size(unsigned char*pkgContent,int len,int *pkgSize)
{
	if (len <= 1)
	{
		return -1;
	}

	*pkgSize = (pkgContent[0] | pkgContent[1] << 8);
	return 0;
}

static void on_bin_protocal_recved(struct session* s,struct io_package* io_data)
{
	printf("IOCP_RECV io_data->recved size = %d\n", io_data->recved);

	while (io_data->recved)
	{
		int pkg_size = 0;
		if (get_recv_header_size(io_data->pkg, io_data->recved, &pkg_size) != 0)
		{
			printf("IOCP_RECV package size error");
			//一个数据头都没有收到说明不足以去解析，所以继续投递，但是有假如收到了一个或者几个字节
			//，所以接受的buf len是减去已经接收的了
			DWORD dwRecv = 0;
			DWORD dwFlags = 0;

			io_data->wsabuffer.buf = io_data->pkg + io_data->recved;
			io_data->wsabuffer.len = MAX_RECV_SIZE - io_data->recved;

			int ret = WSARecv(s->c_sock, &(io_data->wsabuffer),
				1, &dwRecv, &dwFlags,
				&(io_data->overlapped), NULL);
			break;
		}
		else
		{

			if (pkg_size >= MAX_PKG_SIZE)
			{
				close_session(s);
				my_free(io_data);
				io_data = NULL;
			}
			//至少接收了一个包
			if (io_data->recved >= pkg_size)
			{

				printf("IOCP_RECV package size = %d \n", pkg_size);
				char* content = malloc((size_t)pkg_size);

				//io_data->pkg 不是刚才指定的long_pkg??我让wsabuffer.buf 指向了pkg_long 
				if (io_data->long_pkg != NULL)
				{
					memcpy(content, io_data->long_pkg + 2, pkg_size);
				}
				else
				{
					memcpy(content, io_data->pkg + 2, pkg_size);
				}
				printf("IOCP_RECV package content = %s \n", content);

				if (io_data->recved > pkg_size)
				{
					memmove(io_data->pkg,io_data->pkg + pkg_size,io_data->recved - pkg_size);
				}

				io_data->recved -= pkg_size;

				if (io_data->long_pkg != NULL)
				{
					my_free(io_data->long_pkg);
					io_data->long_pkg = NULL;
				}

				//这段不需要，如果要的话，在下面这里会有错
				/*
				int ret = GetQueuedCompletionStatus(iocp, &dwTrans, (LPDWORD)&s, (LPOVERLAPPED*)&io_data, WSA_INFINITE);
				if (ret == 0) {
					printf("===iocp error===");
					continue;
				}
				*/
				//if (io_data->recved == 0) { // 重新投递请求
				//	DWORD dwRecv = 0;
				//	DWORD dwFlags = 0;
				//	io_data->wsabuffer.buf = io_data->pkg + io_data->recved;
				//	io_data->wsabuffer.len = MAX_RECV_SIZE - io_data->recved;

				//	int ret = WSARecv(s->c_sock, &(io_data->wsabuffer),
				//		1, &dwRecv, &dwFlags,
				//		&(io_data->overlapped), NULL);
				//	break;
				//}


			}  //没有接收完一个包
			else
			{
				unsigned char* recv_buffer = io_data->pkg;

				if (pkg_size > MAX_RECV_SIZE)
				{
					if (io_data->long_pkg == NULL)
					{
						io_data->long_pkg = my_malloc(pkg_size + 1);
						memcpy(io_data->long_pkg,io_data->pkg,io_data->recved);
					}
					recv_buffer = io_data->long_pkg;
				}

				/*char* content = malloc(io_data->recved);
				memcpy(content, io_data->pkg + 2, io_data->recved);
				printf("IOCP_RECV package content = %s \n", content);*/


				DWORD dwRecv = 0;
				DWORD dwFlags = 0;
				io_data->wsabuffer.buf = recv_buffer + io_data->recved;
				io_data->wsabuffer.len = pkg_size - io_data->recved;

				int ret = WSARecv(s->c_sock, &(io_data->wsabuffer),
					1, &dwRecv, &dwFlags,
					&(io_data->overlapped), NULL);
				break;
			}
			
		}
	}
	

}

static void on_json_protocal_recved(struct session* s, struct io_package* io_data)
{
	
}
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
	memset(pkg,0,sizeof(struct io_package));

	post_accept(l_sock,iocp,pkg);

	//这个其实是客户端发过来的需要接收的数据大小如果为0 ，那么就是断开连接了
	DWORD dwTrans; 
	struct session*s;
	struct io_package* io_data;

	while (1)
	{
		//clear_offline_session();
		s = NULL;
		io_data = NULL;
		int ret = GetQueuedCompletionStatus(iocp, &dwTrans, (LPDWORD)&s, (LPOVERLAPPED*)&io_data, WSA_INFINITE);
		if (ret == 0) {
			printf("===iocp error===");
			continue;
		}


		if (dwTrans == 0 && io_data->opt == IOCP_RECV) { // socket 关闭
			close_session(s);
			my_free(io_data);
			continue;
		}

		switch (io_data->opt)
		{
			case IOCP_RECV:
				io_data->recved += dwTrans;
				if (socket_type == TCP_SOCKET_IO)
				{
					if (protocal_type == JSON_PROTOCAL)
					{
						on_json_protocal_recved(s, io_data);
					}
					else if (protocal_type == BIN_PROTOCAL)
					{
						on_bin_protocal_recved(s,io_data);
					}

				}
				
				

				break;

			case IOCP_ACCPET:
				{
					int client_fd = io_data->accpet_sock;
					int addr_size = (sizeof(struct sockaddr_in) + 16);
					struct sockaddr_in* l_addr = NULL;
					int l_len = sizeof(struct sockaddr_in);

					struct sockaddr_in* r_addr = NULL;
					int r_len = sizeof(struct sockaddr_in);

					lpfnGetAcceptExSockaddrs(io_data->wsabuffer.buf,
						0, /*io_data->wsabuffer.len - addr_size * 2, */
						addr_size, addr_size,
						(struct sockaddr**)&l_addr, &l_len,
						(struct sockaddr**)&r_addr, &r_len);

					struct session* s = save_session(client_fd, inet_ntoa(r_addr->sin_addr), ntohs(r_addr->sin_port));
					CreateIoCompletionPort((HANDLE)client_fd, iocp, (DWORD)s, 0);
					post_recv(client_fd, iocp);
					post_accept(l_sock, iocp, io_data);
				}
				break;
		}


	}

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