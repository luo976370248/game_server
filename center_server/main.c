#include <stdlib.h>
#include <stdio.h>
#include "../utils/log.h"

int main(int argc, char** argv) {
	LOGINFO("hello word");
	LOGWARNING("hello");
	LOGERROR("sssss");
	LOGCRITICAL("sdsdsd");
	system("pause");
	return 0;
}