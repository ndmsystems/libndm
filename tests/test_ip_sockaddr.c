#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ndm/macro.h>
#include <ndm/ip_sockaddr.h>
#include "test.h"

#define IPV4_TEST_ADDRESS		"192.168.34.82"

#define IPV6_TEST_ADDRESS		"2a6b:338e::33a0:bf02:da12"

int main()
{
	struct ndm_ip_sockaddr_t a;
	struct ndm_ip_sockaddr_t b;
	struct sockaddr_in in;
	struct sockaddr_in6 in6;
	char addr[NDM_IP_SOCKADDR_LEN];

	/* IPv4 any address test */

	in.sin_family = AF_INET;
	in.sin_port = 0;
	in.sin_addr.s_addr = htonl(INADDR_ANY);

	ndm_ip_sockaddr_assign(&a, &in);

	NDM_TEST(ndm_ip_sockaddr_family(&a) == AF_INET);
	NDM_TEST(ndm_ip_sockaddr_domain(&a) == PF_INET);
	NDM_TEST(ndm_ip_sockaddr_is_v4(&a));
	NDM_TEST(!ndm_ip_sockaddr_is_v6(&a));
	NDM_TEST(ndm_ip_sockaddr_ntop(&a, addr, sizeof(addr)) != NULL);
	NDM_TEST(strcmp(addr, "0.0.0.0") == 0);

	b = a;

	NDM_TEST(ndm_ip_sockaddr_is_equal(&a, &b));
	NDM_TEST(ndm_ip_sockaddr_is_any(&a));
	NDM_TEST(ndm_ip_sockaddr_address_is_any(&a));

	ndm_ip_sockaddr_get_loopback(ndm_ip_sockaddr_family(&a), &b);

	NDM_TEST(!ndm_ip_sockaddr_is_equal(&a, &b));

	NDM_TEST(!ndm_ip_sockaddr_is_v4_mapped(&a));
	NDM_TEST(!ndm_ip_sockaddr_is_v4_compat(&a));

	NDM_TEST(ndm_ip_sockaddr_pton(addr, &b));
	NDM_TEST(ndm_ip_sockaddr_is_equal(&a, &b));
	NDM_TEST(ndm_ip_sockaddr_address_is_equal(
		&a, ndm_ip_sockaddr_get_any(AF_INET)));

	NDM_TEST(ndm_ip_sockaddr_port(&a) == 0);

	ndm_ip_sockaddr_set_port(&a, 8081);

	NDM_TEST(ndm_ip_sockaddr_port(&a) == 8081);

	NDM_TEST(ndm_ip_sockaddr_get_v4_mapped(&a, &b));
	NDM_TEST_BREAK_IF(ndm_ip_sockaddr_ntop(&b, addr, sizeof(addr)) == NULL);
	NDM_TEST(strcmp(addr, "::ffff:0.0.0.0") == 0);

	NDM_TEST(ndm_ip_sockaddr_get_v4_compat(&a, &b));
	NDM_TEST_BREAK_IF(ndm_ip_sockaddr_ntop(&b, addr, sizeof(addr)) == NULL);
	NDM_TEST(strcmp(addr, "::") == 0);

	NDM_TEST(ndm_ip_sockaddr_get_v4(&a, &b));
	NDM_TEST(ndm_ip_sockaddr_is_equal(&a, &b));
	NDM_TEST(!ndm_ip_sockaddr_is_any(&a));

	/* IPv4 loopback address test */

	in.sin_family = AF_INET;
	in.sin_port = 0;
	in.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	ndm_ip_sockaddr_assign(&a, &in);

	NDM_TEST(ndm_ip_sockaddr_family(&a) == AF_INET);
	NDM_TEST(ndm_ip_sockaddr_domain(&a) == PF_INET);
	NDM_TEST(ndm_ip_sockaddr_is_v4(&a));
	NDM_TEST(!ndm_ip_sockaddr_is_v6(&a));
	NDM_TEST(ndm_ip_sockaddr_ntop(&a, addr, sizeof(addr)) != NULL);
	NDM_TEST(strcmp(addr, "127.0.0.1") == 0);

	b = a;

	NDM_TEST(ndm_ip_sockaddr_is_equal(&a, &b));
	NDM_TEST(!ndm_ip_sockaddr_is_any(&a));
	NDM_TEST(!ndm_ip_sockaddr_address_is_any(&a));

	ndm_ip_sockaddr_get_loopback(ndm_ip_sockaddr_family(&a), &b);

	NDM_TEST(ndm_ip_sockaddr_is_equal(&a, &b));

	NDM_TEST(!ndm_ip_sockaddr_is_v4_mapped(&a));
	NDM_TEST(!ndm_ip_sockaddr_is_v4_compat(&a));

	NDM_TEST(ndm_ip_sockaddr_pton(addr, &b));
	NDM_TEST(ndm_ip_sockaddr_is_equal(&a, &b));
	NDM_TEST(ndm_ip_sockaddr_address_is_equal(&a, &b));

	NDM_TEST(ndm_ip_sockaddr_port(&a) == 0);

	ndm_ip_sockaddr_set_port(&a, 8081);

	NDM_TEST(ndm_ip_sockaddr_port(&a) == 8081);

	NDM_TEST(ndm_ip_sockaddr_get_v4_mapped(&a, &b));
	NDM_TEST_BREAK_IF(ndm_ip_sockaddr_ntop(&b, addr, sizeof(addr)) == NULL);
	NDM_TEST(strcmp(addr, "::ffff:127.0.0.1") == 0);

	NDM_TEST(ndm_ip_sockaddr_get_v4_compat(&a, &b));
	NDM_TEST_BREAK_IF(ndm_ip_sockaddr_ntop(&b, addr, sizeof(addr)) == NULL);
	NDM_TEST(strcmp(addr, "::127.0.0.1") == 0);

	NDM_TEST(ndm_ip_sockaddr_get_v4(&a, &b));
	NDM_TEST(ndm_ip_sockaddr_is_equal(&a, &b));
	NDM_TEST(!ndm_ip_sockaddr_is_any(&a));
	NDM_TEST(!ndm_ip_sockaddr_address_is_any(&a));

	/* IPv4 class C address test */

	in.sin_family = AF_INET;
	in.sin_port = 0;
	in.sin_addr.s_addr = inet_addr(IPV4_TEST_ADDRESS);

	ndm_ip_sockaddr_assign(&a, &in);

	NDM_TEST(ndm_ip_sockaddr_family(&a) == AF_INET);
	NDM_TEST(ndm_ip_sockaddr_domain(&a) == PF_INET);
	NDM_TEST(ndm_ip_sockaddr_is_v4(&a));
	NDM_TEST(!ndm_ip_sockaddr_is_v6(&a));
	NDM_TEST(ndm_ip_sockaddr_ntop(&a, addr, sizeof(addr)) != NULL);
	NDM_TEST(strcmp(addr, IPV4_TEST_ADDRESS) == 0);

	b = a;

	NDM_TEST(ndm_ip_sockaddr_is_equal(&a, &b));
	NDM_TEST(!ndm_ip_sockaddr_is_any(&a));
	NDM_TEST(!ndm_ip_sockaddr_address_is_any(&a));

	ndm_ip_sockaddr_get_loopback(ndm_ip_sockaddr_family(&a), &b);

	NDM_TEST(!ndm_ip_sockaddr_is_equal(&a, &b));

	NDM_TEST(!ndm_ip_sockaddr_is_v4_mapped(&a));
	NDM_TEST(!ndm_ip_sockaddr_is_v4_compat(&a));

	NDM_TEST(ndm_ip_sockaddr_pton(addr, &b));
	NDM_TEST(ndm_ip_sockaddr_is_equal(&a, &b));
	NDM_TEST(ndm_ip_sockaddr_address_is_equal(&a, &b));

	NDM_TEST(ndm_ip_sockaddr_port(&a) == 0);

	ndm_ip_sockaddr_set_port(&a, 8081);

	NDM_TEST(ndm_ip_sockaddr_port(&a) == 8081);

	NDM_TEST(ndm_ip_sockaddr_get_v4_mapped(&a, &b));
	NDM_TEST_BREAK_IF(ndm_ip_sockaddr_ntop(&b, addr, sizeof(addr)) == NULL);
	NDM_TEST(strcmp(addr, "::ffff:" IPV4_TEST_ADDRESS) == 0);

	NDM_TEST(ndm_ip_sockaddr_get_v4_compat(&a, &b));
	NDM_TEST_BREAK_IF(ndm_ip_sockaddr_ntop(&b, addr, sizeof(addr)) == NULL);
	NDM_TEST(strcmp(addr, "::" IPV4_TEST_ADDRESS) == 0);

	NDM_TEST(ndm_ip_sockaddr_get_v4(&a, &b));
	NDM_TEST(ndm_ip_sockaddr_is_equal(&a, &b));
	NDM_TEST(!ndm_ip_sockaddr_is_any(&a));
	NDM_TEST(!ndm_ip_sockaddr_address_is_any(&a));

	/* IPv6 any address test */

#ifdef SIN6_LEN
	in6.sin6_len = SIN6_LEN;
#endif  /* SIN6_LEN */
	in6.sin6_family = AF_INET6;
	in6.sin6_flowinfo = 0;
	in6.sin6_port = 0;
	in6.sin6_addr = in6addr_any;

	ndm_ip_sockaddr_assign6(&a, &in6);

	NDM_TEST(ndm_ip_sockaddr_family(&a) == AF_INET6);
	NDM_TEST(ndm_ip_sockaddr_domain(&a) == PF_INET6);
	NDM_TEST(!ndm_ip_sockaddr_is_v4(&a));
	NDM_TEST(ndm_ip_sockaddr_is_v6(&a));
	NDM_TEST(ndm_ip_sockaddr_ntop(&a, addr, sizeof(addr)) != NULL);
	NDM_TEST(strcmp(addr, "::") == 0);

	b = a;

	NDM_TEST(ndm_ip_sockaddr_is_equal(&a, &b));
	NDM_TEST(ndm_ip_sockaddr_is_any(&a));
	NDM_TEST(ndm_ip_sockaddr_address_is_any(&a));

	ndm_ip_sockaddr_get_loopback(ndm_ip_sockaddr_family(&a), &b);

	NDM_TEST(!ndm_ip_sockaddr_is_equal(&a, &b));

	NDM_TEST(!ndm_ip_sockaddr_is_v4_mapped(&a));
	NDM_TEST(ndm_ip_sockaddr_is_v4_compat(&a));

	NDM_TEST(ndm_ip_sockaddr_pton(addr, &b));
	NDM_TEST(ndm_ip_sockaddr_is_equal(&a, &b));
	NDM_TEST(ndm_ip_sockaddr_address_is_equal(
		&a, ndm_ip_sockaddr_get_any(
			ndm_ip_sockaddr_family(&a))));

	NDM_TEST(ndm_ip_sockaddr_port(&a) == 0);

	ndm_ip_sockaddr_set_port(&a, 8081);

	NDM_TEST(ndm_ip_sockaddr_port(&a) == 8081);

	NDM_TEST(ndm_ip_sockaddr_get_v4_mapped(&a, &b));
	NDM_TEST_BREAK_IF(ndm_ip_sockaddr_ntop(&b, addr, sizeof(addr)) == NULL);
	NDM_TEST(strcmp(addr, "::ffff:0.0.0.0") == 0);

	NDM_TEST(ndm_ip_sockaddr_get_v4_compat(&a, &b));
	NDM_TEST_BREAK_IF(ndm_ip_sockaddr_ntop(&b, addr, sizeof(addr)) == NULL);
	NDM_TEST(strcmp(addr, "::") == 0);

	NDM_TEST(ndm_ip_sockaddr_get_v4(&a, &b));

	NDM_TEST(ndm_ip_sockaddr_address_is_equal(
		&b, ndm_ip_sockaddr_get_any(AF_INET)));
	NDM_TEST(ndm_ip_sockaddr_address_is_any(&a));
	NDM_TEST(ndm_ip_sockaddr_address_is_any(&b));

	NDM_TEST(!ndm_ip_sockaddr_is_equal(
		&b, ndm_ip_sockaddr_get_any(AF_INET)));
	NDM_TEST(!ndm_ip_sockaddr_is_any(&a));
	NDM_TEST(!ndm_ip_sockaddr_is_any(&b));

	/* IPv6 loopback address test */

#ifdef SIN6_LEN
	in6.sin6_len = SIN6_LEN;
#endif  /* SIN6_LEN */
	in6.sin6_family = AF_INET6;
	in6.sin6_flowinfo = 0;
	in6.sin6_port = 0;
	in6.sin6_addr = in6addr_loopback;

	ndm_ip_sockaddr_assign6(&a, &in6);

	NDM_TEST(ndm_ip_sockaddr_family(&a) == AF_INET6);
	NDM_TEST(ndm_ip_sockaddr_domain(&a) == PF_INET6);
	NDM_TEST(!ndm_ip_sockaddr_is_v4(&a));
	NDM_TEST(ndm_ip_sockaddr_is_v6(&a));
	NDM_TEST(ndm_ip_sockaddr_ntop(&a, addr, sizeof(addr)) != NULL);
	NDM_TEST(strcmp(addr, "::1") == 0);

	b = a;

	NDM_TEST(ndm_ip_sockaddr_is_equal(&a, &b));
	NDM_TEST(!ndm_ip_sockaddr_is_any(&a));
	NDM_TEST(!ndm_ip_sockaddr_address_is_any(&a));

	ndm_ip_sockaddr_get_loopback(ndm_ip_sockaddr_family(&a), &b);

	NDM_TEST(ndm_ip_sockaddr_is_equal(&a, &b));

	NDM_TEST(!ndm_ip_sockaddr_is_v4_mapped(&a));
	NDM_TEST(ndm_ip_sockaddr_is_v4_compat(&a));

	NDM_TEST(ndm_ip_sockaddr_pton(addr, &b));
	NDM_TEST(ndm_ip_sockaddr_is_equal(&a, &b));

	ndm_ip_sockaddr_get_loopback(ndm_ip_sockaddr_family(&a), &b);

	NDM_TEST(ndm_ip_sockaddr_address_is_equal(&a, &b));

	NDM_TEST(ndm_ip_sockaddr_port(&a) == 0);

	ndm_ip_sockaddr_set_port(&a, 8081);

	NDM_TEST(ndm_ip_sockaddr_port(&a) == 8081);

	NDM_TEST(ndm_ip_sockaddr_get_v4_mapped(&a, &b));
	NDM_TEST(ndm_ip_sockaddr_get_v4_compat(&a, &b));
	NDM_TEST(ndm_ip_sockaddr_get_v4(&a, &b));

	/* IPv6 address test */

#ifdef SIN6_LEN
	in6.sin6_len = SIN6_LEN;
#endif  /* SIN6_LEN */
	in6.sin6_family = AF_INET6;
	in6.sin6_flowinfo = 0;
	in6.sin6_port = 0;

	NDM_TEST(inet_pton(AF_INET6, IPV6_TEST_ADDRESS, &in6.sin6_addr) == 1);

	ndm_ip_sockaddr_assign6(&a, &in6);

	NDM_TEST(ndm_ip_sockaddr_family(&a) == AF_INET6);
	NDM_TEST(ndm_ip_sockaddr_domain(&a) == PF_INET6);
	NDM_TEST(!ndm_ip_sockaddr_is_v4(&a));
	NDM_TEST(ndm_ip_sockaddr_is_v6(&a));
	NDM_TEST(ndm_ip_sockaddr_ntop(&a, addr, sizeof(addr)) != NULL);
	NDM_TEST(strcmp(addr, IPV6_TEST_ADDRESS) == 0);

	b = a;

	NDM_TEST(ndm_ip_sockaddr_is_equal(&a, &b));
	NDM_TEST(!ndm_ip_sockaddr_is_any(&a));
	NDM_TEST(!ndm_ip_sockaddr_address_is_any(&a));

	ndm_ip_sockaddr_get_loopback(ndm_ip_sockaddr_family(&a), &b);

	NDM_TEST(!ndm_ip_sockaddr_is_equal(&a, &b));

	NDM_TEST(!ndm_ip_sockaddr_is_v4_mapped(&a));
	NDM_TEST(!ndm_ip_sockaddr_is_v4_compat(&a));

	NDM_TEST(ndm_ip_sockaddr_pton(addr, &b));
	NDM_TEST(ndm_ip_sockaddr_is_equal(&a, &b));
	NDM_TEST(!ndm_ip_sockaddr_address_is_equal(
		&a, ndm_ip_sockaddr_get_any(
			ndm_ip_sockaddr_family(&a))));

	NDM_TEST(ndm_ip_sockaddr_port(&a) == 0);

	ndm_ip_sockaddr_set_port(&a, 8081);

	NDM_TEST(ndm_ip_sockaddr_port(&a) == 8081);

	NDM_TEST(!ndm_ip_sockaddr_get_v4_mapped(&a, &b));
	NDM_TEST(!ndm_ip_sockaddr_get_v4_compat(&a, &b));
	NDM_TEST(!ndm_ip_sockaddr_get_v4(&a, &b));

	NDM_TEST(!ndm_ip_sockaddr_address_is_equal(
		&b, ndm_ip_sockaddr_get_any(AF_INET)));
	NDM_TEST(!ndm_ip_sockaddr_address_is_any(&a));
	NDM_TEST(!ndm_ip_sockaddr_address_is_any(&b));

	NDM_TEST(!ndm_ip_sockaddr_is_equal(
		&b, ndm_ip_sockaddr_get_any(AF_INET)));
	NDM_TEST(!ndm_ip_sockaddr_is_any(&a));
	NDM_TEST(!ndm_ip_sockaddr_is_any(&b));

	return NDM_TEST_RESULT;
}

