#include <ndm/endian.h>
#include <ndm/ip_checksum.h>

uint32_t ndm_ip_checksum_get(
		const void *data,
		const unsigned long octet_count)
{
	uint32_t s = 0;
	uint16_t *words = (uint16_t *) data;
	unsigned long i = octet_count;

	while (i > 1) {
		s += *words++;

		if (s & 0x80000000) {
			s = (s & 0x0000ffff) + ((s >> 16) & 0x0000fffff);
		}

		i -= 2;
	}

	if (i == 1) {
		const uint16_t b = *((uint8_t *) words);

		s += ndm_endian_htobe16((uint16_t) (b << 8));
	}

	return s;
}

uint16_t ndm_ip_checksum_finish(const uint32_t sum)
{
	uint32_t s = sum;

	while ((s & 0xffff0000) != 0) {
		s = (s & 0x0000ffff) + ((s >> 16) & 0x0000fffff);
	}

	return (uint16_t) ~s;
}

uint16_t ndm_ip_checksum(
		const void *packet,
		const unsigned long packet_size)
{
	return ndm_ip_checksum_finish(
		ndm_ip_checksum_get(packet, packet_size));
}

