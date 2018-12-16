#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ndm/mac_socket.h>

#ifdef __MACH__
#include <net/if_dl.h>

#define sockaddr_ll							sockaddr_dl
#define sll_family							sdl_family
#define sll_protocol						sdl_type
#define sll_ifindex							sdl_index
#define sll_halen							sdl_alen
#define sll_addr							sdl_data

#define sll_iface_t							u_short
#define sll_protocol_t						u_char

#define PF_PACKET							PF_LINK
#define AF_PACKET							PF_PACKET
#else
#include <netpacket/packet.h>

#define sll_iface_t							int
#define sll_protocol_t						uint16_t
#endif

bool ndm_mac_socket_open(
		struct ndm_mac_socket_t *s,
		const uint16_t protocol,
		const unsigned int *iface_index,
		struct ndm_mac_addr_t *local)
{
	bool opened = false;

	s->__protocol = (sll_protocol_t) htons(protocol);

	if ((s->__fd = socket(PF_PACKET, SOCK_RAW, s->__protocol)) < 0) {
		/* failed to open a socket */
		s->__fd = -1;
	} else
	if (iface_index == NULL) {
		/* do not bind a socket */
		opened = true;
	} else {
		struct sockaddr_ll sa;

		memset(&sa, 0, sizeof(sa));

		sa.sll_family = AF_PACKET;
		sa.sll_protocol = (sll_protocol_t) s->__protocol;
		sa.sll_ifindex = (sll_iface_t) *iface_index;

		if (bind(s->__fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
			/* failed to bind */
		} else {
			opened = true;

			if (local != NULL) {
				socklen_t sa_size = sizeof(sa);

				if (getsockname(s->__fd,
						(struct sockaddr *) &sa, &sa_size) < 0)
				{
					/* failed to get a local hardware address */
					opened = false;
				} else
				if (sa.sll_family != AF_PACKET ||
					sa.sll_halen != NDM_MAC_SIZE)
				{
					/* unexpected hardware address type? */
					errno = EADDRNOTAVAIL;
					opened = false;
				} else {
					*local = NDM_MAC_ADDR_ZERO;
					memcpy(local->sa.sa_data, sa.sll_addr, sa.sll_halen);
				}
			}
		}
	}

	if (!opened) {
		ndm_mac_socket_close(s);
	}

	return opened;
}

void ndm_mac_socket_close(
		struct ndm_mac_socket_t *s)
{
	if (s->__fd >= 0) {
		/* do not try to close invalid descriptors,
		 * since close() changes errno value */
		close(s->__fd);
	}

	ndm_mac_socket_init(s);
}

ssize_t ndm_mac_socket_send(
		const struct ndm_mac_socket_t *s,
		const void *data,
		const size_t size,
		const int flags,
		const unsigned int iface_index,
		const struct ndm_mac_addr_t *dst)
{
	struct sockaddr_ll sa;

	memset(&sa, 0, sizeof(sa));

	sa.sll_halen = NDM_MAC_SIZE;
	sa.sll_family = AF_PACKET;
	sa.sll_ifindex = (sll_iface_t) iface_index;

	memcpy(sa.sll_addr, dst->sa.sa_data, sa.sll_halen);

	return sendto(s->__fd, data, size, flags,
		(struct sockaddr *) &sa, sizeof(sa));
}

ssize_t ndm_mac_socket_recv(
		const struct ndm_mac_socket_t *s,
		void *data,
		const size_t size,
		const int flags,
		unsigned int *iface_index)
{
	struct sockaddr_ll address;
	socklen_t address_length = sizeof(address);
	const ssize_t recvb = recvfrom(
		s->__fd, data, size, flags,
		(struct sockaddr *) &address, &address_length);

	if (recvb >= 0) {
		*iface_index = (unsigned int) address.sll_ifindex;
	}

	return recvb;
}
