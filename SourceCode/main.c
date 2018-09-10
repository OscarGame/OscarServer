#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "web_handler.h"
#include "user_login.h"
#include "tcp_iocp.h"

int main(int argc,char** argv)
{
	init_web_handler_manager();
	register_web_handler("/login", user_login_get, NULL);

	start_server(8000);

	return 0;
}