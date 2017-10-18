#ifndef __NDM_NET_H__
#define __NDM_NET_H__

#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "attr.h"

bool ndm_net_is_domain_name(const char *const name) NDM_ATTR_WUR;

int ndm_getaddrinfo(const char *node, const char *service,
		const struct addrinfo *hints, struct addrinfo **res) NDM_ATTR_WUR;

void ndm_freeaddrinfo(struct addrinfo *res);

#endif	/* __NDM_NET_H__ */

