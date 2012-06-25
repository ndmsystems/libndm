#include "ndm_common.h"

long long ndmTime_get_monotonic_us()
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	
	return ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000;
}

unsigned ndmTime_get_monotonic_s()
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec;
}

void ndmTime_get_time(struct timeval *t)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	t->tv_sec = ts.tv_sec;
	t->tv_usec = ts.tv_nsec / 1000;
}

/* 10 us precision, the maximum is < 12 hours */

unsigned long ndmTime_elapsed_from_in10us(
		const struct timeval *start)
{
	struct timeval t;

	ndmTime_get_time(&t);

	if (t.tv_usec < start->tv_usec) {
		t.tv_sec -= start->tv_sec + 1;
		t.tv_usec += 1000000 - start->tv_usec;
	} else {
		t.tv_sec -= start->tv_sec;
		t.tv_usec -= start->tv_usec;
	}

	return t.tv_sec * 100000 + t.tv_usec / 10;
}
