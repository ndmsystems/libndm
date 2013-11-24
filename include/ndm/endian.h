#ifndef __NDM_ENDIAN_H__
#define __NDM_ENDIAN_H__

#include <stdint.h>
#include <stdbool.h>

static inline bool ndm_endian_is_le()
{
	/* @c TEST_ should not be static to allow compile-time optimization. */
	const uint16_t TEST_ = 0xff00;

	return *((const uint8_t *) (&TEST_)) == 0;
}

static inline uint16_t ndm_endian_letoh16(const uint16_t x)
{
	if (ndm_endian_is_le()) {
		return x;
	}

	const uint8_t *p = (const uint8_t *) &x;

	return (uint16_t)
		(((uint16_t) *(p + 0)) << 0  |
		 ((uint16_t) *(p + 1)) << 8 );
}

static inline uint16_t ndm_endian_htole16(const uint16_t x)
{
	return ndm_endian_letoh16(x);
}

static inline uint32_t ndm_endian_letoh32(const uint32_t x)
{
	if (ndm_endian_is_le()) {
		return x;
	}

	const uint8_t *p = (const uint8_t*) &x;

	return (uint32_t)
		(((uint32_t) *(p + 0)) << 0	 |
		 ((uint32_t) *(p + 1)) << 8	 |
		 ((uint32_t) *(p + 2)) << 16 |
		 ((uint32_t) *(p + 3)) << 24);
}

static inline uint32_t ndm_endian_htole32(const uint32_t x)
{
	return ndm_endian_letoh32(x);
}

static inline uint64_t ndm_endian_letoh64(const uint64_t x)
{
	if (ndm_endian_is_le()) {
		return x;
	}

	const uint8_t *p = (const uint8_t *) &x;

	return (uint64_t)
		(((uint64_t) *(p + 0)) << 0	 |
		 ((uint64_t) *(p + 1)) << 8	 |
		 ((uint64_t) *(p + 2)) << 16 |
		 ((uint64_t) *(p + 3)) << 24 |
		 ((uint64_t) *(p + 4)) << 32 |
		 ((uint64_t) *(p + 5)) << 40 |
		 ((uint64_t) *(p + 6)) << 48 |
		 ((uint64_t) *(p + 7)) << 56);
}

static inline uint64_t ndm_endian_htole64(const uint64_t x)
{
	return ndm_endian_letoh64(x);
}

static inline uint16_t ndm_endian_betoh16(const uint16_t x)
{
	if (!ndm_endian_is_le()) {
		return x;
	}

	const uint8_t *p = (const uint8_t *) &x;

	return (uint16_t)
		(((uint16_t) *(p + 1)) << 0  |
		 ((uint16_t) *(p + 0)) << 8 );
}

static inline uint16_t ndm_endian_htobe16(const uint16_t x)
{
	return ndm_endian_betoh16(x);
}

static inline uint32_t ndm_endian_betoh32(const uint32_t x)
{
	if (!ndm_endian_is_le()) {
		return x;
	}

	const uint8_t *p = (const uint8_t *) &x;

	return (uint32_t)
		(((uint32_t) *(p + 3)) << 0  |
		 ((uint32_t) *(p + 2)) << 8  |
		 ((uint32_t) *(p + 1)) << 16 |
		 ((uint32_t) *(p + 0)) << 24);
}

static inline uint32_t ndm_endian_htobe32(const uint32_t x)
{
	return ndm_endian_betoh32(x);
}

static inline uint64_t ndm_endian_betoh64(const uint64_t x)
{
	if (!ndm_endian_is_le()) {
		return x;
	}

	const uint8_t *p = (const uint8_t *) &x;

	return (uint64_t)
		(((uint64_t) *(p + 7)) << 0  |
		 ((uint64_t) *(p + 6)) << 8  |
		 ((uint64_t) *(p + 5)) << 16 |
		 ((uint64_t) *(p + 4)) << 24 |
		 ((uint64_t) *(p + 3)) << 32 |
		 ((uint64_t) *(p + 2)) << 40 |
		 ((uint64_t) *(p + 1)) << 48 |
		 ((uint64_t) *(p + 0)) << 56);
}

static inline uint64_t ndm_endian_htobe64(const uint64_t x)
{
	return ndm_endian_betoh64(x);
}

#endif	/* __NDM_ENDIAN_H__ */

