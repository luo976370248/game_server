#ifndef __GAME_RESULT_H__
#define __GAME_RESULT_H__
// respones status code
enum {
	OK = 1, // �ɹ�
	INVALID_CMD = -100, // ��֧�ֵ������
	INVALID_PARAMS = -101, // ��������
	ACCOUNT_IS_NOT_GUEST = -102, // �����ο��˺�
	SYSTEM_ERROR = -103, // ϵͳ����
	INVALID_OPT = -104, // �Ƿ��Ĳ���
	UNAME_IS_USING = -105, // �û����Ѿ�ռ�ã���������
	UNAME_OR_UPWD_ERROR = -106, // �û������������
	OLD_UPWD_ERROR = -106, // �ɵ��������
	NO_LOGIN_BONUES = -107, // û�е�½����
	USER_IS_CLOSE_DOWN = -108, // �û��Ѿ������
	USER_IS_NOT_IN_SEVER = -109, // �û����ڷ�������.
	USER_IS_IN_ZONE = -110, // �û��Ѿ����������
	INVALIDI_ZOOM_ID = -111, // ������û�id��
	USER_IS_NOT_IN_ZONE = -112, // �û������������
	USER_IS_IN_GAME = -113, // �û�����Ϸ��
};

#endif

