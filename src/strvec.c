#include <stdlib.h>
#include <string.h>
#include <ndm/string.h>
#include <ndm/strvec.h>

typedef int (*__ndm_strvec_cmp_t)(const void *, const void *);

static const char *__EMPTY_ARRAY[] = {NULL};

const char **ndm_strvec_array(
		const struct ndm_strvec_t *v)
{
	return (v->__size == 0 || v->__data == NULL) ?
		(const char **) __EMPTY_ARRAY :
		(const char **) v->__data;
}

bool ndm_strvec_is_equal(
		const struct ndm_strvec_t *v,
		const struct ndm_strvec_t *r)
{
	bool equal = false;

	if (v->__size == r->__size) {
		size_t i = 0;

		while (i < v->__size && strcmp(v->__data[i], r->__data[i]) == 0) {
			++i;
		}

		equal = (i == v->__size) ? true : false;
	}

	return equal;
}

const char *ndm_strvec_insert_at(
		struct ndm_strvec_t *v,
		const size_t i,
		const char *const s)
{
	char *s_copy = ndm_string_dup(s);

	if (s_copy != NULL) {
		char **data = realloc(v->__data, sizeof(void *)*(v->__size + 2));

		if (data == NULL) {
			free(s_copy);
			s_copy = NULL;
		} else {
			size_t k = v->__size - 1;

			v->__data = data;
			v->__size++;

			while (k + 1 != i) {
				v->__data[k + 1] = v->__data[k];
				--k;
			}

			v->__data[i] = s_copy;
			v->__data[v->__size] = NULL;
		}
	}

	return s_copy;
}

void ndm_strvec_remove_at(
		struct ndm_strvec_t *v,
		const size_t i)
{
	size_t k = i;

	free(v->__data[k]);

	while (k < v->__size) {
		v->__data[k] = v->__data[k + 1];
		++k;
	}

	v->__size--;

	if (v->__size > 0) {
		v->__data = realloc(v->__data, sizeof(void *)*(v->__size + 1));
	} else {
		free(v->__data);
		v->__data = NULL;
	}
}

size_t ndm_strvec_find(
		const struct ndm_strvec_t *v,
		const char *const s)
{
	size_t i = 0;

	while (i < v->__size && strcmp(v->__data[i], s) != 0) {
		++i;
	}

	return i;
}

bool ndm_strvec_assign(
		struct ndm_strvec_t *v,
		const struct ndm_strvec_t *r)
{
	bool done = true;

	ndm_strvec_clear(v);

	if (!ndm_strvec_is_empty(r)) {
		v->__data = malloc(sizeof(void *)*(r->__size + 1));

		if (v->__data != NULL) {
			while (
				v->__size < r->__size &&
				(v->__data[v->__size] =
					ndm_string_dup(r->__data[v->__size])) != NULL)
			{
				++v->__size;
			}

			v->__data[v->__size] = NULL;

			if (v->__size != r->__size) {
				ndm_strvec_clear(v);
				done = false;
			}
		} else {
			v->__size = 0;
			done = false;
		}
	}

	return done ? true : false;
}

static bool __ndm_strvec_append_array(
		struct ndm_strvec_t *v,
		const char *const *a)
{
	size_t i = 0;

	while (a[i] != NULL && ndm_strvec_push_back(v, a[i]) != NULL) {
		++i;
	}

	if (a[i] != NULL) {
		ndm_strvec_clear(v);
	}

	return (a[i] == NULL) ? true : false;
}

bool ndm_strvec_assign_array(
		struct ndm_strvec_t *v,
		const char *const *a)
{
	ndm_strvec_clear(v);

	return __ndm_strvec_append_array(v, a);
}

bool ndm_strvec_append(
		struct ndm_strvec_t *v,
		const struct ndm_strvec_t *r)
{
	return __ndm_strvec_append_array(v, ndm_strvec_array(r));
}

void ndm_strvec_swap(
		struct ndm_strvec_t *v,
		struct ndm_strvec_t *r)
{
	char **const data = v->__data;
	const size_t size = v->__size;

	v->__data = r->__data;
	v->__size = r->__size;

	r->__data = data;
	r->__size = size;
}

void ndm_strvec_sort(
		struct ndm_strvec_t *v,
		int (*compare)(const char **l, const char **r))
{
	if (!ndm_strvec_is_empty(v)) {
		qsort(v->__data, v->__size, sizeof(char *),
			(__ndm_strvec_cmp_t) compare);
	}
}

static int __ndm_strvec_sort_by_ascending(
		const char **l,
		const char **r)
{
	return strcmp(*l, *r);
}

void ndm_strvec_sort_ascending(
		struct ndm_strvec_t *v)
{
	ndm_strvec_sort(v, __ndm_strvec_sort_by_ascending);
}

static int __ndm_strvec_sort_by_descending(
		const char **l,
		const char **r)
{
	return -strcmp(*l, *r);
}

void ndm_strvec_sort_descending(
		struct ndm_strvec_t *v)
{
	ndm_strvec_sort(v, __ndm_strvec_sort_by_descending);
}

