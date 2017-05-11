#ifndef _TCP_SESSION_H_
#define _TCP_SESSION_H_

#define MAX_SEND_PKG 2048

struct session {
	char c_ip[32];
	int c_port;
	void* c_sock;
	int removed;
	int is_shake_hand;
	int socket_type;
	int is_server_session;
	// unsigned int s_key;
	unsigned int uid; // �û�uid
	struct session* next;
	// unsigned char send_buf[MAX_SEND_PKG]; // 90%���͵������
};

void init_session_manager(int socket_type, int protocal_type);
void exit_session_manager();


// �пͷ��˽������������sesssion;
struct session* save_session(void* c_sock, char* ip, int port);
void close_session(struct session* s);
void remove_session(struct session* s);

// ��������session�������������session
void foreach_online_session(int(*callback)(struct session* s, void* p), void*p);

// �������ǵ�����
void clear_offline_session();
// end 

int get_socket_type();

int get_proto_type();


#endif // !_TCP_SESSION_H_
