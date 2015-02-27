#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ndm/sys.h>
#include <ndm/time.h>

#define SYS_DOTS_					"[...]"
#define SYS_ERROR_MSG_MAX_LENGTH_	512			/* > sizeof(SYS_DOTS_)	*/

static volatile int __interrupted = 0;
static char __error_message[SYS_ERROR_MSG_MAX_LENGTH_ + 1];
static const struct timespec NDM_SLEEP_GRANULARITY_ =
	NDM_TIME_MSEC_INITIALIZER(NDM_SYS_SLEEP_GRANULARITY_MSEC);

bool ndm_sys_init()
{
	const bool done = ndm_time_init();

	if (done) {
		struct timespec now;

		ndm_time_get_monotonic(&now);
		srand((unsigned int)
			((((unsigned int) getpid())*36969 +
			  ((unsigned int) (now.tv_nsec % 1000000000)))*33317));
	}

	return done;
}

bool ndm_sys_is_interrupted()
{
	return (__interrupted == 0) ? false : true;
}

void ndm_sys_set_interrupted()
{
	__interrupted = -1;
}

static void __ndm_sys_signal_handler(int sig)
{
	ndm_sys_set_interrupted();
}

bool ndm_sys_set_default_signals()
{
	struct sigaction ignore_action;
	struct sigaction terminate_action;
	sigset_t signals;

	memset(&ignore_action, 0, sizeof(ignore_action));

	ignore_action.sa_handler = SIG_IGN;
	ignore_action.sa_flags = 0;

	memset(&terminate_action, 0, sizeof(terminate_action));

	terminate_action.sa_handler = __ndm_sys_signal_handler;
	terminate_action.sa_flags = 0;

	return
		(sigfillset(&signals) == 0 &&
		 sigdelset(&signals, SIGINT) == 0 &&
		 sigdelset(&signals, SIGHUP) == 0 &&
		 sigdelset(&signals, SIGFPE) == 0 &&
		 sigdelset(&signals, SIGILL) == 0 &&
		 sigdelset(&signals, SIGSEGV) == 0 &&
		 sigdelset(&signals, SIGABRT) == 0 &&
		 sigdelset(&signals, SIGTERM) == 0 &&
		 sigdelset(&signals, SIGKILL) == 0 &&
		 sigdelset(&signals, SIGSTOP) == 0 &&
		 sigprocmask(SIG_BLOCK, &signals, NULL) == 0 &&
		 sigemptyset(&terminate_action.sa_mask) == 0 &&
		 sigaction(SIGPIPE, &ignore_action, NULL) == 0 &&
		 sigaction(SIGINT, &terminate_action, NULL) == 0 &&
		 sigaction(SIGTERM, &terminate_action, NULL) == 0) ? true : false;
}

int ndm_sys_rand()
{
	return rand();
}

const struct timespec *ndm_sys_sleep_granularity()
{
	return &NDM_SLEEP_GRANULARITY_;
}

const char *ndm_sys_strerror(const int error)
{
	const char *message = strerror(error);

	*__error_message = '\0';

	if (message == NULL) {
		snprintf(
			__error_message, sizeof(__error_message),
			"error %i occured", error);
	} else {
		snprintf(__error_message, sizeof(__error_message), "%s", message);

		if (strlen(message) > sizeof(__error_message) - 1) {
			snprintf(
				__error_message + sizeof(__error_message) -
				sizeof(SYS_DOTS_) - 1,
				sizeof(SYS_DOTS_), "%s", SYS_DOTS_);
		}

		if (islower(__error_message[1]) ||
			isspace(__error_message[1]))
		{
			*__error_message = (char) tolower(*__error_message);
		}
	}

	return __error_message;
}

bool ndm_sys_sleep(
		const struct timespec *interval)
{
	if (ndm_time_greater(interval, &NDM_TIME_ZERO)) {
		struct timespec end;
		struct timespec now;

		ndm_time_get_monotonic(&now);

		end = now;
		ndm_time_add(&end, interval);

		do {
			struct timespec period = end;

			ndm_time_sub(&period, &now);

			if (ndm_time_greater(&period, &NDM_SLEEP_GRANULARITY_)) {
				period = NDM_SLEEP_GRANULARITY_;
			}

			nanosleep(&period, NULL);
			ndm_time_get_monotonic(&now);
		} while (ndm_time_less(&now, &end) && !ndm_sys_is_interrupted());
	}

	return !ndm_sys_is_interrupted();
}

bool ndm_sys_sleep_msec(const int64_t msec)
{
	struct timespec interval;

	ndm_time_from_msec(&interval, msec);

	return ndm_sys_sleep(&interval);
}

