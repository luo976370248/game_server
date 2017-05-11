#ifndef __TIMESTAMP_H__
#define __TIMESTAMP_H__

// ��ȡ��ǰ��ʱ���
unsigned int timestamp();

// ��ȡ�������ڵ�ʱ���"%Y(��)%m(��)%d(��)%H(Сʱ)%M(��)%S(��)"
unsigned int date2timestamp(const char* fmt_date, const char* date);

// fmt_date "%Y(��)%m(��)%d(��)%H(Сʱ)%M(��)%S(��)"
void timestamp2date(unsigned int t, char*fmt_date, 
               char* out_buf, int buf_len);


unsigned int timestamp_today();

unsigned int timestamp_yesterday();

#endif

