#ifndef __NDM_MACRO_H__
#define __NDM_MACRO_H__

#include <stdint.h>
#include <inttypes.h>

#define __NDM_ARRAY_SIZE__(a)		(sizeof(a)/sizeof((a)[0]))

#define __NDM_MIN__(a, b)			((a) < (b) ? (a) : (b))
#define __NDM_MAX__(a, b)			((a) > (b) ? (a) : (b))

#ifndef NDEBUG
#define __NDM_HEX_DUMP__(p, size)							\
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
#define __NDM_HEX_DUMP__(p, size)
#endif	/* NDEBUG */

#endif	/* __NDM_MACRO_H__ */

