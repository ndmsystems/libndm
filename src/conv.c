#include <ctype.h>
#include <errno.h>
#include <stddef.h>
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <sys/types.h>
#include <ndm/conv.h>
#include <ndm/macro.h>
#include <ndm/endian.h>

#define NDM_CONV_INVALID_					-1

#define NDM_CONV_SHIFT_FLAGS_				0
#define NDM_CONV_SHIFT_FROM_				8
#define NDM_CONV_SHIFT_TO_					16

#define NDM_CONV_MASK_FLAGS_				0xff
#define NDM_CONV_MASK_FROM_					0xff
#define NDM_CONV_MASK_TO_					0xff

#define NDM_CONV_FLAGS_(c)					\
	(((c) >> NDM_CONV_SHIFT_FLAGS_) & NDM_CONV_MASK_FLAGS_)

#define NDM_CONV_FROM_(c)					\
	(((c) >> NDM_CONV_SHIFT_FROM_) & NDM_CONV_MASK_FROM_)

#define NDM_CONV_TO_(c)						\
	(((c) >> NDM_CONV_SHIFT_TO_) & NDM_CONV_MASK_TO_)

#define NDM_CONV_UNICODE_MAX_				0x10ffff

#define NDM_CONV_UNICODE_SUR_HIGH_START_	0xd800
#define NDM_CONV_UNICODE_SUR_HIGH_END_		0xdbff

#define NDM_CONV_UNICODE_SUR_LOW_START_		0xdc00
#define NDM_CONV_UNICODE_SUR_LOW_END_		0xdfff

#define NDM_CONV_UNICODE_REPLACEMENT_CHAR_	'?'		/* Usually 0xfffd. */

#define NDM_CONV_REPLACEMENT_CHAR_			'?'

/**
 * Should be called with @a in_bytes > 0.
 * @c Decode returns:
 *      > 0, a decoded byte count;
 *      = 0, an input byte sequence is too short (not enough bytes);
 *      < 0, a negative offset of an illegal byte sequence.
 **/

typedef long (*decode_func_t_)(
		const uint8_t *const in,
		const size_t in_bytes,
		uint32_t *cp,
		const unsigned long flags);

/**
 * @a cp is always a legal UTF-32 code point.
 * @c Encode returns
 *      > 0, an encoded byte count even there is no free space
 *           in an output buffer;
 *      = 0, an input code point can not be encoded in this charset.
 **/

typedef long (*encode_func_t_)(
		uint32_t cp,
		uint8_t *out,
		const size_t out_bytes,
		const unsigned long flags);

/* ISO-8859-1 */

static long decode_iso88591_(
		const uint8_t *const in,
		const size_t in_bytes,
		uint32_t *cp,
		const unsigned long flags)
{
	assert (in_bytes > 0);

	const uint8_t b0 = *in;

	if (b0 >= 0x80) {
		/* Invalid character. */

		if (!(flags & NDM_CONV_FLAGS_ENCODE_ILLEGAL)) {
			/* Offset of an illegal byte. */

			return -1;
		}

		/**
		 * Return replacement character for an invalid symbol.
		 **/

		*cp = NDM_CONV_REPLACEMENT_CHAR_;
	} else {
		*cp = b0;
	}

	return 1;
}

static long encode_iso88591_(
		uint32_t cp,
		uint8_t *out,
		const size_t out_bytes,
		const unsigned long flags)
{
	uint8_t b0 = NDM_CONV_REPLACEMENT_CHAR_;

	if (cp >= 0x80) {
		if (!(flags & NDM_CONV_FLAGS_ENCODE_NON_MAPPED)) {
			return 0;
		}
	} else {
		b0 = (uint8_t) cp;
	}

	if (out_bytes > 0) {
		/* Place only if there is free space. */

		*out = b0;
	}

	return 1;
}

/* UTF common */

static inline long utf_truncated_(
		const size_t in_bytes,
		uint32_t *cp,
		const unsigned long flags)
{
	/* Not enough bytes to decode. */

	if (flags & NDM_CONV_FLAGS_ENCODE_TRUNCATED) {
		/**
		 * Return a replacement character and stop decoding.
		 **/

		*cp = NDM_CONV_UNICODE_REPLACEMENT_CHAR_;

		return (long) in_bytes;
	}

	return 0;
}

/* UTF-8 */

static inline bool utf8_is_legal_start_(const uint8_t b0)
{
	return (b0 < 0x80) || (0xc2 <= b0 && b0 <= 0xf4);
}

static long utf8_find_legal_start_(
		const long i,
		const uint8_t *const in,
		const size_t in_bytes,
		uint32_t *cp,
		const unsigned long flags)
{
	if (!(flags & NDM_CONV_FLAGS_ENCODE_ILLEGAL)) {
		/* Return a negative index of an invalid byte. */

		return -(i + 1);
	}

	/**
	 * Return a replacement character and try to find a legal start byte.
	 **/

	*cp = NDM_CONV_UNICODE_REPLACEMENT_CHAR_;

	size_t k = (size_t) i;

	while (k < in_bytes && !utf8_is_legal_start_(in[k])) {
		++k;
	}

	return (long) k;
}

static long decode_utf8_(
		const uint8_t *const in,
		const size_t in_bytes,
		uint32_t *cp,
		const unsigned long flags)
{
	assert (in_bytes > 0);

	const uint8_t b0 = *in;

	/**
	 * See RFC 3629, "4. Syntax of UTF-8 Byte Sequences" and
	 * http://www.unicode.org/versions/corrigendum1.html
	 * "Table 3.1B. Legal UTF-8 Byte Sequences".
	 **/

	if (b0 < 0x80) {
		/**
		 * 1 byte sequence: 0xxxxxxx.
		 * 0x00..0x7f
		 **/

		*cp = b0;

		return 1;
	} else
	if (b0 < 0xc2) {
		/**
		 * 1 byte illegal sequence.
		 * 0x80..0xc1
		 * 10xxxxxx (continuation bytes),
		 * 11000000, 11000001 (invalid bytes).
		 **/

		return utf8_find_legal_start_(0, in, in_bytes, cp, flags);
	} else
	if (b0 < 0xe0) {
		/**
		 * 2 byte sequence: 110xxxxx.
		 * 0xc2..0xdf 0x80..0xbf
		 **/

		if (in_bytes < 2) {
			return utf_truncated_(in_bytes, cp, flags);
		}

		const uint8_t b1 = in[1];

		if ((b1 & 0xc0) != 0x80) {
			return utf8_find_legal_start_(1, in, in_bytes, cp, flags);
		}

		*cp =
			(((uint32_t) (b0 & 0x1f)) << 6) |
			(((uint32_t) (b1 & 0x3f)) << 0);

		assert (*cp >= 0x80);

		return 2;
	} else
	if (b0 < 0xf0) {
		/**
		 * 3 byte sequence: 1110xxxx, 10xxxxxx, 10xxxxxx.
		 * 0xe0..0xe0 0xa0..0xbf 0x80..0xbf
		 * 0xe1..0xef 0x80..0xbf 0x80..0xbf
		 **/

		if (in_bytes < 3) {
			return utf_truncated_(in_bytes, cp, flags);
		}

		const uint8_t b1 = in[1];
		const uint8_t b2 = in[2];

		if (b0 == 0xe0 && b1 < 0xa0) {
			return utf8_find_legal_start_(1, in, in_bytes, cp, flags);
		}

		if ((b1 & 0xc0) != 0x80) {
			return utf8_find_legal_start_(1, in, in_bytes, cp, flags);
		}

		if ((b2 & 0xc0) != 0x80) {
			return utf8_find_legal_start_(2, in, in_bytes, cp, flags);
		}

		*cp =
			(((uint32_t) (b0 & 0x0f)) << 12) |
			(((uint32_t) (b1 & 0x3f)) <<  6) |
			(((uint32_t) (b2 & 0x3f)) <<  0);

		assert (*cp >= 0x800);

		return 3;
	} else
	if (b0 < 0xf5) {
		/**
		 * 4 byte sequence: 11110 xxx, 10xxxxxx, 10xxxxxx, 10xxxxxx.
		 * 0xf0..0xf0 0x90..0xbf 0x80..0xbf 0x80..0xbf
		 * 0xf1..0xf3 0x80..0xbf 0x80..0xbf 0x80..0xbf
		 * 0xf4..0xf4 0x80..0x8f 0x80..0xbf 0x80..0xbf
		 **/

		if (in_bytes < 4) {
			return utf_truncated_(in_bytes, cp, flags);
		}

		const uint8_t b1 = in[1];
		const uint8_t b2 = in[2];
		const uint8_t b3 = in[3];

		if ((b0 == 0xf0 && b1 <  0x90) ||
			(b0 == 0xf4 && b1 >= 0x90))
		{
			return utf8_find_legal_start_(1, in, in_bytes, cp, flags);
		}

		if ((b1 & 0xc0) != 0x80) {
			return utf8_find_legal_start_(1, in, in_bytes, cp, flags);
		}

		if ((b2 & 0xc0) != 0x80) {
			return utf8_find_legal_start_(2, in, in_bytes, cp, flags);
		}

		if ((b3 & 0xc0) != 0x80) {
			return utf8_find_legal_start_(3, in, in_bytes, cp, flags);
		}

		*cp =
			(((uint32_t) (b0 & 0x07)) << 18) |
			(((uint32_t) (b1 & 0x3f)) << 12) |
			(((uint32_t) (b2 & 0x3f)) <<  6) |
			(((uint32_t) (b3 & 0x3f)) <<  0);

		assert (*cp >= 0x10000);

		return 4;
	}

	/**
	 * [11110 101 -- 11110 111], [0xf5 -- 0xf7] +
	 * [11111 000 -- 11111 111], [0xf8 -- 0xff].
	 * are start bytes of sequences that could only encode numbers
	 * larger or equal than UNICODE_MAX_.
	 **/

	return utf8_find_legal_start_(0, in, in_bytes, cp, flags);
}

static long encode_utf8_(
		uint32_t cp,
		uint8_t *out,
		const size_t out_bytes,
		const unsigned long flags)
{
	/* @c cp is always legal here. */

	long size =
		(cp < 0x80)    ? 1 :
		(cp < 0x800)   ? 2 :
		(cp < 0x10000) ? 3 : 4;

	if (out_bytes < (size_t) size) {
		/**
		 * An output buffer is too small to really encode a code point,
		 * return a greater than an @a out_bytes value to indicate
		 * an overflow case.
		 **/

		return size;
	}

	switch (size) {
		case 4: {
			out[3] = (uint8_t) (0x80 | (cp & 0x3f));
			cp = (cp >> 6) | 0x10000;
		}

		case 3: {
			out[2] = (uint8_t) (0x80 | (cp & 0x3f));
			cp = (cp >> 6) | 0x800;
		}

		case 2: {
			out[1] = (uint8_t) (0x80 | (cp & 0x3f));
			cp = (cp >> 6) | 0xc0;
		}

		case 1: {
			out[0] = (uint8_t) (cp);
		}

		default: {
			// to suppress a compiler warning
		}
	}

	return size;
}

/* UTF-16 */

#define NDM_CONV_CHARSET_UTF16_CONVERTERS_(x)							\
static inline bool utf16_##x##_is_legal_start_(const uint16_t w0)		\
{																		\
	return																\
		w0 < NDM_CONV_UNICODE_SUR_LOW_START_ ||							\
		w0 > NDM_CONV_UNICODE_SUR_LOW_END_;								\
}																		\
																		\
static long utf16_##x##_find_legal_start_(								\
		const long i,													\
		const uint8_t *const in,										\
		const size_t in_bytes,											\
		uint32_t *cp,													\
		const unsigned long flags)										\
{																		\
	if (!(flags & NDM_CONV_FLAGS_ENCODE_ILLEGAL) ) {					\
		/* Return a negative index of an invalid byte. */				\
																		\
		return -(i + 1);												\
	}																	\
																		\
	/* Return a replacement character and	*/							\
	/* try to find a legal start byte.		*/							\
																		\
	*cp = NDM_CONV_UNICODE_REPLACEMENT_CHAR_;							\
																		\
	size_t k = (size_t) i;												\
																		\
	while(																\
		k + 2 <= in_bytes &&											\
		!utf16_##x##_is_legal_start_(ndm_endian_##x##toh16(				\
			*((uint16_t*) (in + k)))))									\
	{																	\
		k += 2;															\
	}																	\
																		\
	/* Illegal byte sequence and (possibly) truncated data. */			\
																		\
	return (long) (k > in_bytes ? in_bytes : k);						\
}																		\
																		\
static long decode_utf16_##x##_(										\
		const uint8_t *const in,										\
		const size_t in_bytes,											\
		uint32_t *cp,													\
		const unsigned long flags)										\
{																		\
	if (in_bytes < 2) {													\
		return utf_truncated_(in_bytes, cp, flags);						\
	}																	\
																		\
	const uint16_t w0 = ndm_endian_##x##toh16(*((uint16_t*) in));		\
																		\
	if (w0 < NDM_CONV_UNICODE_SUR_HIGH_START_ ||						\
		w0 > NDM_CONV_UNICODE_SUR_LOW_END_ )							\
	{																	\
		/* It is not a surrogate pair. */								\
		*cp = w0;														\
																		\
		return 2;														\
	}																	\
																		\
	if (w0 >= NDM_CONV_UNICODE_SUR_LOW_START_) {						\
		/* Illegal first UTF-16 code. */								\
																		\
		return utf16_##x##_find_legal_start_(							\
			0, in, in_bytes, cp, flags);								\
	}																	\
																		\
	/* w0 is first surrogate code. */									\
																		\
	if (in_bytes < 4) {													\
		return utf_truncated_(in_bytes, cp, flags);						\
	}																	\
																		\
	const uint16_t w1 = ndm_endian_##x##toh16(*((uint16_t*) (in + 2)));	\
																		\
	if (w1 < NDM_CONV_UNICODE_SUR_LOW_START_ ||							\
		w1 > NDM_CONV_UNICODE_SUR_LOW_END_ )							\
	{																	\
		/* Invalid second surrogate code. */							\
																		\
		return utf16_##x##_find_legal_start_(							\
			2, in, in_bytes, cp, flags);								\
	}																	\
																		\
	*cp =																\
		((((uint32_t) (w0 & 0x3ff)) << 10)  |							\
		 (((uint32_t) (w1 & 0x3ff)) <<  0)) + 0x10000;					\
																		\
	return 4;															\
}																		\
																		\
static long encode_utf16_##x##_(										\
		uint32_t cp,													\
		uint8_t *out,													\
		const size_t out_bytes,											\
		const unsigned long flags)										\
{																		\
	/* @a cp is always valid here. */									\
																		\
	if (cp < 0x10000) {													\
		if (out_bytes >= 2) {											\
			*((uint16_t*) out) = ndm_endian_hto##x##16((uint16_t) cp);	\
		}																\
																		\
		return 2;														\
	}																	\
																		\
	if (out_bytes >= 4) {												\
		cp -= 0x10000;													\
																		\
		*((uint16_t*) (out + 0)) =										\
			ndm_endian_hto##x##16((uint16_t) (							\
				((cp >> 10) & 0x3ff) |									\
				NDM_CONV_UNICODE_SUR_HIGH_START_));						\
		*((uint16_t*) (out + 2)) =										\
			ndm_endian_hto##x##16((uint16_t) (							\
				((cp >>  0) & 0x3ff) |									\
				NDM_CONV_UNICODE_SUR_LOW_START_));						\
	}																	\
																		\
	return 4;															\
}

NDM_CONV_CHARSET_UTF16_CONVERTERS_(be)
NDM_CONV_CHARSET_UTF16_CONVERTERS_(le)

/* UTF-32 */

#define NDM_CONV_CHARSET_UTF32_CONVERTERS_(x)							\
static inline bool utf32_##x##_is_legal_start_(const uint32_t cp)		\
{																		\
	return																\
		 cp <= NDM_CONV_UNICODE_MAX_ &&									\
		(cp <  NDM_CONV_UNICODE_SUR_HIGH_START_ ||						\
		 cp >  NDM_CONV_UNICODE_SUR_LOW_END_);							\
}																		\
																		\
static long utf32_##x##_find_legal_start_(								\
		const long i,													\
		const uint8_t *const in,										\
		const size_t in_bytes,											\
		uint32_t *cp,													\
		const unsigned long flags)										\
{																		\
	if (!(flags & NDM_CONV_FLAGS_ENCODE_ILLEGAL)) {						\
		/* Return a negative index of an invalid byte. */				\
																		\
		return -(i + 1);												\
	}																	\
																		\
	/* Return a replacement character and	*/							\
	/* try to find a legal start code point.*/							\
																		\
	*cp = NDM_CONV_UNICODE_REPLACEMENT_CHAR_;							\
																		\
	size_t k = (size_t) i;												\
																		\
	while(																\
		k + 4 <= in_bytes &&											\
		!utf32_##x##_is_legal_start_(ndm_endian_##x##toh32(				\
			*((uint32_t*) (in + k)))))									\
	{																	\
		k += 4;															\
	}																	\
																		\
	/* Illegal byte sequence and (possibly) truncated data.	*/			\
																		\
	return (long) (k > in_bytes ? in_bytes : k);						\
}																		\
																		\
static long decode_utf32_##x##_(										\
		const uint8_t *const in,										\
		const size_t in_bytes,											\
		uint32_t *cp,													\
		const unsigned long flags)										\
{																		\
	if (in_bytes < 4) {													\
		return utf_truncated_(in_bytes, cp, flags);						\
	}																	\
																		\
	*cp = ndm_endian_##x##toh32(*((uint32_t*) in));						\
																		\
	if (!utf32_##x##_is_legal_start_(*cp)) {							\
		return utf32_##x##_find_legal_start_(							\
			0, in, in_bytes, cp, flags);								\
	}																	\
																		\
	return 4;															\
}																		\
																		\
static long encode_utf32_##x##_(										\
		uint32_t cp,													\
		uint8_t *out,													\
		const size_t out_bytes,											\
		const unsigned long flags)										\
{																		\
	/* @a cp is always valid here. */									\
																		\
	if (out_bytes >= 4) {												\
		*((uint32_t*) out) = ndm_endian_hto##x##32(cp);					\
	}																	\
																		\
	return 4;															\
}

NDM_CONV_CHARSET_UTF32_CONVERTERS_(be)
NDM_CONV_CHARSET_UTF32_CONVERTERS_(le)

static const struct ndm_conv_pair_t_ {
	const char *const name_;	//!< allows lower alphanumeric chars only
	decode_func_t_ decode_;
	encode_func_t_ encode_;
} NDM_CONV_PAIRS_[] = {
	{"iso88591", decode_iso88591_,	encode_iso88591_},
	{"utf8",	 decode_utf8_,		encode_utf8_	},
	{"utf16le",  decode_utf16_le_,	encode_utf16_le_},
	{"utf16be",  decode_utf16_be_,	encode_utf16_be_},
	{"utf32le",  decode_utf32_le_,	encode_utf32_le_},
	{"utf32be",  decode_utf32_be_,	encode_utf32_be_}
};

static ssize_t ndm_conv_find_(const char *const name)
{
	ssize_t i = 0;

	while (i < NDM_ARRAY_SIZE(NDM_CONV_PAIRS_)) {
		const char *p = name;
		const char *q = NDM_CONV_PAIRS_[i].name_;

		while (*q != '\0' && *p != '\0') {
			if (!isalnum(*p)) {
				++p;
			} else
			if (tolower(*p) == *q) {
				++p;
				++q;
			} else {
				/* Different alphanumeric characters found. */
				break;
			}
		}

		if (*p == *q) {
			return i;
		}

		++i;
	}

	return -1;
}

ndm_conv_t ndm_conv_open_ex(
		const char *const to,
		const char *const from,
		const enum ndm_conv_flags_t flags)
{
	const ssize_t to_index = ndm_conv_find_(to);
	const ssize_t from_index = ndm_conv_find_(from);

	if (from_index < 0 || to_index < 0) {
		errno = EINVAL;

		return NDM_CONV_INVALID_;
	}

	if ((flags & (unsigned long)
			~(NDM_CONV_FLAGS_ENCODE_STRICTLY  |
			  NDM_CONV_FLAGS_ENCODE_TRUNCATED |
			  NDM_CONV_FLAGS_ENCODE_ILLEGAL   |
			  NDM_CONV_FLAGS_ENCODE_NON_MAPPED)))
	{
		errno = EINVAL;

		return NDM_CONV_INVALID_;
	}

	return
		(ndm_conv_t) ((from_index << NDM_CONV_SHIFT_FROM_)) |
		(ndm_conv_t) ((to_index << NDM_CONV_SHIFT_TO_)) |
		(ndm_conv_t) ((flags << NDM_CONV_SHIFT_FLAGS_));
}

ndm_conv_t ndm_conv_open(
		const char *const to,
		const char *const from)
{
	return ndm_conv_open_ex(to, from, NDM_CONV_FLAGS_ENCODE_STRICTLY);
}

size_t ndm_conv(
		const ndm_conv_t cd,
		const char **inp,
		size_t *in_bytes_left,
		char **outp,
		size_t *out_bytes_left)
{
	if (inp == NULL || *inp == NULL) {
		/* Reset @c conv to its initial shift state
		 * for state-dependent encodings, @c outp value ignored.
		 * Here the state-dependent encondings do not supported,
		 * so zero returned. */

		return 0;
	}

	assert (outp != NULL && *outp != NULL);

	const unsigned long flags = (unsigned long) NDM_CONV_FLAGS_(cd);
	decode_func_t_ decode =	NDM_CONV_PAIRS_[NDM_CONV_FROM_(cd)].decode_;
	encode_func_t_ encode =	NDM_CONV_PAIRS_[NDM_CONV_TO_(cd)].encode_;

	while (*in_bytes_left != 0 && out_bytes_left != 0) {
		uint32_t cp;
		const long d = decode(
			(const uint8_t *) *inp, *in_bytes_left, &cp, flags);

		if (d > 0) {
			const long e = encode(
				cp, (uint8_t *) *outp, *out_bytes_left, flags);

			if (e > 0) {
				*inp += d;
				*in_bytes_left -= (size_t) d;
				*outp += e;
				*out_bytes_left -= (size_t) e;
			} else {
				errno = EINVAL;

				return (size_t) -1;
			}
		} else
		if (d == 0) {
			errno = E2BIG;

			return (size_t) -1;
		} else {
			errno = EILSEQ;

			return (size_t) -1;
		}
	}

	return 0;
}

int ndm_conv_close(
		ndm_conv_t cd)
{
	return 0;
}

