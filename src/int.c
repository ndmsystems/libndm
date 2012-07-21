#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ndm/int.h>
#include <ndm/macro.h>

NDM_BUILD_ASSERT(signed_char, NDM_INT_IS_SIGNED(char));
NDM_BUILD_ASSERT(signed_short, NDM_INT_IS_SIGNED(short));
NDM_BUILD_ASSERT(signed_int, NDM_INT_IS_SIGNED(int));
NDM_BUILD_ASSERT(signed_long, NDM_INT_IS_SIGNED(long));
NDM_BUILD_ASSERT(signed_long_long, NDM_INT_IS_SIGNED(long long));
NDM_BUILD_ASSERT(twos_complement, NDM_INT_IS_TWOS_COMPLEMENT(int));

bool ndm_int_parse_long(const char *const str, long *value)
{
	char *e;
	long v;

	if (!isdigit(*str) && *str != '-') {
		errno = EINVAL;
	} else {
		errno = 0;
		v = strtol(str, &e, 10);

		if (errno == 0 && *e == 0) {
			*value = v;

			return true;
		}
	}

	return false;
}

bool ndm_int_parse_ulong(const char *const str, unsigned long *value)
{
	char *e;
	unsigned long v;

	if (!isdigit(*str)) {
		errno = EINVAL;
	} else {
		errno = 0;
		v = strtoul(str, &e, 10);

		if (errno == 0 && *e == 0) {
			*value = v;

			return true;
		}
	}

	return false;
}

