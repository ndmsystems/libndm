#ifndef __NDM_CODE_H__
#define __NDM_CODE_H__

#include <stdint.h>
#include <inttypes.h>

typedef uint32_t ndm_code_t;

/* ndm_code_t is unsigned integer type always */

/**
 * Macro is used in printf-like functions to print a message code in
 * octal format.
 * @par Example
 * @snippet core_message_code.c octal code printf
 */
#define NDM_CODE_PRIo	PRIo32

/**
 * Macro is used in printf-like functions to print a message code in
 * unsigned decimal format.
 * @par Example
 * @snippet core_message_code.c decimal code printf
 */

#define NDM_CODE_PRIu	PRIu32

/**
 * Macro is used in printf-like functions to print a message code in
 * hexadecimal format.
 * @par Example
 * @snippet core_message_code.c hex code printf
 */

#define NDM_CODE_PRIx	PRIx32

/**
 * Macro is used in printf-like functions to print a message code in
 * hexadecimal format (uppercase).
 */

#define NDM_CODE_PRIX	PRIX32

/**
 * Macro is used in scanf-like functions for unsigned octal format.
 */

#define NDM_CODE_SCNo	SCNo32

/**
 * Macro is used in scanf-like functions for unsigned decimal format.
 */

#define NDM_CODE_SCNu	SCNu32

/**
 * Macro is used in scanf-like functions for hexadecimal format.
 */

#define NDM_CODE_SCNx	SCNx32

/**
 * Macro is used to form returned values of @a info type.
 *
 * @param group Group of codes, which is the NDM class number (12 bits).
 * @param local Returned value (16 bits).
 */

#define NDM_CODE_I(group, local)	(0x00000000 | (group << 16) | (local))

/**
 * Macro is used to form returned values of @a warning type.
 *
 * @param group Group of codes, which is the NDM class number (12 bits).
 * @param local Returned value (16 bits).
 */

#define NDM_CODE_W(group, local)	(0x40000000 | (group << 16) | (local))

/**
 * Macro is used to form returned values of @a error type.
 *
 * @param group Group of codes, which is the NDM class number (12 bits).
 * @param local Returned value (16 bits).
 */

#define NDM_CODE_E(group, local)	(0x80000000 | (group << 16) | (local))

/**
 * Macro is used to form returned values of @a critical type.
 *
 * @param group Group of codes, which is the NDM class number (12 bits).
 * @param local Returned value (16 bits).
 */

#define NDM_CODE_C(group, local)	(0xc0000000 | (group << 16) | (local))

/**
 * Number of special codes group used to form returned values which ​​are not tied
 * to any class (global error codes).
 */

#define NDM_FAIL_CODEGROUP			0xffd

/**
 * Macro is used to form returned values related to memory allocation errors.
 */

#define NDM_OOM_CODEGROUP			0xffe

/**
 * Number of special codes group used to form returned values ​​for NDM special
 * situations encoding: presence/absence of the system updates, absence of
 * command configurations, error codes which are transferred from deprecated
 * code (during translation codes from exceptions).
 */

#define NDM_SPECIAL_CODEGROUP		0xfff

/**
 * Macro is used to check the code type.
 *
 * @param code Code of returned value to check.
 *
 * @b Returns A nonzero value if the code type is @a info or @a warning.
 */

#define NDM_SUCCEEDED(code)			(((code) & 0x80000000) == 0x00000000)

/**
 * Macro is used to check the code type.
 *
 * @param code Code of returned value to check.
 *
 * @b Returns A nonzero value if the code type is @a error or @a critical.
 */

#define NDM_FAILED(code)			(((code) & 0x80000000) == 0x80000000)

/**
 * Macro is used to check the code type.
 *
 * @param code Code of returned value to check.
 *
 * @b Returns A nonzero value if the code type is @a warning.
 */

#define NDM_WARNING(code)			(((code) & 0xc0000000) == 0x40000000)

/**
 * Macro is used to get number of codes group and returned value both.
 *
 * @param code Code of returned value.
 */

#define NDM_CODEID(code)			( (code) & 0x0fffffff)

/**
 * Macro is used to get number of codes group by returned value.
 *
 * @param code Code of returned value.
 */

#define NDM_CODEGROUP(code)			(((code) & 0x0fff0000) >> 16)

/**
 * Macro is used to get number of a local code by returned value.
 *
 * @param code Code of returned value.
 */

#define NDM_CODELOCAL(code)			(((code) & 0x0000ffff) >>  0)

#endif	/* __NDM_CODE_H__ */
