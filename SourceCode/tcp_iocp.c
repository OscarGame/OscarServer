#include<stdio.h>
#include<string.h>
#include <stdlib.h>

#include "tcp_iocp.h"

#include <WinSock2.h>
#include <mswsock.h>

#include <windows.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "WSOCK32.LIB")
#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "odbccp32.lib")

#include "tcp_session.h"
//#include "webserver.h"

#include "3rd/http_parser/http_parser.h"
#include "3rd/crypt/sha1.h"
#include "3rd/crypt/base64_encoder.h"


enum {
	IOCP_ACCPET = 0,
	IOCP_RECV,
	IOCP_WRITE,
};

#define MAX_RECV_SIZE 8192
struct io_package {
	WSAOVERLAPPED overlapped;
	int opt; // ���һ�����ǵ�ǰ�����������;
	int accpet_sock;
	WSABUF wsabuffer;
	unsigned char pkg[MAX_RECV_SIZE];
};

static void 
post_accept(SOCKET l_sock, HANDLE iocp) {
	struct io_package* pkg = malloc(sizeof(struct io_package));
	memset(pkg, 0, sizeof(struct io_package));

	pkg->wsabuffer.buf = pkg->pkg;
	pkg->wsabuffer.len = MAX_RECV_SIZE - 1;
	pkg->opt = IOCP_ACCPET;

	DWORD dwBytes = 0;
	SOCKET client = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	int addr_size = (sizeof(struct sockaddr_in) + 16);
	pkg->accpet_sock = client;

	AcceptEx(l_sock, client, pkg->wsabuffer.buf, 0/*pkg->wsabuffer.len - addr_size* 2*/,
		addr_size, addr_size, &dwBytes, &pkg->overlapped);
}

static void 
post_recv(SOCKET client_fd, HANDLE iocp) {
	// �첽��������;
	// ʲô���첽? recv 8K���ݣ��������ʱ��û�����ݣ�
	// ��ͨ��ͬ��(����)�̹߳��𣬵ȴ����ݵĵ���;
	// �첽�������û�����ݷ�����Ҳ�᷵�ؼ���ִ��;
	struct io_package* io_data = malloc(sizeof(struct io_package));
	// ��0����ҪĿ����Ϊ������overlapped��0;
	memset(io_data, 0, sizeof(struct io_package));

	io_data->opt = IOCP_RECV;
	io_data->wsabuffer.buf = io_data->pkg;
	io_data->wsabuffer.len = MAX_RECV_SIZE - 1;

	// ������recv������;
	// 
	DWORD dwRecv = 0;
	DWORD dwFlags = 0;
	int ret = WSARecv(client_fd, &(io_data->wsabuffer),
		1, &dwRecv, &dwFlags,
		&(io_data->overlapped), NULL);
}

void 
start_server(int port) {
	init_session_manager(1);

	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data);
	// �½�һ����ɶ˿�;
	SOCKET l_sock = INVALID_SOCKET;
	HANDLE iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (iocp == NULL) {
		goto failed;
	}

	// ����һ���߳�
	// CreateThread(NULL, 0, ServerWorkThread, (LPVOID)iocp, 0, 0);
	// end

	// ��������socket
	l_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (l_sock == INVALID_SOCKET) {
		goto failed;
	}
	// bind socket
	struct sockaddr_in s_address;
	memset(&s_address, 0, sizeof(s_address));
	s_address.sin_family = AF_INET;
	s_address.sin_addr.s_addr = inet_addr("127.0.0.1");
	s_address.sin_port = htons(port);

	if (bind(l_sock, (struct sockaddr *) &s_address, sizeof(s_address)) != 0) {
		goto failed;
	}

	if (listen(l_sock, SOMAXCONN) != 0) {
		goto failed;
	}

	// start 
	CreateIoCompletionPort((HANDLE)l_sock, iocp, (DWORD)0, 0);
	post_accept(l_sock, iocp);
	// end 
	
	DWORD dwTrans;
	struct session* s;
	//  ������������¼��������Ժ�,
	// GetQueuedCompletionStatus �᷵�����������
	// ʱ���WSAOVERLAPPED �ĵ�ַ,���������ַ���ҵ�
	// io_data, �ҵ���io_data,Ҳ����ζ�������ҵ���,
	// ���Ļ�������
	struct io_package* io_data;

	while (1) {
		clear_offline_session();
		// ������������IOCP��������߳��������Ѿ������¼�
		// ��ʱ�򣬲Ż������̻߳���;
		// IOCP ��select��һ�����ȴ�����һ����ɵ��¼�;
		s = NULL;
		dwTrans = 0;
		io_data = NULL;
		int ret = GetQueuedCompletionStatus(iocp, &dwTrans, (LPDWORD)&s, (LPOVERLAPPED*)&io_data, WSA_INFINITE);
		if (ret == 0) {
			printf("iocp error");
			continue;
		}
		// IOCP�˿ڻ�����һ�������̣߳�
		// �������û���socket������¼�������;
		// printf("IOCP have event\n");
		if (dwTrans == 0 && io_data->opt == IOCP_RECV) { // socket �ر�
			close_session(s);
			free(io_data);
			continue;
		}// end

		switch (io_data->opt) {
			case IOCP_RECV: { // ��ɶ˿���ζ�������Ѿ�����
				io_data->pkg[dwTrans] = 0;
				// printf("IOCP recv %d��%s\n", dwTrans, io_data->pkg);
				ws_process_request(s->c_sock, io_data->pkg, dwTrans);
				// ���Ϲر��������;
				close_session(s);
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

				GetAcceptExSockaddrs(io_data->wsabuffer.buf, 
					0, /*io_data->wsabuffer.len - addr_size * 2, */
					addr_size, addr_size, 
					(struct sockaddr**)&l_addr, &l_len,
					(struct sockaddr**)&r_addr, &r_len);

				struct session* s = save_session(client_fd, inet_ntoa(r_addr->sin_addr), ntohs(r_addr->sin_port));
				CreateIoCompletionPort((HANDLE)client_fd, iocp, (DWORD)s, 0);
				post_recv(client_fd, iocp);
				post_accept(l_sock, iocp);
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
	WSACleanup();
}

