#pragma 

struct web_handler
{
	char* handler_url[1024];
	void(*get)(int session,char* params);
	void(*post)(int session, char*params, char* body);
};

void
init_web_handler_manager();

void
exit_web_handler_manager();

void
register_web_handler(const char* url,
	void(*get)(int sock, char* params),
	void(*post)(int sock, char* params, char* body));


struct web_handler*
	find_web_handler(const char* url, int len);
