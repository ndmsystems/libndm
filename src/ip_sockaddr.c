#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ndm/ip_sockaddr.h>

const struct ndm_ip_sockaddr_t NDM_IP_SOCKADDR_ANY =
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
	}
};

const struct ndm_ip_sockaddr_t NDM_IP_SOCKADDR_ANY6 =
{
	.un =
	{
		.in6 =
		{
#ifdef SIN6_LEN
			.sin6_len = sizeof(struct sockaddr_in6),
#endif	/* SIN6_LEN */
			.sin6_family = AF_INET6,
			.sin6_flowinfo = 0,
			.sin6_port = 0,
			.sin6_addr =
			{
				.s6_addr = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
			}
		}
	}
};

const char *ndm_ip_sockaddr_ntop(
		const struct ndm_ip_sockaddr_t *const sa,
		char *const dst,
		const socklen_t dst_size)
{
	assert(ndm_ip_sockaddr_is_v4(sa) || ndm_ip_sockaddr_is_v6(sa));

	const sa_family_t family = ndm_ip_sockaddr_family(sa);

	return inet_ntop(
		family,
		(family == AF_INET) ?
			(void *) &sa->un.in.sin_addr.s_addr :
			(void *) &sa->un.in6.sin6_addr.s6_addr,
		dst, dst_size);
}

bool ndm_ip_sockaddr_pton(
		const char *const src,
		struct ndm_ip_sockaddr_t *const sa)
{
	const sa_family_t family = ndm_ip_sockaddr_family(sa);

	return inet_pton(
		family,
		src,
		(family == AF_INET) ?
			(void *) &sa->un.in.sin_addr.s_addr :
			(void *) &sa->un.in6.sin6_addr.s6_addr) == 0 ?
		false : true;
}

bool ndm_ip_sockaddr_is_equal(
		const struct ndm_ip_sockaddr_t *const sa1,
		const struct ndm_ip_sockaddr_t *const sa2)
{
	const sa_family_t family = ndm_ip_sockaddr_family(sa1);

	if (family != ndm_ip_sockaddr_family(sa2)) {
		return false;
	}

	if (family == AF_INET) {
		const struct sockaddr_in *const sin1 = &sa1->un.in;
		const struct sockaddr_in *const sin2 = &sa2->un.in;

		return
			(sin1->sin_port == sin2->sin_port &&
			 sin1->sin_addr.s_addr == sin2->sin_addr.s_addr) ?
				true : false;
	}

	if (family == AF_INET6) {
		const struct sockaddr_in6 *const sin1 = &sa1->un.in6;
		const struct sockaddr_in6 *const sin2 = &sa2->un.in6;

		return (
#ifdef SIN6_LEN
			sin1->sin6_len == sin2->sin6_len &&
#endif	/* SIN6_LEN */
			sin1->sin6_flowinfo == sin2->sin6_flowinfo &&
			sin1->sin6_port == sin2->sin6_port &&
			memcmp(
				sin1->sin6_addr.s6_addr,
				sin2->sin6_addr.s6_addr,
				sizeof(sin1->sin6_addr.s6_addr)) == 0) ?
			true : false;
	}

	return false;
}

bool ndm_ip_sockaddr_address_is_equal(
		const struct ndm_ip_sockaddr_t *const sa1,
		const struct ndm_ip_sockaddr_t *const sa2)
{
	const sa_family_t family = ndm_ip_sockaddr_family(sa1);

	if (family != ndm_ip_sockaddr_family(sa2)) {
		return false;
	}

	if (family == AF_INET) {
		return memcmp(
			&sa1->un.in.sin_addr.s_addr,
			&sa2->un.in.sin_addr.s_addr,
			sizeof(sa1->un.in.sin_addr.s_addr)) == 0 ?
				true : false;
	}

	if (family == AF_INET6) {
		return memcmp(
			&sa1->un.in6.sin6_addr.s6_addr,
			&sa2->un.in6.sin6_addr.s6_addr,
			sizeof(sa1->un.in6.sin6_addr.s6_addr)) == 0 ?
				true : false;
	}

	return false;
}

bool ndm_ip_sockaddr_is_any(
		const struct ndm_ip_sockaddr_t *const sa)
{
	assert(ndm_ip_sockaddr_is_v4(sa) || ndm_ip_sockaddr_is_v6(sa));

	return ndm_ip_sockaddr_is_equal(
		sa,
		ndm_ip_sockaddr_is_v4(sa) ?
			&NDM_IP_SOCKADDR_ANY :
			&NDM_IP_SOCKADDR_ANY6);
}

bool ndm_ip_sockaddr_address_is_any(
		const struct ndm_ip_sockaddr_t *const sa)
{
	assert(ndm_ip_sockaddr_is_v4(sa) || ndm_ip_sockaddr_is_v6(sa));

	return ndm_ip_sockaddr_address_is_equal(
		sa,
		ndm_ip_sockaddr_is_v4(sa) ?
			&NDM_IP_SOCKADDR_ANY :
			&NDM_IP_SOCKADDR_ANY6);
}

static inline bool ndm_ip_sockaddr_has_prefix_(
		const struct ndm_ip_sockaddr_t *const sa,
		const uint8_t *prefix,
		const size_t prefix_size)
{
	return
		(ndm_ip_sockaddr_is_v6(sa) &&
		 memcmp(
			&sa->un.in6.sin6_addr.s6_addr[0],
			prefix, prefix_size) == 0) ? true : false;
}

bool ndm_ip_sockaddr_is_v4_mapped(
		const struct ndm_ip_sockaddr_t *const sa)
{
	static const uint8_t V4MAPPED_PREFIX_[] =
	{
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0xff, 0xff
	};

	return ndm_ip_sockaddr_has_prefix_(
		sa, V4MAPPED_PREFIX_, sizeof(V4MAPPED_PREFIX_));
}

bool ndm_ip_sockaddr_is_v4_compat(
		const struct ndm_ip_sockaddr_t *const sa)
{
	static const uint8_t V4COMPAT_PREFIX_[] =
	{
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00
	};

	return ndm_ip_sockaddr_has_prefix_(
		sa, V4COMPAT_PREFIX_, sizeof(V4COMPAT_PREFIX_));
}

bool ndm_ip_sockaddr_get_v4(
		const struct ndm_ip_sockaddr_t *const sa,
		struct ndm_ip_sockaddr_t *sa4)
{
	bool done = true;

	if (ndm_ip_sockaddr_is_v4(sa)) {
		ndm_ip_sockaddr_assign(sa4, &sa->un.in);
	} else
	if (ndm_ip_sockaddr_is_v4_mapped(sa) ||
		ndm_ip_sockaddr_is_v4_compat(sa))
	{
		const struct in6_addr *in6 = &sa->un.in6.sin6_addr;
		struct sockaddr_in *sin = &sa4->un.in;

		sin->sin_family = AF_INET;
		sin->sin_port = sa->un.in6.sin6_port;
		sin->sin_addr.s_addr = htonl(
			(((uint32_t) in6->s6_addr[12]) << 24) |
			(((uint32_t) in6->s6_addr[13]) << 16) |
			(((uint32_t) in6->s6_addr[14]) <<  8) |
			(((uint32_t) in6->s6_addr[15]) <<  0));
	} else {
		done = false;
	}

	return done;
}

bool ndm_ip_sockaddr_get_v4_mapped(
		const struct ndm_ip_sockaddr_t *const sa,
		struct ndm_ip_sockaddr_t *sa6)
{
	if (ndm_ip_sockaddr_get_v4_compat(sa, sa6)) {
		struct in6_addr *in6 = &sa6->un.in6.sin6_addr;

		in6->s6_addr[10] = 0xff;
		in6->s6_addr[11] = 0xff;

		return true;
	}

	return false;
}

bool ndm_ip_sockaddr_get_v4_compat(
		const struct ndm_ip_sockaddr_t *const sa,
		struct ndm_ip_sockaddr_t *sa6)
{
	struct in6_addr *in6 = &sa6->un.in6.sin6_addr;

	if (ndm_ip_sockaddr_is_v4(sa)) {
		const uint32_t s4 = ntohl(sa->un.in.sin_addr.s_addr);

		ndm_ip_sockaddr_assign6(sa6, &NDM_IP_SOCKADDR_ANY6.un.in6);

		in6->s6_addr[12] = (uint8_t) (s4 >> 24);
		in6->s6_addr[13] = (uint8_t) (s4 >> 16);
		in6->s6_addr[14] = (uint8_t) (s4 >>  8);
		in6->s6_addr[15] = (uint8_t) (s4 >>  0);

		sa6->un.in6.sin6_port = sa->un.in.sin_port;

		return true;
	}

	if (ndm_ip_sockaddr_is_v4_mapped(sa) ||
		ndm_ip_sockaddr_is_v4_compat(sa))
	{
		ndm_ip_sockaddr_assign6(sa6, &sa->un.in6);

		in6->s6_addr[10] = 0;
		in6->s6_addr[11] = 0;

		return true;
	}

	return false;
}

void ndm_ip_sockaddr_get_loopback(
		const sa_family_t family,
		struct ndm_ip_sockaddr_t *sa)
{
	assert(family == AF_INET || family == AF_INET6);

	if (family == AF_INET) {
		ndm_ip_sockaddr_assign(sa, &NDM_IP_SOCKADDR_ANY.un.in);
		sa->un.in.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	} else {
		ndm_ip_sockaddr_assign6(sa, &NDM_IP_SOCKADDR_ANY6.un.in6);
		memcpy(
			&sa->un.in6.sin6_addr,
			&in6addr_loopback,
			sizeof(in6addr_loopback));
	}
}

void ndm_ip_sockaddr_set_port(
		struct ndm_ip_sockaddr_t *const sa,
		const uint16_t port)
{
	assert(ndm_ip_sockaddr_is_v4(sa) || ndm_ip_sockaddr_is_v6(sa));

	if (ndm_ip_sockaddr_family(sa) == AF_INET) {
		sa->un.in.sin_port = (uint16_t) htons(port);
	} else {
		sa->un.in6.sin6_port = (uint16_t) htons(port);
	}
}

uint16_t ndm_ip_sockaddr_port(
		const struct ndm_ip_sockaddr_t *const sa)
{
	assert(ndm_ip_sockaddr_is_v4(sa) || ndm_ip_sockaddr_is_v6(sa));

	if (ndm_ip_sockaddr_family(sa) == AF_INET) {
		return (uint16_t) ntohs(sa->un.in.sin_port);
	}

	return (uint16_t) ntohs(sa->un.in6.sin6_port);
}

