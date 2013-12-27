#ifndef __NDM_INT_H__
#define __NDM_INT_H__

#include <limits.h>
#include <stdbool.h>
#include "attr.h"

/**
 * Non-zero value if @a t is a signed integer type, @c 0 — otherwise.
 *
 * @param t Type name.
 */

#define NDM_INT_IS_SIGNED(t)							\
	(!((t) 0 < (t) -1))

/**
 * Non-zero value if @a t is an integer type, described by two's complement,
 * @c 0 — otherwise.
 *
 * @param t Type name.
 */

#define NDM_INT_IS_TWOS_COMPLEMENT(t)					\
	((t) (-((t) 1)) == (t) (~((t) 1) + ((t) 1)))

/**
 * The minimum numeric value of @a t.
 *
 * @param t Type name.
 */

#define NDM_INT_MIN(t)									\
	((t) (NDM_INT_IS_SIGNED(t) ?						\
		~((t) 0) << (sizeof(t)*CHAR_BIT - 1) : (t) 0))

/**
 * The maximum numeric value of @a t.
 *
 * @param t Type name.
 */

#define NDM_INT_MAX(t)									\
	((t) (~((t) 0) - NDM_INT_MIN(t)))

/**
 * The minimum length of the string, enough to store
 * the maximum value of @a t.
 *
 * @param t Type name.
 */

#define NDM_INT_MAX_STRLEN(t)							\
	(((sizeof(t)*CHAR_BIT -								\
	   (NDM_INT_IS_SIGNED(t) ? 1 : 0))*151)/500 +		\
	 (NDM_INT_IS_SIGNED(t) ? 1 : 0) + 1)

/**
 * Size of buffer, enough to store the maximum value of @a t.
 * Is defined as \n NDM_INT_MAX_STRLEN + 1.
 *
 * @param t Type name.
 */

#define NDM_INT_MAX_BUFSIZE(t)							\
	(NDM_INT_MAX_STRLEN(t) + 1)

/**
 * The minimum length of the buffer (including the final null character),
 * enough to store the maximum value of the largest integer type for
 * the current architecture.
 */

#define NDM_INT_BUFSIZE									\
	NDM_MAX(											\
		NDM_INT_MAX_BUFSIZE(intmax_t),					\
		NDM_INT_MAX_BUFSIZE(uintmax_t))

/**
 * Non-zero value if @a n is an power of two, @c 0 — otherwise.
 *
 * @param n An integer constant.
 */

#define NDM_INT_IS_POWER_OF2(n)							\
	(((n) != 0) && !((n) & ((n) - 1)))

/**
 * Integer @a n which is alligned by @a align.
 *
 * @param n An integer constant.
 * @param align Alignment attribute.
 */

#define NDM_INT_ALIGN(n, align)							\
	(NDM_INT_IS_POWER_OF2(align) ?						\
	 (((n) + (align) - 1) & ~((align) - 1))	:			\
	 (((n - ((n) % (align))) + (((n) % (align)) == 0 ? 0 : (align)))))

/**
 * Convert string entry to its integer representation.
 * String must not contain any characters except digits.
 *
 * @param str String for conversion.
 * @param value Resulting value of the char type.
 *
 * @returns @c true if conversion process is successful, @c false — otherwise.
 */

bool ndm_int_parse_char(
		const char *const str,
		char *value) NDM_ATTR_WUR;

/**
 * Convert string entry to its unsigned char representation.
 * String must not contain any characters except digits.
 *
 * @param str String for conversion.
 * @param value Resulting value of the unsigned char type.
 *
 * @returns @c true if conversion process is successful, @c false — otherwise.
 */

bool ndm_int_parse_uchar(
		const char *const str,
		unsigned char *value) NDM_ATTR_WUR;

/**
 * Convert string entry to its integer representation.
 * String must not contain any characters except digits.
 *
 * @param str String for conversion.
 * @param value Resulting value of the short type.
 *
 * @returns @c true if conversion process is successful, @c false — otherwise.
 */

bool ndm_int_parse_short(
		const char *const str,
		short *value) NDM_ATTR_WUR;

/**
 * Convert string entry to its unsigned char representation.
 * String must not contain any characters except digits.
 *
 * @param str String for conversion.
 * @param value Resulting value of the unsigned short type.
 *
 * @returns @c true if conversion process is successful, @c false — otherwise.
 */

bool ndm_int_parse_ushort(
		const char *const str,
		unsigned short *value) NDM_ATTR_WUR;

/**
 * Convert string entry to its integer representation.
 * String must not contain any characters except digits.
 *
 * @param str String for conversion.
 * @param value Resulting value of the integer type.
 *
 * @returns @c true if conversion process is successful, @c false — otherwise.
 */

bool ndm_int_parse_int(
		const char *const str,
		int *value) NDM_ATTR_WUR;

/**
 * Convert string entry to its unsigned integer representation.
 * String must not contain any characters except digits.
 *
 * @param str String for conversion.
 * @param value Resulting value of the unsigned integer type.
 *
 * @returns @c true if conversion process is successful, @c false — otherwise.
 */

bool ndm_int_parse_uint(
		const char *const str,
		unsigned int *value) NDM_ATTR_WUR;

/**
 * Convert string entry to its long representation.
 * String must not contain any characters except digits.
 *
 * @param str String for conversion.
 * @param value Resulting value of the long type.
 *
 * @returns @c true if conversion process is successful, @c false — otherwise.
 */

bool ndm_int_parse_long(
		const char *const str,
		long *value) NDM_ATTR_WUR;

/**
 * Convert string entry to its unsigned long representation.
 * String must not contain any characters except digits.
 *
 * @param str String for conversion.
 * @param value Resulting value of the unsigned long type.
 *
 * @returns @c true if conversion process is successful, @c false — otherwise.
 */

bool ndm_int_parse_ulong(
		const char *const str,
		unsigned long *value) NDM_ATTR_WUR;

/**
 * Convert string entry to its long long representation.
 * String must not contain any characters except digits.
 *
 * @param str String for conversion.
 * @param value Resulting value of the long long type.
 *
 * @returns @c true if conversion process is successful, @c false — otherwise.
 */

bool ndm_int_parse_llong(
		const char *const str,
		long long *value) NDM_ATTR_WUR;

/**
 * Convert string entry to its unsigned long long representation.
 * String must not contain any characters except digits.
 *
 * @param str String for conversion.
 * @param value Resulting value of the unsigned long long type.
 *
 * @returns @c true if conversion process is successful, @c false — otherwise.
 */

bool ndm_int_parse_ullong(
		const char *const str,
		unsigned long long *value) NDM_ATTR_WUR;

#endif	/* __NDM_INT_H__ */

