#ifndef __GAME_PROTOCAL_H__
#define __GAME_PROTOCAL_H__

// ��ʶһ���꣬������������ĸ����ݽṹ��
enum {
	USER_LOGION = 0,
};

struct user_login_req {
	char* uname;
	char* upsd;
	int channel;
};

int 
command_login_pack(int cmd_type, struct user_login_req* req, 
                   unsigned char* dst);

void 
command_login_unpack(unsigned char* data, int lenm, 
                    struct user_login_req* out);

struct user_login_respons {
	int status; // �Ƿ��д�ı���;
	char* name;
	int level;
};

int 
login_respons_pack(int cmd_type, struct user_login_respons* respons, 
                   unsigned char* out);

void 
login_respons_unpack(unsigned char* data, int len, 
                     struct user_login_respons* out);





#endif
