#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ndm/time.h>

#define SYS_DOTS_					"[...]"
#define SYS_ERROR_MSG_MAX_LENGTH_	512			/* > sizeof(SYS_DOTS_)	*/

static volatile int __interrupted = 0;
static char __error_message[SYS_ERROR_MSG_MAX_LENGTH_ + 1];
static const struct timespec NDM_SLEEP_GRANULARITY_ = {
	.tv_sec = 0,
	.tv_nsec = 200000000
};

const struct timespec *ndm_sys_sleep_granularity()
{
	return &NDM_SLEEP_GRANULARITY_;
}

bool ndm_sys_is_interrupted()
{
	return (__interrupted == 0) ? false : true;
}

void ndm_sys_set_interrupted()
{
	__interrupted = -1;
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

		if (!isupper(__error_message[1])) {
			/* ignore abbreviations */
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

