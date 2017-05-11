#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "login_model.h"

#include "../../database/center_database.h"

int model_guest_login(char* ukey, char* unick, int usex, int uface, struct user_info* uinfo) {
	
	//1.判断游客是否存在
	if (get_unifo_by_ukey(ukey, uinfo) == 0) {
		if (uinfo->is_guest) {

		}
	}
}


