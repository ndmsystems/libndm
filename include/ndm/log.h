#ifndef __NDM_LOG_H__
#define __NDM_LOG_H__

#include <stdbool.h>

enum level_t
{
	LINFO,
	LWARNING,
	LERROR,
	LCRITICAL,
	LDEBUG
};

const char *ndm_log_get_ident(
		char *argv[]);

bool ndm_log_init(
		const char *const ident,
		const char *const source,
		const bool console_mode,
		const bool daemon_mode);

void ndm_log(
		const enum level_t level,
		const char *const format,
		...);

#define NDM_LOG_INFO(fmt, ...)		ndm_log(LINFO, fmt, __VA_ARGS__)
#define NDM_LOG_WARNING(fmt, ...) 	ndm_log(LWARNING, fmt, __VA_ARGS__)
#define NDM_LOG_ERROR(fmt, ...)		ndm_log(LERROR, fmt, __VA_ARGS__)
#define NDM_LOG_CRITICAL(fmt, ...)	ndm_log(LCRITICAL, fmt, __VA_ARGS__)

#ifndef NDEBUG
#define NDM_LOG_DEBUG(fmt, ...)		ndm_log(LDEBUG, fmt, __VA_ARGS__)
#else	/* NDEBUG */
#define NDM_LOG_DEBUG(fmt, ...)
#endif	/* NDEBUG */

#endif	/* __NDM_LOG_H__ */

