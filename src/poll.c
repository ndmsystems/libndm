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
	const int SLEEP_GRANULARITY = (int)
		ndm_time_to_msec(ndm_sys_sleep_granularity());
	struct timespec start;
	int elapsed = 0;
	int ret = 0;

	ndm_time_get_monotonic(&start);

	do {
		struct timespec now;
		int period = 1;

		ndm_time_get_monotonic(&now);
		ndm_time_sub(&now, &start);
		elapsed = (int) ndm_time_to_msec(&now);

		if (elapsed < interval) {
			const int left = interval - elapsed;

			period =
				(left < SLEEP_GRANULARITY) ?
				 left : SLEEP_GRANULARITY;
		}

		ret = poll(fds, nfds, period);

		if (ret < 0 && (errno == EINTR || errno == EAGAIN)) {
			ret = 0;
		}
	} while (ret == 0 && !ndm_sys_is_interrupted() && elapsed < interval);

	if (ndm_sys_is_interrupted()) {
		ret = -1;
		errno = EINTR;
	}

	return ret;
}

