#ifndef __NDM_IP_SOCKADDR_H__
#define __NDM_IP_SOCKADDR_H__

#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>
#include "attr.h"

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
		const struct ndm_ip_sockaddr_t *const sa2) NDM_ATTR_WUR;

bool ndm_ip_sockaddr_address_is_equal(
		const struct ndm_ip_sockaddr_t *const sa1,
		const struct ndm_ip_sockaddr_t *const sa2) NDM_ATTR_WUR;

bool ndm_ip_sockaddr_is_zero(
		const struct ndm_ip_sockaddr_t *const sa) NDM_ATTR_WUR;

bool ndm_ip_sockaddr_address_is_zero(
		const struct ndm_ip_sockaddr_t *const sa) NDM_ATTR_WUR;

bool ndm_ip_sockaddr_is_v4(
		const struct ndm_ip_sockaddr_t *const sa) NDM_ATTR_WUR;

bool ndm_ip_sockaddr_is_v4_mapped(
		const struct ndm_ip_sockaddr_t *const sa) NDM_ATTR_WUR;

bool ndm_ip_sockaddr_is_v4_compat(
		const struct ndm_ip_sockaddr_t *const sa) NDM_ATTR_WUR;

bool ndm_ip_sockaddr_get_v4(
		const struct ndm_ip_sockaddr_t *const sa,
		struct ndm_ip_sockaddr_t *sa4) NDM_ATTR_WUR;

bool ndm_ip_sockaddr_get_v4_mapped(
		const struct ndm_ip_sockaddr_t *const sa,
		struct ndm_ip_sockaddr_t *sa6) NDM_ATTR_WUR;

bool ndm_ip_sockaddr_get_v4_compat(
		const struct ndm_ip_sockaddr_t *const sa,
		struct ndm_ip_sockaddr_t *sa6) NDM_ATTR_WUR;

bool ndm_ip_sockaddr_is_v6(
		const struct ndm_ip_sockaddr_t *const sa) NDM_ATTR_WUR;

const char *ndm_ip_sockaddr_ntop(
		const struct ndm_ip_sockaddr_t *const sa,
		char *const dst,
		socklen_t size);

bool ndm_ip_sockaddr_pton(
		const char *const src,
		struct ndm_ip_sockaddr_t *const sa) NDM_ATTR_WUR;

const struct ndm_ip_sockaddr_t *ndm_ip_sockaddr_get_zero(
		const int family) NDM_ATTR_WUR;

void ndm_ip_sockaddr_get_loopback(
		const int family,
		struct ndm_ip_sockaddr_t *sa);

void ndm_ip_sockaddr_set_port(
		struct ndm_ip_sockaddr_t *const sa,
		const uint16_t port);

uint16_t ndm_ip_sockaddr_port(
		const struct ndm_ip_sockaddr_t *const sa) NDM_ATTR_WUR;

int ndm_ip_sockaddr_domain(
		const struct ndm_ip_sockaddr_t *const sa) NDM_ATTR_WUR;

sa_family_t ndm_ip_sockaddr_family(
		const struct ndm_ip_sockaddr_t *const sa) NDM_ATTR_WUR;

extern const struct ndm_ip_sockaddr_t NDM_IP4_SOCKADDR_ZERO;
extern const struct ndm_ip_sockaddr_t NDM_IP6_SOCKADDR_ZERO;

#endif	/* __NDM_IP_SOCKADDR_H__ */

