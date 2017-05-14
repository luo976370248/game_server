#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../game_stype.h"
#include "../game_command.h"
#include "../game_result.h"

#include "module/login_model.h"

#include "auth.h"

#define MAX_UNICK_LEN 64

#ifdef GAME_DEVLOP

#include "../gataway_server/session_key_man.h"

static void do_relogin(struct session* prev_session) {
	json_t* ret = json_new_command(STYPE_CENTER, RELOGIN);
	session_send_json(prev_session, ret);
	json_free_value(&ret);
	
	//关掉以前session
	close_session(prev_session);
}


static void check_relogin(struct session*s, unsigned int uid) {
	struct session* prev_login = get_session_with_uid(uid);
	if (prev_login != NULL) {
		clear_session_with_uid(uid);
		do_relogin(prev_login);
	}

	s->uid = uid;
	save_session_with_uid(uid, s);
}
#endif


// 游客登录
void guest_login(void* module_data, json_t* json, struct session* s, unsigned int uid, unsigned int s_key) {
	int len = json_object_size(json);
	if (len != 6 + 2) {
		write_error(s, STYPE_CENTER, GUEST_LOGIN, INVALID_PARAMS, uid, s_key);
		return;
	}

	char* ukey = json_object_get_string(json, "2");
	char* unick = json_object_get_string(json, "3");
	int usex = json_object_get_number(json, "4");
	int uface = json_object_get_number(json, "5");
	if (ukey == NULL || unick == NULL) {
		write_error(s, STYPE_CENTER, GUEST_LOGIN, INVALID_PARAMS, uid, s_key);
		return;
	}

	if (strlen(unick) >= MAX_UNICK_LEN) {
		unick[MAX_UNICK_LEN - 1] = 0;
	}

	struct user_info uinfo;
	int ret = model_guest_login(ukey, unick, usex, uface, &uinfo);

	if (ret == MODEL_ACCOUNT_IS_NOT_GUEST) {
		write_error(s, STYPE_CENTER, GUEST_LOGIN, ACCOUNT_IS_NOT_GUEST, uid, s_key);
		return;
	}

	if (ret != MODEL_SUCCESS) {
		write_error(s, STYPE_CENTER, GUEST_LOGIN, SYSTEM_ERROR, uid, s_key);
		return;
	}

#ifdef GAME_DEVLOP
	check_relogin(s, uinfo.uid);
#endif
	
	json_t* send_json = json_new_server_return_cmd(STYPE_CENTER, GUEST_LOGIN, uinfo.uid, s_key);
	json_object_push_number(send_json, "2", OK);
	json_object_push_string(send_json, "3", uinfo.unick);
	json_object_push_number(send_json, "4", uinfo.uface);
	json_object_push_number(send_json, "5", uinfo.usex);
	session_send_json(s, send_json);
	json_free_value(&send_json);
}

// 修改用户资料
void edit_user_profile(void* module_data, json_t* json, struct session* s, unsigned int uid, unsigned int s_key) {
	int len = json_object_size(json);
	if (len != 5 + 2) {
		write_error(s, STYPE_CENTER, EDIT_PROFILE, INVALID_PARAMS, uid, s_key);
		return;
	}

	if (uid == 0) {
		write_error(s, STYPE_CENTER, EDIT_PROFILE, INVALID_OPT, uid, s_key);
		return;
	}


	char* unick = json_object_get_string(json, "2");
	if (unick == NULL) {
		write_error(s, STYPE_CENTER, EDIT_PROFILE, INVALID_PARAMS, uid, s_key);
		return;
	}

	if (strlen(unick) >= MAX_UNICK_LEN) {
		unick[MAX_UNICK_LEN - 1] = 0;
	}
	int usex = json_object_get_number(json, "3");
	if (usex != 0 && usex != 1) {
		write_error(s, STYPE_CENTER, EDIT_PROFILE, INVALID_PARAMS, uid, s_key);
		return;
	}

	int uface = json_object_get_number(json, "4");
	if (uface <= 0 || uface > 12) {
		write_error(s, STYPE_CENTER, EDIT_PROFILE, INVALID_PARAMS, uid, s_key);
		return;
	}

	if (model_edit_user_profile(uid, unick, usex, uface) != MODEL_SUCCESS) {
		write_error(s, STYPE_CENTER, EDIT_PROFILE, SYSTEM_ERROR, uid, s_key);
		return;
	}

	json_t* ret = json_new_server_return_cmd(STYPE_CENTER, EDIT_PROFILE, uid, s_key);
	json_object_push_number(ret, "2", OK);
	json_object_push_string(ret, "3", unick);
	json_object_push_number(ret, "4", uface);
	json_object_push_number(ret, "5", usex);
	session_send_json(s, ret);
	json_free_value(&ret);
}

static int is_phone_number(char* phone_str) {
	if (strlen(phone_str) != 11) {
		return 0;
	}
	for (int i = 0; i < 11; i++) {
		if (phone_str[i] < '0' || phone_str[i] > '9') {
			return 0;
		}
	}
	return 1;
}

// 账号升级(游客->会员)
void guest_upgrade_account(void* module_data, json_t* json, struct session* s, unsigned int uid, unsigned int s_key) {
	int len = json_object_size(json);
	if (len != 4 + 2) {
		write_error(s, STYPE_CENTER, ACCOUNT_UPGRADE, INVALID_PARAMS, uid, s_key);
		return;
	}

	char* uname = json_object_get_string(json, "2");
	if (uname == NULL || is_phone_number(uname) == 0) {
		write_error(s, STYPE_CENTER, ACCOUNT_UPGRADE, INVALID_PARAMS, uid, s_key);
		return;
	}

	char* upwd_md5 = json_object_get_string(json, "3");
	if (upwd_md5 == NULL || strlen(upwd_md5) == 0) {
		write_error(s, STYPE_CENTER, ACCOUNT_UPGRADE, INVALID_PARAMS, uid, s_key);
		return;
	}

	int ret = model_upgrade_account(uid, uname, upwd_md5);
	if (ret == MODEL_UNAME_IS_USING) {
		write_error(s, STYPE_CENTER, ACCOUNT_UPGRADE, UNAME_IS_USING, uid, s_key);
		return;
	}

	if (ret != MODEL_SUCCESS) {
		write_error(s, STYPE_CENTER, ACCOUNT_UPGRADE, SYSTEM_ERROR, uid, s_key);
		return;
	}

	json_t* ret_cmd = json_new_server_return_cmd(STYPE_CENTER, ACCOUNT_UPGRADE, uid, s_key);
	json_object_push_number(ret_cmd, "2", OK);
	session_send_json(s, ret_cmd);
	json_free_value(&ret_cmd);
}

// 用户登录
void user_login(void* module_data, json_t* json,struct session* s,unsigned int uid, unsigned int s_key) {
	int len = json_object_size(json);
	if (len != 4 + 2) {
		write_error(s, STYPE_CENTER, USER_LOGIN, INVALID_PARAMS, uid, s_key);
		return;
	}

	char* uname = json_object_get_string(json, "2");
	char* upwd = json_object_get_string(json, "3");
	if (uname == NULL || upwd == NULL) {
		write_error(s, STYPE_CENTER, USER_LOGIN, INVALID_PARAMS, uid, s_key);
		return;
	}
	int ret;
	struct user_info uinfo;

	ret = model_user_login(uname, upwd, &uinfo);
	if (ret == MODEL_UNAME_OR_UPWD_ERR) {
		write_error(s, STYPE_CENTER, USER_LOGIN, UNAME_OR_UPWD_ERROR, uid, s_key);
		return;
	}


#ifdef GAME_DEVLOP
	check_relogin(s, uinfo.uid);
#endif

	// ret
	json_t* send_json = json_new_server_return_cmd(STYPE_CENTER, USER_LOGIN, uinfo.uid, s_key);
	json_object_push_number(send_json, "2", OK);
	json_object_push_string(send_json, "3", uinfo.unick);
	json_object_push_number(send_json, "4", uinfo.uface);
	json_object_push_number(send_json, "5", uinfo.usex);
	session_send_json(s, send_json);
	json_free_value(&send_json);
	// end 
}

// 修改密码
void user_modify_upwd(void* module_data, json_t* json,struct session* s,unsigned int uid, unsigned int s_key) {

	int len = json_object_size(json);
	if (len != 4 + 2) {
		write_error(s, STYPE_CENTER, MODIFY_UPWD, INVALID_PARAMS, uid, s_key);
		return;
	}

	char* old_pwd_md5 = json_object_get_string(json, "2");
	char* new_pwd_md5 = json_object_get_string(json, "3");
	if (old_pwd_md5 == NULL || new_pwd_md5 == NULL) {
		write_error(s, STYPE_CENTER, MODIFY_UPWD, INVALID_PARAMS, uid, s_key);
		return;
	}

	// 没有登陆，不能修改
	if (uid == 0) {
		write_error(s, STYPE_CENTER, MODIFY_UPWD, INVALID_OPT, uid, s_key);
		return;
	}
	// end 

	int ret = model_modify_upwd(uid, old_pwd_md5, new_pwd_md5);
	if (ret == MODEL_OLD_PWD_ERR) {
		write_error(s, STYPE_CENTER, MODIFY_UPWD, INVALID_OPT, uid, s_key);
		return;
	}

	json_t* ret_cmd = json_new_server_return_cmd(STYPE_CENTER, MODIFY_UPWD, uid, s_key);
	json_object_push_number(ret_cmd, "2", OK);
	session_send_json(s, ret_cmd);
	json_free_value(&ret_cmd);
}