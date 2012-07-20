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
		i -= 2;
	}

	if (i == 1) {
		s += (*(uint8_t *) words);
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

