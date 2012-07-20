#include "ndm_common.h"
#include <poll.h>
#include <sys/socket.h>

#define POLL_TIMEOUT_10US			10000		/* 0.1 sec.			*/

int ndmSocket_wait_for(
		const int s,
		const short mode,
		const unsigned long ms)
{
	struct pollfd fds = {
		.fd = s,
		.events = mode
	};
	int ret = 0;
	struct timeval start;
	unsigned long elapsed = 0;
	unsigned long left = 0;

	ndmTime_get_time(&start);

	do {
		elapsed = ndmTime_elapsed_from_in10us(&start);

		if (elapsed < ms) {
			left = ms - elapsed;

			if (left > POLL_TIMEOUT_10US) {
				left = POLL_TIMEOUT_10US;
			}

			/* in ms. */
			left /= 100;

			if (left == 0) {
				++left;
			}

			ret = poll(&fds, 1, left);

			if (ret < 0) {
				if (errno == EINTR || errno == EAGAIN) {
					/* interrupted system call, check
					 * the __interrupted flag */
				} else {
					fprintf(stderr, "I/O error occured: %s.\n",
						strerror(errno));
				}
			}
		}
	} while (ret <= 0 && elapsed < ms);

	return ret > 0;
}



