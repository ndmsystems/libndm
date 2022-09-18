#ifndef __NDM_CORE_HIDDEN_H__
#define __NDM_CORE_HIDDEN_H__

/**
 * Send a feedback request to the NDM core and check a core handle result.
 *
 * @param timeout_msec The maximum time allowed to wait, in milliseconds.
 * @param argv An array of environment variables for the feedback to send.
 * It must contain at least one entry with a pointer to the name of
 * the caller, as well as an optional set of additional arguments.
 * The list must be terminated with @c NULL.
 * @param env_argv An array of strings, conventionally of the form key=value,
 * which are passed as feedback environment.
 * The array must be terminated with @c NULL.
 *
 * @returns @c true if request handling is successfull, @c false —
 * otherwise, @a errno stores an error code.
 */

bool ndm_core_feedback_ve(
		const int64_t timeout_msec,
		const char *const argv[],
		const char *const env_argv[]);

#endif	/* __NDM_CORE_HIDDEN_H__ */

