#include <string.h>
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
				.s_addr = INADDR_ANY
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
			.sin6_family = AF_INET6,
			.sin6_port = 0,
			.sin6_addr =
			{
				.s6_addr = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
			}
		}
	},
	.size = sizeof(struct sockaddr_in6)
};

void ndm_ip_sockaddr_assign(
		struct ndm_ip_sockaddr_t *sa,
		const struct sockaddr_in *in)
{
	sa->un.in = *in;
	sa->size = sizeof(*in);
}

void ndm_ip_sockaddr_assign6(
		struct ndm_ip_sockaddr_t *sa,
		const struct sockaddr_in6 *in6)
{
	sa->un.in6 = *in6;
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
	return family == AF_INET ?
		&NDM_IP4_SOCKADDR_ZERO :
		&NDM_IP6_SOCKADDR_ZERO;
}

