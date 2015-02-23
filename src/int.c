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

#define NDM_INT_PARSE_(type, min, max, umax)							\
bool ndm_int_parse_##type(const char *const str, type *value)			\
{																		\
	long l;																\
																		\
	if (ndm_int_parse_long(str, &l) &&									\
		min <= l && l <= max)											\
	{																	\
		*value = (type) l;												\
																		\
		return true;													\
	}																	\
																		\
	errno = EINVAL;														\
																		\
	return false;														\
}																		\
																		\
bool ndm_int_parse_u##type(												\
		const char *const str,											\
		unsigned type *value)											\
{																		\
	unsigned long l;													\
																		\
	if (ndm_int_parse_ulong(str, &l) && l <= umax)						\
	{																	\
		*value = (unsigned type) l;										\
																		\
		return true;													\
	}																	\
																		\
	errno = EINVAL;														\
																		\
	return false;														\
}

NDM_INT_PARSE_(char, CHAR_MIN, CHAR_MAX, UCHAR_MAX)
NDM_INT_PARSE_(short, SHRT_MIN, SHRT_MAX, USHRT_MAX)
NDM_INT_PARSE_(int, INT_MIN, INT_MAX, UINT_MAX)

bool ndm_int_parse_long(const char *const str, long *value)
{
	char *e;

	if (!isdigit(*str) && *str != '-') {
		errno = EINVAL;
	} else {
		long v;

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

	if (!isdigit(*str)) {
		errno = EINVAL;
	} else {
		unsigned long v;

		errno = 0;
		v = strtoul(str, &e, 10);

		if (errno == 0 && *e == 0) {
			*value = v;

			return true;
		}
	}

	return false;
}

bool ndm_int_parse_llong(const char *const str, long long *value)
{
	char *e;

	if (!isdigit(*str) && *str != '-') {
		errno = EINVAL;
	} else {
		long long v;

		errno = 0;
		v = strtoll(str, &e, 10);

		if (errno == 0 && *e == 0) {
			*value = v;

			return true;
		}
	}

	return false;
}

bool ndm_int_parse_ullong(const char *const str, unsigned long long *value)
{
	char *e;

	if (!isdigit(*str)) {
		errno = EINVAL;
	} else {
		unsigned long long v;

		errno = 0;
		v = strtoull(str, &e, 10);

		if (errno == 0 && *e == 0) {
			*value = v;

			return true;
		}
	}

	return false;
}

