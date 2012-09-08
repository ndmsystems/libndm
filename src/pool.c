#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ndm/pool.h>
#include <ndm/macro.h>

#define NDM_POOL_ALIGN_(s) ((s + sizeof(void *) - 1) & ~(sizeof(void *) - 1))

struct ndm_pool_block_t
{
	size_t size;
	struct ndm_pool_block_t *previous;
	char data[];
};

struct ndm_pool_t
{
	const size_t initial_block_size;
	const size_t dynamic_block_size;
	struct ndm_pool_block_t *current;
	size_t available;
	size_t total_allocated;
	size_t total_block_size;
	bool valid;
	struct ndm_pool_block_t initial;	/* should be the last field */
};

static void __ndm_pool_reinitialize(
		struct ndm_pool_t *pool)
{
	pool->current = &pool->initial;
	pool->available = pool->initial_block_size - sizeof(*pool);
	pool->total_allocated = 0;
	pool->total_block_size = pool->initial_block_size;
	pool->valid = true;
	pool->initial.previous = NULL;
	pool->initial.size = pool->available;
}

void ndm_pool_clear(
		struct ndm_pool_t *pool)
{
	struct ndm_pool_block_t *b = pool->current;

	while (b != &pool->initial) {
		struct ndm_pool_block_t *previous = b->previous;

		free(b);
		b = previous;
	}

	__ndm_pool_reinitialize(pool);
}

bool ndm_pool_create(
		struct ndm_pool_t **pool,
		const size_t initial_block_size,
		const size_t dynamic_block_size)
{
	static const size_t MIN_DYNAMIC_BLOCK_SIZE_ =
		NDM_POOL_ALIGN_(sizeof(struct ndm_pool_block_t));
	bool created = false;
	const size_t dynamic_size = NDM_POOL_ALIGN_(dynamic_block_size);

	if (pool == NULL || *pool != NULL ||
		dynamic_size <= MIN_DYNAMIC_BLOCK_SIZE_)
	{
		errno = EINVAL;
	} else {
		const size_t initial_size = NDM_POOL_ALIGN_(
			NDM_MAX(sizeof(**pool), initial_block_size));

		if ((*pool = malloc(initial_size)) == NULL) {
			errno = ENOMEM;
		} else {
			*((size_t *)(&(*pool)->initial_block_size)) = initial_size;
			*((size_t *)(&(*pool)->dynamic_block_size)) = dynamic_size;
			__ndm_pool_reinitialize(*pool);
			created = true;
		}
	}

	return created;
}

void ndm_pool_destroy(
		struct ndm_pool_t **pool)
{
	if (pool != NULL && *pool != NULL) {
		ndm_pool_clear(*pool);
		free(*pool);
		*pool = NULL;
	}
}

bool ndm_pool_is_valid(
		struct ndm_pool_t *pool)
{
	return pool->valid;
}

void *ndm_pool_malloc(
		struct ndm_pool_t *pool,
		const size_t size)
{
	void *p = NULL;

	if (!pool->valid) {
		errno = ENOMEM;
	} else
	if (pool->available >= size) {
		p = pool->current->data + pool->current->size - pool->available;
		pool->available -= size;
		pool->total_allocated += size;
	} else {
		const size_t need = NDM_POOL_ALIGN_(
			size + sizeof(struct ndm_pool_block_t));
		const size_t alloc_size =
			need < pool->dynamic_block_size ?
			pool->dynamic_block_size : need;

		p = malloc(alloc_size);

		if (p == NULL) {
			errno = ENOMEM;
			pool->valid = false;
		} else {
			struct ndm_pool_block_t *previous = pool->current;

			pool->current = (struct ndm_pool_block_t *) p;
			pool->current->previous = previous;
			pool->current->size =
				alloc_size - sizeof(struct ndm_pool_block_t);
			pool->available = pool->current->size - size;

			pool->total_allocated += size;
			pool->total_block_size += alloc_size;

			p = pool->current->data;
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

size_t ndm_pool_size(
		struct ndm_pool_t *pool)
{
	return pool->total_allocated;
}

size_t ndm_pool_allocated(
		struct ndm_pool_t *pool)
{
	return pool->total_block_size;
}

