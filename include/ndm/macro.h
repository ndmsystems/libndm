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

/**
 * @def NDM_PRINTF
 *
 * This attribute macro allows to assign printf-like or scanf-like
 * characteristics to the declared function, and this enables the compiler
 * to check the format string against the parameters provided throughout
 * the code. This is exceptionally helpful in tracking down hard-to-find
 * bugs.
 *
 * @param m The number of the "format string" parameter.
 * @param n The number of the first variadic parameter.
 *
 * @par Example
 * @code
 int debug(int level, const char *format, ...) NDM_PRINTF(1, 2);
 @endcode
 *
 * When the function is so declared, the compiler will examine the argument
 * lists.
 *
 * @code
 int n = debug(level, "s = %s", 35);
 // warning: format ‘%s’ expects argument of type ‘char*’, but
 // argument 3 has type ‘int’ [-Wformat]

 int n = debug(level, "i = %i, j = %i", i);
 // warning: format ‘%i’ expects a matching ‘int’ argument [-Wformat]
 @endcode
 *
 * @note When this attribute is used with a class member function, @a m and
 * @a n should be incremented by one, because class members always have
 * implicit @c this as a first argument.
 */

#define NDM_PRINTF(m, n) __attribute__((format(printf, m, n)))

/**
 * @def NDM_WUR
 *
 * This attribute causes a warning to be emitted if a caller of the function
 * with this attribute does not use its return value. This is useful for
 * functions where not checking the result is either a security problem or
 * always a bug.
 */

#define NDM_WUR __attribute__((warn_unused_result))

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

