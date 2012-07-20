#include "ndm_common.h"

const char *ident = NULL;

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

void ndm_get_ident(const char *full_path)
{
	const char *name = NULL;
	name = strrchr(full_path, '/');

/*
	if (name)
		name = name + 1;
	else 
		name = full_path;
*/
	ident = name ? name + 1 : full_path;

	return;
}
