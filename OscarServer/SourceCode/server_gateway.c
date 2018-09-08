
#include "server_gateway.h"

#define MAX_SERVICES 512




struct
{
	struct service_module* services[MAX_SERVICES];
}gateway;

void init_server_gateway()
{
	memset(&gateway, 0, sizeof(gateway));
}

void exit_server_gateway()
{
}

void register_service(int stype,struct service_module* module)
{
	if (stype <= 0 || stype >= MAX_SERVICES)
	{
		gateway.services[stype] = module;
	}
}

void on_json_protocal_recv_entry(struct session* s, unsigned char* data, int len)
{
	data[len] = 0;
	json_t* root = NULL;

	int ret = json_parse_document(&root, data);

	if (ret != JSON_OK ||root == NULL)
	{
		return;
	}

	json_t* server_type = json_find_first_label(&root, "0");
	if (server_type == NULL || server_type->type != JSON_NUMBER)
	{
		return;
	}

	int stype = atoi(server_type->text);

	if (gateway.services[stype] && gateway.services[stype]->on_json_protocal_recv)
	{
		int ret = gateway.services[stype]->on_json_protocal_recv(gateway.services[stype]->module_data,
			s, root, data, len);
		if (ret < 0) {
			close_session(s);
		}
	}
}