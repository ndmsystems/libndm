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
	va_list ap;

	va_start(ap, format);
	ndm_vlog(level, format, ap);
	va_end(ap);
}

void ndm_vlog(
		const enum level_t level,
		const char *const format,
		va_list ap)
{
	const char *trailer = "";
	const bool source_empty = (__source == NULL) || (*__source == '\0');
	char message[NDM_LOG_MESSAGE_SIZE_];
	int size = 0;
	va_list aq;

	va_copy(aq, ap);
	size = vsnprintf(message, sizeof(message), format, aq);
	va_end(aq);

	if (size > 0 && size < sizeof(message) &&
		message[size - 1] != '.' &&
		(level == LINFO ||
		 level == LWARNING ||
		 level == LERROR ||
		 level == LCRITICAL))
	{
		trailer = ".";
	}

	if (!__console_mode) {
		syslog(__facility |
			((level == LINFO)  ?	LOG_INFO :
			 (level == LWARNING) ?	LOG_WARNING :
			 (level == LERROR) ?	LOG_ERR :
			 (level == LCRITICAL) ?	LOG_CRIT : LOG_DEBUG), "%s%s%s%s",
			source_empty ? "" : __source,
			source_empty ? "" : ": ",
			message, trailer);
	} else {
		static const char *MONTHS[] = {
			"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
		};
		struct timespec s;
		time_t now;
		struct tm t;

		ndm_time_get(&s);
		now = (time_t) ndm_time_to_sec(&s);
		localtime_r(&now, &t);

		fprintf(
			(level == LERROR || level == LCRITICAL) ? stderr : stdout,
			"[%c] %s %02i %02i:%02i:%02i %s: %s%s%s%s%s\n",
			((level == LINFO)  ?	'I' :
			 (level == LWARNING) ?	'W' :
			 (level == LERROR) ?	'E' :
			 (level == LCRITICAL) ?	'C' : '-'),
			MONTHS[((unsigned long) t.tm_mon) % NDM_ARRAY_SIZE(MONTHS)],
			t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,
			__ident,
			source_empty ? "" : "\"",
			source_empty ? "" : __source,
			source_empty ? "" : "\": ",
			message, trailer);
	}
}

