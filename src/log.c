#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <syslog.h>
#include <ndm/log.h>
#include <ndm/time.h>
#include <ndm/macro.h>

#define NDM_LOG_MESSAGE_SIZE_				4096

static const char *__ident = "";
static const char *__source = NULL;
static bool __console_mode = false;
static int __facility = LOG_USER;

const char *ndm_log_get_ident(
		char *argv[])
{
	const char *const name = strrchr(argv[0], '/');
	const char *const program = (name == NULL) ? argv[0] : name + 1;

	return program;
}

bool ndm_log_init(
		const char *const ident,
		const char *const source,
		const bool console_mode,
		const bool daemon_mode)
{
	__ident = ident;
	__source = source;
	__console_mode = console_mode;
	__facility = daemon_mode ? LOG_DAEMON : LOG_USER;

	if (!__console_mode) {
		openlog(ident, LOG_NDELAY, __facility);
	}

	return console_mode ? ndm_time_init() : true;
}

void ndm_log(
		const enum level_t level,
		const char *const format,
		...)
{
	va_list vargs;
	char message[NDM_LOG_MESSAGE_SIZE_];

	va_start(vargs, format);
	vsnprintf(message, sizeof(message), format, vargs);
	va_end(vargs);

	if (__console_mode) {
		syslog(__facility |
			((level == LINFO)  ?	LOG_INFO :
			 (level == LWARNING) ?	LOG_WARNING :
			 (level == LERROR) ?	LOG_ERR :
			 (level == LCRITICAL) ?	LOG_CRIT : LOG_DEBUG), ": %s%s%s",
			(__source == NULL) ? "" : __source,
			(__source == NULL) ? "" : ": ",
			message);
	} else {
		static const char *MONTHS[] = {
			"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
		};
		static const char *WEEKDAYS[] = {
			"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
		};
		struct timespec s;
		time_t now;
		struct tm t;

		ndm_time_get(&s);
		now = (time_t) ndm_time_to_sec(&s);
		localtime_r(&now, &t);

		fprintf(
			(level == LERROR || level == LCRITICAL) ? stderr : stdout,
			"[%c] %s %s %02i:%02i:%02i %s: %s%s%s%s\n",
			((level == LINFO)  ?	'I' :
			 (level == LWARNING) ?	'W' :
			 (level == LERROR) ?	'E' :
			 (level == LCRITICAL) ?	'C' : '-'),
			MONTHS[t.tm_mday % __NDM_ARRAY_SIZE__(MONTHS)],
			WEEKDAYS[t.tm_wday % __NDM_ARRAY_SIZE__(WEEKDAYS)],
			(int) t.tm_hour, (int) t.tm_min, (int) t.tm_sec,
			__ident,
			(__source == NULL) ? "" : __source,
			(__source == NULL) ? "" : ": ",
			message,
			(message[strlen(message) - 1] != '.') &&
			(level == LINFO ||
			 level == LWARNING ||
			 level == LERROR ||
			 level == LCRITICAL) ? "." : "");
	}
}

