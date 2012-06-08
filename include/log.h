#ifndef NDM_LOG_H
#define NDM_LOG_H 1

#include <syslog.h>
#include <stdarg.h>

typedef enum
{
	LINFO,
	LERROR,
	LWARNING
} MSG_TYPE;

#define LOG_MESSAGE_SIZE			512

#define ndmLog_info(...) ndm_log(__FUNCTION__,LINFO,__VA_ARGS__)
#define ndmLog_warn(...) ndm_log(__FUNCTION__,LWARNING,__VA_ARGS__)
#define ndmLog_error(...) ndm_log(__FUNCTION__,LERROR,__VA_ARGS__)

#endif /* NDM_LOG_H */
