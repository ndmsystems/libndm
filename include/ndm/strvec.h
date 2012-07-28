#ifndef __NDM_STRVEC_H__
#define __NDM_STRVEC_H__

#include <stddef.h>
#include <stdbool.h>
#include "attr.h"

#define NDM_STRVEC_INIT										\
	{.__data = NULL, .__size = 0}

struct ndm_strvec_t
{
	char **__data;
	size_t __size;
};

const char **ndm_strvec_array(
		const struct ndm_strvec_t *v) NDM_ATTR_WUR;

static inline void ndm_strvec_init(
		struct ndm_strvec_t *v)
{
	v->__data = NULL;
	v->__size = 0;
}

static size_t ndm_strvec_size(
		const struct ndm_strvec_t *v) NDM_ATTR_WUR;

static inline size_t ndm_strvec_size(
		const struct ndm_strvec_t *v)
{
	return v->__size;
}

static bool ndm_strvec_is_empty(
		const struct ndm_strvec_t *v) NDM_ATTR_WUR;

static inline bool ndm_strvec_is_empty(
		const struct ndm_strvec_t *v)
{
	return (v->__size == 0);
}

bool ndm_strvec_is_equal(
		const struct ndm_strvec_t *v,
		const struct ndm_strvec_t *r) NDM_ATTR_WUR;

static const char *ndm_strvec_at(
		const struct ndm_strvec_t *v,
		const size_t i) NDM_ATTR_WUR;

static inline const char *ndm_strvec_at(
		const struct ndm_strvec_t *v,
		const size_t i)
{
	return v->__data[i];
}

static const char *ndm_strvec_front(
		const struct ndm_strvec_t *v) NDM_ATTR_WUR;

static inline const char *ndm_strvec_front(
		const struct ndm_strvec_t *v)
{
	return v->__data[0];
}

static const char *ndm_strvec_back(
		const struct ndm_strvec_t *v) NDM_ATTR_WUR;

static inline const char *ndm_strvec_back(
		const struct ndm_strvec_t *v)
{
	return v->__data[v->__size - 1];
}

const char *ndm_strvec_insert_at(
		struct ndm_strvec_t *v,
		const size_t i,
		const char *const s) NDM_ATTR_WUR;

void ndm_strvec_remove_at(
		struct ndm_strvec_t *v,
		const size_t i);

static const char *ndm_strvec_push_front(
		struct ndm_strvec_t *v,
		const char *const s) NDM_ATTR_WUR;

static inline const char *ndm_strvec_push_front(
		struct ndm_strvec_t *v,
		const char *const s)
{
	return ndm_strvec_insert_at(v, 0, s);
}

static const char *ndm_strvec_push_back(
		struct ndm_strvec_t *v,
		const char *const s) NDM_ATTR_WUR;

static inline const char *ndm_strvec_push_back(
		struct ndm_strvec_t *v,
		const char *const s)
{
	return ndm_strvec_insert_at(v, v->__size, s);
}

static inline void ndm_strvec_pop_front(
		struct ndm_strvec_t *v)
{
	ndm_strvec_remove_at(v, 0);
}

static inline void ndm_strvec_pop_back(
		struct ndm_strvec_t *v)
{
	ndm_strvec_remove_at(v, v->__size - 1);
}

size_t ndm_strvec_find(
		const struct ndm_strvec_t *v,
		const char *const s) NDM_ATTR_WUR;

bool ndm_strvec_assign(
		struct ndm_strvec_t *v,
		const struct ndm_strvec_t *r) NDM_ATTR_WUR;

bool ndm_strvec_assign_array(
		struct ndm_strvec_t *v,
		const char **vec) NDM_ATTR_WUR;

bool ndm_strvec_append(
		struct ndm_strvec_t *v,
		const struct ndm_strvec_t *r) NDM_ATTR_WUR;

void ndm_strvec_swap(
		struct ndm_strvec_t *v,
		struct ndm_strvec_t *r);

static inline void ndm_strvec_truncate(
		struct ndm_strvec_t *v,
		const size_t size)
{
	while (v->__size > size) {
		ndm_strvec_pop_back(v);
	}
}

static inline void ndm_strvec_clear(
		struct ndm_strvec_t *v)
{
	ndm_strvec_truncate(v, 0);
}

void ndm_strvec_sort(
		struct ndm_strvec_t *v,
		int (*compare)(const char **l, const char **r));

void ndm_strvec_sort_ascending(
		struct ndm_strvec_t *v);
void ndm_strvec_sort_descending(
		struct ndm_strvec_t *v);

#endif	/* __NDM_STRVEC_H__ */

