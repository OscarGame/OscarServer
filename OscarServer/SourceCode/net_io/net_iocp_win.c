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
#define MAX_RECV_SIZE 16//2047

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "odbccp32.lib")

#include "../server_gateway.h"
#include "tcp_session.h"
#include "../net_io/game_protocol.h"

#include "../3rd/http_parser/http_parser.h"

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


extern void on_bin_protocal_recv_entry(struct session* s, unsigned char* data, int len);

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

int isLoop;

void trim(char *strIn, char *strOut) {

	int i, j;

	i = 0;

	j = strlen(strIn) - 1;

	while (strIn[i] == ' ')
		++i;

	while (strIn[j] == ' ')
		--j;
	strncpy(strOut, strIn + i, j - i + 1);
	strOut[j - i + 1] = '\0';
}

static void on_bin_protocal_recved(struct session* s,struct io_package* io_data)
{
	printf("IOCP_RECV on_bin_protocal_recved io_data->recved size = %d\n", io_data->recved);

	while (io_data->recved)
	{
		int pkg_size = 0;
		unsigned char* pkg_data = io_data->long_pkg == NULL ? io_data->pkg : io_data->long_pkg;
		if (get_recv_header_size(pkg_data, io_data->recved, &pkg_size) != 0)
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

				printf("IOCP_RECV on_bin_protocal_recved package size = %d \n", pkg_size);
				/*
				
				char* content = malloc((size_t)pkg_size-2);
				//io_data->pkg 不是刚才指定的long_pkg??我让wsabuffer.buf 指向了pkg_long 
				//这里+2的意思是要移动两个位置，因为这两个位置是包的大小
				if (io_data->long_pkg != NULL)
				{
					memcpy(content, io_data->long_pkg+2, pkg_size-2);
				}
				else
				{
					memcpy(content, io_data->pkg+2, pkg_size-2);
				}
				printf("IOCP_RECV package content = %s \n", content);
				*/

				/*
				在这里进行数据的解析
				struct user_login_req req;
				command_login_unpack(pkg_data+2+4, pkg_size-2-4, &req);

				// 查数据库，验证密码，获取用户资料；
				printf("user login %s:%s %d\n", req.uname, req.upsd, req.channel);

				
				struct user_login_respons res;
				res.level = 1;
				res.status = 1; // OK
				res.name = "小红";
				// end 

				char send_buf[256];
				int send_len = login_respons_pack(USER_LOGION, &res, send_buf);

				binary_session_send(s, send_buf, send_len);

				*/

				//前面两个字节是数据包的大小，
				on_bin_protocal_recv_entry(s, pkg_data + 2, pkg_size - 2);
				if (io_data->recved > pkg_size)
				{
					memmove(io_data->pkg, io_data->pkg + pkg_size, io_data->recved - pkg_size);
				}

				io_data->recved -= pkg_size;

				if (io_data->long_pkg != NULL)
				{
					my_free(io_data->long_pkg);
					io_data->long_pkg = NULL;
				}


				if (io_data->recved == 0) { // 重新投递请求
					DWORD dwRecv = 0;
					DWORD dwFlags = 0;
					io_data->wsabuffer.buf = io_data->pkg + io_data->recved;
					io_data->wsabuffer.len = MAX_RECV_SIZE - io_data->recved;

					int ret = WSARecv(s->c_sock, &(io_data->wsabuffer),
						1, &dwRecv, &dwFlags,
						&(io_data->overlapped), NULL);
					break;
				}


			}  //没有接收完一个包
			else
			{
				unsigned char* recv_buffer = io_data->pkg;

				if (pkg_size > MAX_RECV_SIZE)
				{
					if (io_data->long_pkg == NULL)
					{

						//pkg_size - io_data->recved 只是还没有收完的大小，但是这个long_pkg的大小需要是整个包的大小，
						//所以这里可以理解成一部分装当前的，一部分装剩余的，
						//这里+1的逻辑不是很清楚了，反正最后也释放了
						//io_data->long_pkg = my_malloc(pkg_size - io_data->recved);
						io_data->long_pkg = my_malloc(pkg_size + 1);
						memcpy(io_data->long_pkg,io_data->pkg,io_data->recved);
					}
					recv_buffer = io_data->long_pkg;
				}

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

static int read_pkg_tail(unsigned char* pkg_data,int recv,int*pkg_size)
{
	if (recv < 2)
	{
		return -1;
	}

	int i = 0;
	*pkg_size = 0;
	while (i < recv -1)
	{
		if (pkg_data[i] == '\r' && pkg_data[i+1] == '\n')
		{
			*pkg_size = i + 2;
			return 0;
		}
		i++;
	}

	return -1;
}

static void on_json_protocal_recved(struct session* s, struct io_package* io_data)
{
	while (io_data->recved > 0)
	{
		int pkg_size = 0;
		unsigned char* pkg_data = io_data->long_pkg == NULL ? io_data->pkg : io_data->long_pkg;
		if (read_pkg_tail(pkg_data, io_data->recved, &pkg_size) != 0)
		{
			//数据包没有接受完成

			if (io_data->recved >= MAX_PKG_SIZE) { // 超过了数据包,close session
				close_session(s);
				my_free(io_data);
				break;
			}


			if (io_data->recved >= io_data->max_pkg_len)
			{
				int alloc_len = io_data->recved * 2;
				alloc_len == (alloc_len > MAX_PKG_SIZE) ? MAX_PKG_SIZE : alloc_len;

				if (io_data->long_pkg ==NULL)
				{
					io_data->long_pkg = my_malloc(alloc_len +1);
					memcpy(io_data->long_pkg,io_data->pkg,io_data->recved);
				}
				else
				{
					//my_realloc 不用删除旧的iodata->long_pkg
					io_data->long_pkg = my_realloc(io_data->long_pkg,alloc_len +1);
				}

				io_data->max_pkg_len = alloc_len;

				DWORD dwRecv = 0;
				DWORD dwFlags = 0;
				unsigned char* buf = (io_data->long_pkg != NULL) ? io_data->long_pkg : io_data->pkg;
				io_data->wsabuffer.buf = buf + io_data->recved;
				io_data->wsabuffer.len = io_data->max_pkg_len - io_data->recved;
				int ret = WSARecv(s->c_sock, &(io_data->wsabuffer),
					1, &dwRecv, &dwFlags,
					&(io_data->overlapped), NULL);
				break;
			}
			
		}

		//接收完毕
		char* content = malloc((size_t)pkg_size);
		memcpy(content, pkg_data, pkg_size);
		printf("IOCP_RECV Json package content = %s \n", content);


		if (io_data->recved > pkg_size) {
			memmove(pkg_data, pkg_data + pkg_size, io_data->recved - pkg_size);
		}
		io_data->recved -= pkg_size;
	
		if (io_data->recved == 0) { // IOCP的请求
			DWORD dwRecv = 0;
			DWORD dwFlags = 0;
			if (io_data->long_pkg != NULL) {
				my_free(io_data->long_pkg);
				io_data->long_pkg = NULL;

			}

			io_data->max_pkg_len = MAX_RECV_SIZE;
			io_data->wsabuffer.buf = io_data->pkg + io_data->recved;
			io_data->wsabuffer.len = io_data->max_pkg_len - io_data->recved;

			int ret = WSARecv(s->c_sock, &(io_data->wsabuffer),
				1, &dwRecv, &dwFlags,
				&(io_data->overlapped), NULL);
			break;
		}
		

		



	}
}


// 解析到头的回掉函数
static char header_key[64];
static char client_ws_key[128];
static int has_client_key = 0;
static int on_header_field(http_parser*p,const char *at,size_t length)
{
	length = (length < 63) ? length : 63;
	strncpy(header_key, at, length);
	header_key[length] = 0;
	return 0;
}

static int on_header_value(http_parser* p, const char *at,size_t length)
{
	if (strcmp(header_key, "Sec-WebSocket-Key") != 0) {
		return 0;
	}
	length = (length < 127) ? length : 127;
	strncpy(client_ws_key,at,length);
	client_ws_key[length] = 0;
	has_client_key = 1;

	return 0;
}

static int process_ws_shake_hand(struct session* _session, struct io_package* io_data,
	char* ip, int port)
{
	http_parser p;
	http_parser_init(&p,HTTP_REQUEST);

	http_parser_settings setting;
	http_parser_settings_init(&setting);
	setting.on_header_field = on_header_field;
	setting.on_header_value = on_header_value;

	has_client_key = 0;
	http_parser_execute(&p,&_session,io_data->pkg, io_data->recved);

	if (has_client_key == 0)
	{
		_session->is_shake_hand = 0;
		if (io_data->recved >= MAX_RECV_SIZE)
		{
			closesocket(_session->c_sock);
			my_free(io_data);
		}

		

		if (io_data->long_pkg != NULL)
		{
			my_free(io_data->long_pkg);
			io_data->long_pkg = NULL;
		}
		DWORD dwRecv = 0, dwFlags = 0;
		io_data->max_pkg_len = MAX_RECV_SIZE;
		io_data->wsabuffer.buf = io_data->pkg + io_data->recved;
		io_data->wsabuffer.len = io_data->max_pkg_len - io_data->recved;

		int ret = WSARecv(_session->c_sock,&io_data->wsabuffer,1,&dwRecv,&dwFlags,&io_data->overlapped,NULL);

		return -1;
	}

	static char key_migic[256];
	const char* migic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	sprintf(key_migic,"%s%s",client_ws_key,migic);

	return 0;

}


void start_server(char* ip, int port, int socket_type, int protocal_type)
{

	init_session_manager(socket_type, protocal_type);

	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data);

	// 新建一个完成端口;
	SOCKET l_sock = INVALID_SOCKET;
	HANDLE iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (iocp == NULL) {
		printf("iocp error\n");
		goto failed;
	}

	// 创建一个线程
	// CreateThread(NULL, 0, ServerWorkThread, (LPVOID)iocp, 0, 0);
	// end

	l_sock = socket(AF_INET,SOCK_STREAM,0);
	if (l_sock == INVALID_SOCKET) {
		printf("l_sock error\n");
		goto failed;
	}

	struct sockaddr_in s_address;
	memset(&s_address, 0, sizeof(s_address));
	s_address.sin_family = AF_INET;
	s_address.sin_addr.s_addr = inet_addr(ip);
	s_address.sin_port = htons(port);

	if (bind(l_sock,(struct sockaddr*)&s_address,sizeof(s_address)) != 0 )
	{
		printf("bind error\n");
		goto failed;

	}

	if (listen(l_sock, SOMAXCONN) != 0) {
		printf("listen error\n");
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
				else if (socket_type == WEB_SOCKET_IO)
				{
					if (s->is_shake_hand == 0)
					{
						process_ws_shake_hand(s, io_data, ip, port);
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

					//struct session* s = My_save_session(client_fd, inet_ntoa(r_addr->sin_addr), ntohs(r_addr->sin_port));

					struct session* s = save_session(client_fd, inet_ntoa(r_addr->sin_addr), ntohs(r_addr->sin_port));
					CreateIoCompletionPort((HANDLE)client_fd, iocp, (DWORD)s, 0);
					post_recv(client_fd, iocp);
					post_accept(l_sock, iocp, io_data);
				}
				break;
		}


	}






	if (iocp != NULL) {
		CloseHandle(iocp);
	}

	if (l_sock != INVALID_SOCKET) {
		closesocket(l_sock);
	}

	exit_session_manager();
	exit_server_gateway();

	WSACleanup();

failed:
		exit_session_manager();
		exit_server_gateway();
//	
//	if (l_sock != INVALID_SOCKET) {
//		closesocket(l_sock);
//	}
//
//	if (iocp != NULL) {
//		CloseHandle(iocp);
//	}
//
//
//	exit_session_manager();
//	exit_server_gateway();
//
//	WSACleanup();

}