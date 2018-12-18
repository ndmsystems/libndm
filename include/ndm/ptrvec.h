#ifndef __NDM_PTRVEC_H__
#define __NDM_PTRVEC_H__

#include <stddef.h>
#include <assert.h>
#include <stdbool.h>
#include "attr.h"

#define NDM_PTRVEC_INITIALIZER									\
	{.data_ = NULL, .size_ = 0}

struct ndm_ptrvec_t
{
	void **data_;
	size_t size_;
};

static inline void ndm_ptrvec_init(
		struct ndm_ptrvec_t *v)
{
	v->data_ = NULL;
	v->size_ = 0;
}

static size_t ndm_ptrvec_size(
		const struct ndm_ptrvec_t *v) NDM_ATTR_WUR;

static inline size_t ndm_ptrvec_size(
		const struct ndm_ptrvec_t *v)
{
	return v->size_;
}

static size_t ndm_ptrvec_is_empty(
		const struct ndm_ptrvec_t *v) NDM_ATTR_WUR;

static inline size_t ndm_ptrvec_is_empty(
		const struct ndm_ptrvec_t *v)
{
	return ndm_ptrvec_size(v) == 0;
}

static void *ndm_ptrvec_at(
		const struct ndm_ptrvec_t *v,
		const size_t idx) NDM_ATTR_WUR;

static inline void *ndm_ptrvec_at(
		const struct ndm_ptrvec_t *v,
		const size_t idx)
{
	assert (idx < v->size_);

	return v->data_[idx];
}

static inline void ndm_ptrvec_set(
		struct ndm_ptrvec_t *v,
		const size_t idx,
		void *p)
{
	assert (idx < v->size_);

	v->data_[idx] = p;
}

bool ndm_ptrvec_insert(
		struct ndm_ptrvec_t *v,
		const size_t idx,
		void *p) NDM_ATTR_WUR;

void ndm_ptrvec_remove(
		struct ndm_ptrvec_t *v,
		const size_t idx);

static bool ndm_ptrvec_push_back(
		struct ndm_ptrvec_t *v,
		void *p) NDM_ATTR_WUR;

static inline bool ndm_ptrvec_push_back(
		struct ndm_ptrvec_t *v,
		void *p)
{
	return ndm_ptrvec_insert(v, v->size_, p);
}

static bool ndm_ptrvec_push_front(
		struct ndm_ptrvec_t *v,
		void *p) NDM_ATTR_WUR;

static inline bool ndm_ptrvec_push_front(
		struct ndm_ptrvec_t *v,
		void *p)
{
	return ndm_ptrvec_insert(v, 0, p);
}

static inline void ndm_ptrvec_pop_back(
		struct ndm_ptrvec_t *v)
{
	ndm_ptrvec_remove(v, v->size_ - 1);
}

static inline void ndm_ptrvec_pop_front(
		struct ndm_ptrvec_t *v)
{
	ndm_ptrvec_remove(v, 0);
}

void ndm_ptrvec_clear(
		struct ndm_ptrvec_t *v);

size_t ndm_ptrvec_find(
		struct ndm_ptrvec_t *v,
		const void *p) NDM_ATTR_WUR;

void ndm_ptrvec_sort(
		struct ndm_ptrvec_t *v,
		int (*compare)(const void *lhs, const void *rhs));

void ndm_ptrvec_bubble_sort(
		struct ndm_ptrvec_t *v,
		int (*compare)(const void *lhs, const void *rhs));

static inline void ndm_ptrvec_swap(
		struct ndm_ptrvec_t *l,
		struct ndm_ptrvec_t *r)
{
	struct ndm_ptrvec_t t = *l;

	*l = *r;
	*r = t;
}

bool ndm_ptrvec_assign(
		struct ndm_ptrvec_t *dst,
		const struct ndm_ptrvec_t *src) NDM_ATTR_WUR;

void ndm_ptrvec_rotate_left(
		struct ndm_ptrvec_t *v);

static void **ndm_ptrvec_ptr(
		struct ndm_ptrvec_t *v) NDM_ATTR_WUR;

static inline void **ndm_ptrvec_ptr(
		struct ndm_ptrvec_t *v)
{
	return v->data_;
}

#endif /* __NDM_PTRVEC_H__ */

