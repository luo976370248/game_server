#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../game_stype.h"
#include "../game_command.h"
#include "../game_result.h"
#include "../netbus/netbus.h"
#include "center_services.h"
#include "auth.h"

// 模块初始化入口
static void init_service_module(struct  service_module* module){

}

static int on_json_protocal_recv(void* module_data, struct session* s, json_t* json, unsigned char* data, int len) {

	if (data != NULL) {
		LOGINFO("center_service data = %s", data);
	}


	int cmd = json_object_get_number(json, "1");
	unsigned int uid, s_key;
	json_get_command_tag(json, &uid, &s_key);

	switch (cmd) {
		case GUEST_LOGIN: {
			// 游客登录
			guest_login(module_data, json, s, uid, s_key);
			break;
		}
		case EDIT_PROFILE: {
			// 用户资料修改
			edit_user_profile(module_data, json, s, uid, s_key);
			break;
		}
		case ACCOUNT_UPGRADE: {
			// 账号升级
			guest_upgrade_account(module_data, json, s, uid, s_key);
			break;
		}
		case USER_LOGIN: {
			// 用户密码登录
			user_login(module_data, json, s, uid, s_key);
			break;
		}
		case MODIFY_UPWD:
		{
			//修改密码
			user_modify_upwd(module_data, json, s, uid, s_key);
			break;
		}
		default:{
			break;
		}
	}

	return 0;
}

static void on_connect_lost(void* module_data, struct session* s) {
#ifndef GAME_DEVLOP
	LOGWARNING("gateway lost connected !!!!!");
#endif
}

struct service_module CENTER_SERVICES = {
	STYPE_CENTER,// 服务类型
	init_service_module,// 初始化模块
	NULL,
	on_json_protocal_recv, // json接受
	on_connect_lost, // 连接丢失
	NULL,
};
