#include "ndm_common.h"
#define LOG_MESSAGE_SIZE        256
extern char *ident;

void ndm_log(const char *__ident, const MSG_TYPE type, char const *fmt, ...)
{
        va_list vargs;
        char message[LOG_MESSAGE_SIZE];

        va_start(vargs, fmt);
        vsnprintf(message, sizeof(message), fmt, vargs);
        va_end(vargs);

#ifndef CONSOLE_LOG
        syslog(LOG_DAEMON |
                ((type == LERROR) ? LOG_ERR :
                 (type == LINFO)  ? LOG_INFO : LOG_WARNING), "%s", message);
#else
        fprintf(
                (type == LERROR) ? stderr : stdout,
                "%s: %s> %s\n", __ident,
                (type == LERROR) ? "E" :
                (type == LINFO)  ? "I" : "W", message);
#endif  // CONSOLE_LOG
}


 void ndmLog_setup(char *argv[])
{
    const char *const name = strrchr(argv[0], '/');
    const char *const program = (name == NULL) ? argv[0] : name + 1;

#ifndef CONSOLE_LOG
    openlog(program, LOG_NDELAY, LOG_USER);
#else
   ident = program;
#endif  // CONSOLE_LOG
}


