#ifndef __NDM_IP_CHECKSUM_H__
#define __NDM_IP_CHECKSUM_H__

#include <stdint.h>
#include "attr.h"

uint32_t ndm_ip_checksum_get(
		const void *data,
		const unsigned long octet_count) NDM_ATTR_WUR;

uint16_t ndm_ip_checksum_finish(
		const uint32_t sum) NDM_ATTR_WUR;

uint16_t ndm_ip_checksum(
		const void *packet,
		const unsigned long packet_size) NDM_ATTR_WUR;

#endif	/* __NDM_IP_CHECKSUM_H__ */

