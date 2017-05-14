#ifndef __GAME_PLAYER_H__
#define __GAME_PLAYER_H__

#include "../netbus/netbus.h"

enum {
	PLAYER_INVIEW = 0, // ����Թ�״̬��
	PLAYER_READY = 1, // ���׼�����ˣ�
	PLAYER_INGAME = 2, // ���������Ϸ��;
	PLAYER_INCHECKOUT = 3, // ������ڽ���
};

struct game_player {
	unsigned int uid; // ��ҵ�uid
					  // ��ǰ���������ڷ������ķ����
	int stype;
	int online;
	struct session* s; // ���������ң��������ݵ�session
					   // end 

					   // ��ҵ��û���Ϣ
	char unick[32];
	int usex;
	int uface;
	// end 

	// ��ҵ���Ϸ��Ϣ
	int uchip;
	int uvip;
	int uexp;
	// end 

	// ��ҵ�ս����Ϣ
	int win_round;
	int lost_round;
	// end 

	// �����Ϸ�еķ��������Ϣ
	int zid;
	int roomid; // ˵�������ڵȴ��б����棬�������idΪ0�Ļ�
	int sv_seatid; // �������Ϸ�е���λ��
	int status; // ��¼��ҵ�״̬

	struct game_player* wait_next; // �ȴ��б��������һ���ڵ�
								   // end 

};

void player_init();

int player_load_uinfo(struct game_player* player);

int player_load_ugame_info(struct game_player* player);

int player_load_uscore_info(struct game_player* player);

// json array �������������ǵķ�����Ϣ
json_t* get_player_room_info(struct game_player* player);

// json array �������������ǵ���Ϸ��Ϣ
json_t* get_player_game_info(struct game_player* player);

// json array �������������ǵ������Ϣ
json_t* get_player_user_info(struct game_player* player);

void player_on_game_start(struct game_player* player);

void player_on_game_checkout(struct game_player* player, int winner_seat);

void player_on_after_checkout(struct game_player* player);

#endif

