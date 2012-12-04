#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <ndm/ip_sockaddr.h>

const struct ndm_ip_sockaddr_t NDM_IP4_SOCKADDR_ZERO =
{
	.un =
	{
		.in =
		{
			.sin_family = AF_INET,
			.sin_port = 0,
			.sin_addr =
			{
				.s_addr = INADDR_ANY	/* it is in a host order really */
			}
		}
	},
	.size = sizeof(struct sockaddr_in)
};

const struct ndm_ip_sockaddr_t NDM_IP6_SOCKADDR_ZERO =
{
	.un =
	{
		.in6 =
		{
#ifdef SIN6_LEN
			.sin6_len = SIN6_LEN,
#endif	/* SIN6_LEN */
			.sin6_family = AF_INET6,
			.sin6_flowinfo = 0,
			.sin6_port = 0,
			.sin6_addr = IN6ADDR_ANY_INIT
		}
	},
	.size = sizeof(struct sockaddr_in6)
};

void ndm_ip_sockaddr_assign(
		struct ndm_ip_sockaddr_t *sa,
		const struct sockaddr_in *in)
{
	assert(in->sin_family == AF_INET);

	sa->un.in = *in;
	sa->size = sizeof(*in);
}

void ndm_ip_sockaddr_assign6(
		struct ndm_ip_sockaddr_t *sa,
		const struct sockaddr_in6 *in6)
{
	assert(in6->sin6_family == AF_INET6);

	sa->un.in6 = *in6;
#ifdef SIN6_LEN
	sa->un.in6.sin6_len = sizeof(*in6);
#endif	/* SIN6_LEN */
	sa->un.in6.sin6_family = AF_INET6;
	sa->size = sizeof(*in6);
}

const char *ndm_ip_sockaddr_ntop(
		const struct ndm_ip_sockaddr_t *const sa,
		char *const dst,
		const socklen_t dst_size)
{
	return inet_ntop(
		sa->un.family, (sa->un.family == AF_INET) ?
		(void *) &sa->un.in.sin_addr :
		(void *) &sa->un.in6.sin6_addr,
		dst, dst_size);
}

bool ndm_ip_sockaddr_pton(
		const char *const src,
		struct ndm_ip_sockaddr_t *sa)
{
	const int ret = inet_pton(sa->un.family, src,
		(sa->un.family == AF_INET) ?
		(void *) &sa->un.in.sin_addr :
		(void *) &sa->un.in6.sin6_addr);

	if (ret == 1) {
		sa->size = (sa->un.family == AF_INET) ?
			sizeof(sa->un.in) : sizeof(sa->un.in6);
	}

	return (ret == 0) ? false : true;
}

bool ndm_ip_sockaddr_is_equal(
		const struct ndm_ip_sockaddr_t *const sa1,
		const struct ndm_ip_sockaddr_t *const sa2)
{
	return sa1->un.family == sa2->un.family &&
		((sa1->un.family == AF_INET &&
		  memcmp(&sa1->un.in, &sa2->un.in, sizeof(sa1->un.in)) == 0) ||
		 (sa1->un.family == AF_INET6 &&
		  memcmp(&sa1->un.in6, &sa2->un.in6, sizeof(sa1->un.in6)) == 0));
}

bool ndm_ip_sockaddr_address_is_equal(
		const struct ndm_ip_sockaddr_t *const sa1,
		const struct ndm_ip_sockaddr_t *const sa2)
{
	return sa1->un.family == sa2->un.family &&
		((sa1->un.family == AF_INET &&
		  memcmp(&sa1->un.in.sin_addr, &sa2->un.in.sin_addr,
		  	sizeof(sa1->un.in.sin_addr)) == 0) ||
		 (sa1->un.family == AF_INET6 &&
		  memcmp(&sa1->un.in6.sin6_addr, &sa2->un.in6.sin6_addr,
		  	sizeof(sa1->un.in6.sin6_addr)) == 0));
}

bool ndm_ip_sockaddr_is_zero(
		const struct ndm_ip_sockaddr_t *const sa)
{
	bool is_zero = false;

	assert(sa->un.family == AF_INET || sa->un.family == AF_INET6);

	if (sa->un.family == AF_INET) {
		is_zero = ndm_ip_sockaddr_is_equal(sa, &NDM_IP4_SOCKADDR_ZERO);
	} else
	if (sa->un.family == AF_INET6) {
		is_zero = ndm_ip_sockaddr_is_equal(sa, &NDM_IP6_SOCKADDR_ZERO);
	}

	return is_zero;
}

const struct ndm_ip_sockaddr_t *ndm_ip_sockaddr_get_zero(
		const int family)
{
	assert(family == AF_INET || family == AF_INET6);

	return family == AF_INET ?
		&NDM_IP4_SOCKADDR_ZERO :
		&NDM_IP6_SOCKADDR_ZERO;
}

void ndm_ip_sockaddr_get_loopback(
		const int family,
		struct ndm_ip_sockaddr_t *sa)
{
	assert(family == AF_INET || family == AF_INET6);

	if (family == AF_INET) {
		sa->un.in.sin_family = AF_INET;
		sa->un.in.sin_port = 0;
		sa->un.in.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		sa->size = sizeof(struct sockaddr_in);
	} else {
#ifdef SIN6_LEN
		sa->un.in6.sin6_len = SIN6_LEN;
#endif	/* SIN6_LEN */
		sa->un.in6.sin6_family = AF_INET6;
		sa->un.in6.sin6_flowinfo = 0;
		sa->un.in6.sin6_port = 0;
		sa->un.in6.sin6_addr = in6addr_loopback;
		sa->size = sizeof(struct sockaddr_in6);
	};
}

void ndm_ip_sockaddr_set_port(
		struct ndm_ip_sockaddr_t *const sa,
		const uint16_t port)
{
	assert(sa->un.family == AF_INET || sa->un.family == AF_INET6);

	if (sa->un.family == AF_INET) {
		sa->un.in.sin_port = (uint16_t) htons(port);
	} else {
		sa->un.in6.sin6_port = (uint16_t) htons(port);
	}
}

uint16_t ndm_ip_sockaddr_port(
		const struct ndm_ip_sockaddr_t *const sa)
{
	assert(sa->un.family == AF_INET || sa->un.family == AF_INET6);

	if (sa->un.family == AF_INET) {
		return (uint16_t) ntohs(sa->un.in.sin_port);
	}

	return (uint16_t) ntohs(sa->un.in6.sin6_port);
}

int ndm_ip_sockaddr_domain(
		const struct ndm_ip_sockaddr_t *const sa)
{
	assert(sa->un.family == AF_INET || sa->un.family == AF_INET6);

	return sa->un.family == AF_INET ? PF_INET : PF_INET6;
}

sa_family_t ndm_ip_sockaddr_family(
		const struct ndm_ip_sockaddr_t *const sa)
{
	return sa->un.family;
}

