#ifndef __NDM_ATTR_H__
#define __NDM_ATTR_H__

#ifdef DOXYGEN // {

/**
 * Allows to assign printf-like or scanf-like characteristics to the declared
 * function, and this enables the compiler to check the format string against
 * the parameters provided throughout the code. This is exceptionally helpful
 * in tracking down hard-to-find bugs.
 *
 * @param m The number of the "format string" parameter.
 * @param n The number of the first variadic parameter.
 *
 * @par Example
 * @code
 * int debug(int level, const char *format, ...) NDM_ATTR_PRINTF(1, 2);
 * @endcode
 *
 * When the function is declared in this way the compiler will examine
 * the argument lists.
 *
 * @code
 * int n = debug(level, "s = %s", 35);
 * // warning: format ‘%s’ expects argument of ‘char*’ type, but third
 * // argument has ‘int’ type [-Wformat]

 * int n = debug(level, "i = %i, j = %i", i);
 * // warning: format ‘%i’ expects a matching argument
 * // of ‘int’ type [-Wformat]
 * @endcode
 */

#define NDM_ATTR_PRINTF(m, n)
#else // } {
#define NDM_ATTR_PRINTF(m, n) __attribute__((format(printf, m, n)))
#endif // } !DOXYGEN

#ifdef DOXYGEN // {
/**
 * This attribute causes a warning to be emitted if a caller of the function
 * with this attribute does not use its return value. This is useful for
 * functions where not checking the result is either a security problem or
 * always a bug.
 *
 * @par Example
 * @code
 *       int fn () NDM_ATTR_WUR;
 *       int foo ()
 *       {
 *         if (fn () < 0) return -1;
 *         fn ();
 *         return 0;
 *        }
 * // warning: ignoring return value of ‘fn’, declared with attribute
 * // warn_unused_result [-Wunused-result]
 * @endcode
 *
 * Results in warning on line 5.
 */

#define NDM_ATTR_WUR
#else // } {
#define NDM_ATTR_WUR __attribute__((warn_unused_result))
#endif // } !DOXYGEN

#ifdef DOXYGEN // {
/**
 * Specifies that a variable or structure field should have the smallest
 * possible alignment — one byte for a variable, and one bit for a field,
 * unless you specify a larger value with the aligned attribute.
 *
 * @par Example
 * Here is a structure in which the field x is packed, so that it immediately
 * follows a:
 * @code
 *        struct foo
 *        {
 *          char a;
 *          int x[2] NDM_ATTR_PACKED;
 *        };
 * @endcode
 *
 * If attribute is used after the whole structure it will be packed entirely:
 * @code
 *        struct foo
 *        {
 *          char a;
 *          int x[2];
 *        } NDM_ATTR_PACKED;
 * @endcode
 */

#define NDM_ATTR_PACKED
#else // } {
#define NDM_ATTR_PACKED __attribute__((packed))
#endif // } !DOXYGEN

#endif	/* __NDM_ATTR_H__ */

