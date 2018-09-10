#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#include <WinSock2.h>
#include <mswsock.h>
#include <windows.h>
#endif

#include "tcp_session.h"

#define MAX_SESSION_NUM 6000
#define my_malloc malloc
#define my_free free

#define MAX_RECV_BUFFER 8096

struct {
	struct session* online_session;

	struct session* cache_mem;
	struct session* free_list;


	char recv_buffer[MAX_RECV_BUFFER];
	int readed; // ��ǰ�Ѿ���socket�����ȡ������;

	int has_removed;
	int prot_mode; // 0 ��ʾ������Э�飬size + ���ݵ�ģʽ
	               // 1,��ʾ�ı�Э�飬�Իس��������ֽ��յ�������Ϊһ����
}session_manager;

static struct session* cache_alloc() {
	struct session* s = NULL;
	if (session_manager.free_list != NULL) {
		s = session_manager.free_list;
		session_manager.free_list = s->next;
	}
	else { // ����ϵͳ�ĺ��� malloc
		s = my_malloc(sizeof(struct session));
	}
	memset(s, 0, sizeof(struct session));

	return s;
}

static void cache_free(struct session* s) {
	// �ж�һ�£��Ǵ�cache�����ȥ�ģ����Ǵ�ϵͳmy_malloc�����ȥ�ģ�
	if (s >= session_manager.cache_mem && s < session_manager.cache_mem + MAX_SESSION_NUM) {
		s->next = session_manager.free_list;
		session_manager.free_list = s;
	}
	else { 
		my_free(s);
	}
	// 
}

void init_session_manager() {
	memset(&session_manager, 0, sizeof(session_manager));

	// ��6000��sessionһ�η��������
	session_manager.cache_mem = (struct session*)my_malloc(MAX_SESSION_NUM * sizeof(struct session));
	memset(session_manager.cache_mem, 0, MAX_SESSION_NUM * sizeof(struct session));
	// end 

	for (int i = 0; i < MAX_SESSION_NUM; i++) {
		session_manager.cache_mem[i].next = session_manager.free_list;
		session_manager.free_list = &session_manager.cache_mem[i];
	}
}

void exit_session_manager() {

}

struct session* save_session(int c_sock, char* ip, int port) {
	struct session* s = cache_alloc();
	s->c_sock = c_sock;
	s->c_port = port;
	int len = strlen(ip);
	if (len >= 32) {
		len = 31;
	}
	strncpy(s->c_ip, ip, len);
	s->c_ip[len] = 0;

	s->next = session_manager.online_session;
	session_manager.online_session = s;
	return s;
}

void foreach_online_session(int(*callback)(struct session* s, void* p), void*p) {
	if (callback == NULL) {
		return;
	}

	struct session* walk = session_manager.online_session;
	while (walk) {
		if (walk->removed == 1) {
			walk = walk->next;
			continue;
		}
		if (callback(walk, p)) {
			return;
		}
		walk = walk->next;
	}
}

void close_session(struct session* s) {
	s->removed = 1;
	session_manager.has_removed = 1;
	printf("client %s:%d exit\n", s->c_ip, s->c_port);
}

static void text_process_package(struct session* s) {

}

static void bin_process_package(struct session* s) {
	if (session_manager.readed < 4) { // ���������ݣ�return
		return;
	}

	// �����Ʒְ��Ĺ���
	int* pack_size = (int*)session_manager.recv_buffer;
	int pack_len = (*pack_size);
	if (pack_len > MAX_RECV_BUFFER) {
		goto pack_failed;
	}
	// end 
	// ��û������
	int total = 0; // �ܹ�������İ������ݴ�С
	while ((session_manager.readed - total) >= pack_len) {
		//  ��������,�����ҽ��д���
		// end 

		// �ƶ�����һ�������д���
		total += pack_len;

		if (session_manager.readed - total < 4) {
			if (session_manager.readed > total) {
				memmove(session_manager.recv_buffer, session_manager.recv_buffer + total, (session_manager.readed - total));
			}
			session_manager.readed -= total;
			return;
		}
		pack_size = (int*)(session_manager.recv_buffer + total);
		pack_len = (*pack_size);
		if (pack_len > MAX_RECV_BUFFER) {
			goto pack_failed;
		}

		if ((session_manager.readed - total) < pack_len) {
			memmove(session_manager.recv_buffer, session_manager.recv_buffer + total, (session_manager.readed - total));
			session_manager.readed -= total;
			return;
		}
		// end
	}

	return;
pack_failed:
	close_session(s);
}

// �������ǵ�socket����
void session_on_recv(struct session* s) {
	int readed = recv(s->c_sock, session_manager.recv_buffer + session_manager.readed, MAX_RECV_BUFFER - session_manager.readed, 0);
	if (readed <= 0) { // �ͷ����Ѿ��ر���socket
		close_session(s);
		return;
	}

	session_manager.readed += readed;

	if (session_manager.prot_mode == 0) {
		bin_process_package(s);
	}
	else {
		text_process_package(s);
	}
}

void clear_offline_session() {
	if (session_manager.has_removed == 0) {
		return;
	}

	struct session** walk = &session_manager.online_session;
	while (*walk) {
		struct session* s = (*walk);
		if (s->removed) {
			*walk = s->next;
			s->next = NULL;
			closesocket(s->c_sock);
			s->c_sock = 0;
			// �ͷ�session
			cache_free(s);
		}
		else {
			walk = &(*walk)->next;
		}
	}
	session_manager.has_removed = 0;
}