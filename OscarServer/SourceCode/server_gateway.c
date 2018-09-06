
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