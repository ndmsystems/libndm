#ifndef __NDM_MAC_SOCKET_H__
#define __NDM_MAC_SOCKET_H__

#include <stdint.h>
#include <stdbool.h>
#include "attr.h"
#include "mac_addr.h"

#define NDM_MAC_SOCKET_INITIALIZER							\
	{.__fd = -1, .__protocol = 0}

struct ndm_mac_socket_t
{
	int __fd;
	uint16_t __protocol;
};

static inline void ndm_mac_socket_init(
		struct ndm_mac_socket_t *s)
{
	s->__fd = -1;
	s->__protocol = 0;
}

bool ndm_mac_socket_open(
		struct ndm_mac_socket_t *s,
		const uint16_t protocol,
		const unsigned int *iface_index,
		struct ndm_mac_addr_t *local) NDM_ATTR_WUR;

void ndm_mac_socket_close(
		struct ndm_mac_socket_t *p);

ssize_t ndm_mac_socket_send(
		const struct ndm_mac_socket_t *s,
		const void *data,
		const size_t size,
		const int flags,
		const unsigned int iface_index,
		const struct ndm_mac_addr_t *dst) NDM_ATTR_WUR;

ssize_t ndm_mac_socket_recv(
		const struct ndm_mac_socket_t *s,
		void *data,
		const size_t size,
		const int flags,
		unsigned int *iface_index) NDM_ATTR_WUR;

static int ndm_mac_socket_fd(
		const struct ndm_mac_socket_t *s) NDM_ATTR_WUR;

static inline int ndm_mac_socket_fd(
		const struct ndm_mac_socket_t *s)
{
	return s->__fd;
}

#endif	/* __NDM_MAC_SOCKET_H__ */

