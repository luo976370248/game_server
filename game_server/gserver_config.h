#ifndef __CENTER_CONFIG_H__
#define __CENTER_CONFIG_H__

struct gserver_config {
	char* ip;
	int port;

	// player ����ش�С
	int max_cache_player;
	// room �����С
	int max_cache_room;
	// ˮ������Ĵ�С
	int max_cache_fruit;

	int zid_time_set[10]; // ÿ��zid����Ϸʱ��
	int checkout_time; // ��Ϸ����ʱ��
};

extern struct gserver_config GSERVER_CONF;
#endif