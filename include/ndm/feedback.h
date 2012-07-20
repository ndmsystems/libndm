#ifndef __NDM_FEEDBACK_H__
#define __NDM_FEEDBACK_H__

#include <stdbool.h>

#define NDM_FEEDBACK_ENV_SEPARATOR		'\n'

/**
 * #define ESEP NDM_FEEDBACK_ENV_SEPARATOR
 *
 * if (ndm_feedback(executable,
 * 		"ENV1=%s" ESEP
 * 		"ENV2=%i" ESEP,
 * 		str_arg, int_arg))
 * {
 * 		...
 */

bool ndm_feedback(
		const char *const executable,
		const char *const env_format,
		...);

#endif	/* __NDM_FEEDBACK_H__ */

