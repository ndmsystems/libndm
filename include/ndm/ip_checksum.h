#ifndef __NDM_IP_CHECKSUM_H__
#define __NDM_IP_CHECKSUM_H__

#include <stdint.h>

uint32_t ndm_ip_checksum_get(
		const void *data,
		const unsigned long octet_count);

uint16_t ndm_ip_checksum_finish(
		const uint32_t sum);

inline uint16_t ndm_ip_checksum(
		const void *packet,
		const unsigned long packet_size)
{
	return ndm_ip_checksum_finish(ndm_ip_checksum_get(packet, packet_size));
}

#endif	/* __NDM_IP_CHECKSUM_H__ */

