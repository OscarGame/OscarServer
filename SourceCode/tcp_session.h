#ifndef __TCP_SESSION_H__
#define __TCP_SESSION_H__

struct session {
	char c_ip[32];
	int c_port;
	int c_sock;
	int removed;
	int is_shake_hand;

	void* player; // 指向玩家数据;

	struct session* next;
	
};

void init_session_manager();
void exit_session_manager();


// 有客服端进来，保存这个sesssion;
struct session* save_session(int c_sock, char* ip, int port);
void close_session(struct session* s);

// 遍历我们session集合里面的所有session
void foreach_online_session(int(*callback)(struct session* s, void* p), void*p);

// 处理我们的数据
void session_on_recv(struct session* s);
void clear_offline_session();
// end 
#endif

