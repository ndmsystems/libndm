#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ndm/strmap.h>

struct ndm_strmap_entry_t
{
	char *value;
	size_t value_size;
	size_t key_size;
	char key[];
};

typedef int (*ndm_strmap_key_comp_t)(
		const char *l,
		const char *r,
		const size_t n);

static inline struct ndm_strmap_entry_t *ndm_strmap_entry_alloc_(
		const char *const key,
		const size_t key_size,
		const char *const value,
		const size_t value_size)
{
	struct ndm_strmap_entry_t *e =
		(struct ndm_strmap_entry_t *) malloc(sizeof(*e) + key_size + 1);

	if (e == NULL) {
		return NULL;
	}

	e->value = (char *) malloc(value_size + 1);

	if (e->value == NULL) {
		free(e);

		return NULL;
	}

	memcpy(e->key, key, key_size);
	e->key_size = key_size;
	e->key[e->key_size] = '\0';

	memcpy(e->value, value, value_size);
	e->value_size = value_size;
	e->value[e->value_size] = '\0';

	return e;
}

static inline void ndm_strmap_entry_free_(
		struct ndm_strmap_entry_t *e)
{
	free(e->value);
	free(e);
}

void ndm_strmap_clear(
		struct ndm_strmap_t *map)
{
	struct ndm_ptrvec_t *v = &map->vec_;
	const size_t n = ndm_ptrvec_size(v);
	size_t i = 0;

	while (i < n) {
		ndm_strmap_entry_free_(
			(struct ndm_strmap_entry_t *) ndm_ptrvec_at(v, i));

		++i;
	}

	ndm_ptrvec_clear(v);
}

bool ndm_strmap_nset(
		struct ndm_strmap_t *map,
		const char *const key,
		const size_t key_size,
		const char *const value,
		const size_t value_size)
{
	struct ndm_ptrvec_t *v = &map->vec_;
	const size_t i = ndm_strmap_nfind(map, key, key_size);

	if (i < ndm_ptrvec_size(v)) {
		/* update a value only */
		struct ndm_strmap_entry_t *e =
			(struct ndm_strmap_entry_t *) ndm_ptrvec_at(v, i);

		if (e->value_size != value_size) {
			/* try to truncate or extend it */
			char *reallocated = (char *) realloc(e->value, value_size + 1);

			if (reallocated == NULL) {
				return false;
			}

			e->value = reallocated;
			e->value_size = value_size;
		}

		memcpy(e->value, value, value_size);
		e->value[e->value_size] = '\0';

		return true;
	}

	/* append a new entry */
	struct ndm_strmap_entry_t *e =
		ndm_strmap_entry_alloc_(key, key_size, value, value_size);

	if (e == NULL) {
		return false;
	}

	if (!ndm_ptrvec_push_back(v, e)) {
		ndm_strmap_entry_free_(e);

		return false;
	}

	return true;
}

const char *ndm_strmap_nget(
		struct ndm_strmap_t *map,
		const char *const key,
		const size_t key_size)
{
	const size_t idx = ndm_strmap_nfind(map, key, key_size);

	if (idx < ndm_ptrvec_size(&map->vec_)) {
		return ndm_strmap_get_by_index(map, idx);
	}

	return "";
}

const char *ndm_strmap_get_by_index(
		struct ndm_strmap_t *map,
		const size_t idx)
{
	assert (idx < ndm_ptrvec_size(&map->vec_));

	return ((struct ndm_strmap_entry_t *)
		ndm_ptrvec_at(&map->vec_, idx))->value;
}

bool ndm_strmap_nremove(
		struct ndm_strmap_t *map,
		const char *const key,
		const size_t key_size)
{
	const size_t idx = ndm_strmap_nfind(map, key, key_size);

	if (idx < ndm_ptrvec_size(&map->vec_)) {
		ndm_strmap_remove_by_index(map, idx);

		return true;
	}

	return false;
}

void ndm_strmap_remove_by_index(
		struct ndm_strmap_t *map,
		const size_t idx)
{
	struct ndm_ptrvec_t *v = &map->vec_;

	assert (idx < ndm_ptrvec_size(v));

	ndm_strmap_entry_free_(
		(struct ndm_strmap_entry_t *)
			ndm_ptrvec_at(v, idx));
	ndm_ptrvec_remove(v, idx);
}

const char *ndm_strmap_get_key(
		struct ndm_strmap_t *map,
		const size_t idx)
{
	assert (idx < ndm_ptrvec_size(&map->vec_));

	return ((struct ndm_strmap_entry_t *)
		ndm_ptrvec_at(&map->vec_, idx))->key;
}

inline bool ndm_strmap_assign(
		struct ndm_strmap_t *dst,
		const struct ndm_strmap_t *src)
{
	if (dst == src) {
		return true;
	}

	struct ndm_ptrvec_t v = NDM_PTRVEC_INITIALIZER;

	if (!ndm_ptrvec_assign(&v, &src->vec_)) {
		return false;
	}

	const size_t n = ndm_ptrvec_size(&src->vec_);
	size_t i = 0;

	while (i < n) {
		struct ndm_strmap_entry_t *se =
			(struct ndm_strmap_entry_t *) ndm_ptrvec_at(&src->vec_, i);
		struct ndm_strmap_entry_t *e = ndm_strmap_entry_alloc_(
			se->key, se->key_size, se->value, se->value_size);

		if (e == NULL) {
			while (i-- > 0) {
				ndm_strmap_entry_free_(
					(struct ndm_strmap_entry_t *) ndm_ptrvec_at(&v, i));
			}

			ndm_ptrvec_clear(&v);

			return false;
		}

		ndm_ptrvec_set(&v, i, e);
		++i;
	}

	ndm_strmap_clear(dst);

	ndm_ptrvec_swap(&dst->vec_, &v);
	dst->flags_ = src->flags_;

	return true;
}

size_t ndm_strmap_nfind(
		const struct ndm_strmap_t *map,
		const char *const key,
		const size_t key_size)
{
	const struct ndm_ptrvec_t *v = &map->vec_;
	const size_t n = ndm_ptrvec_size(v);
	size_t i = 0;
	ndm_strmap_key_comp_t comp =
		(map->flags_ & NDM_STRMAP_FLAGS_CASE_INSENSITIVE) ?
		(ndm_strmap_key_comp_t) strncasecmp :
		(ndm_strmap_key_comp_t) memcmp;

	while (i < n) {
		struct ndm_strmap_entry_t *e =
			(struct ndm_strmap_entry_t *) ndm_ptrvec_at(v, i);

		if (e->key_size == key_size &&
			comp(e->key, key, key_size) == 0)
		{
			return i;
		}

		++i;
	}

	return i;
}

void ndm_strmap_swap(
		struct ndm_strmap_t *lmap,
		struct ndm_strmap_t *rmap)
{
	struct ndm_strmap_t tmap = *lmap;

	*lmap = *rmap;
	*rmap = tmap;
}

