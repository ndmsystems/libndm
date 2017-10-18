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
#include <ndm/net.h>
#include <ndm/ip_sockaddr.h>
#include <ndm/poll.h>

#define NDM_NET_DOMAIN_MIN_LEN_			1
#define NDM_NET_DOMAIN_MAX_LEN_			253

#define NDM_NET_SUBDOMAIN_MAX_LEN_		63

#define NDN_RPC_PORT_					54321
#define NDN_RPC_TIMEOUT_				5000 // ms

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

int ndm_getaddrinfo(const char *node, const char *service,
		const struct addrinfo *hints, struct addrinfo **res)
{
	if (res == NULL) {
		return EAI_SYSTEM;
	}

	*res = NULL;

	struct ndm_ip_sockaddr_t sa = NDM_IP_SOCKADDR_ANY;

	if (node != NULL &&
		ndm_ip_sockaddr_pton(node, &sa) &&
		!ndm_ip_sockaddr_is_equal(&sa, &NDM_IP_SOCKADDR_ANY)) {

		struct addrinfo *r = (struct addrinfo *)malloc(sizeof(struct addrinfo));

		if (r == NULL) {
			return EAI_MEMORY;
		}

		memset(r, 0, sizeof(*r));

		r->ai_family = ndm_ip_sockaddr_family(&sa);
		r->ai_addrlen =
			r->ai_family == PF_INET ?
				sizeof(struct sockaddr_in) :
				sizeof(struct sockaddr_in6);

		struct sockaddr *s = (struct sockaddr *)malloc(sizeof(struct sockaddr));

		if (s == NULL) {
			free(r);
			return EAI_MEMORY;
		}

		memset(s, 0, sizeof(*s));
		memcpy(s, &sa, r->ai_addrlen);

		r->ai_addr = s;
		*res = r;

		return 0;
	}

	if (node == NULL ||
		!ndm_net_is_domain_name(node) ||
		res == NULL) {
		return EAI_SYSTEM;
	}

	if (hints != NULL &&
		!(hints->ai_family == PF_UNSPEC || hints->ai_family == PF_INET)) {
		return EAI_FAMILY;
	}

	char buffer[16 * 1024];

	memset(buffer, 0, sizeof(buffer));

	/* request is "resolv-conf a <fqdn>" */

	const int req_size = snprintf(buffer, sizeof(buffer) - 1,
			"resolv-conf a %s", node);

	if (req_size < 0) {
		return EAI_SYSTEM;
	}

	const int sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (sockfd < 0) {
		return EAI_SYSTEM;
	}

	int flags = 0;

	if ((flags = fcntl(sockfd, F_GETFL, 0)) == -1) {
		close(sockfd);
		return EAI_SYSTEM;
	}

	if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
		close(sockfd);
		return EAI_SYSTEM;
	}

	struct sockaddr_in ndnp_addr;

	memset(&ndnp_addr, 0, sizeof(ndnp_addr));

	ndnp_addr.sin_family = AF_INET;
	ndnp_addr.sin_addr.s_addr = htonl(0x7F000001); // loopback
	ndnp_addr.sin_port = htons(NDN_RPC_PORT_);

	if (sendto(sockfd, &buffer, (size_t)req_size, 0,
			(const struct sockaddr *)&ndnp_addr, sizeof(ndnp_addr)) < 0) {
		close(sockfd);
		return EAI_SYSTEM;
	}

	struct pollfd pfd =
	{
		.fd = sockfd,
		.events = POLLIN | POLLERR | POLLHUP | POLLNVAL,
		.revents = 0
	};

	const ssize_t n = ndm_poll(&pfd, 1, NDN_RPC_TIMEOUT_);

	if (n <= 0 || !(pfd.events & POLLIN)) {
		close(sockfd);
		return EAI_SYSTEM;
	}

	memset(buffer, 0, sizeof(buffer));

	socklen_t ndnp_addr_len = 0;

	const ssize_t answ = recvfrom(sockfd, &buffer, sizeof(buffer) - 1, 0,
			(struct sockaddr *)&ndnp_addr, &ndnp_addr_len);

	close(sockfd);

	if (ndnp_addr_len != sizeof(ndnp_addr)) {
		return EAI_SYSTEM;
	}

	if (answ < 1) {
		return EAI_SYSTEM;
	}

	/* One of three answers can be "not resolved" */

	const size_t not_res_len = strlen("not resolved");

	if (answ >= not_res_len && !memcmp(buffer, "not resolved", not_res_len)) {
		return EAI_NONAME;
	}

	/* Second answer can be "poll <hex-token>" */

	const size_t tok_len = strlen("poll ");

	if (answ >= tok_len && !memcmp(buffer, "poll ", tok_len)) {
		return EAI_AGAIN;
	}

	/* Third answer is a list of "<fqdn> <a> <address>" */

	char *sptr = NULL;
	char *token = strtok_r(buffer, " ", &sptr);
	unsigned int step = 0;

	while (token != NULL) {
		if (step % 3 == 2) {
			char ab[NDM_IP_SOCKADDR_LEN];
			char *pab = ab;
			char *pt = token;

			memset(ab, 0, sizeof(ab));

			/* strtok_r() often leaves delimiters, so strip it */

			while (*pt != '\0') {
				if (!isspace(*pt) && (pab < ab + NDM_IP_SOCKADDR_LEN)) {
					*pab = *pt;
					++pab;
				}
				++pt;
			}

			sa = NDM_IP_SOCKADDR_ANY;

			if (ndm_ip_sockaddr_pton(ab, &sa) &&
				!ndm_ip_sockaddr_is_equal(&sa, &NDM_IP_SOCKADDR_ANY)) {

				struct addrinfo *r = (struct addrinfo *)malloc(
					sizeof(struct addrinfo));

				if (r == NULL) {
					goto next_step;
				}

				memset(r, 0, sizeof(*r));

				r->ai_family = ndm_ip_sockaddr_family(&sa);
				r->ai_addrlen =
					r->ai_family == PF_INET ?
						sizeof(struct sockaddr_in) :
						sizeof(struct sockaddr_in6);

				struct sockaddr *s = (struct sockaddr *)malloc(
						sizeof(struct sockaddr));

				if (s == NULL) {
					free(r);
					goto next_step;
				}

				memset(s, 0, sizeof(*s));
				memcpy(s, &sa, r->ai_addrlen);

				r->ai_addr = s;
				r->ai_next = *res; 
				*res = r;
			}
		}

next_step:
		++step;
		token = strtok_r(NULL, " ", &sptr);
	}

	return 0;
}

void ndm_freeaddrinfo(struct addrinfo *res)
{
	while (res != NULL) {
		struct addrinfo *curr = res;

		res = res->ai_next;

		free(curr->ai_addr);
		free(curr);
	}
}
