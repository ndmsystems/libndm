#ifndef __NDM_POOL__
#define __NDM_POOL__

#include <stddef.h>
#include <stdbool.h>
#include "attr.h"

struct ndm_pool_t
{
	void *const __static_block;
	const size_t __static_block_size;
	void *__dynamic_block;
	const size_t __dynamic_block_size;
	size_t __available;
	size_t __total_allocated;
	size_t __total_dynamic_size;
	bool __is_valid;
};

#define NDM_POOL_INITIALIZER(							\
		static_block,									\
		static_block_size,								\
		dynamic_block_size)								\
	{													\
		.__static_block = static_block,					\
		.__static_block_size = static_block_size,		\
		.__dynamic_block = NULL,						\
		.__dynamic_block_size = dynamic_block_size,		\
		.__available = static_block_size,				\
		.__total_allocated = 0,							\
		.__total_dynamic_size = 0,						\
		.__is_valid = true								\
	}

void ndm_pool_initialize(
		struct ndm_pool_t *pool,
		void *static_block,
		const size_t static_block_size,
		const size_t dynamic_block_size);

static bool ndm_pool_is_valid(
		const struct ndm_pool_t *pool) NDM_ATTR_WUR;

static inline bool ndm_pool_is_valid(
		const struct ndm_pool_t *pool)
{
	return pool->__is_valid;
}

void *ndm_pool_malloc(
		struct ndm_pool_t *pool,
		const size_t size) NDM_ATTR_WUR;
void *ndm_pool_calloc(
		struct ndm_pool_t *pool,
		const size_t nmemb,
		const size_t size) NDM_ATTR_WUR;
char *ndm_pool_strdup(
		struct ndm_pool_t *pool,
		const char *const s) NDM_ATTR_WUR;

void ndm_pool_clear(
		struct ndm_pool_t *pool);

static size_t ndm_pool_allocated(
		const struct ndm_pool_t *pool) NDM_ATTR_WUR;

static inline size_t ndm_pool_allocated(
		const struct ndm_pool_t *pool)
{
	return pool->__total_allocated;
}

static size_t ndm_pool_total_dynamic_size(
		const struct ndm_pool_t *pool) NDM_ATTR_WUR;

static inline size_t ndm_pool_total_dynamic_size(
		const struct ndm_pool_t *pool)
{
	return pool->__total_dynamic_size;
}

#endif	/* __NDM_POOL__ */

