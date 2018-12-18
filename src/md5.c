#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <ndm/md5.h>
#include <ndm/endian.h>

static inline void revert_bytes_(
		void* buf,
		size_t longs)
{
	if (!ndm_endian_is_le()) {
		unsigned char *p = (unsigned char *) buf;

		do {
			*((uint32_t *) p) =
				((uint32_t) (p[3] << 24)) |
				((uint32_t) (p[2] << 16)) |
				((uint32_t) (p[1] <<  8)) |
				((uint32_t) (p[0] <<  0));
			p += 4;
		} while (--longs);
	}
}

#define F1_(x, y, z) (z ^ (x & (y ^ z)))
#define F2_(x, y, z) F1_(z, x, y)
#define F3_(x, y, z) (x ^ y ^ z)
#define F4_(x, y, z) (y ^ (x | ~z))

#define MD5STEP_(f, w, x, y, z, data, s) \
	(w += f(x, y, z) + data,  w = w << s | w >> (32 - s),  w += x)

static void transform_(
		uint32_t buf[4],
		uint32_t const in[16])
{
	register uint32_t a, b, c, d;

	a = buf[0];
	b = buf[1];
	c = buf[2];
	d = buf[3];

	MD5STEP_(F1_, a, b, c, d, in[ 0] + 0xd76aa478,  7);
	MD5STEP_(F1_, d, a, b, c, in[ 1] + 0xe8c7b756, 12);
	MD5STEP_(F1_, c, d, a, b, in[ 2] + 0x242070db, 17);
	MD5STEP_(F1_, b, c, d, a, in[ 3] + 0xc1bdceee, 22);
	MD5STEP_(F1_, a, b, c, d, in[ 4] + 0xf57c0faf,  7);
	MD5STEP_(F1_, d, a, b, c, in[ 5] + 0x4787c62a, 12);
	MD5STEP_(F1_, c, d, a, b, in[ 6] + 0xa8304613, 17);
	MD5STEP_(F1_, b, c, d, a, in[ 7] + 0xfd469501, 22);
	MD5STEP_(F1_, a, b, c, d, in[ 8] + 0x698098d8,  7);
	MD5STEP_(F1_, d, a, b, c, in[ 9] + 0x8b44f7af, 12);
	MD5STEP_(F1_, c, d, a, b, in[10] + 0xffff5bb1, 17);
	MD5STEP_(F1_, b, c, d, a, in[11] + 0x895cd7be, 22);
	MD5STEP_(F1_, a, b, c, d, in[12] + 0x6b901122,  7);
	MD5STEP_(F1_, d, a, b, c, in[13] + 0xfd987193, 12);
	MD5STEP_(F1_, c, d, a, b, in[14] + 0xa679438e, 17);
	MD5STEP_(F1_, b, c, d, a, in[15] + 0x49b40821, 22);

	MD5STEP_(F2_, a, b, c, d, in[ 1] + 0xf61e2562,  5);
	MD5STEP_(F2_, d, a, b, c, in[ 6] + 0xc040b340,  9);
	MD5STEP_(F2_, c, d, a, b, in[11] + 0x265e5a51, 14);
	MD5STEP_(F2_, b, c, d, a, in[ 0] + 0xe9b6c7aa, 20);
	MD5STEP_(F2_, a, b, c, d, in[ 5] + 0xd62f105d,  5);
	MD5STEP_(F2_, d, a, b, c, in[10] + 0x02441453,  9);
	MD5STEP_(F2_, c, d, a, b, in[15] + 0xd8a1e681, 14);
	MD5STEP_(F2_, b, c, d, a, in[ 4] + 0xe7d3fbc8, 20);
	MD5STEP_(F2_, a, b, c, d, in[ 9] + 0x21e1cde6,  5);
	MD5STEP_(F2_, d, a, b, c, in[14] + 0xc33707d6,  9);
	MD5STEP_(F2_, c, d, a, b, in[ 3] + 0xf4d50d87, 14);
	MD5STEP_(F2_, b, c, d, a, in[ 8] + 0x455a14ed, 20);
	MD5STEP_(F2_, a, b, c, d, in[13] + 0xa9e3e905,  5);
	MD5STEP_(F2_, d, a, b, c, in[ 2] + 0xfcefa3f8,  9);
	MD5STEP_(F2_, c, d, a, b, in[ 7] + 0x676f02d9, 14);
	MD5STEP_(F2_, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

	MD5STEP_(F3_, a, b, c, d, in[ 5] + 0xfffa3942,  4);
	MD5STEP_(F3_, d, a, b, c, in[ 8] + 0x8771f681, 11);
	MD5STEP_(F3_, c, d, a, b, in[11] + 0x6d9d6122, 16);
	MD5STEP_(F3_, b, c, d, a, in[14] + 0xfde5380c, 23);
	MD5STEP_(F3_, a, b, c, d, in[ 1] + 0xa4beea44,  4);
	MD5STEP_(F3_, d, a, b, c, in[ 4] + 0x4bdecfa9, 11);
	MD5STEP_(F3_, c, d, a, b, in[ 7] + 0xf6bb4b60, 16);
	MD5STEP_(F3_, b, c, d, a, in[10] + 0xbebfbc70, 23);
	MD5STEP_(F3_, a, b, c, d, in[13] + 0x289b7ec6,  4);
	MD5STEP_(F3_, d, a, b, c, in[ 0] + 0xeaa127fa, 11);
	MD5STEP_(F3_, c, d, a, b, in[ 3] + 0xd4ef3085, 16);
	MD5STEP_(F3_, b, c, d, a, in[ 6] + 0x04881d05, 23);
	MD5STEP_(F3_, a, b, c, d, in[ 9] + 0xd9d4d039,  4);
	MD5STEP_(F3_, d, a, b, c, in[12] + 0xe6db99e5, 11);
	MD5STEP_(F3_, c, d, a, b, in[15] + 0x1fa27cf8, 16);
	MD5STEP_(F3_, b, c, d, a, in[ 2] + 0xc4ac5665, 23);

	MD5STEP_(F4_, a, b, c, d, in[ 0] + 0xf4292244,  6);
	MD5STEP_(F4_, d, a, b, c, in[ 7] + 0x432aff97, 10);
	MD5STEP_(F4_, c, d, a, b, in[14] + 0xab9423a7, 15);
	MD5STEP_(F4_, b, c, d, a, in[ 5] + 0xfc93a039, 21);
	MD5STEP_(F4_, a, b, c, d, in[12] + 0x655b59c3,  6);
	MD5STEP_(F4_, d, a, b, c, in[ 3] + 0x8f0ccc92, 10);
	MD5STEP_(F4_, c, d, a, b, in[10] + 0xffeff47d, 15);
	MD5STEP_(F4_, b, c, d, a, in[ 1] + 0x85845dd1, 21);
	MD5STEP_(F4_, a, b, c, d, in[ 8] + 0x6fa87e4f,  6);
	MD5STEP_(F4_, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
	MD5STEP_(F4_, c, d, a, b, in[ 6] + 0xa3014314, 15);
	MD5STEP_(F4_, b, c, d, a, in[13] + 0x4e0811a1, 21);
	MD5STEP_(F4_, a, b, c, d, in[ 4] + 0xf7537e82,  6);
	MD5STEP_(F4_, d, a, b, c, in[11] + 0xbd3af235, 10);
	MD5STEP_(F4_, c, d, a, b, in[ 2] + 0x2ad7d2bb, 15);
	MD5STEP_(F4_, b, c, d, a, in[ 9] + 0xeb86d391, 21);

	buf[0] += a;
	buf[1] += b;
	buf[2] += c;
	buf[3] += d;
}

void ndm_md5_init(struct ndm_md5_t *md5)
{
	md5->buf_[0] = 0x67452301;
	md5->buf_[1] = 0xefcdab89;
	md5->buf_[2] = 0x98badcfe;
	md5->buf_[3] = 0x10325476;

	md5->bits_[0] = 0;
	md5->bits_[1] = 0;
}

void ndm_md5_update(
		struct ndm_md5_t *md5,
		const void* const buffer,
		const size_t length)
{
	uint32_t t;
	size_t len = length;
	const unsigned char *buf = (const unsigned char *) buffer;

	t = md5->bits_[0];

	if ((md5->bits_[0] = t + (((uint32_t) len) << 3)) < t) {
		md5->bits_[1]++;
	}

	md5->bits_[1] += (uint32_t) (len >> 29);

	t = (t >> 3) & 0x3f;

	if (t > 0) {
		unsigned char *p = ((unsigned char *) md5->in_) + t;

		t = 64 - t;

		if (len < t) {
			memcpy(p, buf, len);

			return;
		}

		memcpy(p, buf, t);
		revert_bytes_(md5->in_, 16);
		transform_(md5->buf_, md5->in_);
		buf += t;
		len -= t;
	}

	while (len >= 64) {
		memcpy(md5->in_, buf, 64);
		revert_bytes_(md5->in_, 16);
		transform_(md5->buf_, md5->in_);
		buf += 64;
		len -= 64;
	}

	memcpy(md5->in_, buf, len);
}

void ndm_md5_digest(
		struct ndm_md5_t *md5,
		unsigned char digest[NDM_MD5_BINARY_BUFFER_SIZE])
{
	size_t count = (md5->bits_[0] >> 3) & 0x3F;
	unsigned char *p = ((unsigned char *) md5->in_) + count;

	*p++ = 0x80;
	count = 64 - 1 - count;

	if (count < 8) {
		memset(p, 0, count);
		revert_bytes_(md5->in_, 16);
		transform_(md5->buf_, md5->in_);
		memset(md5->in_, 0, 56);
	} else {
		memset(p, 0, count - 8);
	}

	revert_bytes_(md5->in_, 14);

	md5->in_[14] = md5->bits_[0];
	md5->in_[15] = md5->bits_[1];

	transform_(md5->buf_, md5->in_);
	revert_bytes_(((unsigned char *) md5->buf_), 4);
	memcpy(digest, md5->buf_, 16);
	memset(md5, 0, sizeof(*md5));
}

const char* ndm_md5_text_digest(
		struct ndm_md5_t *md5,
		char digest[NDM_MD5_TEXT_BUFFER_SIZE])
{
	uint32_t bdigest[4];

	ndm_md5_digest(md5, (unsigned char *) bdigest);
	snprintf(
		digest, NDM_MD5_TEXT_BUFFER_SIZE,
		"%08" PRIx32 "%08" PRIx32 "%08" PRIx32 "%08" PRIx32,
		(uint32_t) (ndm_endian_htobe32(bdigest[0])),
		(uint32_t) (ndm_endian_htobe32(bdigest[1])),
		(uint32_t) (ndm_endian_htobe32(bdigest[2])),
		(uint32_t) (ndm_endian_htobe32(bdigest[3])));

	return digest;
}

