#include <poll.h>
#include <errno.h>
#include <ndm/sys.h>
#include <ndm/time.h>

static void __ndm_poll_calculate_msec_deadline(
		struct timespec *deadline,
		const int timeout)
{
	ndm_time_get_monotonic(deadline);
	ndm_time_add_msec(deadline, timeout);
}

static int __ndm_poll_msec_to_deadline(
		const struct timespec *deadline)
{
	struct timespec now;
	struct timespec left = *deadline;

	ndm_time_get_monotonic(&now);
	ndm_time_sub(&left, &now);

	return (int) ndm_time_to_msec(&left);
}

/* interruptible poll */
int ndm_poll(
		struct pollfd *fds,
		const nfds_t nfds,
		const int interval)
{
	struct timespec deadline;
	int left = 0;
	int n = 0;

	__ndm_poll_calculate_msec_deadline(&deadline, interval);

	do {
		left = (interval < 0) ?
			NDM_SYS_SLEEP_GRANULARITY_MSEC :
			__ndm_poll_msec_to_deadline(&deadline);

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

