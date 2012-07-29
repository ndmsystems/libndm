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

