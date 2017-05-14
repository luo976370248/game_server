#ifndef _LOGION_MODEL_H_
#define _LOGION_MODEL_H_

#include "../../database/center_database.h"

enum 
{
	MODEL_SUCCESS = 0, // �����ɹ�
	MODEL_ERROR = -1, // ͨ�õ�error
	MODEL_ACCOUNT_IS_NOT_GUEST = -2, // �˺Ų����ο��˺�
	MODEL_SYSTEM_ERR = -3, // ϵͳ����
	MODEL_INVALID_OPT = -4, // �Ƿ��Ĳ���
	MODEL_UNAME_IS_USING = -5, // ���ֱ���������ʹ��
	MODEL_UNAME_OR_UPWD_ERR = -6, // �û������������,
	MODEL_OLD_PWD_ERR = -7, // �ɵ����벻��
};

int model_guest_login(char* ukey, char* unick, int usex, int uface, struct user_info* out_info);

int model_user_login(char* uname, char* upwd, struct user_info* out_info);

int model_edit_user_profile(unsigned int uid, char* unick, int usex, int uface);

int model_upgrade_account(unsigned int uid, char* uname, char* upwd_md5);

int model_modify_upwd(unsigned int uid, char* old_upwd_md5, char* new_upwd_md5);


#endif // !_LOGION_MODEL_H_
