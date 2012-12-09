#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
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
	},
	.size = sizeof(struct sockaddr_in)
};

const struct ndm_ip_sockaddr_t NDM_IP_SOCKADDR_ANY6 =
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
	return
		sa1->un.family == sa2->un.family &&
		sa1->size == sa2->size &&
		((sa1->un.in.sin_family == AF_INET &&
		  sa1->un.in.sin_port == sa2->un.in.sin_port &&
		  sa1->un.in.sin_addr.s_addr == sa2->un.in.sin_addr.s_addr) ||
		 (sa1->un.in6.sin6_family == AF_INET6 &&
#ifdef SIN6_LEN
		  sa1->un.in6.sin6_len == sa2->un.in6.sin6_len &&
#endif	/* SIN6_LEN */
		  sa1->un.in6.sin6_flowinfo == sa2->un.in6.sin6_flowinfo &&
		  sa1->un.in6.sin6_port == sa2->un.in6.sin6_port &&
		  memcmp(
		  	&sa1->un.in6.sin6_addr.s6_addr,
			&sa2->un.in6.sin6_addr.s6_addr,
			sizeof(sa1->un.in6.sin6_addr.s6_addr)) == 0)) ?
		true : false;
}

bool ndm_ip_sockaddr_address_is_equal(
		const struct ndm_ip_sockaddr_t *const sa1,
		const struct ndm_ip_sockaddr_t *const sa2)
{
	return
		sa1->un.family == sa2->un.family &&
		((sa1->un.family == AF_INET &&
		  memcmp(
		  	&sa1->un.in.sin_addr.s_addr,
			&sa2->un.in.sin_addr.s_addr,
		  	sizeof(sa1->un.in.sin_addr.s_addr)) == 0) ||
		 (sa1->un.family == AF_INET6 &&
		  memcmp(
		  	&sa1->un.in6.sin6_addr.s6_addr,
		  	&sa2->un.in6.sin6_addr.s6_addr,
		  	sizeof(sa1->un.in6.sin6_addr.s6_addr)) == 0)) ? true : false;
}

bool ndm_ip_sockaddr_is_any(
		const struct ndm_ip_sockaddr_t *const sa)
{
	assert(ndm_ip_sockaddr_is_v4(sa) || ndm_ip_sockaddr_is_v6(sa));

	return ndm_ip_sockaddr_is_equal(sa,
		ndm_ip_sockaddr_get_any(ndm_ip_sockaddr_family(sa)));
}

bool ndm_ip_sockaddr_address_is_any(
		const struct ndm_ip_sockaddr_t *const sa)
{
	assert(ndm_ip_sockaddr_is_v4(sa) || ndm_ip_sockaddr_is_v6(sa));

	return ndm_ip_sockaddr_address_is_equal(sa,
		ndm_ip_sockaddr_get_any(ndm_ip_sockaddr_family(sa)));
}

bool ndm_ip_sockaddr_is_v4(
		const struct ndm_ip_sockaddr_t *const sa)
{
	return (ndm_ip_sockaddr_family(sa) == AF_INET) ? true : false;
}

static inline bool __ndm_ip_sockaddr_has_prefix(
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
	static const uint8_t V4MAPPED_PREFIX[] =
	{
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0xff, 0xff
	};

	return __ndm_ip_sockaddr_has_prefix(sa,
		V4MAPPED_PREFIX, sizeof(V4MAPPED_PREFIX));
}

bool ndm_ip_sockaddr_is_v4_compat(
		const struct ndm_ip_sockaddr_t *const sa)
{
	static const uint8_t V4COMPAT_PREFIX[] =
	{
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00
	};

	return __ndm_ip_sockaddr_has_prefix(sa,
		V4COMPAT_PREFIX, sizeof(V4COMPAT_PREFIX));
}

bool ndm_ip_sockaddr_get_v4(
		const struct ndm_ip_sockaddr_t *const sa,
		struct ndm_ip_sockaddr_t *sa4)
{
	bool done = true;

	if (ndm_ip_sockaddr_is_v4(sa)) {
		*sa4 = *sa;
	} else
	if (ndm_ip_sockaddr_is_v4_mapped(sa) ||
		ndm_ip_sockaddr_is_v4_compat(sa))
	{
		const struct in6_addr *in6 = &sa->un.in6.sin6_addr;

		sa4->un.in.sin_family = AF_INET;
		sa4->un.in.sin_port = sa->un.in6.sin6_port;
		sa4->un.in.sin_addr.s_addr = htonl(
			(((uint32_t) in6->s6_addr[12]) << 24) |
			(((uint32_t) in6->s6_addr[13]) << 16) |
			(((uint32_t) in6->s6_addr[14]) << 8) |
			 ((uint32_t) in6->s6_addr[15]));
		sa4->size = sizeof(sa4->un.in);
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
	bool done = true;
	struct in6_addr *in6 = &sa6->un.in6.sin6_addr;

	if (ndm_ip_sockaddr_is_v4(sa)) {
		const uint32_t s4 = ntohl(sa->un.in.sin_addr.s_addr);

		*sa6 = NDM_IP_SOCKADDR_ANY6;
		in6->s6_addr[12] = (uint8_t) (s4 >> 24);
		in6->s6_addr[13] = (uint8_t) (s4 >> 16);
		in6->s6_addr[14] = (uint8_t) (s4 >> 8);
		in6->s6_addr[15] = (uint8_t) (s4);
		sa6->un.in6.sin6_port = sa->un.in.sin_port;
	} else
	if (ndm_ip_sockaddr_is_v4_mapped(sa) ||
		ndm_ip_sockaddr_is_v4_compat(sa))
	{
		*sa6 = *sa;
		in6->s6_addr[10] = 0;
		in6->s6_addr[11] = 0;
	} else {
		done = false;
	}

	return done;
}

bool ndm_ip_sockaddr_is_v6(
		const struct ndm_ip_sockaddr_t *const sa)
{
	return (ndm_ip_sockaddr_family(sa) == AF_INET6) ? true : false;
}

const struct ndm_ip_sockaddr_t *ndm_ip_sockaddr_get_any(
		const int family)
{
	assert(family == AF_INET || family == AF_INET6);

	return (family == AF_INET) ?
		&NDM_IP_SOCKADDR_ANY :
		&NDM_IP_SOCKADDR_ANY6;
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
	assert(ndm_ip_sockaddr_is_v4(sa) || ndm_ip_sockaddr_is_v6(sa));

	if (sa->un.family == AF_INET) {
		sa->un.in.sin_port = (uint16_t) htons(port);
	} else {
		sa->un.in6.sin6_port = (uint16_t) htons(port);
	}
}

uint16_t ndm_ip_sockaddr_port(
		const struct ndm_ip_sockaddr_t *const sa)
{
	assert(ndm_ip_sockaddr_is_v4(sa) || ndm_ip_sockaddr_is_v6(sa));

	if (sa->un.family == AF_INET) {
		return (uint16_t) ntohs(sa->un.in.sin_port);
	}

	return (uint16_t) ntohs(sa->un.in6.sin6_port);
}

int ndm_ip_sockaddr_domain(
		const struct ndm_ip_sockaddr_t *const sa)
{
	assert(ndm_ip_sockaddr_is_v4(sa) || ndm_ip_sockaddr_is_v6(sa));

	return (sa->un.family == AF_INET) ? PF_INET : PF_INET6;
}

sa_family_t ndm_ip_sockaddr_family(
		const struct ndm_ip_sockaddr_t *const sa)
{
	return sa->un.family;
}

