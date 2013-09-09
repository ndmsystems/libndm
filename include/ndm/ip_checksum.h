#ifndef __NDM_IP_CHECKSUM_H__
#define __NDM_IP_CHECKSUM_H__

#include <stdint.h>
#include "attr.h"

/**
 * Perform an intermediate calculation of the 32-bit checksum for the array
 * pointed by @a data, the size of @a octet_count.
 *
 * @param data Pointer to array.
 * @param octet_count An array size.
 *
 * @returns 32-bit checksum value.
 */

uint32_t ndm_ip_checksum_get(
		const void *data,
		const unsigned long octet_count) NDM_ATTR_WUR;

/**
 * Calculate the final 16-bit checksum value
 * using the intermediate 32-bit value.
 *
 * @param sum Intermediate 32-bit checksum value.
 *
 * @returns Final 16-bit checksum value.
 */

uint16_t ndm_ip_checksum_finish(
		const uint32_t sum) NDM_ATTR_WUR;

/**
 * Calculate the final 16-bit checksum value for the array pointed by @a packet,
 * the size of @a packet_size.
 *
 * @param packet Pointer to array.
 * @param packet_size An array size.
 *
 * @returns Final 16-bit checksum value.
 */

uint16_t ndm_ip_checksum(
		const void *packet,
		const unsigned long packet_size) NDM_ATTR_WUR;

#endif	/* __NDM_IP_CHECKSUM_H__ */

