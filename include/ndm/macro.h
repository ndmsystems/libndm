#ifndef __NDM_MACRO_H__
#define __NDM_MACRO_H__

#include <stdint.h>
#include <inttypes.h>

#define NDM_ARRAY_SIZE(a)				(sizeof(a)/sizeof((a)[0]))

#define NDM_MIN(a, b)					((a) < (b) ? (a) : (b))
#define NDM_MAX(a, b)					((a) > (b) ? (a) : (b))

#define NDM_STRINGIFY(s)				#s
#define NDM_TO_STRING(s)				NDM_STRINGIFY(s)

#define NDM_BUILD_ASSERT(name, expr)	\
enum { name = 1/(!!expr) }

#ifndef NDEBUG
#define NDM_HEX_DUMP(p, size)								\
	do {													\
		unsigned long __i__ = 0;							\
															\
		while (__i__ < size) {								\
			if (__i__ % 16 == 0) {							\
				printf("\n%08"PRIx32":", (uint32_t) __i__);	\
			}												\
															\
			printf(" %02"PRIx8, ((uint8_t *) p)[__i__++]);	\
		}													\
															\
		if ((__i__ - 1) % 16 != 0) {						\
			printf("\n");									\
		}													\
	} while (0)
#else	/* NDEBUG */
#define NDM_HEX_DUMP(p, size)
#endif	/* NDEBUG */

#endif	/* __NDM_MACRO_H__ */

