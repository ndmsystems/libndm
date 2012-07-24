#ifndef __NDM_IP_SOCKADDR_H__
#define __NDM_IP_SOCKADDR_H__

#include <stdbool.h>
#include <netinet/in.h>

#define NDM_IP_SOCKADDR_LEN				INET6_ADDRSTRLEN

struct ndm_ip_sockaddr_t
{
	union
	{
		sa_family_t family;
		struct sockaddr_in in;
		struct sockaddr_in6 in6;
	} un;

	socklen_t size;
};

void ndm_ip_sockaddr_assign(
		struct ndm_ip_sockaddr_t *sa,
		const struct sockaddr_in *in);

void ndm_ip_sockaddr_assign6(
		struct ndm_ip_sockaddr_t *sa,
		const struct sockaddr_in6 *in6);

bool ndm_ip_sockaddr_is_equal(
		const struct ndm_ip_sockaddr_t *const sa1,
		const struct ndm_ip_sockaddr_t *const sa2);

bool ndm_ip_sockaddr_address_is_equal(
		const struct ndm_ip_sockaddr_t *const sa1,
		const struct ndm_ip_sockaddr_t *const sa2);

bool ndm_ip_sockaddr_is_zero(
		const struct ndm_ip_sockaddr_t *const sa);

const char *ndm_ip_sockaddr_ntop(
		const struct ndm_ip_sockaddr_t *const sa,
		char *const dst,
		socklen_t size);

bool ndm_ip_sockaddr_pton(
		const char *const src,
		struct ndm_ip_sockaddr_t *const sa);

const struct ndm_ip_sockaddr_t *ndm_ip_sockaddr_get_zero(
		const int family);

extern const struct ndm_ip_sockaddr_t NDM_IP4_SOCKADDR_ZERO;
extern const struct ndm_ip_sockaddr_t NDM_IP6_SOCKADDR_ZERO;

#endif	/* __NDM_IP_SOCKADDR_H__ */

