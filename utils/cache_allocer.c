#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "cache_allocer.h"

#define my_malloc malloc
#define my_free free


/*
���˼·: һ���Եķ�������ǵ������������ڴ�����
������ڴ����������󣬼����������������й���

*/

struct elem_node {
	struct elem_node* next;
};

struct cache_alloc {
	int elem_size;
	int elem_count;
	unsigned char* elem_mem_ptr; // ����ڴ��ַ

								 // ����ؿ��û���ڵ������ͷָ��
	struct elem_node* free_list;
};

struct cache_alloc* create_cache_alloc(int elem_count, int elem_size) {
	if (elem_size < sizeof(struct elem_node*)) {
		elem_size = sizeof(struct elem_node*);
	}
	struct cache_alloc* alloc = my_malloc(sizeof(struct cache_alloc));
	memset(alloc, 0, sizeof(struct cache_alloc));
	alloc->elem_size = elem_size; // ÿ��Ԫ�صĴ�С
	alloc->elem_count = elem_count; // ����

	alloc->elem_mem_ptr = my_malloc(elem_size * elem_count);
	memset(alloc->elem_mem_ptr, 0, elem_size * elem_count);

	for (int i = 0; i < elem_count; i++) {
		struct elem_node* walk = (struct elem_node*)(alloc->elem_mem_ptr + i * elem_size);
		walk->next = alloc->free_list;
		alloc->free_list = walk;
	}


	return alloc;
}

void destroy_cache_alloc(struct cache_alloc* cache) {
	if (cache->elem_mem_ptr) {
		my_free(cache->elem_mem_ptr);
	}

	my_free(cache);
}

void* cache_alloc(struct cache_alloc* cache, int elem_size) {
	assert(cache->elem_size == elem_size);

	struct elem_node* ptr;
	if (cache->free_list) {
		ptr = cache->free_list;
		cache->free_list = cache->free_list->next;
		ptr->next = NULL;
	}
	else {
		ptr = my_malloc(elem_size);
	}


	memset(ptr, 0, sizeof(elem_size));
	return ptr;
}

#if 0
void cache_free(struct cache_alloc* cache, void* elem_ptr) {
	struct elem_node* ptr = (struct elem_node*)elem_ptr;
	memset(ptr, 0, sizeof(struct elem_node));
	ptr->next = cache->free_list;
	cache->free_list = ptr;
}
#else
void cache_free(struct cache_alloc* cache, void* elem_ptr) {
	struct elem_node* ptr = (struct elem_node*)elem_ptr;
	memset(ptr, 0, sizeof(struct elem_node));

	if (((unsigned char*)ptr) < cache->elem_mem_ptr ||
		((unsigned char*)ptr) >= cache->elem_mem_ptr + (cache->elem_count * cache->elem_size)) {
		my_free(ptr);
	}
	else { // 
		ptr->next = cache->free_list;
		cache->free_list = ptr;
	}
}

#endif

