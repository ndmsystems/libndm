#ifndef __NDM_LOG_H__
#define __NDM_LOG_H__

#include <stdbool.h>
#include "attr.h"

enum level_t
{
	LINFO,
	LWARNING,
	LERROR,
	LCRITICAL,
	LDEBUG
};

const char *ndm_log_get_ident(
		char *argv[]) NDM_ATTR_WUR;

bool ndm_log_init(
		const char *const ident,
		const char *const source,
		const bool console_mode,
		const bool daemon_mode) NDM_ATTR_WUR;

void ndm_log(
		const enum level_t level,
		const char *const format,
		...) NDM_ATTR_PRINTF(2, 3);

#define NDM_LOG_INFO(fmt, args...)		ndm_log(LINFO, fmt, ##args)
#define NDM_LOG_WARNING(fmt, args...) 	ndm_log(LWARNING, fmt, ##args)
#define NDM_LOG_ERROR(fmt, args...)		ndm_log(LERROR, fmt, ##args)
#define NDM_LOG_CRITICAL(fmt, args...)	ndm_log(LCRITICAL, fmt, ##args)

#ifndef NDEBUG
#define NDM_LOG_DEBUG(fmt, args...)		\
	ndm_log(LDEBUG,						\
		"[" __FILE__ ":" NDM_TO_STRING(__LINE__) "] " fmt, ##args)
#else	/* NDEBUG */
#define NDM_LOG_DEBUG(fmt, args...)
#endif	/* NDEBUG */

#endif	/* __NDM_LOG_H__ */

