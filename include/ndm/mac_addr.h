#ifndef __NDM_MAC_ADDR_H__
#define __NDM_MAC_ADDR_H__

#include <stdint.h>
#include <stdbool.h>
#include <sys/socket.h>
#include "attr.h"

#define NDM_MAC_SIZE 				6
#define NDM_MAC_BUFSIZE				sizeof("00:00:00:00:00:00")

struct ndm_mac_addr_t
{
	struct sockaddr sa;
	char str[NDM_MAC_BUFSIZE];
};

extern const struct ndm_mac_addr_t NDM_MAC_ADDR_ZERO;
extern const struct ndm_mac_addr_t NDM_MAC_ADDR_BROADCAST;

bool ndm_mac_addr_assign(
		struct ndm_mac_addr_t *addr,
		const uint8_t *mac,
		const size_t mac_length);

const char *ndm_mac_addr_as_string(
		const struct ndm_mac_addr_t *addr) NDM_ATTR_WUR;

bool ndm_mac_addr_parse(
		const char *const str_addr,
		struct ndm_mac_addr_t *addr) NDM_ATTR_WUR;

bool ndm_mac_addr_is_equal(
		const struct ndm_mac_addr_t *addr1,
		const struct ndm_mac_addr_t *addr2) NDM_ATTR_WUR;

bool ndm_mac_addr_is_zero(
		const struct ndm_mac_addr_t *addr) NDM_ATTR_WUR;
bool ndm_mac_addr_is_broadcast(
		const struct ndm_mac_addr_t *addr) NDM_ATTR_WUR;
bool ndm_mac_addr_is_unicast(
		const struct ndm_mac_addr_t *addr) NDM_ATTR_WUR;
bool ndm_mac_addr_is_multicast(
		const struct ndm_mac_addr_t *addr) NDM_ATTR_WUR;
bool ndm_mac_addr_is_oui_enforced(
		const struct ndm_mac_addr_t *addr) NDM_ATTR_WUR;

#endif	/* __NDM_MAC_ADDR_H__ */

