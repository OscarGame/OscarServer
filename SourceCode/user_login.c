
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
	// Step1: д���ǵ�״����;
	sprintf(walk, "HTTP/1.1 %d %s\r\n", status_code, status_desic);
	// end 

	// Step2: д���ǵ�ͷ��Ϣ������������body�Ĵ���ģʽ���Լ����ǵ�body�ĳ���;
	walk = walk + strlen(walk);
	sprintf(walk, "transfer-encoding:%s\r\n", "identity");

	// Step3: д���ǵ�body�ĳ���
	walk = walk + strlen(walk);
	sprintf(walk, "content-length: %d\r\n\r\n", strlen(body));
	// end

	// Step4: д�����ǵ�����
	walk = walk + strlen(walk);
	sprintf(walk, "%s", body);
	// end

	// Step5:�����ݸ��ͻ���
	send(socket, send_line, strlen(send_line), 0);
	// 

}

void
user_login_get(int sock, char* param)
{
	printf("user_name=%s\n", param);

	// �ظ�
	write_ok_identity(sock, "SUCCESS");
}