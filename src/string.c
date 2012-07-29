#include <string.h>
#include <stdlib.h>
#include <ndm/string.h>

char *ndm_string_dup(const char *const s)
{
	char *s_copy = malloc(strlen(s) + 1);

	if (s_copy != NULL) {
		strcpy(s_copy, s);
	}

	return s_copy;
}

char *ndm_string_ndup(
		const char *const value,
		const size_t length)
{
	size_t l = 0;
	char *val = NULL;

	/* a value may not be a null-terminated string */
	while (l < length && value[l] != '\0') {
		++l;
	}

	val = malloc(l + 1);

	if (val != NULL) {
		memcpy(val, value, l);
		val[l] = '\0';
	}

	return val;
}

