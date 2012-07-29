#ifndef __NDM_STRACC_H__
#define __NDM_STRACC_H__

#include <stddef.h>
#include <stdbool.h>
#include "attr.h"

#define NDM_STRACC_INITIALIZER							\
	{.__data = NULL, .__size = 0, .__is_valid = true}

struct ndm_stracc_t
{
	char *__data;
	size_t __size;
	bool __is_valid;
};

static inline void ndm_stracc_init(
		struct ndm_stracc_t *a)
{
	a->__data = NULL;
	a->__size = 0;
	a->__is_valid = true;
}

bool ndm_stracc_assign(
		struct ndm_stracc_t *a,
		const struct ndm_stracc_t *b) NDM_ATTR_WUR;

bool ndm_stracc_is_equal(
		const struct ndm_stracc_t *a,
		const struct ndm_stracc_t *b) NDM_ATTR_WUR;

void ndm_stracc_swap(
		struct ndm_stracc_t *a,
		struct ndm_stracc_t *b);

bool ndm_stracc_append(
		struct ndm_stracc_t *a,
		const char *const format,
		...) NDM_ATTR_PRINTF(2, 3);

static bool ndm_stracc_is_valid(
		const struct ndm_stracc_t *a) NDM_ATTR_WUR;

static inline bool ndm_stracc_is_valid(
		const struct ndm_stracc_t *a)
{
	return a->__is_valid;
}

const char *ndm_stracc_value(
		const struct ndm_stracc_t *a) NDM_ATTR_WUR;

size_t ndm_stracc_size(
		const struct ndm_stracc_t *a) NDM_ATTR_WUR;

static bool ndm_stracc_is_empty(
		const struct ndm_stracc_t *a) NDM_ATTR_WUR;

static inline bool ndm_stracc_is_empty(
		const struct ndm_stracc_t *a)
{
	return ndm_stracc_size(a) == 0;
}

const char *ndm_stracc_next_cstr(
		const struct ndm_stracc_t *a,
		const char **s) NDM_ATTR_WUR;

void ndm_stracc_clear(
		struct ndm_stracc_t *a);

#endif	/* __NDM_STRACC_H__ */

