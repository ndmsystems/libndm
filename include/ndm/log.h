#ifndef __NDM_LOG_H__
#define __NDM_LOG_H__

#include <stdarg.h>
#include <stdbool.h>
#include "attr.h"
#include "macro.h"

enum level_t
{
	LINFO,
	LWARNING,
	LERROR,
	LCRITICAL,
	LDEBUG
};

enum debug_level_t
{
	LDEBUG_OFF	= 0,
	LDEBUG_1	= 1,
	LDEBUG_2	= 2,
	LDEBUG_3	= 3,
	LDEBUG_ALL	= 3
};

const char *ndm_log_get_ident(
		char *argv[]) NDM_ATTR_WUR;

bool ndm_log_init(
		const char *const ident,
		const char *const source,
		const bool console_mode,
		const bool daemon_mode) NDM_ATTR_WUR;

void ndm_log_close();

void ndm_log(
		const enum level_t level,
		const char *const format,
		...) NDM_ATTR_PRINTF(2, 3);

bool ndm_log_debug(
		const enum debug_level_t debug_level,
		const char *const format,
		...) NDM_ATTR_PRINTF(2, 3);

int ndm_log_get_debug();

void ndm_log_set_debug(
		const enum debug_level_t debug_level);

void ndm_vlog(
		const enum level_t level,
		const char *const format,
		va_list ap) NDM_ATTR_PRINTF(2, 0);

#define NDM_LOG_INFO(fmt, args...)		ndm_log(LINFO, fmt, ##args)
#define NDM_LOG_WARNING(fmt, args...)	ndm_log(LWARNING, fmt, ##args)
#define NDM_LOG_ERROR(fmt, args...)		ndm_log(LERROR, fmt, ##args)
#define NDM_LOG_CRITICAL(fmt, args...)	ndm_log(LCRITICAL, fmt, ##args)

#define NDM_LOG_DEBUG(fmt, args...)		ndm_log_debug(LDEBUG_1, fmt, ##args)
#define NDM_LOG_DEBUG_1(fmt, args...)	ndm_log_debug(LDEBUG_1, fmt, ##args)
#define NDM_LOG_DEBUG_2(fmt, args...)	ndm_log_debug(LDEBUG_2, fmt, ##args)
#define NDM_LOG_DEBUG_3(fmt, args...)	ndm_log_debug(LDEBUG_3, fmt, ##args)

#endif	/* __NDM_LOG_H__ */

