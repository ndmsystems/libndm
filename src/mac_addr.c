#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <net/if_arp.h>
#include <ndm/mac_addr.h>

const struct ndm_mac_addr_t NDM_MAC_ADDR_ZERO = {
	.sa = {
		.sa_family = ARPHRD_ETHER,
		.sa_data = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
	},
	.str = "00:00:00:00:00:00"
};

const struct ndm_mac_addr_t NDM_MAC_ADDR_BROADCAST = {
	.sa = {
		.sa_family = ARPHRD_ETHER,
		.sa_data = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
	},
	.str = "ff:ff:ff:ff:ff:ff"
};

const char *ndm_mac_addr_as_string(
		const struct ndm_mac_addr_t *addr)
{
	const char *str =
		(addr == &NDM_MAC_ADDR_ZERO) ? NDM_MAC_ADDR_ZERO.str :
		(addr == &NDM_MAC_ADDR_BROADCAST) ?
			NDM_MAC_ADDR_BROADCAST.str : NULL;

	if (str == NULL) {
		static const char HEX_[] = "0123456789abcdef";
		char* p = (char *) addr->str;
		unsigned int i;

		for (i = 0; i < ETH_ALEN; ++i, p += 3) {
			*(p + 0) = HEX_[((uint8_t) addr->sa.sa_data[i]) >> 4];
			*(p + 1) = HEX_[((uint8_t) addr->sa.sa_data[i]) & 0x0f];
			*(p + 2) = ':';
		}

		((char *) addr->str)[sizeof(addr->str) - 1] = '\0';
		str = (const char *) addr->str;
	}

	return str;
}

bool ndm_mac_addr_parse(
		const char *const str_addr,
		struct ndm_mac_addr_t *addr)
{
	uint8_t octets[ETH_ALEN];

	if (strlen(str_addr) == sizeof(addr->str) - 1 &&
		sscanf(str_addr,
			"%02" SCNx8 ":%02" SCNx8 ":%02" SCNx8
			"%02" SCNx8 ":%02" SCNx8 ":%02" SCNx8,
			&octets[0], &octets[1], &octets[2],
			&octets[3], &octets[4], &octets[5]) == ETH_ALEN)
	{
		addr->sa.sa_family = ARPHRD_ETHER;
		memcpy(addr->sa.sa_data, octets, ETH_ALEN);

		return true;
	}

	return false;
}

bool ndm_mac_addr_is_equal(
		const struct ndm_mac_addr_t *addr1,
		const struct ndm_mac_addr_t *addr2)
{
	return
		addr1->sa.sa_family == ARPHRD_ETHER &&
		addr2->sa.sa_family == ARPHRD_ETHER &&
		memcmp(&addr1->sa.sa_data, &addr2->sa.sa_data, ETH_ALEN) == 0;
}

bool ndm_mac_addr_is_zero(
		const struct ndm_mac_addr_t *addr)
{
	return ndm_mac_addr_is_equal(addr, &NDM_MAC_ADDR_ZERO);
}

bool ndm_mac_addr_is_broadcast(
		const struct ndm_mac_addr_t *addr)
{
	return ndm_mac_addr_is_equal(addr, &NDM_MAC_ADDR_BROADCAST);
}

bool ndm_mac_addr_is_unicast(
		const struct ndm_mac_addr_t *addr)
{
	return
		!ndm_mac_addr_is_zero(addr) &&
		!ndm_mac_addr_is_broadcast(addr) &&
		addr->sa.sa_family == ARPHRD_ETHER &&
		(addr->sa.sa_data[0] & 0x01) == 0;
}

bool ndm_mac_addr_is_multicast(
		const struct ndm_mac_addr_t *addr)
{
	return
		!ndm_mac_addr_is_broadcast(addr) &&
		addr->sa.sa_family == ARPHRD_ETHER &&
		(addr->sa.sa_data[0] & 0x01) == 0x01;
}

bool ndm_mac_addr_is_oui_enforced(
		const struct ndm_mac_addr_t *addr)
{
	return
		!ndm_mac_addr_is_zero(addr) &&
		!ndm_mac_addr_is_multicast(addr) &&
		addr->sa.sa_family == ARPHRD_ETHER &&
		(addr->sa.sa_data[0] & 0x02) == 0x02;
}

