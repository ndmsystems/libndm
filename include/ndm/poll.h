#ifndef __NDM_POLL_H__
#define __NDM_POLL_H__

#include <poll.h>

/* interruptible poll */
int ndm_poll(
		struct pollfd *fds,
		const nfds_t nfds,
		const int interval);

#endif	/* __NDM_POLL_H__ */

