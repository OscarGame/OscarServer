#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#ifdef WIN32
#include "winsock2.h"
#endif

#include "web_handler.h"

#include "3rd/http_parser/http_parser.h"

#define TEST "D:\\Work\\OscarWorkSpace\\Server\\OscarServer\\www_root\\test.html"
#define INDEX "D:\\Work\\OscarWorkSpace\\Server\\OscarServer\\www_root\\index.html"

struct
{
	char url[1024];
}WS_HTTP;

static void init_ws_params()
{
	memset(&WS_HTTP,0,sizeof(WS_HTTP));
}

static int on_url(http_parser* p, const char *at, size_t length)
{

	printf("Url: %.*s\n", (int)length, at);

	strncpy(WS_HTTP.url,at,length);
	WS_HTTP.url[length] = 0;
	return 0;
}

static int filter_url(char* url)
{
	int len = 0;
	char* walk = url;
	while ((*walk) && (*walk) != '?') {
		walk++;
		len++;
	}
	return len;
}

static char*
open_file(char* file_name) {
	FILE* f = fopen(file_name, "rb");
	int file_size = 0;
	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	fseek(f, 0, 0);

	char* file_data = malloc(file_size + 1);
	fread(file_data, 1, file_size, f);
	file_data[file_size] = 0;
	fclose(f);

	return file_data;
}

static void
write_ok_identity(int sock, char* body) {
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
	// end

	// Step3: 写我们的body的长度
	walk = walk + strlen(walk);
	sprintf(walk, "content-length: %d\r\n\r\n", strlen(body));
	// end

	// Step4: 写入我们的数据
	walk = walk + strlen(walk);
	sprintf(walk, "%s", body);
	// end

	// Step5:回数据给客户端
	send(sock, send_line, strlen(send_line), 0);
	// 
}

void
ws_process_request(int sock, const char* http_req, int len) {
	// 解析我们的http报文
	http_parser p;
	http_parser_init(&p, HTTP_REQUEST);

	http_parser_settings s;
	http_parser_settings_init(&s);

	s.on_url = on_url; // 解析到了url,就会回掉这个函数;
					   // end 
	init_ws_params(); // 重置我们的解析信息;
	http_parser_execute(&p, &s, http_req, len);

	switch (p.method) { // 报文的响应的方式:
	case HTTP_GET: {
		int len = filter_url(WS_HTTP.url);
		// 决定了要访问哪一个网页
		if (strncmp("/test", WS_HTTP.url, len) == 0) {
			char* page = open_file(TEST);
			write_ok_identity(sock, page);
			free(page);
		}
		// end 
		else if (strncmp("/index", WS_HTTP.url, len) == 0) {
			char* page = open_file(INDEX);
			write_ok_identity(sock, page);
			free(page);
		}
		//else { // 函数处理
		//	struct web_handler* handler = find_web_handler(WS_HTTP.url, len);
		//	if (handler && handler->get != NULL) {
		//		handler->get(sock, WS_HTTP.url + len);
		//	}
		//}
	}
				   break;

	case HTTP_POST:
		break;
	}
}




