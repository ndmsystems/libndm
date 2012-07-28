#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <ndm/mac_socket.h>

struct ndm_mac_socket_t {
	int fd;
	uint16_t protocol;
};

struct ndm_mac_socket_t *ndm_mac_socket_open(
		const uint16_t protocol,
		const unsigned int iface_index,
		struct ndm_mac_addr_t *local)
{
	bool opened = false;
	struct ndm_mac_socket_t *s = malloc(sizeof(*s));

	if (s != NULL) {
		s->protocol = htons(protocol);

		if ((s->fd = socket(PF_PACKET, SOCK_RAW, s->protocol)) < 0) {
			/* failed to open a socket */
			s->fd = -1;
		} else {
			struct sockaddr_ll sa;

			memset(&sa, 0, sizeof(sa));

			sa.sll_family = AF_PACKET;
			sa.sll_protocol = s->protocol;
			sa.sll_ifindex = (int) iface_index;

			if (bind(s->fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
				/* failed to bind */
			} else {
				opened = true;

				if (local != NULL) {
					socklen_t sa_size = sizeof(sa);

					if (getsockname(s->fd,
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
			ndm_mac_socket_close(&s);
		}
	}

	return s;
}

void ndm_mac_socket_close(
		struct ndm_mac_socket_t **sp)
{
	if (*sp != NULL) {
		if ((*sp)->fd >= 0) {
			/* do not try to close invalid descriptors,
			 * since close() changes errno value */
			close((*sp)->fd);
		}

		free(*sp);
		*sp = NULL;
	}
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
	sa.sll_ifindex = (int) iface_index;

	memcpy(sa.sll_addr, dst->sa.sa_data, sa.sll_halen);

	return sendto(s->fd, data, size, flags,
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
		s->fd, data, size, flags,
		(struct sockaddr *) &address, &address_length);

	if (recvb >= 0) {
		*iface_index = (unsigned int) address.sll_ifindex;
	}

	return recvb;
}

int ndm_mac_socket_fd(
		const struct ndm_mac_socket_t *s)
{
	return s->fd;
}

