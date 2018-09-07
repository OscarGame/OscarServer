
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "server_gateway.h"
	

#include<winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "odbccp32.lib")

struct Node
{
	char* content;
	struct Node* next;
};

int main(int argc, char** argv) 
{
	// start_tcp_listener(5150);
	//start_server(5150);
	
	start_server("127.0.0.1", 8000, TCP_SOCKET_IO,BIN_PROTOCAL);

	//char* conet = "oscar=";
	//printf("conet = %s",conet);




	return 0;
}