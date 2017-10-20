#include <errno.h>
#include <ctype.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ndm/net.h>
#include <ndm/int.h>
#include <ndm/poll.h>
#include <ndm/ip_sockaddr.h>

#define NDM_NET_DOMAIN_MIN_LEN_			1
#define NDM_NET_DOMAIN_MAX_LEN_			253

#define NDM_NET_SUBDOMAIN_MAX_LEN_		63

#define NDM_NET_NDN_RPC_PORT_			54321
#define NDM_NET_NDN_RPC_TIMEOUT_		5000 // ms

#define NDM_NET_ANSWER_NOT_RES_			"not resolved"
#define NDM_NET_ANSWER_POLL_			"poll "

#define NDM_NET_ANSWER_MAX_SIZE_		(16 * 1024)

#define NDM_NET_AI_FLAGS_ANY_			\
	(AI_PASSIVE		| \
	 AI_CANONNAME	| \
	 AI_NUMERICHOST | \
	 AI_ADDRCONFIG	| \
	 AI_V4MAPPED	| \
	 AI_NUMERICSERV	| \
	 AI_ALL)

bool ndm_net_is_domain_name(const char *const name)
{
	// ^([a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?\.) ->
	// -> ([a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9]))$

	size_t name_size = strlen(name);
	bool valid = false;

	if (name_size >= NDM_NET_DOMAIN_MIN_LEN_ &&
		name_size <= NDM_NET_DOMAIN_MAX_LEN_)
	{
		size_t i = (size_t) -1;

		do {
			/* A subdomain name should start with
			 * an alphanumeric character. */

			++i;

			if (!isalnum(name[i++])) {
				valid = false;
			} else {
				/* the subdomain name should contain
				 * only alphanumeric characters and '-' symbols.
				 * It should end with an alphanumeric character
				 * and to be shorter than @c SUBDOMAIN_MAX_LEN_. */

				const size_t s = i - 1;

				while (
					i < name_size &&
					(isalnum(name[i]) || name[i] == '-'))
				{
					++i;
				}

				valid =
					isalnum(name[i - 1]) &&
					(name[i] == '.' || i == name_size) &&
					i - s <= NDM_NET_SUBDOMAIN_MAX_LEN_;
			}
		} while (valid && i < name_size);
	}

	return valid;
}

static int ndm_net_fill_addrinfo(
		const char *const node,
		struct addrinfo *prev,
		struct addrinfo **res)
{
	struct ndm_ip_sockaddr_t sa = NDM_IP_SOCKADDR_ANY;

	if (!ndm_ip_sockaddr_pton(node, &sa)) {
		return EAI_NONAME;
	}

	struct addrinfo *r;
	const size_t info_size = NDM_INT_ALIGN(sizeof(*r), sizeof(void *));
	const socklen_t addrlen = ndm_ip_sockaddr_size(&sa);

	r = (struct addrinfo *) calloc(1, info_size + addrlen);

	if (r == NULL) {
		return EAI_MEMORY;
	}

	r->ai_family = ndm_ip_sockaddr_family(&sa);
	r->ai_addrlen = addrlen;
	r->ai_addr = (struct sockaddr *) (((uint8_t *) r) + info_size);
	r->ai_next = prev;

	memcpy(r->ai_addr, &sa, addrlen);

	*res = r;

	return 0;
}

static inline void ndm_net_skip_spaces(char **p)
{
	while (isspace(**p)) {
		(*p)++;
	}
}

static void ndm_net_skip_nonspaces(char **p)
{
	char *ptr = *p;

	while (!isspace(*ptr) && *ptr != '\0') {
		ptr++;
	}

	*p = ptr;
}

int ndm_net_getaddrinfo(
		const char *node,
		const char *service,
		const struct addrinfo *hints,
		struct addrinfo **res)
{
	if (service != NULL) {
		errno = EINVAL;

		return EAI_SYSTEM;
	}

	if (res == NULL) {
		errno = EFAULT;

		return EAI_SYSTEM;
	}

	*res = NULL;

	if (node == NULL) {
		return EAI_NONAME;
	}

	struct addrinfo dh;

	if (hints == NULL) {
		memset(&dh, 0, sizeof(dh));

		if (PF_UNSPEC != 0) {
			dh.ai_family = PF_UNSPEC;
		}

		hints = &dh;
	}

	if (hints->ai_flags & ~NDM_NET_AI_FLAGS_ANY_) {
		return EAI_BADFLAGS;
	}

	if (hints->ai_flags & AI_CANONNAME) {
		return EAI_BADFLAGS;
	}

	int exit_code = ndm_net_fill_addrinfo(node, NULL, res);

	if (exit_code == 0) {
		return 0;
	}

	if (exit_code != EAI_NONAME) {
		return exit_code;
	}

	if (hints->ai_flags & AI_NUMERICHOST) {
		return EAI_NONAME;
	}

	if (!ndm_net_is_domain_name(node)) {
		return EAI_NONAME;
	}

	if (!(hints->ai_family == PF_UNSPEC || hints->ai_family == PF_INET)) {
		return EAI_FAMILY;
	}

	char buf[NDM_NET_ANSWER_MAX_SIZE_];

	/* request is "resolv-conf a <fqdn>" */

	const int req_len = snprintf(buf, sizeof(buf), "resolv-conf a %s", node);

	if (req_len < 0) {
		return EAI_SYSTEM;
	}

	if (req_len >= sizeof(buf)) {
		errno = ENOMEM;

		return EAI_SYSTEM;
	}

	const int sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (sockfd < 0) {
		return EAI_SYSTEM;
	}

	const int flags = fcntl(sockfd, F_GETFL, 0);

	if (flags == -1) {
		const int err = errno;

		close(sockfd);
		errno = err;

		return EAI_SYSTEM;
	}

	if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
		const int err = errno;

		close(sockfd);
		errno = err;

		return EAI_SYSTEM;
	}

	struct sockaddr_in ndnp_addr =
	{
		.sin_family = AF_INET,
		.sin_port = htons(NDM_NET_NDN_RPC_PORT_),
		.sin_addr.s_addr = htonl(INADDR_LOOPBACK)
	};

	if (sendto(sockfd, buf, (size_t) req_len, 0,
			(const struct sockaddr *) &ndnp_addr, sizeof(ndnp_addr)) < 0)
	{
		const int err = errno;

		close(sockfd);
		errno = err;

		return EAI_SYSTEM;
	}

	struct pollfd pfd =
	{
		.fd = sockfd,
		.events = POLLIN,
		.revents = 0
	};
	const ssize_t n = ndm_poll(&pfd, 1, NDM_NET_NDN_RPC_TIMEOUT_);
	const int err_poll = errno;

	if (n < 0) {
		close(sockfd);
		errno = err_poll;

		return EAI_SYSTEM;
	}

	if (n == 0) {
		close(sockfd);
		errno = ETIMEDOUT;

		return EAI_SYSTEM;
	}

	if (pfd.revents & (POLLHUP | POLLERR)) {
		close(sockfd);
		errno = ECONNRESET;

		return EAI_SYSTEM;
	}

	if (pfd.revents & POLLNVAL) {
		close(sockfd);
		errno = EBADF;

		return EAI_SYSTEM;
	}

	if (!(pfd.revents & POLLIN)) {
		close(sockfd);
		errno = ETIMEDOUT;

		return EAI_SYSTEM;
	}

	socklen_t ndnp_addr_len = 0;
	const ssize_t answ = recvfrom(sockfd, buf, sizeof(buf) - 1, 0,
			(struct sockaddr *) &ndnp_addr, &ndnp_addr_len);
	const int err_answ = errno;

	close(sockfd);

	if (ndnp_addr_len != sizeof(ndnp_addr)) {
		errno = EIO;

		return EAI_SYSTEM;
	}

	if (answ < 0) {
		errno = err_answ;

		return EAI_SYSTEM;
	}

	if (answ == 0) {
		errno = EIO;

		return EAI_SYSTEM;
	}

	buf[answ] = '\0';

	/* One of three answers can be "not resolved" */

	if (strncmp(buf, NDM_NET_ANSWER_NOT_RES_, (size_t) answ) == 0) {
		return EAI_NONAME;
	}

	/* Second answer can be "poll <hex-token>" */

	if (strncmp(buf, NDM_NET_ANSWER_POLL_, (size_t) answ) == 0) {
		return EAI_AGAIN;
	}

	/* Third answer is a list of "<fqdn> <a> <address> " */

	char *p = buf;

	ndm_net_skip_spaces(&p);

	while (*p != '\0') {
		ndm_net_skip_nonspaces(&p); /* skip FQDN */
		ndm_net_skip_spaces(&p);
		ndm_net_skip_nonspaces(&p); /* skip address type */
		ndm_net_skip_spaces(&p);

		const char *const addr = p;

		ndm_net_skip_nonspaces(&p);

		const char ch = *p;

		*p = '\0';
		exit_code = ndm_net_fill_addrinfo(addr, *res, res);

		if (exit_code != 0) {
			ndm_net_freeaddrinfo(*res);
			*res = NULL;

			return exit_code;
		}

		*p = ch;
		ndm_net_skip_spaces(&p);
	}

	return *res == NULL ? EAI_NONAME : 0;
}

void ndm_net_freeaddrinfo(struct addrinfo *res)
{
	while (res != NULL) {
		struct addrinfo *ai = res;

		res = res->ai_next;
		free(ai);
	}
}

const char *ndm_net_gai_strerror(int errcode)
{
	return gai_strerror(errcode);
}
