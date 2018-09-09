
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include<winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "odbccp32.lib")

#include "server_gateway.h"
#include "gateway_server/from_client.h"
#include "game_stype.h"

struct Node
{
	char* content;
	struct Node* next;
};

int main(int argc, char** argv) 
{
	/*register_from_client_module()*/
	init_server_gateway();
	

	register_from_client_module(STYPE_CENTER);

	start_server("127.0.0.1", 8000, TCP_SOCKET_IO, BIN_PROTOCAL);

	return 0;
}