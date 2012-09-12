#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ndm/pool.h>
#include <ndm/macro.h>

#define NDM_POOL_ALIGN_(s) ((s + sizeof(void *) - 1) & ~(sizeof(void *) - 1))

struct ndm_pool_block_t
{
	struct ndm_pool_block_t *previous;
	uint8_t data[];
};

void ndm_pool_init(
		struct ndm_pool_t *pool,
		void *static_block,
		const size_t static_block_size,
		const size_t dynamic_block_size)
{
	*((void **) &(pool->__static_block)) = static_block;
	*((size_t *) &(pool->__static_block_size)) = static_block_size;
	*((size_t *) &(pool->__dynamic_block_size)) = dynamic_block_size;
	pool->__dynamic_block = NULL;
	pool->__available = static_block_size;
	pool->__total_allocated = 0;
	pool->__total_dynamic_size = 0;
	pool->__is_valid = true;
}

void *ndm_pool_malloc(
		struct ndm_pool_t *pool,
		const size_t size)
{
	void *p = NULL;

	if (!ndm_pool_is_valid(pool)) {
		errno = ENOMEM;
	} else
	if (pool->__available >= size) {
		uint8_t *block_end = (pool->__dynamic_block == NULL) ?
			((uint8_t *) pool->__static_block) + pool->__static_block_size :
			((uint8_t *) pool->__dynamic_block) + pool->__dynamic_block_size;

		p = block_end - pool->__available;
		pool->__available -= size;
		pool->__total_allocated += size;
	} else {
		/* new dynamic block allocation */
		const size_t need = NDM_POOL_ALIGN_(
			size + sizeof(struct ndm_pool_block_t));
		const size_t alloc_size =
			need < pool->__dynamic_block_size ?
			pool->__dynamic_block_size : need;
		struct ndm_pool_block_t *new_block =
			(struct ndm_pool_block_t *) malloc(alloc_size);

		if (new_block == NULL) {
			errno = ENOMEM;
			pool->__is_valid = false;
		} else {
			new_block->previous =
				(struct ndm_pool_block_t *) pool->__dynamic_block;
			pool->__dynamic_block = new_block;
			pool->__available = alloc_size - need;

			pool->__total_allocated += size;
			pool->__total_dynamic_size += alloc_size;

			p = new_block->data;
		}
	}

	return p;
}

void *ndm_pool_calloc(
		struct ndm_pool_t *pool,
		const size_t nmemb,
		const size_t size)
{
	const size_t s = nmemb*size;
	void *p = ndm_pool_malloc(pool, s);

	if (p != NULL) {
		memset(p, 0, s);
	}

	return p;
}

char *ndm_pool_strdup(
		struct ndm_pool_t *pool,
		const char *const s)
{
	const size_t n = strlen(s) + 1;
	char *p = ndm_pool_malloc(pool, n);

	if (p != NULL) {
		memcpy(p, s, n);
	}

	return p;
}

void ndm_pool_clear(
		struct ndm_pool_t *pool)
{
	struct ndm_pool_block_t *b =
		(struct ndm_pool_block_t *) pool->__dynamic_block;

	while (b != NULL) {
		struct ndm_pool_block_t *previous = b->previous;

		free(b);
		b = previous;
	}

	ndm_pool_init(
		pool,
		pool->__static_block,
		pool->__static_block_size,
		pool->__dynamic_block_size);
}

