#include "from_client.h"
#include "../server_gateway.h"
#include "../net_io/game_protocol.h"


int on_bin_protocal_recv(void* module_data, struct session* s,unsigned char* data, int len)
{
	struct user_login_req req;
	command_login_unpack(data + 4, data - 4, &req);

	struct user_login_respons res;
	res.level = 1;
	res.status = 1; // OK
	res.name = "Ð¡ºì";
	// end 

	char send_buf[256];
	int send_len = login_respons_pack(USER_LOGION, &res, send_buf);

	binary_session_send(s, send_buf, send_len);

	return 1;
}

void register_from_client_module(int stype)
{
	struct service_module* module = malloc(sizeof(struct service_module));
	memset(module,0,sizeof(struct service_module));

	module->stype = stype;
	module->init_service_module = NULL;
	module->on_bin_protocal_recv = on_bin_protocal_recv;
	module->on_json_protocal_recv = NULL;
	module->on_connect_lost = NULL;
	module->module_data = (void*)stype;

	register_service(stype, module);
}

