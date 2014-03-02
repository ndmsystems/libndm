#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ndm/ptrvec.h>

#define NDM_PTRVEC_BLOCK_SIZE_  (1U << 3) /* must be power of 2 */

static void ndm_ptrvec_bubble_sort_(
		void **base,
		size_t n,
		int (*compare)(const void *, const void *))
{
	if (n < 2) {
		return;
	}

	void **lo = base;
	void **hi = base + (n - 1);

	while (hi > lo) {
		for(void **p = lo; p < hi; ++p) {
			void **q = p + 1;

			if (compare(p, q) > 0 ) {
				void *tmp = *p;

				*p = *q;
				*q = tmp;
			}
		}

		--hi;
	}
}

bool ndm_ptrvec_insert(
		struct ndm_ptrvec_t *v,
		const size_t index,
		void *p)
{
	assert (index <= v->size_);

	const bool do_realloc = v->size_ > 0 ?
		(v->size_ & (NDM_PTRVEC_BLOCK_SIZE_ - 1)) == 0 :
		v->data_ == NULL;

	if (do_realloc) {
		void **data = (void **) realloc(
			v->data_,
			(v->size_ + NDM_PTRVEC_BLOCK_SIZE_) * sizeof(*v));

		if (v == NULL) {
			return false;
		}

		v->data_ = data;
	}

	if (index < v->size_) {
		memmove(
			v->data_ + index + 1,
			v->data_ + index,
			(v->size_ - index) * sizeof(void *));
	}

	v->data_[index] = p;
	++v->size_;

	return true;
}

void ndm_ptrvec_remove(
		struct ndm_ptrvec_t *v,
		const size_t index)
{
	assert (index < v->size_);

	if (index < v->size_ - 1) {
		memmove(
			v->data_ + index,
			v->data_ + index + 1,
			(v->size_ - index - 1) * sizeof(void *));
	}

	--v->size_;

	if ((v->size_ & (NDM_PTRVEC_BLOCK_SIZE_ - 1)) == 0 && v->size_ > 0) {
		v->data_ = (void **) realloc(v->data_, v->size_ * sizeof(void *));
	}
}

void ndm_ptrvec_clear(
		struct ndm_ptrvec_t *v)
{
	free(v->data_);
	ndm_ptrvec_init(v);
}

size_t ndm_ptrvec_find(
		struct ndm_ptrvec_t *v,
		const void *p)
{
	size_t i = 0;

	while (i < v->size_) {
		if (v->data_[i] == p) {
			return i;
		}

		++i;
	}

	return i;
}

void ndm_ptrvec_sort(
		struct ndm_ptrvec_t *v,
		int (*compare)(const void *, const void *))
{
	qsort(v->data_, v->size_, sizeof(void *), compare);
}

void ndm_ptrvec_bubble_sort(
		struct ndm_ptrvec_t *v,
		int (*compare)(const void *, const void *))
{
	ndm_ptrvec_bubble_sort_(v->data_, v->size_, compare);
}

bool ndm_ptrvec_assign(
		struct ndm_ptrvec_t *dst,
		const struct ndm_ptrvec_t *src)
{
	void **data = (void **) malloc(
		((src->size_ + NDM_PTRVEC_BLOCK_SIZE_ - 1) &
			~(NDM_PTRVEC_BLOCK_SIZE_ - 1)) * sizeof(void *));

	if (data == NULL) {
		return false;
	}

	free(dst->data_);

	dst->data_ = data;
	dst->size_ = src->size_;

	memcpy(dst->data_, src->data_, src->size_ * sizeof(void *));

	return true;
}

void ndm_ptrvec_rotate_left(
		struct ndm_ptrvec_t *v)
{
	if (v->size_ > 1) {
		void *p = v->data_[0];
		size_t i = 0;

		while (i < v->size_ - 1) {
			v->data_[i] = v->data_[i + 1];
			++i;
		}

		v->data_[v->size_ - 1] = p;
	}
}
