#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ndm/int.h>

bool ndm_int_parse_long(char *str, long *value)
{
	char *e;
	long v;

	if (strlen(str) > 0) {
		errno = 0;
		v = strtol(str, &e, 10);

		if (errno == 0 && *e == 0) {
			*value = v;

			return true;
		}
	}

	return false;
}

bool ndm_int_parse_ulong(char *str, unsigned long *value)
{
	char *e;
	long v;

	if (strlen(str) > 0) {
		errno = 0;
		v = strtoul(str, &e, 10);

		if (errno == 0 && *e == 0) {
			*value = v;

			return true;
		}
	}

	return false;
}

