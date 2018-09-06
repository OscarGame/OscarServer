
#include "3rd/mjson/json.h"

struct service_module {
	int stype; // 服务的类型，系统根据这个服务的类型来讲消息分发给对应的服务

	int  // 如果不为0，底层会关闭掉这个socket;
	(*on_bin_protocal_recv)(void* module_data, struct session* s,
		unsigned char* data, int len);

	int
	(*on_json_protocal_recv)(void* module_data, struct session* s,
		json_t* json, unsigned char* data, int len);

	void
	(*on_connect_lost)(void* module_data, struct session* s); // 连接丢失，收到这个函数;

	void* module_data; // 用来携带service_module用户自定义数据的;
};


void init_server_gateway();
void exit_server_gateway();

void start_server(char* ip, int port, int socket_type, int protocal_type);