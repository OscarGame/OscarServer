#include "web_handler.h"

struct {
	struct web_handler w_handler_set[1024];
	int handler_count; // ¸öÊı;
}H_MAN;

void init_web_handler_manager()
{
	memset(&H_MAN,0,sizeof(H_MAN));
}

void register_web_handler(const char* url, void(*get)(int sock, char* params), void(*post)(int sock, char* params, char* body))
{
	strcpy(H_MAN.w_handler_set[H_MAN.handler_count].handler_url, url);
	H_MAN.w_handler_set[H_MAN.handler_count].get = get;
	H_MAN.w_handler_set[H_MAN.handler_count].post = post;
	H_MAN.handler_count++;
}

struct web_handler* find_web_handler(const char* url, int len)
{
	for (int i =0;i < H_MAN.handler_count;i++)
	{
		if (strncmp(url,H_MAN.w_handler_set[i].handler_url,len) == 0)
		{
			return &H_MAN.w_handler_set[i];
		}
	}
}