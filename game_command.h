#ifndef __GAME_COMMAND_H__
#define __GAME_COMMAND_H__

#include "3rd/mjson/json.h"
void
json_get_command_tag(json_t* cmd, unsigned int* uid, unsigned int* s_key);

json_t*
json_new_server_return_cmd(int stype, int cmd,
                      unsigned int uid,
					  unsigned int s_key);

void
write_error(struct session* s, int stype,
            int cmd, int status,
			unsigned int uid, unsigned int s_key);

// ���еķ���������
enum {
	USER_LOST_CONNECT = 1,
	COMMON_NUM,
};

// SYPTE_CENTER
enum {
	GUEST_LOGIN = COMMON_NUM, // �ο͵�½
	USER_LOGIN, // �û������¼
	EDIT_PROFILE, // �޸��û�����
	RELOGIN, // �û����µĵط���½
	ACCOUNT_UPGRADE, // �˺�����
	MODIFY_UPWD, // �޸�����
};

// STYPE_SYSTEM, �ʼ�������, ���񣬽���
enum {
	CHECK_LOGIN_BONUES = COMMON_NUM, // ���͵�½�ɹ�����Ϣ������ϵͳ��������
	GET_LOGIN_BONUSES_INFO = 3, // ��ȡ��½����
	RECV_LOGIN_BONUSES = 4, // ��ȡ��½���� 
	GET_UGAME_COMMON_INFO = 5, // ��ȡ��ҵĹ�����Ϣ����ң�VIP, ����, ��ʯ��
	GET_GAME_RANK_INFO = 6, // ��ȡ��Ϸ���а�
	GET_MASK_AND_BONUES_INFO = 7, // ��ȡ���ǵ�����ͽ���
	TASK_WIN_EVENT = 8, // ��һ�����Ӯ��һ������
	RECV_TASK_BONUES = 9, // ��ȡ���ǵ�������
};

// STYPE_GAME_INTERNET 
// �����ս����
enum {
	ENTER_SERVER = COMMON_NUM, // ���������
	EXIT_SERVER = 3, // �뿪������

	ENTER_ZONE = 4, // ���뵽�ĸ���
	EXIT_ZONE = 5, // �뿪�ĸ���

	ENTER_ROOM = 6, // ���뷿��
	EXIT_ROOM = 7, // �뿪����
	USER_ARRIVED = 8, // ���������û�������
	USER_STANDUP = 9, // ���վ��

	GAME_START = 10, // ��Ϸ��ʼ
	GAME_CHECKOUT = 11, // ��Ϸ����
	ROOM_RECONNECT = 12, // �����������

	GEN_FRUIT = 13, // ������һ��ˮ��
	SLICE_FRUIT = 14, // ��ˮ�����ж�
	PLAYER_SEND_READY = 15, // ���׼��

	AFTER_CHECKOUT = 16, // ����ʱ�����
	PLAYER_SEND_PROP = 17, // ��ҷ��͵���
};

// ���Ѷ�ս

#endif

