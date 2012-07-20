#ifndef __NDM_TIME_H__
#define __NDM_TIME_H__

#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>

extern const struct timespec NDM_TIME_ZERO;

#define NDM_TIME_SEC												1
#define NDM_TIME_MSEC												1000
#define NDM_TIME_USEC												1000000
#define NDM_TIME_NSEC												1000000000

#define NDM_TIME_SEC_INIT(s)										\
{																	\
	.tv_sec = s/NDM_TIME_SEC,										\
	.tv_nsec = (s % NDM_TIME_SEC)*(NDM_TIME_NSEC/NDM_TIME_SEC)		\
}

#define NDM_TIME_MSEC_INIT(s)										\
{																	\
	.tv_sec = s/NDM_TIME_MSEC,										\
	.tv_nsec = (s % NDM_TIME_MSEC)*(NDM_TIME_NSEC/NDM_TIME_MSEC)	\
}

#define NDM_TIME_USEC_INIT(s)										\
{																	\
	.tv_sec = s/NDM_TIME_USEC,										\
	.tv_nsec = (s % NDM_TIME_USEC)*(NDM_TIME_NSEC/NDM_TIME_USEC)	\
}

#define NDM_TIME_NSEC_INIT(s)										\
{																	\
	.tv_sec = s/NDM_TIME_NSEC,										\
	.tv_nsec = (s % NDM_TIME_NSEC)*(NDM_TIME_NSEC/NDM_TIME_NSEC)	\
}

bool ndm_time_init();

void ndm_time_get(
		struct timespec *t);

void ndm_time_get_monotonic(
		struct timespec *t);

bool ndm_time_is_zero(
		const struct timespec *t);

bool ndm_time_equal(
		const struct timespec *t,
		const struct timespec *u);

bool ndm_time_less(
		const struct timespec *t,
		const struct timespec *u);

bool ndm_time_less_or_equal(
		const struct timespec *t,
		const struct timespec *u);

bool ndm_time_greater(
		const struct timespec *t,
		const struct timespec *u);

bool ndm_time_greater_or_equal(
		const struct timespec *t,
		const struct timespec *u);

void ndm_time_add(
		struct timespec *t,
		const struct timespec *u);

void ndm_time_sub(
		struct timespec *t,
		const struct timespec *u);

void ndm_time_add_sec(
		struct timespec *t,
		const int64_t sec);

void ndm_time_sub_sec(
		struct timespec *t,
		const int64_t sec);

int64_t ndm_time_to_sec(
		const struct timespec *t);

void ndm_time_from_sec(
		struct timespec *t,
		const int64_t sec);

void ndm_time_add_msec(
		struct timespec *t,
		const int64_t msec);

void ndm_time_sub_msec(
		struct timespec *t,
		const int64_t msec);

int64_t ndm_time_to_msec(
		const struct timespec *t);

void ndm_time_from_msec(
		struct timespec *t,
		const int64_t msec);

void ndm_time_add_usec(
		struct timespec *t,
		const int64_t usec);

void ndm_time_sub_usec(
		struct timespec *t,
		const int64_t usec);

int64_t ndm_time_to_usec(
		const struct timespec *t);

void ndm_time_from_usec(
		struct timespec *t,
		const int64_t usec);

void ndm_time_add_nsec(
		struct timespec *t,
		const int64_t nsec);

void ndm_time_sub_nsec(
		struct timespec *t,
		const int64_t nsec);

int64_t ndm_time_to_nsec(
		const struct timespec *t);

void ndm_time_from_nsec(
		struct timespec *t,
		const int64_t nsec);

void ndm_time_add_timeval(
		struct timespec *t,
		const struct timeval *u);

void ndm_time_sub_timeval(
		struct timespec *t,
		const struct timeval *u);

void ndm_time_to_timeval(
		struct timeval *t,
		const struct timespec *u);

void ndm_time_from_timeval(
		struct timespec *t,
		const struct timeval *u);

#endif	/* __NDM_TIME_H__ */

