#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ndm/stdio.h>
#include <ndm/stracc.h>

static void __ndm_stracc_invalidate(struct ndm_stracc_t *acc)
{
	free(acc->__data);
	acc->__data = NULL;
	acc->__size = 0;
	acc->__is_valid = false;
}

bool ndm_stracc_assign(
		struct ndm_stracc_t *a,
		const struct ndm_stracc_t *b)
{
	bool assigned = true;

	if (b->__size == 0) {
		ndm_stracc_clear(a);
		a->__is_valid = b->__is_valid;
	} else {
		char *data = malloc(b->__size + 1);

		if (data == NULL) {
			assigned = false;
		} else {
			a->__data = data;
			a->__size = b->__size;
			a->__is_valid = b->__is_valid;
			memcpy(a->__data, b->__data, a->__size + 1);
		}
	}

	return assigned;
}

bool ndm_stracc_is_equal(
		const struct ndm_stracc_t *a,
		const struct ndm_stracc_t *b)
{
	const bool a_valid = ndm_stracc_is_valid(a);
	const bool b_valid = ndm_stracc_is_valid(b);

	return
		(!a_valid && !b_valid) ||
		(a_valid && b_valid &&
		 ndm_stracc_size(a) == ndm_stracc_size(b) &&
		 memcmp(
			ndm_stracc_value(a),
			ndm_stracc_value(b),
			ndm_stracc_size(a)) == 0);
}

void ndm_stracc_swap(
		struct ndm_stracc_t *a,
		struct ndm_stracc_t *b)
{
	char *data = a->__data;
	const size_t size = a->__size;
	const bool is_valid = a->__is_valid;

	a->__data = b->__data;
	a->__size = b->__size;
	a->__is_valid = b->__is_valid;

	b->__data = data;
	b->__size = size;
	b->__is_valid = is_valid;
}

bool ndm_stracc_append(
		struct ndm_stracc_t *acc,
		const char *const format,
		...)
{
	if (acc->__is_valid) {
		char *s;
		int size;
		va_list ap;

		va_start(ap, format);
		size = ndm_vasprintf(&s, format, ap);
		va_end(ap);

		if (s == NULL || s < 0) {
			__ndm_stracc_invalidate(acc);
		} else
		if (size > 0) {
			char *p = realloc(acc->__data,
				acc->__size + ((size_t) size + 1));

			if (p == NULL) {
				__ndm_stracc_invalidate(acc);
			} else {
				acc->__data = p;

				if (acc->__size == 0) {
					*acc->__data = '\0';
				}

				snprintf(acc->__data + acc->__size,
					(size_t) size + 1, "%s", s);
				acc->__size += (size_t) size;
			}
		}

		free(s);
	}

	return acc->__is_valid;
}

const char *ndm_stracc_value(const struct ndm_stracc_t *acc)
{
	return (!acc->__is_valid || acc->__data == NULL) ? "" : acc->__data;
}

size_t ndm_stracc_size(const struct ndm_stracc_t *acc)
{
	return acc->__is_valid ? acc->__size : 0;
}

const char *ndm_stracc_next_cstr(
		const struct ndm_stracc_t *acc,
		const char **s)
{
	if (!acc->__is_valid ||
		*s < acc->__data ||
		*s >= acc->__data + acc->__size)
	{
		*s = NULL;
	} else {
		*s += strlen(*s) + 1;

		if (*s >= acc->__data + acc->__size) {
			*s = NULL;
		}
	}

	return *s;
}

void ndm_stracc_clear(struct ndm_stracc_t *acc)
{
	__ndm_stracc_invalidate(acc);
	acc->__is_valid = true;
}

