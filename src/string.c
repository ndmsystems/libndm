#include <string.h>
#include <stdlib.h>
#include <ndm/string.h>

char *ndm_string_dup(const char *const s)
{
	const size_t buf_size = strlen(s) + 1;
	char *s_copy = malloc(buf_size);

	if (s_copy != NULL) {
		memcpy(s_copy, s, buf_size);
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

