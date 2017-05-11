#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "log.h"

#ifndef WIN32
#include <sys/time.h>
#endif

#include <windows.h>

#include <stdio.h>  
#include <time.h>  


int g_dbg_level = LEVEL_INFOR | LELEL_WARNING | LEVEL_ERROR | LEVEL_CRITICAL;
FILE *g_log_fp = NULL;

void init_color_console(level) {
	if (level == LEVEL_INFOR) {
		//system("color 02");
		return;
	}
	if (level == LELEL_WARNING) {
		//system("color 06");

		return;
	}
	if (level == LEVEL_ERROR) {
		//system("color 04");
	
		return;
	}
	if (level == LEVEL_CRITICAL) {
		//system("color 03");
	
		return;
	}
	//printf("\n\n%d",level);
}

void LOG(char* file_name, int line, char* func, int level, char* fmt, ...){
	
	char buf[256];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 256, fmt, args);
	va_end(args);

	time_t t = time(NULL);
	struct tm  *tmm = localtime(&t);

	if (g_dbg_level & level) {
		
		init_color_console(g_dbg_level & level);
		printf("[%d-%02d-%02d %02d:%02d:%02d][%s][%s:%d]:%s\n",
			tmm->tm_year + 1900, tmm->tm_mon + 1, tmm->tm_mday,
			tmm->tm_hour, tmm->tm_min, tmm->tm_sec,
			func, file_name, line, buf);
	}


	if ((g_log_fp != NULL) && (level >= LEVEL_CRITICAL)) {

		fprintf(g_log_fp, "[%d-%02d-%02d %02d:%02d:%02d][%s][%s:%d]:%s\n",
			tmm->tm_year + 1900, tmm->tm_mon + 1, tmm->tm_mday,
			tmm->tm_hour, tmm->tm_min, tmm->tm_sec,
			func, file_name, line, buf);
	}
	
}


void init_log() {

}
