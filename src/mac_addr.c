#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <ndm/mac_addr.h>

#ifndef ARPHRD_ETHER
#define ARPHRD_ETHER									1
#endif

#define MAC_ADDR_PATTERN_								"00:00:00:00:00:00"
#define NH_												UINT8_MAX

static const uint8_t HEX_DIGITS_[256] = {
	NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,
	NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,
	NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,
	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,NH_,NH_,NH_,NH_,NH_,NH_,
	NH_, 10, 11, 12, 13, 14, 15,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,
	NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,
	NH_, 10, 11, 12, 13, 14, 15,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,
	NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,
	NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,
	NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,
	NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,
	NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,
	NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,
	NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,
	NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,
	NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_,NH_
};

const struct ndm_mac_addr_t NDM_MAC_ADDR_ZERO = {
	.sa =
	{
		.sa_family = ARPHRD_ETHER,
		.sa_data = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
	},
	.str = "00:00:00:00:00:00"
};

const struct ndm_mac_addr_t NDM_MAC_ADDR_BROADCAST = {
	.sa =
	{
		.sa_family = ARPHRD_ETHER,
		.sa_data = {'\xff', '\xff', '\xff', '\xff', '\xff', '\xff'}
	},
	.str = "ff:ff:ff:ff:ff:ff"
};

void ndm_mac_addr_init(
		struct ndm_mac_addr_t *a)
{
	ndm_mac_addr_assign(a, &NDM_MAC_ADDR_ZERO);
}

void ndm_mac_addr_assign(
		struct ndm_mac_addr_t *a,
		const struct ndm_mac_addr_t *b)
{
	memcpy(a, b, sizeof(*b));
}

bool ndm_mac_addr_assign_array(
		struct ndm_mac_addr_t *addr,
		const uint8_t *mac,
		const size_t mac_size)
{
	bool assigned = false;

	if (mac_size != NDM_MAC_SIZE) {
		errno = EINVAL;
	} else {
		memset(addr, 0, sizeof(*addr));
		addr->sa.sa_family = ARPHRD_ETHER;
		memcpy(addr->sa.sa_data, mac, NDM_MAC_SIZE);

		assigned = true;
	}

	return assigned;
}

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

		for (i = 0; i < NDM_MAC_SIZE; ++i, p += 3) {
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
	const size_t size = strlen(str_addr);

	if (size != sizeof(MAC_ADDR_PATTERN_) - 1) {
		return false;
	}

	uint8_t octets[sizeof(addr->sa.sa_data)] = {0};
	uint8_t offs = 0;

	for (size_t i = 0; i < size; i++) {
		const uint8_t p = (uint8_t) MAC_ADDR_PATTERN_[i];
		const uint8_t c = (uint8_t) str_addr[i];
		const uint8_t d = HEX_DIGITS_[c];

		if (d != NH_) {
			const uint8_t o = offs >> 1;

			if (o >= sizeof(octets)) {
				return false;
			}

			offs++;
			octets[o] |= (uint8_t) (d << ((offs & 0x01) << 2));
		} else if (p != c) {
			return false;
		}
	}

	addr->sa.sa_family = ARPHRD_ETHER;
	memcpy(addr->sa.sa_data, octets, sizeof(addr->sa.sa_data));

	return true;
}

bool ndm_mac_addr_is_equal(
		const struct ndm_mac_addr_t *addr1,
		const struct ndm_mac_addr_t *addr2)
{
	return
		addr1->sa.sa_family == ARPHRD_ETHER &&
		addr2->sa.sa_family == ARPHRD_ETHER &&
		memcmp(&addr1->sa.sa_data, &addr2->sa.sa_data, NDM_MAC_SIZE) == 0;
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

