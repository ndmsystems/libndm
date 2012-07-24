#ifndef __NDM_FEEDBACK_H__
#define __NDM_FEEDBACK_H__

#include <stdint.h>
#include <stdbool.h>
#include "attr.h"

#define NDM_FEEDBACK_ENV_SEPARATOR		"\n"

#define NDM_FEEDBACK_TIMEOUT_MSEC		15000	/* 15 sec.	*/

/**
 * #define ESEP NDM_FEEDBACK_ENV_SEPARATOR
 *
 * const char args[] =
 * {
 * 		"executable",
 * 		"arg1",
 * 		"arg2",
 * 		NULL
 * };
 *
 * if (ndm_feedback(
 * 		NDM_FEEDBACK_TIMEOUT_MSEC,
 * 		args,
 * 		"ENV1=%s" ESEP
 * 		"ENV2=%i" ESEP,
 * 		str_arg, int_arg))
 * {
 * 		...
 */

bool ndm_feedback(
		const int64_t timeout_msec,
		const char *const argv[],
		const char *const env_format,
		...) NDM_ATTR_WUR;

#endif	/* __NDM_FEEDBACK_H__ */

