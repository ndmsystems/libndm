#ifndef __NDM_NET_H__
#define __NDM_NET_H__

#include <stdbool.h>
#include "attr.h"

struct addrinfo;

bool ndm_net_is_domain_name(const char *const name) NDM_ATTR_WUR;

int ndm_net_getaddrinfo(
		const char *node,
		const char *service,
		const struct addrinfo *hints,
		struct addrinfo **res) NDM_ATTR_WUR;

void ndm_net_freeaddrinfo(struct addrinfo *res);

const char *ndm_net_gai_strerror(int errcode);

#endif	/* __NDM_NET_H__ */

