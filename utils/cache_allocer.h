#ifndef __CACHE_ALLOC_H__
#define __CACHE_ALLOC_H__

// ��������һ���ڴ�����
// ����1: Ԫ�صĸ���
// ����2: ÿ��Ԫ�صĴ�С
struct cache_alloc* create_cache_alloc(int elem_count, int elem_size);
// ��������ڴ��
void destroy_cache_alloc(struct cache_alloc* cache);

// ���ڴ���������һ��Ԫ��
void* cache_alloc(struct cache_alloc* cache, int elem_size);

void cache_free(struct cache_alloc* cache, void* elem_ptr);


#endif