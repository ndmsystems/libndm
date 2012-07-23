#include <stdio.h>
#include <strings.h>
#include <ndm/mac_addr.h>
#include "test.h"

#define MAC_TEST(s, is_zero, is_bcast, is_ucast, is_mcast, is_oui_enforced)	\
	do {																	\
		struct ndm_mac_addr_t __m__;										\
		struct ndm_mac_addr_t __p__;										\
		char __b__[NDM_MAC_BUFSIZE];										\
																			\
		NDM_TEST(ndm_mac_addr_parse(s, &__m__));							\
		NDM_TEST_BREAK_IF(													\
			snprintf(__b__, sizeof(__b__), "%s",							\
				ndm_mac_addr_as_string(&__m__)) <= 0);						\
		NDM_TEST(strcasecmp(s, __b__) == 0);								\
		NDM_TEST(ndm_mac_addr_parse(s, &__p__));							\
		NDM_TEST(!!ndm_mac_addr_is_equal(&__p__, &__m__));					\
		NDM_TEST(!!ndm_mac_addr_is_zero(&__m__) == !!is_zero);				\
		NDM_TEST(!!ndm_mac_addr_is_broadcast(&__m__) == !!is_bcast);		\
		NDM_TEST(!!ndm_mac_addr_is_unicast(&__m__) == !!is_ucast);			\
		NDM_TEST(!!ndm_mac_addr_is_multicast(&__m__) == !!is_mcast);		\
		NDM_TEST(!!ndm_mac_addr_is_oui_enforced(&__m__) ==					\
			!!is_oui_enforced);												\
	} while (false);

int main()
{
	const uint8_t mac[ETH_ALEN] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
	struct ndm_mac_addr_t m;
	struct ndm_mac_addr_t zero = NDM_MAC_ADDR_ZERO;
	struct ndm_mac_addr_t bcast = NDM_MAC_ADDR_BROADCAST;

	NDM_TEST(strcasecmp("00:00:00:00:00:00",
		ndm_mac_addr_as_string(&NDM_MAC_ADDR_ZERO)) == 0);

	NDM_TEST(strcasecmp("ff:FF:ff:FF:ff:FF",
		ndm_mac_addr_as_string(&NDM_MAC_ADDR_BROADCAST)) == 0);

	NDM_TEST(!ndm_mac_addr_parse("", &m));
	NDM_TEST(!ndm_mac_addr_parse(" ", &m));
	NDM_TEST(!ndm_mac_addr_parse("  ", &m));
	NDM_TEST(!ndm_mac_addr_parse("a", &m));
	NDM_TEST(!ndm_mac_addr_parse("1", &m));
	NDM_TEST(!ndm_mac_addr_parse(" 00:00:00:00:00:00", &m));
	NDM_TEST(!ndm_mac_addr_parse("00:00:00:00:00:00 ", &m));
	NDM_TEST(!ndm_mac_addr_parse(" 00:00:00:00:00:00 ", &m));
	NDM_TEST(!ndm_mac_addr_parse("00:00:00: 00:00:00", &m));
	NDM_TEST(!ndm_mac_addr_parse("00:00:00:00:00:r0", &m));
	NDM_TEST(!ndm_mac_addr_parse("00:00:00;00:00:00", &m));

	NDM_TEST(ndm_mac_addr_parse("00:00:00:00:00:00", &m));
	NDM_TEST(ndm_mac_addr_is_zero(&m));
	NDM_TEST(ndm_mac_addr_is_equal(&zero, &m));

	NDM_TEST(ndm_mac_addr_parse("ff:ff:ff:ff:ff:ff", &m));
	NDM_TEST(ndm_mac_addr_is_broadcast(&m));
	NDM_TEST(ndm_mac_addr_is_equal(&bcast, &m));

	NDM_TEST(ndm_mac_addr_parse("FF:FF:FF:FF:FF:FF", &m));
	NDM_TEST(ndm_mac_addr_is_broadcast(&m));
	NDM_TEST(ndm_mac_addr_is_equal(&bcast, &m));

	NDM_TEST(ndm_mac_addr_parse("FF:FF:ff:ff:FF:FF", &m));
	NDM_TEST(ndm_mac_addr_is_broadcast(&m));
	NDM_TEST(ndm_mac_addr_is_equal(&bcast, &m));

	NDM_TEST(ndm_mac_addr_parse("00:11:22:33:44:55", &m));

	/* MAC_TEST(s, is_zero, is_bcast, is_ucast, is_mcast, is_oui_enforced) */

	MAC_TEST("00:00:00:00:00:00", true, false, false, false, false);
	MAC_TEST("00:11:22:33:44:55", false, false, true, false, false);
	MAC_TEST("11:11:22:33:44:55", false, false, false, true, false);
	MAC_TEST("12:11:22:33:44:55", false, false, true, false, true);
	MAC_TEST("ff:ff:ff:ff:ff:ff", false, true, false, false, true);

	ndm_mac_addr_set(&m, mac);

	NDM_TEST(strcasecmp("00:11:22:33:44:55",
		ndm_mac_addr_as_string(&m)) == 0);

	return NDM_TEST_RESULT;
}

