#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "login_model.h"

#include "../../database/center_database.h"

int model_guest_login(char* ukey, char* unick, int usex, int uface, struct user_info* uinfo) {
	
	//1.�ж��ο��Ƿ����
	if (get_uinfo_by_ukey(ukey, uinfo) == 0) {
		if (uinfo->is_guest) {
			// ����˺Ų����ο��˺�
			return MODEL_ACCOUNT_IS_NOT_GUEST;
		}

		// ˢ�µ��ڴ�����ȥ
		set_uinfo_inredis(uinfo);

		return MODEL_ERROR;
	}

	// ��ʾ���ݿ���û������û���Ϣ��
	if (insert_guest_with_ukey(ukey, unick, uface, usex) == 0) {
		get_uinfo_by_ukey(ukey, uinfo);
		set_uinfo_inredis(uinfo);
		return MODEL_SUCCESS;
	}
	return MODEL_ERROR;
}


int model_edit_user_profile(unsigned int uid, char* unick, int usex, int uface) {
	if (update_user_profile(uid, unick, uface, usex) != 0) {
		return MODEL_SYSTEM_ERR;
	}

	struct user_info uinfo;
	get_uinfo_by_uid(uid, &uinfo);
	set_uinfo_inredis(&uinfo);

	return MODEL_SUCCESS;
}

int model_upgrade_account(unsigned int uid, char* uname, char* upwd_md5) {
	// ��֤���uid��һ���ο��˺�
	struct user_info uinfo;
	if (get_uinfo_by_uid(uid, &uinfo) < 0) {
		return MODEL_INVALID_OPT;
	}

	if (uinfo.is_guest == 0) {
		return MODEL_INVALID_OPT;
	}
	if (is_uname_exist(uid, uname) < 0) {
		return MODEL_UNAME_IS_USING;
	}

	if (upgrade_account(uid, uname, upwd_md5) < 0) {
		return MODEL_SYSTEM_ERR;
	}


	uinfo.is_guest = 0;
	set_uinfo_inredis(&uinfo);

	return MODEL_SUCCESS;
}

int model_user_login(char* uname, char* upwd, struct user_info* uinfo) {
	if (get_uinfo_by_uname_upwd(uname, upwd, uinfo) < 0) {
		return MODEL_UNAME_OR_UPWD_ERR;
	}

	// �������ǵ�redis
	set_uinfo_inredis(uinfo);
	// end 

	return MODEL_SUCCESS;
}


int model_modify_upwd(unsigned int uid, char* old_upwd_md5, char* new_upwd_md5) {
	// ��֤���uid��һ���ο��˺�
	if (update_user_upwd(uid, old_upwd_md5, new_upwd_md5) != 0) {
		return MODEL_OLD_PWD_ERR;
	}

	return MODEL_SUCCESS;
}

