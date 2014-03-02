#ifndef __NDM_MD5_H__
#define __NDM_MD5_H__

#include <stdint.h>

#define NDM_MD5_TEXT_BUFFER_SIZE									33
#define NDM_MD5_BINARY_BUFFER_SIZE									16

struct ndm_md5_t
{
	uint32_t buf_[4];
	uint32_t bits_[2];
	uint32_t in_[16];
};

#define NDM_MD5_INITIALIZER											\
	{																\
		.buf_ = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476},	\
		.bits_ = {0, 0}												\
	}

void ndm_md5_init(
		struct ndm_md5_t *md5);

void ndm_md5_update(
		struct ndm_md5_t *md5,
		const void* const buffer,
		const size_t length);

void ndm_md5_digest(
		struct ndm_md5_t *md5,
		unsigned char digest[NDM_MD5_BINARY_BUFFER_SIZE]);

const char* ndm_md5_text_digest(
		struct ndm_md5_t *md5,
		char text[NDM_MD5_TEXT_BUFFER_SIZE]);

#endif /* __NDM_MD5_H__ */

