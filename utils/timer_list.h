#ifndef _TIMER_LIST_
#define _TIMER_LIST_

// ����timer list,
struct timer_list* create_timer_list();

int update_timer_list(struct timer_list* list);

// ���ٵ�timer list
void destroy_timer_list(struct timer_list* list);

#endif // !_TIMER_LIST_
