#ifndef __TCP_SESSION_H__
#define __TCP_SESSION_H__

struct session {
	char c_ip[32];
	int c_port;
	int c_sock;
	int removed;
	int is_shake_hand;

	void* player; // ָ���������;

	struct session* next;
	
};

void init_session_manager();
void exit_session_manager();


// �пͷ��˽������������sesssion;
struct session* save_session(int c_sock, char* ip, int port);
void close_session(struct session* s);

// ��������session�������������session
void foreach_online_session(int(*callback)(struct session* s, void* p), void*p);

// �������ǵ�����
void session_on_recv(struct session* s);
void clear_offline_session();
// end 
#endif

