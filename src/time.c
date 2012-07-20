#include <assert.h>
#include <sys/time.h>
#include <ndm/time.h>

#if 0
#if !defined (_POSIX_TIMERS) || (_POSIX_TIMERS <= 0) ||	\
	!defined (_POSIX_MONOTONIC_CLOCK)
#error POSIX timers support required.
#endif
#endif

const struct timespec NDM_TIME_ZERO = {
	.tv_sec = 0,
	.tv_nsec = 0
};

bool ndm_time_init()
{
	struct timespec ts;

	return
		clock_gettime(CLOCK_MONOTONIC, &ts) == 0 &&
		clock_gettime(CLOCK_REALTIME, &ts) == 0;
}

void ndm_time_get(
		struct timespec *t)
{
	clock_gettime(CLOCK_REALTIME, t);
}

void ndm_time_get_monotonic(
		struct timespec *t)
{
	clock_gettime(CLOCK_MONOTONIC, t);
}

bool ndm_time_is_zero(
		const struct timespec *t)
{
	return ndm_time_equal(t, &NDM_TIME_ZERO);
}

bool ndm_time_equal(
		const struct timespec *t,
		const struct timespec *u)
{
	return t->tv_sec == u->tv_sec && t->tv_nsec == u->tv_nsec;
}

bool ndm_time_less(
		const struct timespec *t,
		const struct timespec *u)
{
	return
		(t->tv_sec == u->tv_sec) ?
		 t->tv_nsec < u->tv_nsec : t->tv_sec < u->tv_sec;
}

bool ndm_time_less_or_equal(
		const struct timespec *t,
		const struct timespec *u)
{
	return ndm_time_equal(t, u) || ndm_time_less(t, u);
}

bool ndm_time_greater(
		const struct timespec *t,
		const struct timespec *u)
{
	return !ndm_time_less_or_equal(t, u);
}

bool ndm_time_greater_or_equal(
		const struct timespec *t,
		const struct timespec *u)
{
	return !ndm_time_less(t, u);
}

void ndm_time_add(
		struct timespec *t,
		const struct timespec *u)
{
	const int tpos = (t->tv_sec >= 0) && (t->tv_nsec >= 0);
	const int upos = (u->tv_sec >= 0) && (u->tv_nsec >= 0);

	if ((tpos && upos) || (!tpos && !upos)) {
		suseconds_t carry;
		suseconds_t correction;

		t->tv_nsec += u->tv_nsec;

		carry = t->tv_nsec/NDM_TIME_NSEC;
		correction = carry*NDM_TIME_NSEC;

		t->tv_sec += u->tv_sec + carry;
		t->tv_nsec -= correction;

		if ((tpos && t->tv_sec < 0) || (!tpos && t->tv_sec > 0)) {
			t->tv_nsec += carry - correction;
		}
	} else {
		t->tv_sec += u->tv_sec;
		t->tv_nsec += u->tv_nsec;

		if ((t->tv_sec >= 0 && t->tv_nsec >= 0) ||
			(t->tv_sec <= 0 && t->tv_nsec <= 0))
		{
			/* no correction need */
		} else
		if (t->tv_sec > 0) {
			t->tv_sec--;
			t->tv_nsec += NDM_TIME_NSEC;
		} else {
			t->tv_sec++;
			t->tv_nsec -= NDM_TIME_NSEC;
		}
	}

	assert(
		(t->tv_sec >= 0 && t->tv_nsec >= 0) ||
		(t->tv_sec <= 0 && t->tv_nsec <= 0));
}

void ndm_time_sub(
		struct timespec *t,
		const struct timespec *u)
{
	struct timespec v = {
		.tv_sec = -u->tv_sec,
		.tv_nsec = -u->tv_nsec
	};

	ndm_time_add(t, &v);
}

void ndm_time_to_timeval(
		struct timeval *t,
		const struct timespec *u)
{
	t->tv_sec = u->tv_sec;
	t->tv_usec = u->tv_nsec/(NDM_TIME_NSEC/NDM_TIME_USEC);
}

void ndm_time_from_timeval(
		struct timespec *t,
		const struct timeval *u)
{
	t->tv_sec = u->tv_sec;
	t->tv_nsec = u->tv_usec*(NDM_TIME_NSEC/NDM_TIME_USEC);
}

#define NDM_TIME_FUNCS(p, d)		 			\
												\
void ndm_time_add_##p(							\
		struct timespec *t,						\
		const int64_t p)						\
{												\
	const struct timespec u = {					\
		.tv_sec = p/d,							\
		.tv_nsec = (p % d)*(NDM_TIME_NSEC/d)	\
	};											\
												\
	ndm_time_add(t, &u);						\
}												\
												\
void ndm_time_sub_##p(							\
		struct timespec *t,						\
		const int64_t p)						\
{												\
	const struct timespec u = {					\
		.tv_sec = -p/d,							\
		.tv_nsec = ((-p) % d)*(NDM_TIME_NSEC/d)	\
	};											\
												\
	ndm_time_add(t, &u);						\
}												\
												\
int64_t ndm_time_to_##p(						\
		const struct timespec *t)				\
{												\
	return										\
		t->tv_sec*d +							\
		t->tv_nsec/(NDM_TIME_NSEC/d);			\
}												\
												\
void ndm_time_from_##p(							\
		struct timespec *t,						\
		const int64_t p)						\
{												\
	t->tv_sec = p/d;							\
	t->tv_nsec = (p % d)*(NDM_TIME_NSEC/d);		\
}

NDM_TIME_FUNCS(sec,  NDM_TIME_SEC)
NDM_TIME_FUNCS(msec, NDM_TIME_MSEC)
NDM_TIME_FUNCS(usec, NDM_TIME_USEC)
NDM_TIME_FUNCS(nsec, NDM_TIME_NSEC)

