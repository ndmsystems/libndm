#include "ndm_common.h"

int ndm_str2long(char *string_value, unsigned long *value)
{
	char *e;
	unsigned long v;
	int converted = 0;

	if (strlen(string_value) > 0) {
		errno = 0;
		v = strtoul(string_value, &e, 10);

		if (errno == 0 && *e == 0) {
			converted = 1;
			*value = v;
		}
	}

	return converted;
}

char *ndm_get_ident(char *full_path)
{
	const char *const name = strrchr(full_path, '/');
	const char *const ident = (name == NULL) ? full_path : name + 1;
	return ident;
}
