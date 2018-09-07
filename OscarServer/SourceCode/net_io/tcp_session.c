#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#include <WinSock2.h>
#include <Windows.h>
#endif

#include "tcp_session.h"


#define MAX_SESSION_NUM 6000
#define my_malloc malloc
#define my_free free


struct {
	struct session* online_session;

	struct session* cache_mem;
	struct session* free_list;


	int readed; // 当前已经从socket里面读取的数据;

	int has_removed;
	int socket_type;  // 0 表示TCP socket, 1表示是 websocket
	int protocal_type;// 0 表示二进制协议，size + 数据的模式
					  // 1,表示文本协议，以回车换行来分解收到的数据为一个包
}session_manager;

void init_session_manager(int socket_type, int protocal_type)
{
	memset(&session_manager,0,sizeof(session_manager));

	session_manager.socket_type = socket_type;
	session_manager.protocal_type = protocal_type;

	session_manager.cache_mem = (struct session*)my_malloc(MAX_SESSION_NUM * sizeof(struct session));
	memset(session_manager.cache_mem, 0, MAX_SESSION_NUM * sizeof(struct session));

	for (int i = 0; i < MAX_SESSION_NUM; i++) {
		session_manager.cache_mem[i].next = session_manager.free_list;
		session_manager.free_list = &session_manager.cache_mem[i];
	}
}

void exit_session_manager() {

}

//归还的session是放回链表中还是释放
static void cache_free(struct session*s)
{
	if (s >= session_manager.cache_mem && s < session_manager.cache_mem + MAX_SESSION_NUM)
	{
		s->next = session_manager.free_list;
		session_manager.free_list = s;
	}
	else
	{
		my_free(s);
	}
}

//Get session from linklist
static struct session* cache_alloc()
{
	struct session* s = NULL;

	if (session_manager.free_list !=NULL)
	{
		s = session_manager.free_list;
		session_manager.free_list = s->next;
	}
	else
	{
		s = my_malloc(sizeof(struct session));
	}

	memset(s,0,sizeof(struct session));
	return s;
}


struct session* save_session(int c_sock, char* ip , int port)
{
	struct session*s = cache_alloc();
	s->c_sock = c_sock;
	s->c_port = port;

	int len = strlen(ip);
	if (len >= 32)
	{
		len = 31;
	}

	strncpy(s->c_ip, ip, len);
	s->c_ip[len] = 0;

	s->next = session_manager.online_session;
	session_manager.online_session = s;

	return s;
}

void foreach_online_session(int(*callback)(struct session* s,void* p),void* p)
{
	if (callback == NULL)
	{
		return;
	}

	struct session* walk = session_manager.online_session;

	while (walk)
	{
		if (walk->removed == 1)
		{
			walk = walk->next;
			continue;
		}

		if (callback(walk,p))
		{
			return;
		}

		walk = walk->next;
	}
}

void close_session(struct session*s)
{
	s->removed = 1;
	session_manager.has_removed = 1;

	printf("client %s:%d exit\n", s->c_ip, s->c_port);
}

void clear_offline_session(struct session*s)
{
	if (session_manager.has_removed == 0)
	{
		return;
	}

	struct session** walk = &session_manager.online_session;
	while (*walk)
	{
		struct session* s = (*walk);
		if (s->removed)
		{
			walk = s->next;
			s->next = NULL;
			


			s->c_sock = 0;
			cache_free(s);
		}
		else
		{
			walk = &(*walk)->next;
		}
	}
	
	session_manager.has_removed = 0;
}
