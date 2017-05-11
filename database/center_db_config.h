#ifndef __CENTER_DB_CONFIG_H__
#define __CENTER_DB_CONFIG_H__

struct center_db_config
{
	char* mysql_ip;
	char* mysql_name;
	int mysql_port;
	char* mysql_pwd;
	char* database_name;

	char* redis_ip;
	int redis_port;
	char* redis_pwd;
	int redis_dbid;
};

extern struct center_db_config CENTER_DB_CONF;

#endif