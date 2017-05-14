#ifndef __CENTER_DATABASE_H__
#define __CENTER_DATABASE_H__

struct user_info {
	unsigned int uid;
	char unick[64];
	char phone_number[64];
	int uface;
	int usex;
	int is_guest;
};

// extern void* mysql_center;
void connect_to_center();

// -1, ����ʧ�ܣ�0,��ʾ���ǲ����ɹ�
int get_uinfo_by_ukey(char* ukey, struct user_info* uinfo);

int get_uinfo_by_uid(unsigned int uid, struct user_info* uinfo);

int get_uinfo_by_uname_upwd(char* uname, char* upwd, struct user_info* uinfo);

// ����һ���ο��˺�
int insert_guest_with_ukey(char* ukey, char* unick,int uface, int usex);


int update_user_profile(unsigned int uid, char* unick, int uface, int usex);

int update_user_upwd(unsigned int uid, char* old_upwd_md5, char* new_upwd_md5);

// Ϊ0����ʾû��ռ�ã�-1,��ʾռ����
int is_uname_exist(unsigned int uid, char* uname);

// 0, ��ʾ�ɹ���-1,��ʾʧ��
int upgrade_account(unsigned int uid, char* uname, char* upwd_md5);

// �����Ǵ����ݿ�����uinfo���浽���ǵ�redis����ȥ
void set_uinfo_inredis(struct user_info* uinfo);
// Ϊ0����ʾ���������߻�ȡʧ��
int get_uinfo_inredis(unsigned int uid, struct user_info* uinfo);

#endif