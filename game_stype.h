#ifndef __GAME_STYPE_H__
#define __GAME_STYPE_H__

enum {
	STYPE_SERVER_POST = 1,
	STYPE_CENTER = 2, // �û����ݣ���½�����Ϸ���
	STYPE_SYSTEM = 3, // �����ʼ�������������
	STYPE_GAME_INTERNET = 4, // �����ս
	STYPE_GAME_FRIEND = 5, // ���Ѷ�ս
};

// ���������������512������ģ��[0, 255],[256, 511]
#ifdef GAME_DEVLOP
#define STYPE_RETURN 0
#else
#define STYPE_RETURN 0xff
#endif

// return��

#endif

