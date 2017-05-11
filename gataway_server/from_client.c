#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../netbus/netbus.h"
#include "from_client.h"
#include "server_session.h"
#include "../game_command.h"
#include "session_key_man.h"
#include "../game_stype.h"

#define my_malloc malloc

static int
on_json_protocal_recv(void* module_data, struct session* s,
json_t* json, unsigned char* data, int len) {
	int stype = (int)(module_data);
	struct session* server_session = get_server_session(stype);
	if (server_session == NULL) { // gateway��������ڽ��̶Ͽ�����������
		// ��һ���ͻ��������쳣�Ĵ���
		// end 
		return 0;
	}
	// ��֤,����û��Ѿ���½��l
	unsigned int uid = s->uid; // ͨ��session�����uid,
	// end 

	// ���ķ�����⣬���еķ���Ҫ������½
	if (stype != STYPE_CENTER && uid == 0) {
		return 0;
	}

	// ת������Ӧ�ķ�����
	// token_key, ���������ҵ�client_session
	json_object_push_number(json, "uid", uid);

	unsigned int key;
	if (stype == STYPE_CENTER) {
		key = get_key_from_session_map();
		save_session_with_key(key, s);
	}
	else {
		key = 0;
	}
	
	// s->s_key = key;

	json_object_push_number(json, "s_key", key);

	session_send_json(server_session, json);
	// end 
	return 0;
}

static void
on_connect_lost(void* module_data, struct session* s) {
	if (s->is_server_session) {
		return;
	}

	clear_session_with_value(s);

	unsigned int uid = s->uid;
	if (uid == 0) { // û�е�½�����½���ɹ�
		return;
	}

	int stype = (int)(module_data);
	if (stype == STYPE_CENTER) {
		clear_session_with_uid(uid);
	}

	
	// ֪ͨ������������û�������
	json_t* json = json_new_command(stype, USER_LOST_CONNECT);

	
	json_object_push_number(json, "2", uid);

	json_object_push_number(json, "uid", uid);
	json_object_push_number(json, "s_key", 0);
	struct session* server_session = get_server_session(stype);
	if (server_session == NULL) { // gateway��������ڽ��̶Ͽ�����������
		json_free_value(&json);
		return;
	}
	session_send_json(server_session, json);
	json_free_value(&json);
	// end 
	// clear_session_with_key(s->s_key);

} 

void 
register_from_client_module(int stype) {
	struct service_module* module = my_malloc(sizeof(struct service_module));
	memset(module, 0, sizeof(struct service_module));

	module->stype = stype;
	module->init_service_module = NULL;
	module->on_bin_protocal_recv = NULL;
	module->on_json_protocal_recv = on_json_protocal_recv;
	module->on_connect_lost = on_connect_lost;
	module->module_data = (void*) stype;

	register_service(stype, module);
}