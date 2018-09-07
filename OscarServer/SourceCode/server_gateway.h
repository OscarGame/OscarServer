
#include "3rd/mjson/json.h"

enum {
	TCP_SOCKET_IO = 0,  // tcp socket
	WEB_SOCKET_IO = 1,  // websocket
};

enum {
	BIN_PROTOCAL = 0, // ������Э��
	JSON_PROTOCAL = 1, // jsonЭ��
};

struct service_module {
	int stype; // ��������ͣ�ϵͳ����������������������Ϣ�ַ�����Ӧ�ķ���

	int  // �����Ϊ0���ײ��رյ����socket;
	(*on_bin_protocal_recv)(void* module_data, struct session* s,
		unsigned char* data, int len);

	int
	(*on_json_protocal_recv)(void* module_data, struct session* s,
		json_t* json, unsigned char* data, int len);

	void
	(*on_connect_lost)(void* module_data, struct session* s); // ���Ӷ�ʧ���յ��������;

	void* module_data; // ����Я��service_module�û��Զ������ݵ�;
};


void init_server_gateway();
void exit_server_gateway();

void start_server(char* ip, int port, int socket_type, int protocal_type);