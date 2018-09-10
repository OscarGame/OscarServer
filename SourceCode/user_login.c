
#include  <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "user_login.h"

#ifdef WIN32
#include <WinSock2.h>
#endif

static void 
write_ok_identity(int socket,char* body)
{
	static char send_line[8096];
	int status_code = 200;
	const char* status_desic = "OK";
	char* walk = send_line;
	// Step1: 写我们的状体行;
	sprintf(walk, "HTTP/1.1 %d %s\r\n", status_code, status_desic);
	// end 

	// Step2: 写我们的头信息，包含两个，body的传送模式，以及我们的body的长度;
	walk = walk + strlen(walk);
	sprintf(walk, "transfer-encoding:%s\r\n", "identity");

	// Step3: 写我们的body的长度
	walk = walk + strlen(walk);
	sprintf(walk, "content-length: %d\r\n\r\n", strlen(body));
	// end

	// Step4: 写入我们的数据
	walk = walk + strlen(walk);
	sprintf(walk, "%s", body);
	// end

	// Step5:回数据给客户端
	send(socket, send_line, strlen(send_line), 0);
	// 

}

void
user_login_get(int sock, char* param)
{
	printf("user_name=%s\n", param);

	// 回复
	write_ok_identity(sock, "SUCCESS");
}