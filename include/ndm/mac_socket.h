#ifndef __NDM_MAC_SOCKET_H__
#define __NDM_MAC_SOCKET_H__

#include <stdint.h>
#include "macro.h"
#include "mac_addr.h"

struct ndm_mac_socket_t;

struct ndm_mac_socket_t *ndm_mac_socket_open(
		const uint16_t protocol,
		const unsigned int iface_index,
		struct ndm_mac_addr_t *local) NDM_WUR;

void ndm_mac_socket_close(
		struct ndm_mac_socket_t **sp);

ssize_t ndm_mac_socket_send(
		const struct ndm_mac_socket_t *s,
		const void *data,
		const size_t size,
		const int flags,
		const unsigned int iface_index,
		const struct ndm_mac_addr_t *dst) NDM_WUR;

ssize_t ndm_mac_socket_recv(
		const struct ndm_mac_socket_t *s,
		void *data,
		const size_t size,
		const int flags,
		unsigned int *iface_index) NDM_WUR;

int ndm_mac_socket_fd(
		const struct ndm_mac_socket_t *s) NDM_WUR;

#endif	/* __NDM_MAC_SOCKET_H__ */

