#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "center_db_config.h"

struct center_db_config CENTER_DB_CONF = {
	// MYSQL
	"127.0.0.1",
	"root",
	3306,
	"123456",
	"user_center",
	// end

	// redis
	"127.0.0.1",
	6379,
	NULL, // û�������ΪNULL��"123456"
	0,   // 0�����ݲֿ���������ǵ����ķ����������ݵ�redis���ݿ�
		 // end
};
