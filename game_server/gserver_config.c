#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gserver_config.h"

struct gserver_config GSERVER_CONF = {
	"127.0.0.1",
	6082,

	10000, // player ����ش�С
	10000, // room �����С
	100000, // ˮ������Ĵ�С

	{ 60, 60, 60, 0, 0, 0, 0, 0, 0, 0 },  // ÿ��zid����Ϸʱ��

	5, // ��Ϸ����ʱ��
};
