
//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
//
//#include "protocol/game_protocol.h"
//
//#ifdef WIN32
//#include<winsock2.h>
//#include <windows.h>
//#pragma comment(lib, "ws2_32.lib")
//#pragma comment(lib, "odbc32.lib")
//#pragma comment(lib, "odbccp32.lib")
///*
//ws2_32.lib
//odbc32.lib
//odbccp32.lib
//*/
//#endif
//
//
//int main(int argc, char** argv) {
//
//#ifdef WIN32
//	WORD wVersionRequested;
//	WSADATA wsaData;
//	int err;
//	wVersionRequested = MAKEWORD(2, 2);
//
//	err = WSAStartup(wVersionRequested, &wsaData);
//	if (err != 0) {
//		return -1;
//	}
//#endif
//
//	int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//	if (s == INVALID_SOCKET) {
//		return -1;
//	}
//	struct sockaddr_in sockaddr;
//	sockaddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
//	sockaddr.sin_family = AF_INET;
//	// sockaddr.sin_port = htons(6000);
//	// sockaddr.sin_port = htons(5150);
//	sockaddr.sin_port = htons(8000);
//
//	int ret = connect(s, ((struct sockaddr*) &sockaddr), sizeof(sockaddr));
//	if (ret != 0) {
//		printf("connect error\n");
//		closesocket(s);
//		system("pause");
//		goto out;
//		return -1;
//	}
//
//	// send(s, "hello", 5, 0);
//
//	int isSendBin = 1;
//	if (isSendBin)
//	{
//
//		char send_buf[4096];
//		sprintf(send_buf, "%s", "hello oscarhello oscarhello oscarhello oscarhello oscar====");
//		int len = strlen(send_buf);
//		unsigned char* pkg_ptr = malloc(len + 2);
//		memcpy(pkg_ptr + 2, send_buf, len);
//		pkg_ptr[0] = ((len + 2) & 0x000000ff);
//		pkg_ptr[1] = ((len + 2) & 0x0000ff00) >> 8;
//		send(s, pkg_ptr, len + 2, 0);
//	}
//	else
//	{
//		//send json
//		char send_buf[4096];
//		sprintf(send_buf, "%s", "{helloWorldtestqqqqqqsssss}\r\n");
//		int len = strlen(send_buf);
//		send(s, send_buf, 16, 0);
//		Sleep(1);
//		send(s, send_buf + 16, len - 16, 0);
//		Sleep(1);
//
//	}
//
//	struct user_login_req req;
//	req.channel = 1;
//	req.uname = "xiaohong";
//	req.upsd = "123456"; // MD5
//
//
//
//						 /*int len = command_login_pack(USER_LOGION, &req, send_buf + 2);
//						 (*(unsigned short*)send_buf) = (len + 2);
//						 send(s, send_buf, len + 2, 0);
//						 */
//
//
//#if 1
//
//#else
//	send(s, send_buf, len, 0);
//#endif
//	/*len = recv(s, send_buf, 4096, 0);
//
//	struct user_login_respons respons;
//	login_respons_unpack(send_buf + 2 + 4, len - 2 - 4, &respons);
//
//	printf("respons = %s\n", respons.name);*/
//
//	printf("connect success\n");
//	system("pause");
//
//
//
//	closesocket(s);
//out:
//#ifdef WIN32
//	WSACleanup();
//#endif
//	return 0;
//}

