#ifndef __NDM_CRC32_H__
#define __NDM_CRC32_H__

#include <stdint.h>
#include <stddef.h>

struct ndm_crc32_t
{
	uint32_t digest_;
};

#define NDM_CRC32_INITIALIZER										\
	{																\
		.digest_ = UINT32_MAX										\
	}

static inline void
ndm_crc32_init(
		struct ndm_crc32_t *crc32)
{
	crc32->digest_ = UINT32_MAX;
}

void ndm_crc32_update(
		struct ndm_crc32_t *crc32,
		const void *const buffer,
		const size_t length);

static inline uint32_t
ndm_crc32_digest(
		struct ndm_crc32_t *crc32)
{
	return crc32->digest_ ^ UINT32_MAX;
}

#endif /* __NDM_CRC32_H__ */

