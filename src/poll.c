#include <poll.h>
#include <errno.h>
#include <ndm/sys.h>
#include <ndm/time.h>

/* interruptible poll */
int ndm_poll(
		struct pollfd *fds,
		const nfds_t nfds,
		const int interval)
{
	struct timespec deadline;
	int left = 0;
	int n = 0;

	ndm_time_get_monotonic_plus_msec(&deadline, interval);

	do {
		left = (interval < 0) ?
			NDM_SYS_SLEEP_GRANULARITY_MSEC :
			(int) ndm_time_left_monotonic_msec(&deadline);

		n = poll(fds, nfds,
			(left < 0) ? 0 :
			(left < NDM_SYS_SLEEP_GRANULARITY_MSEC) ? left :
				NDM_SYS_SLEEP_GRANULARITY_MSEC);

		if (n < 0 && (errno == EINTR || errno == EAGAIN)) {
			n = 0;
		}

		if (ndm_sys_is_interrupted()) {
			n = -1;
			errno = EINTR;
		}
	} while (n == 0 && left > 0);

	return n;
}

