#ifndef __NDM_INT_H__
#define __NDM_INT_H__

#include <stdbool.h>
#include "macro.h"

#define NDM_INT_IS_SIGNED(t)							\
	(!((t) 0 < (t) -1))

#define NDM_INT_IS_TWOS_COMPLEMENT(t)					\
	((t) (-((t) 1)) == (t) (~((t) 1) + ((t) 1)))

#define NDM_INT_MIN(t)									\
	((t) (NDM_INT_IS_SIGNED(t) ?						\
		~((t) 0) << (sizeof(t)*CHAR_BIT - 1) : (t) 0))

#define NDM_INT_MAX(t)									\
	((t) (~((t) 0) - NDM_INT_MIN(t)))

#define NDM_INT_MAX_STRLEN(t)							\
	(((sizeof(t)*CHAR_BIT -								\
	   (NDM_INT_IS_SIGNED(t) ? 1 : 0))*151)/500 +		\
	 (NDM_INT_IS_SIGNED(t) ? 1 : 0) + 1)

#define NDM_INT_MAX_BUFSIZE(t)							\
	(NDM_INT_MAX_STRLEN(t) + 1)

#define NDM_INT_BUFSIZE									\
	NDM_MAX(											\
		NDM_INT_MAX_BUFSIZE(intmax_t),					\
		NDM_INT_MAX_BUFSIZE(uintmax_t))

bool ndm_int_parse_long(
		const char *const str,
		long *value) NDM_WUR;

bool ndm_int_parse_ulong(
		const char *const str,
		unsigned long *value) NDM_WUR;

#endif	/* __NDM_INT_H__ */

