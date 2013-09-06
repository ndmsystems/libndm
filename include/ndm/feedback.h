#ifndef __NDM_FEEDBACK_H__
#define __NDM_FEEDBACK_H__

#include <stdint.h>
#include <stdbool.h>
#include "attr.h"

/**
 * The character which is used to separate the environment variables
 * in the format string when ndm_feedback() is calling. Now it is newline
 * character.
 */

#define NDM_FEEDBACK_ENV_SEPARATOR		"\n"

/**
 * Default timeout for ndm_feedback() function call, in milliseconds.
 */

#define NDM_FEEDBACK_TIMEOUT_MSEC		15000	/* 15 sec.	*/

/*
 * define ESEP NDM_FEEDBACK_ENV_SEPARATOR
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

/**
 * Calls the external application and checks the result of its launch.
 * Application is a special program (or a symbolic link to it), which is able
 * to send its arguments and environment variables to the NDM core.
 *
 * @param timeout_msec The maximum time allowed to run, in milliseconds.
 * @param argv An array of arguments for the application to run. It must contain
 * at least one entry with a pointer to the name of the caller, as well as
 * an optional set of additional arguments. The list must be terminated with
 * @c NULL.
 * @param env_format String with a format that creates a list of environment
 * variables. The format is a string (similar to the format string from a group
 * of functions @b printf) separated by '=' and NDM_FEEDBACK_ENV_SEPARATOR
 * characters.
 *
 * @returns @c true if launch and sending of arguments and environment variables
 * are successful, @c false — otherwise, @a errno stores an error code.
 */

bool ndm_feedback(
		const int64_t timeout_msec,
		const char *const argv[],
		const char *const env_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(3, 4);

#endif	/* __NDM_FEEDBACK_H__ */

