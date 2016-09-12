#ifndef __NDM_MACRO_H__
#define __NDM_MACRO_H__

#include <stdint.h>
#include <inttypes.h>
#include "int.h"

#ifndef NDEBUG
#include <ctype.h>
#endif

#define NDM_ARRAY_SIZE(a)				(sizeof(a)/sizeof((a)[0]))

#define NDM_MIN(a, b)					((a) < (b) ? (a) : (b))
#define NDM_MAX(a, b)					((a) > (b) ? (a) : (b))

#define NDM_STRINGIFY(s)				#s
#define NDM_TO_STRING(s)				NDM_STRINGIFY(s)

#define NDM_BUILD_ASSERT(name, expr)	\
enum { name = 1/(!!(expr)) }

#ifndef NDEBUG
#ifndef NDM_HEX_DUMP_WIDTH
#define NDM_HEX_DUMP_WIDTH				16
#endif	/* NDM_HEX_DUMP_WIDTH */
#define NDM_HEX_DUMP(p, size)											\
	do {																\
		size_t __b__ = 0;												\
																		\
		while (__b__ < (size_t) size) {									\
			size_t __i__ = 0;											\
			size_t __e__ = (size_t)										\
				NDM_MIN(NDM_HEX_DUMP_WIDTH, ((size_t) size) - __b__);	\
																		\
			printf("\n%08"PRIx32":", (uint32_t) __b__);					\
																		\
			while (__i__ < __e__) {										\
				printf(" %02"PRIx8, ((uint8_t *) p)[__b__ + __i__]);	\
				++__i__;												\
			}															\
																		\
			while (__i__ < NDM_HEX_DUMP_WIDTH) {						\
				printf("   ");											\
				++__i__;												\
			}															\
																		\
			printf(" | ");												\
			__i__ = 0;													\
																		\
			while (__i__ < __e__) {										\
				const char __c__ = ((char *) p)[__b__ + __i__];			\
																		\
				printf("%c", isprint(__c__) ? __c__ : '.');				\
				++__i__;												\
			}															\
																		\
			__b__ += NDM_HEX_DUMP_WIDTH;								\
		}																\
																		\
		printf("\n");													\
	} while (0)
#else	/* NDEBUG */
#define NDM_HEX_DUMP(p, size)
#endif	/* NDEBUG */

#endif	/* __NDM_MACRO_H__ */

