#ifndef __NDM_POOL__
#define __NDM_POOL__

#include <stddef.h>
#include <stdbool.h>
#include "attr.h"

#define NDM_POOL_INITIALIZER			NULL

struct ndm_pool_t;

bool ndm_pool_create(
		struct ndm_pool_t **pool,
		const size_t initial_block_size,
		const size_t dynamic_block_size) NDM_ATTR_WUR;
void ndm_pool_destroy(
		struct ndm_pool_t **pool);

bool ndm_pool_is_valid(
		struct ndm_pool_t *pool) NDM_ATTR_WUR;

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

size_t ndm_pool_size(
		struct ndm_pool_t *pool) NDM_ATTR_WUR;
size_t ndm_pool_allocated(
		struct ndm_pool_t *pool) NDM_ATTR_WUR;

#endif	/* __NDM_POOL__ */

