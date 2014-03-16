#ifndef __NDM_IP_SOCKADDR_H__
#define __NDM_IP_SOCKADDR_H__

#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>
#include "attr.h"

/**
 * Size of the buffer to store the string representation of IP address
 * of maximum length.
 */

#define NDM_IP_SOCKADDR_LEN				INET6_ADDRSTRLEN

#ifndef INADDR_LOOPBACK
#define INADDR_LOOPBACK					((uint32_t) 0x7f000001)
#endif

#ifndef PF_INET
#define PF_INET							AF_INET
#endif

#ifndef PF_INET6
#define PF_INET6						AF_INET6
#endif

/**
 * Structure to store IPv4 or IPv6 address information.
 */

struct ndm_ip_sockaddr_t
{
	union ndm_ip_sockaddr_un_t
	{
		/**
		 * Address family: AF_INET or AF_INET6.
		 */

		sa_family_t family;

		/**
		 * IPv4 address.
		 */

		struct sockaddr_in in;

		/**
		 * IPv6 address.
		 */

		struct sockaddr_in6 in6;
	} un;

	/**
	 * Size of @a in or @a in6 depending on the value stored in @a un.family.
	 */

	socklen_t size;
};

/**
 * Assign IPv4 address to socket.
 *
 * @param[out] sa Socket for assigning.
 * @param[in] in IPv4 address to assign.
 */

void ndm_ip_sockaddr_assign(
		struct ndm_ip_sockaddr_t *sa,
		const struct sockaddr_in *in);

/**
 * Assign IPv6 address to socket.
 *
 * @param[out] sa Socket for assigning.
 * @param[in] in6 IPv6 address to assign.
 */

void ndm_ip_sockaddr_assign6(
		struct ndm_ip_sockaddr_t *sa,
		const struct sockaddr_in6 *in6);

/**
 * Equality checking for two sockets.
 *
 * @param sa1 First socket for comparing.
 * @param sa2 Second socket for comparing.
 *
 * @returns @c true if the sockets are equal, @c false - otherwise.
 */

bool ndm_ip_sockaddr_is_equal(
		const struct ndm_ip_sockaddr_t *const sa1,
		const struct ndm_ip_sockaddr_t *const sa2) NDM_ATTR_WUR;

/**
 * Equality checking for IP addresses of two sockets.
 *
 * @param sa1 First socket for comparing.
 * @param sa2 Second socket for comparing.
 *
 * @returns @c true if the IP addresses of sockets are equal,
 * @c false - otherwise.
 */

bool ndm_ip_sockaddr_address_is_equal(
		const struct ndm_ip_sockaddr_t *const sa1,
		const struct ndm_ip_sockaddr_t *const sa2) NDM_ATTR_WUR;

/**
 * Check if the socket is equal to @c NDM_IP_SOCKADDR_ANY or
 * @c NDM_IP_SOCKADDR_ANY6.
 *
 * @param sa Socket for comparing.
 *
 * @returns @c true if the sockets are equal, @c false - otherwise.
 */

bool ndm_ip_sockaddr_is_any(
		const struct ndm_ip_sockaddr_t *const sa) NDM_ATTR_WUR;

/**
 * Check if the IP address of socket is equal to @c INADDR_ANY or
 * @c IN6ADDR_ANY_INIT.
 *
 * @param sa Socket for comparing.
 *
 * @returns @c true if the IP addreses are equal, @c false - otherwise.
 */

bool ndm_ip_sockaddr_address_is_any(
		const struct ndm_ip_sockaddr_t *const sa) NDM_ATTR_WUR;

/**
 * Check if the socket contains IPv4 address.
 *
 * @param sa Socket for checking.
 *
 * @returns @c true if contains, @c false - otherwise.
 */

bool ndm_ip_sockaddr_is_v4(
		const struct ndm_ip_sockaddr_t *const sa) NDM_ATTR_WUR;

/**
 * Check if IPv6 address of socket is IPv4-mapped IPv6 address (has
 * @c \::ffff:xx.xx.xx.xx format).
 *
 * @param sa Socket for checking.
 *
 * @returns @c true if socket has IPv4-mapped IPv6 address,
 * @c false - otherwise.
 */

bool ndm_ip_sockaddr_is_v4_mapped(
		const struct ndm_ip_sockaddr_t *const sa) NDM_ATTR_WUR;

/**
 * Check if IPv6 address of socket is IPv4-compatible IPv6 address (has
 * @c \::xx.xx.xx.xx format).
 *
 * @param sa Socket for checking.
 *
 * @returns @c true if socket has IPv4-compatible IPv6 address,
 * @c false - otherwise.
 */

bool ndm_ip_sockaddr_is_v4_compat(
		const struct ndm_ip_sockaddr_t *const sa) NDM_ATTR_WUR;

/**
 * Copy @a sa socket to @a sa4 if @a sa contains IPv4 address or IPv4-mapped
 * IPv6 address or IPv4-compatible IPv6 address.
 *
 * @param[in] sa Socket to copy.
 * @param[out] sa4 Socket for copying.
 *
 * @returns @c true if copying was successful, @c false - otherwise.
 */

bool ndm_ip_sockaddr_get_v4(
		const struct ndm_ip_sockaddr_t *const sa,
		struct ndm_ip_sockaddr_t *sa4) NDM_ATTR_WUR;

/**
 * Copy @a sa socket to @a sa6 if @a sa contains IPv4 address or IPv4-compatible
 * IPv6 address or IPv4-mapped IPv6 address. In the first two cases
 * conversion to IPv4-mapped IPv6 address is performed.
 *
 * @param[in] sa Socket to copy.
 * @param[out] sa6 Socket for copying.
 *
 * @returns @c true if copying was successful, @c false - otherwise.
 */

bool ndm_ip_sockaddr_get_v4_mapped(
		const struct ndm_ip_sockaddr_t *const sa,
		struct ndm_ip_sockaddr_t *sa6) NDM_ATTR_WUR;

/**
 * Copy @a sa socket to @a sa6 if @a sa contains IPv4 address or IPv4-mapped
 * IPv6 address or IPv4-compatible IPv6 address. In the first two cases
 * conversion to IPv4-compatible IPv6 address is performed.
 *
 * @param[in] sa Socket to copy.
 * @param[out] sa6 Socket for copying.
 *
 * @returns @c true if copying was successful, @c false - otherwise.
 */

bool ndm_ip_sockaddr_get_v4_compat(
		const struct ndm_ip_sockaddr_t *const sa,
		struct ndm_ip_sockaddr_t *sa6) NDM_ATTR_WUR;

/**
 * Check if the socket contains IPv6 address.
 *
 * @param sa Socket for checking.
 *
 * @returns @c true if contains, @c false - otherwise.
 */

bool ndm_ip_sockaddr_is_v6(
		const struct ndm_ip_sockaddr_t *const sa) NDM_ATTR_WUR;

/**
 * Convert socket address from network view to string view.
 *
 * @param[in] sa Socket to convert.
 * @param[out] dst String view of socket address.
 * @param[in] size String view size.
 *
 * @returns String view of socket address if conversion was successful, or
 * @c NULL.
 */

const char *ndm_ip_sockaddr_ntop(
		const struct ndm_ip_sockaddr_t *const sa,
		char *const dst,
		socklen_t size);

/**
 * Convert socket address from string view to network view.
 *
 * @param[in] src String view of socket address.
 * @param[out] sa Socket for conversion.
 *
 * @returns @c true if conversion was successful, @c false - otherwise.
 */

bool ndm_ip_sockaddr_pton(
		const char *const src,
		struct ndm_ip_sockaddr_t *const sa) NDM_ATTR_WUR;

/**
 * Get value of @c NDM_IP_SOCKADDR_ANY or @c NDM_IP_SOCKADDR_ANY6 depending on
 * @a family.
 *
 * @param family AF_INET or AF_INET6 value.
 *
 * @returns Value of requested constant.
 */

const struct ndm_ip_sockaddr_t *ndm_ip_sockaddr_get_any(
		const int family) NDM_ATTR_WUR;

/**
 * Get loopback IP address for @a sa depending on @a family.
 *
 * @param[in] family AF_INET or AF_INET6 value.
 * @param[out] sa Resulting socket.
 */

void ndm_ip_sockaddr_get_loopback(
		const int family,
		struct ndm_ip_sockaddr_t *sa);

/**
 * Set port number for @a sa socket.
 *
 * @param[out] sa Socket for assigning.
 * @param[in] port Port number.
 */

void ndm_ip_sockaddr_set_port(
		struct ndm_ip_sockaddr_t *const sa,
		const uint16_t port);

/**
 * Get port number of the socket.
 *
 * @param sa Socket address.
 *
 * @returns Port number.
 */

uint16_t ndm_ip_sockaddr_port(
		const struct ndm_ip_sockaddr_t *const sa) NDM_ATTR_WUR;

/**
 * Get the protocol family for @a sa.
 *
 * @param sa Socket address.
 *
 * @returns @c PF_INET or @c PF_INET6 value.
 */

int ndm_ip_sockaddr_domain(
		const struct ndm_ip_sockaddr_t *const sa) NDM_ATTR_WUR;

/**
 * Get the address family for @a sa.
 *
 * @param sa Socket address.
 *
 * @returns @c AF_INET or @c AF_INET6 value.
 */

sa_family_t ndm_ip_sockaddr_family(
		const struct ndm_ip_sockaddr_t *const sa) NDM_ATTR_WUR;

/**
 * IP address that indicates that the server must listen for client activity
 * on all network interfaces. It is equal to <tt>0.0.0.0</tt>.
 */

extern const struct ndm_ip_sockaddr_t NDM_IP_SOCKADDR_ANY;

/**
 * IPv6 address that indicates that the server must listen for client
 * activity on all network interfaces. It is equal to <tt>::</tt>. See also
 * @c #NDM_IP_SOCKADDR_ANY.
 */

extern const struct ndm_ip_sockaddr_t NDM_IP_SOCKADDR_ANY6;

#endif	/* __NDM_IP_SOCKADDR_H__ */

