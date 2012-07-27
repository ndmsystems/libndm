#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ndm/stdio.h>
#include <ndm/stracc.h>

struct ndm_stracc_t
{
	char *str;
	size_t size;
	bool is_valid;
};

static void __ndm_stracc_invalidate(struct ndm_stracc_t *acc)
{
	free(acc->str);
	acc->str = NULL;
	acc->size = 0;
	acc->is_valid = false;
}

struct ndm_stracc_t *ndm_stracc_alloc()
{
	struct ndm_stracc_t *acc = malloc(sizeof(*acc));

	if (acc != NULL) {
		acc->str = NULL;
		acc->size = 0;
		acc->is_valid = true;
	}

	return acc;
}

void ndm_stracc_append(
		struct ndm_stracc_t *acc,
		const char *const format,
		...)
{
	if (acc->is_valid) {
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
			char *p = realloc(acc->str, acc->size + ((size_t) size + 1));

			if (p == NULL) {
				__ndm_stracc_invalidate(acc);
			} else {
				acc->str = p;

				if (acc->size == 0) {
					*acc->str = '\0';
				}

				snprintf(acc->str + acc->size, (size_t) size + 1, "%s", s);
				acc->size += (size_t) size;
			}
		}

		free(s);
	}
}

bool ndm_stracc_is_valid(struct ndm_stracc_t *acc)
{
	return acc->is_valid;
}

const char *ndm_stracc_value(struct ndm_stracc_t *acc)
{
	return
		(acc == NULL || !acc->is_valid || acc->str == NULL) ?
		"" : acc->str;
}

size_t ndm_stracc_size(struct ndm_stracc_t *acc)
{
	return acc->is_valid ? acc->size : 0;
}

void ndm_stracc_free(struct ndm_stracc_t **acc)
{
	if (*acc != NULL) {
		__ndm_stracc_invalidate(*acc);
		free(*acc);
		*acc = NULL;
	}
}

