#include "ndm_common.h"

long long __get_monotonic_us()
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	
	return ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000;
}


