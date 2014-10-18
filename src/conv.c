/**
 * http://docs.oracle.com/cd/E38689_01/pt853pbr0/eng/pt/tgbl/ ->
 *	concept_UnderstandingCharacterSets-0769d6.html
 *
 * "Non-Unicode Character Sets
 *
 * Single-byte character sets (SBCSs).
 * Double-byte character sets (DBCSs).
 *
 * Note: For the sake of terminology, some systems, such as Microsoft
 * Windows, refer to two types of character sets: Unicode and ANSI. ANSI,
 * in this context, refers to the American National Standards Institute,
 * which maintains equivalent standards for many national and international
 * standard character sets. Informally, ANSI character sets refer to
 * non-Unicode character sets, which can be any international, national,
 * or vendor standard character set, such as those that are discussed at
 * the beginning of this topic.
 *
 * Single-Byte Character Sets
 *
 * Most character sets use one byte to represent each character and are
 * therefore known as SBCSs. These character sets are relatively simple and
 * can represent up to 255 unique characters. Examples of SBCSs are
 * ISO 8859-1 (Latin1), ISO 8859-2 (Latin2), Microsoft CP1252 (similar to
 * Latin1, but vendor specific), and IBM CCSID 37.
 *
 * Double-Byte Character Sets
 *
 * DBCSs use one or two bytes to represent each character and are typically
 * used for writing ideographic scripts, such as Japanese, Chinese, and
 * Korean. Most DBCSs allow a mix of one-byte and two-byte characters,
 * so you cannot assume an even-string byte length. Encoding with a mix
 * of one- and two-byte characters is also known as variable-width encoding,
 * and such a character set is sometimes referred to as a multi-byte
 * character set (MBCS).
 *
 * There are two types of DBCSs:
 *  - Nonshifting
 *  - Shifting
 *
 * The difference between these types of DBCSs is in the way in which
 * the system determines whether a particular byte represents one character
 * or is part of a two-byte character.
 *
 * Nonshifting DBCSs
 *
 * Nonshifting DBCSs use ranges of codepoints, specified by the character
 * set definition, to determine whether a particular byte represents one
 * character or is part of a two-byte character.
 *
 * In nonshifting DBCSs, the two bytes that are used to form a character
 * are called lead bytes and trail bytes. The lead byte is the first
 * in a two-byte character, and the trail byte is the last. Nonshifting
 * DBCSs differentiate single-byte characters from double-byte characters
 * by the numerical value of the lead byte. For example, in the Japanese
 * Shift-JIS encoding, if a byte is in the range 0x81-0x9F or 0xE0-0xFC,
 * then it is a lead byte and must be paired with the following byte
 * to form a complete character.
 *
 * The most popular client-side Japanese code page, Shift-JIS, uses this
 * lead byte/trail byte encoding scheme, as do most Microsoft Windows and
 * Unix/Linux ASCII-based double-byte character sets that represent Chinese,
 * Japanese, and Korean characters. Contrary to its name, Shift-JIS
 * is a nonshifting double-byte character set.
 *
 * Shifting DBCSs
 *
 * A shifting DBCS is another double-byte encoding scheme in use that
 * doesn't use the lead byte and trail byte concept. The IBM DB2 UDB for
 * OS/390 and z/OS EBCDIC-based Japanese, Chinese, and Korean character
 * sets use this shifting encoding scheme.
 *
 * Instead of reserving a range of bytes as lead bytes, shifting DBCSs
 * always keep track of whether they are in double-byte or single-byte mode.
 * In double-byte mode, every two bytes form a character. In single-byte
 * mode, every byte is a character in itself. To track what mode
 * the character set is in, the system uses shifting characters.
 * By default, the character set is expected as single-byte data.
 * As soon as a double-byte character needs to be represented, a shift-in
 * byte is added to the string. From this point on, all characters
 * are expected to be two bytes. This continues until a shift-out byte
 * is detected, which indicates that the character set should go back
 * to single-byte per characters.
 *
 * This scheme, while more complex than the lead byte and trail byte scheme,
 * provides greater performance, because the system always knows how many
 * bytes should be in any particular character. Unfortunately, it also
 * increases the length of the string. For example, a character string
 * that comprises a mixture of single-byte and double-byte characters
 * could require more space to store in a shifting character set because
 * you need to include the shift-in and shift-out bytes, as well as the
 * data itself."
 **/

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

#ifdef LIBNDM_SBCS_SUPPORT

#include "./charset/8859_1.h"
#include "./charset/8859_2.h"
#include "./charset/8859_3.h"
#include "./charset/8859_4.h"
#include "./charset/8859_5.h"
#include "./charset/8859_6.h"
#include "./charset/8859_7.h"
#include "./charset/8859_8.h"
#include "./charset/8859_9.h"
#include "./charset/8859_10.h"
#include "./charset/8859_11.h"
#include "./charset/8859_13.h"
#include "./charset/8859_14.h"
#include "./charset/8859_15.h"
#include "./charset/8859_16.h"

#include "./charset/cp037.h"
#include "./charset/cp424.h"
#include "./charset/cp437.h"
#include "./charset/cp500.h"
#include "./charset/cp737.h"
#include "./charset/cp775.h"
#include "./charset/cp850.h"
#include "./charset/cp852.h"
#include "./charset/cp855.h"
#include "./charset/cp856.h"
#include "./charset/cp857.h"
#include "./charset/cp860.h"
#include "./charset/cp861.h"
#include "./charset/cp862.h"
#include "./charset/cp863.h"
#include "./charset/cp864.h"
#include "./charset/cp865.h"
#include "./charset/cp866.h"
#include "./charset/cp869.h"
#include "./charset/cp874.h"
#include "./charset/cp1026.h"
#include "./charset/cp1250.h"
#include "./charset/cp1251.h"
#include "./charset/cp1252.h"
#include "./charset/cp1253.h"
#include "./charset/cp1254.h"
#include "./charset/cp1255.h"
#include "./charset/cp1256.h"
#include "./charset/cp1257.h"
#include "./charset/cp1258.h"

#include "./charset/koi8_r.h"
#include "./charset/koi8_u.h"

#include "./charset/kz1048.h"

#include "./charset/nextstep.h"

#include "./charset/celtic.h"
#include "./charset/centeuro.h"
#include "./charset/croatian.h"
#include "./charset/cyrillic.h"
#include "./charset/gaelic.h"
#include "./charset/greek.h"
#include "./charset/iceland.h"
#include "./charset/inuit.h"
#include "./charset/roman.h"
#include "./charset/romanian.h"
#include "./charset/turkish.h"

#endif	/* LIBNDM_SBCS_SUPPORT */

#define NDM_CONV_INVALID_					-1

#define NDM_CONV_SHIFT_FROM_				8
#define NDM_CONV_SHIFT_TO_					16

#define NDM_CONV_MASK_FROM_					0xff
#define NDM_CONV_MASK_TO_					0xff

#define NDM_CONV_FROM_(c)					\
	(((c) >> NDM_CONV_SHIFT_FROM_) & NDM_CONV_MASK_FROM_)

#define NDM_CONV_TO_(c)						\
	(((c) >> NDM_CONV_SHIFT_TO_) & NDM_CONV_MASK_TO_)

#define NDM_CONV_UNICODE_MAX_				0x10ffff

#define NDM_CONV_UNICODE_SUR_HIGH_START_	0xd800
#define NDM_CONV_UNICODE_SUR_HIGH_END_		0xdbff

#define NDM_CONV_UNICODE_SUR_LOW_START_		0xdc00
#define NDM_CONV_UNICODE_SUR_LOW_END_		0xdfff

/* An invalid unicode code point placeholder. */
#define INV_C_								NDM_CONV_UNICODE_SUR_LOW_END_

/* A non-unicode replacement character. */
#define REPNC_								'?'
#define REPNC_SIZE_							1

/**
 * Should be called with @a in_bytes > 0.
 * @c Decode returns:
 *      > 0, a decoded byte count;
 *      = 0, an input byte sequence is too short (not enough bytes);
 *      < 0, an illegal byte sequence encountered, a negative offset
 *           increased by one of an invalid byte returned.
 **/

typedef long (*to_uni_func_t_)(
		const uint8_t *const in,
		const size_t in_bytes,
		uint32_t *cp);

/**
 * @a cp is always a legal UTF-32 code point.
 * @c Encode returns
 *      > 0, an encoded byte count;
 *      = 0, an input code point can not be encoded in this charset;
 *      < 0, there is no free space in an output buffer, a negative
 *           needed byte count starting from an error position returned.
 **/

typedef long (*uni_to_func_t_)(
		uint32_t cp,
		uint8_t *out,
		const size_t out_bytes);

/* UTF-8 */

static long conv_utf8_to_uni_(
		const uint8_t *const in,
		const size_t in_bytes,
		uint32_t *cp)
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

		return -2;
	} else
	if (b0 < 0xe0) {
		/**
		 * 2 byte sequence: 110xxxxx.
		 * 0xc2..0xdf 0x80..0xbf
		 **/

		if (in_bytes < 2) {
			return 0;
		}

		const uint8_t b1 = in[1];

		if ((b1 & 0xc0) != 0x80) {
			return -2;	/* invalid @a b1 */
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
			return 0;
		}

		const uint8_t b1 = in[1];
		const uint8_t b2 = in[2];

		if (b0 == 0xe0 && b1 < 0xa0) {
			return -2;	/* invalid @a b1 */
		}

		if ((b1 & 0xc0) != 0x80) {
			return -2;	/* invalid @a b1 */
		}

		if ((b2 & 0xc0) != 0x80) {
			return -3;	/* invalid @a b2 */
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
			return 0;
		}

		const uint8_t b1 = in[1];
		const uint8_t b2 = in[2];
		const uint8_t b3 = in[3];

		if ((b0 == 0xf0 && b1 <  0x90) ||
			(b0 == 0xf4 && b1 >= 0x90))
		{
			return -2;	/* invalid @a b1 */
		}

		if ((b1 & 0xc0) != 0x80) {
			return -2;	/* invalid @a b1 */
		}

		if ((b2 & 0xc0) != 0x80) {
			return -3;	/* invalid @a b2 */
		}

		if ((b3 & 0xc0) != 0x80) {
			return -4;	/* invalid @a b3 */
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

	return -1;			/* invalid @a b0 */
}

static long conv_uni_to_utf8_(
		uint32_t cp,
		uint8_t *out,
		const size_t out_bytes)
{
	long size =
		(cp < 0x80)    ? 1 :
		(cp < 0x800)   ? 2 :
		(cp < 0x10000) ? 3 : 4;

	if (out_bytes < (size_t) size) {
		return -size;
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
static long conv_utf16_##x##_to_uni_(									\
		const uint8_t *const in,										\
		const size_t in_bytes,											\
		uint32_t *cp)													\
{																		\
	if (in_bytes < 2) {													\
		return 0;														\
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
		return -1;														\
	}																	\
																		\
	/* w0 is first surrogate code. */									\
																		\
	if (in_bytes < 4) {													\
		return 0;														\
	}																	\
																		\
	const uint16_t w1 = ndm_endian_##x##toh16(*((uint16_t*) (in + 2)));	\
																		\
	if (w1 < NDM_CONV_UNICODE_SUR_LOW_START_ ||							\
		w1 > NDM_CONV_UNICODE_SUR_LOW_END_ )							\
	{																	\
		/* Invalid second surrogate code. */							\
																		\
		return -3;														\
	}																	\
																		\
	*cp =																\
		((((uint32_t) (w0 & 0x3ff)) << 10)  |							\
		 (((uint32_t) (w1 & 0x3ff)) <<  0)) + 0x10000;					\
																		\
	return 4;															\
}																		\
																		\
static long conv_uni_to_utf16_##x##_(									\
		uint32_t cp,													\
		uint8_t *out,													\
		const size_t out_bytes)											\
{																		\
	if (cp < 0x10000) {													\
		if (out_bytes < 2) {											\
			return -2;													\
		}																\
																		\
		*((uint16_t*) out) = ndm_endian_hto##x##16((uint16_t) cp);		\
																		\
		return 2;														\
	}																	\
																		\
	if (out_bytes < 4) {												\
		return -4;														\
	}																	\
																		\
	cp -= 0x10000;														\
																		\
	*((uint16_t*) (out + 0)) = ndm_endian_hto##x##16((uint16_t)			\
		(((cp >> 10) & 0x3ff) | NDM_CONV_UNICODE_SUR_HIGH_START_));		\
	*((uint16_t*) (out + 2)) = ndm_endian_hto##x##16((uint16_t)			\
		(((cp >>  0) & 0x3ff) | NDM_CONV_UNICODE_SUR_LOW_START_));		\
																		\
	return 4;															\
}

NDM_CONV_CHARSET_UTF16_CONVERTERS_(be)
NDM_CONV_CHARSET_UTF16_CONVERTERS_(le)

static long conv_utf16_to_uni_(
		const uint8_t *const in,
		const size_t in_bytes,
		uint32_t *cp)
{
	return ndm_endian_is_le() ?
		conv_utf16_le_to_uni_(in, in_bytes, cp) :
		conv_utf16_be_to_uni_(in, in_bytes, cp);
}

static long conv_uni_to_utf16_(
		uint32_t cp,
		uint8_t *out,
		const size_t out_bytes)
{
	return ndm_endian_is_le() ?
		conv_uni_to_utf16_le_(cp, out, out_bytes) :
		conv_uni_to_utf16_be_(cp, out, out_bytes);
}

/* UTF-32 */

#define NDM_CONV_CHARSET_UTF32_CONVERTERS_(x)							\
static long conv_utf32_##x##_to_uni_(									\
		const uint8_t *const in,										\
		const size_t in_bytes,											\
		uint32_t *cp)													\
{																		\
	if (in_bytes < 4) {													\
		return 0;														\
	}																	\
																		\
	*cp = ndm_endian_##x##toh32(*((uint32_t*) in));						\
																		\
	if ( *cp <= NDM_CONV_UNICODE_MAX_ &&								\
		(*cp <  NDM_CONV_UNICODE_SUR_HIGH_START_ ||						\
		 *cp >  NDM_CONV_UNICODE_SUR_LOW_END_))							\
	{																	\
		return 4;														\
	}																	\
																		\
	return -1;															\
}																		\
																		\
static long conv_uni_to_utf32_##x##_(									\
		uint32_t cp,													\
		uint8_t *out,													\
		const size_t out_bytes)											\
{																		\
	if (out_bytes < 4) {												\
		return -4;														\
	}																	\
																		\
	*((uint32_t*) out) = ndm_endian_hto##x##32(cp);						\
																		\
	return 4;															\
}

NDM_CONV_CHARSET_UTF32_CONVERTERS_(be)
NDM_CONV_CHARSET_UTF32_CONVERTERS_(le)

static long conv_utf32_to_uni_(
		const uint8_t *const in,
		const size_t in_bytes,
		uint32_t *cp)
{
	return ndm_endian_is_le() ?
		conv_utf32_le_to_uni_(in, in_bytes, cp) :
		conv_utf32_be_to_uni_(in, in_bytes, cp);
}

static long conv_uni_to_utf32_(
		uint32_t cp,
		uint8_t *out,
		const size_t out_bytes)
{
	return ndm_endian_is_le() ?
		conv_uni_to_utf32_le_(cp, out, out_bytes) :
		conv_uni_to_utf32_be_(cp, out, out_bytes);
}

/**
 * http://www.iana.org/assignments/character-sets/character-sets.xhtml
 **/

static const struct ndm_conv_pair_t_ {
	const char *const name_;
	to_uni_func_t_ to_uni_;
	uni_to_func_t_ uni_to_;
} NDM_CONV_PAIRS_[] = {
	/* UTF encodings */
	{"utf-8",				conv_utf8_to_uni_,		conv_uni_to_utf8_	   },
	{"utf-16",				conv_utf16_to_uni_,		conv_uni_to_utf16_	   },
	{"utf-16le",			conv_utf16_le_to_uni_,	conv_uni_to_utf16_le_  },
	{"utf-16be",			conv_utf16_be_to_uni_,	conv_uni_to_utf16_be_  },
	{"utf-32",				conv_utf32_to_uni_,		conv_uni_to_utf32_	   },
	{"utf-32le",			conv_utf32_le_to_uni_,	conv_uni_to_utf32_le_  },
	{"utf-32be",			conv_utf32_be_to_uni_,	conv_uni_to_utf32_be_  },

#ifdef LIBNDM_SBCS_SUPPORT

	/* ISO-8859-1 and aliases */
	{"iso-8859-1",			conv_8859_1_to_uni_,	conv_uni_to_8859_1_	   },
	{"iso-ir-100",			conv_8859_1_to_uni_,	conv_uni_to_8859_1_	   },
	{"latin1",				conv_8859_1_to_uni_,	conv_uni_to_8859_1_	   },
	{"l1",					conv_8859_1_to_uni_,	conv_uni_to_8859_1_	   },
	{"IBM819",				conv_8859_1_to_uni_,	conv_uni_to_8859_1_	   },
	{"CP819",				conv_8859_1_to_uni_,	conv_uni_to_8859_1_	   },
	{"csISOLatin1",			conv_8859_1_to_uni_,	conv_uni_to_8859_1_	   },
	{"8859-1",				conv_8859_1_to_uni_,	conv_uni_to_8859_1_	   },

	/* ISO-8859-2 and aliases */
	{"iso-8859-2",			conv_8859_2_to_uni_,	conv_uni_to_8859_2_	   },
	{"iso-ir-101",			conv_8859_2_to_uni_,	conv_uni_to_8859_2_	   },
	{"latin2",				conv_8859_2_to_uni_,	conv_uni_to_8859_2_	   },
	{"l2",					conv_8859_2_to_uni_,	conv_uni_to_8859_2_	   },
	{"csISOLatin2",			conv_8859_2_to_uni_,	conv_uni_to_8859_2_	   },
	{"8859-2",				conv_8859_2_to_uni_,	conv_uni_to_8859_2_	   },

	/* ISO-8859-3 and aliases */
	{"iso-8859-3",			conv_8859_3_to_uni_,	conv_uni_to_8859_3_	   },
	{"iso-ir-109",			conv_8859_3_to_uni_,	conv_uni_to_8859_3_	   },
	{"latin3",				conv_8859_3_to_uni_,	conv_uni_to_8859_3_	   },
	{"l3",					conv_8859_3_to_uni_,	conv_uni_to_8859_3_	   },
	{"csISOLatin3",			conv_8859_3_to_uni_,	conv_uni_to_8859_3_	   },
	{"8859-3",				conv_8859_3_to_uni_,	conv_uni_to_8859_3_	   },

	/* ISO-8859-4 and aliases */
	{"iso-8859-4",			conv_8859_4_to_uni_,	conv_uni_to_8859_4_	   },
	{"iso-ir-110",			conv_8859_4_to_uni_,	conv_uni_to_8859_4_	   },
	{"latin4",				conv_8859_4_to_uni_,	conv_uni_to_8859_4_	   },
	{"l4",					conv_8859_4_to_uni_,	conv_uni_to_8859_4_	   },
	{"csISOLatin4",			conv_8859_4_to_uni_,	conv_uni_to_8859_4_	   },
	{"8859-4",				conv_8859_4_to_uni_,	conv_uni_to_8859_4_	   },

	/* ISO-8859-5 and aliases */
	{"iso-8859-5",			conv_8859_5_to_uni_,	conv_uni_to_8859_5_	   },
	{"iso-ir-144",			conv_8859_5_to_uni_,	conv_uni_to_8859_5_	   },
	{"cyrillic",			conv_8859_5_to_uni_,	conv_uni_to_8859_5_	   },
	{"csISOLatinCyrillic",	conv_8859_5_to_uni_,	conv_uni_to_8859_5_	   },
	{"8859-5",				conv_8859_5_to_uni_,	conv_uni_to_8859_5_	   },

	/* ISO-8859-6 and aliases */
	{"iso-8859-6",			conv_8859_6_to_uni_,	conv_uni_to_8859_6_	   },
	{"iso-ir-127",			conv_8859_6_to_uni_,	conv_uni_to_8859_6_	   },
	{"ECMA-114",			conv_8859_6_to_uni_,	conv_uni_to_8859_6_	   },
	{"ASMO-708",			conv_8859_6_to_uni_,	conv_uni_to_8859_6_	   },
	{"arabic",				conv_8859_6_to_uni_,	conv_uni_to_8859_6_	   },
	{"csISOLatinArabic",	conv_8859_6_to_uni_,	conv_uni_to_8859_6_	   },
	{"8859-6",				conv_8859_6_to_uni_,	conv_uni_to_8859_6_	   },

	/* ISO-8859-7 and aliases */
	{"iso-8859-7",			conv_8859_7_to_uni_,	conv_uni_to_8859_7_	   },
	{"iso-ir-126",			conv_8859_7_to_uni_,	conv_uni_to_8859_7_	   },
	{"ELOT_928",			conv_8859_7_to_uni_,	conv_uni_to_8859_7_	   },
	{"ECMA-118",			conv_8859_7_to_uni_,	conv_uni_to_8859_7_	   },
	{"greek",				conv_8859_7_to_uni_,	conv_uni_to_8859_7_	   },
	{"greek8",				conv_8859_7_to_uni_,	conv_uni_to_8859_7_	   },
	{"csISOLatinGreek",		conv_8859_7_to_uni_,	conv_uni_to_8859_7_	   },
	{"8859-7",				conv_8859_7_to_uni_,	conv_uni_to_8859_7_	   },

	/* ISO-8859-8 and aliases */
	{"iso-8859-8",			conv_8859_8_to_uni_,	conv_uni_to_8859_8_	   },
	{"iso-ir-138",			conv_8859_8_to_uni_,	conv_uni_to_8859_8_	   },
	{"hebrew",				conv_8859_8_to_uni_,	conv_uni_to_8859_8_	   },
	{"csISOLatinHebrew",	conv_8859_8_to_uni_,	conv_uni_to_8859_8_	   },
	{"8859-8",				conv_8859_8_to_uni_,	conv_uni_to_8859_8_	   },

	/* ISO-8859-9 and aliases */
	{"iso-8859-9",			conv_8859_9_to_uni_,	conv_uni_to_8859_9_	   },
	{"iso-ir-148",			conv_8859_9_to_uni_,	conv_uni_to_8859_9_	   },
	{"latin5",				conv_8859_9_to_uni_,	conv_uni_to_8859_9_	   },
	{"l5",					conv_8859_9_to_uni_,	conv_uni_to_8859_9_	   },
	{"csISOLatin5",			conv_8859_9_to_uni_,	conv_uni_to_8859_9_	   },
	{"8859-9",				conv_8859_9_to_uni_,	conv_uni_to_8859_9_	   },

	/* ISO-8859-10 and aliases */
	{"iso-8859-10",			conv_8859_10_to_uni_,	conv_uni_to_8859_10_   },
	{"iso-ir-157",			conv_8859_10_to_uni_,	conv_uni_to_8859_10_   },
	{"l6",					conv_8859_10_to_uni_,	conv_uni_to_8859_10_   },
	{"ISO_8859-10:1992",	conv_8859_10_to_uni_,	conv_uni_to_8859_10_   },
	{"csISOLatin6",			conv_8859_10_to_uni_,	conv_uni_to_8859_10_   },
	{"latin6",				conv_8859_10_to_uni_,	conv_uni_to_8859_10_   },
	{"8859-10",				conv_8859_10_to_uni_,	conv_uni_to_8859_10_   },

	/* ISO-8859-11 and aliases */
	{"iso-8859-11",			conv_8859_11_to_uni_,	conv_uni_to_8859_11_   },
	{"csTIS620",			conv_8859_11_to_uni_,	conv_uni_to_8859_11_   },
	{"TIS-620",				conv_8859_11_to_uni_,	conv_uni_to_8859_11_   },
	{"8859-11",				conv_8859_11_to_uni_,	conv_uni_to_8859_11_   },

	/* ISO-8859-13 and aliases */
	{"iso-8859-13",			conv_8859_13_to_uni_,	conv_uni_to_8859_13_   },
	{"csISO885913",			conv_8859_13_to_uni_,	conv_uni_to_8859_13_   },
	{"8859-13",				conv_8859_13_to_uni_,	conv_uni_to_8859_13_   },

	/* ISO-8859-14 and aliases */
	{"iso-8859-14",			conv_8859_14_to_uni_,	conv_uni_to_8859_14_   },
	{"iso-ir-199",			conv_8859_14_to_uni_,	conv_uni_to_8859_14_   },
	{"latin8",				conv_8859_14_to_uni_,	conv_uni_to_8859_14_   },
	{"iso-celtic",			conv_8859_14_to_uni_,	conv_uni_to_8859_14_   },
	{"l8",					conv_8859_14_to_uni_,	conv_uni_to_8859_14_   },
	{"csISO885914",			conv_8859_14_to_uni_,	conv_uni_to_8859_14_   },
	{"8859-14",				conv_8859_14_to_uni_,	conv_uni_to_8859_14_   },

	/* ISO-8859-15 and aliases */
	{"iso-8859-15",			conv_8859_15_to_uni_,	conv_uni_to_8859_15_   },
	{"Latin-9",				conv_8859_15_to_uni_,	conv_uni_to_8859_15_   },
	{"csISO885915",			conv_8859_15_to_uni_,	conv_uni_to_8859_15_   },
	{"8859-15",				conv_8859_15_to_uni_,	conv_uni_to_8859_15_   },

	/* ISO-8859-16 and aliases */
	{"iso-8859-16",			conv_8859_16_to_uni_,	conv_uni_to_8859_16_   },
	{"iso-ir-226",			conv_8859_16_to_uni_,	conv_uni_to_8859_16_   },
	{"ISO_8859-16:2001",	conv_8859_16_to_uni_,	conv_uni_to_8859_16_   },
	{"latin10",				conv_8859_16_to_uni_,	conv_uni_to_8859_16_   },
	{"l10",					conv_8859_16_to_uni_,	conv_uni_to_8859_16_   },
	{"csISO885916",			conv_8859_16_to_uni_,	conv_uni_to_8859_16_   },
	{"8859-16",				conv_8859_16_to_uni_,	conv_uni_to_8859_16_   },

	/* CP037 and aliases */
	{"cp-037",				conv_cp037_to_uni_,		conv_uni_to_cp037_	   },
	{"ebcdic-cp-us",		conv_cp037_to_uni_,		conv_uni_to_cp037_	   },
	{"ebcdic-cp-ca",		conv_cp037_to_uni_,		conv_uni_to_cp037_	   },
	{"ebcdic-cp-wt",		conv_cp037_to_uni_,		conv_uni_to_cp037_	   },
	{"ebcdic-cp-nl",		conv_cp037_to_uni_,		conv_uni_to_cp037_	   },
	{"csIBM037",			conv_cp037_to_uni_,		conv_uni_to_cp037_	   },
	{"IBM037",				conv_cp037_to_uni_,		conv_uni_to_cp037_	   },

	/* CP424 and aliases */
	{"cp-424",				conv_cp424_to_uni_,		conv_uni_to_cp424_	   },
	{"ebcdic-cp-he",		conv_cp424_to_uni_,		conv_uni_to_cp424_	   },
	{"csIBM424",			conv_cp424_to_uni_,		conv_uni_to_cp424_	   },
	{"IBM424",				conv_cp424_to_uni_,		conv_uni_to_cp424_	   },

	/* CP437 and aliases */
	{"cp-437",				conv_cp437_to_uni_,		conv_uni_to_cp437_	   },
	{"cp437",				conv_cp437_to_uni_,		conv_uni_to_cp437_	   },
	{"437",					conv_cp437_to_uni_,		conv_uni_to_cp437_	   },
	{"csPC8CodePage437",	conv_cp437_to_uni_,		conv_uni_to_cp437_	   },
	{"IBM437",				conv_cp437_to_uni_,		conv_uni_to_cp437_	   },

	/* CP500 and aliases */
	{"cp-500",				conv_cp500_to_uni_,		conv_uni_to_cp500_	   },
	{"CP500",				conv_cp500_to_uni_,		conv_uni_to_cp500_	   },
	{"ebcdic-cp-be",		conv_cp500_to_uni_,		conv_uni_to_cp500_	   },
	{"ebcdic-cp-ch",		conv_cp500_to_uni_,		conv_uni_to_cp500_	   },
	{"csIBM500",			conv_cp500_to_uni_,		conv_uni_to_cp500_	   },
	{"IBM500",				conv_cp500_to_uni_,		conv_uni_to_cp500_	   },

	/* CP737 and aliases */
	{"cp-737",				conv_cp737_to_uni_,		conv_uni_to_cp737_	   },

	/* CP775 and aliases */
	{"cp-775",				conv_cp775_to_uni_,		conv_uni_to_cp775_	   },
	{"csPC775Baltic",		conv_cp775_to_uni_,		conv_uni_to_cp775_	   },
	{"IBM775",				conv_cp775_to_uni_,		conv_uni_to_cp775_	   },

	/* CP850 and aliases */
	{"cp-850",				conv_cp850_to_uni_,		conv_uni_to_cp850_	   },
	{"850",					conv_cp850_to_uni_,		conv_uni_to_cp850_	   },
	{"csPC850Multilingual",	conv_cp850_to_uni_,		conv_uni_to_cp850_	   },
	{"IBM850",				conv_cp850_to_uni_,		conv_uni_to_cp850_	   },

	/* CP852 and aliases */
	{"cp-852",				conv_cp852_to_uni_,		conv_uni_to_cp852_	   },
	{"852",					conv_cp852_to_uni_,		conv_uni_to_cp852_	   },
	{"csPCp852",			conv_cp852_to_uni_,		conv_uni_to_cp852_	   },
	{"IBM852",				conv_cp852_to_uni_,		conv_uni_to_cp852_	   },

	/* CP855 and aliases */
	{"cp-855",				conv_cp855_to_uni_,		conv_uni_to_cp855_	   },
	{"855",					conv_cp855_to_uni_,		conv_uni_to_cp855_	   },
	{"csIBM855",			conv_cp855_to_uni_,		conv_uni_to_cp855_	   },
	{"IBM855",				conv_cp855_to_uni_,		conv_uni_to_cp855_	   },

	/* CP856 and aliases */
	{"cp-856",				conv_cp856_to_uni_,		conv_uni_to_cp856_	   },

	/* CP857 and aliases */
	{"cp-857",				conv_cp857_to_uni_,		conv_uni_to_cp857_	   },
	{"857",					conv_cp857_to_uni_,		conv_uni_to_cp857_	   },
	{"csIBM857",			conv_cp857_to_uni_,		conv_uni_to_cp857_	   },
	{"IBM857",				conv_cp857_to_uni_,		conv_uni_to_cp857_	   },

	/* CP860 and aliases */
	{"cp-860",				conv_cp860_to_uni_,		conv_uni_to_cp860_	   },
	{"860",					conv_cp860_to_uni_,		conv_uni_to_cp860_	   },
	{"csIBM860",			conv_cp860_to_uni_,		conv_uni_to_cp860_	   },
	{"IBM860",				conv_cp860_to_uni_,		conv_uni_to_cp860_	   },

	/* CP861 and aliases */
	{"cp-861",				conv_cp861_to_uni_,		conv_uni_to_cp861_	   },
	{"861",					conv_cp861_to_uni_,		conv_uni_to_cp861_	   },
	{"cp-is",				conv_cp861_to_uni_,		conv_uni_to_cp861_	   },
	{"csIBM861",			conv_cp861_to_uni_,		conv_uni_to_cp861_	   },
	{"IBM861",				conv_cp861_to_uni_,		conv_uni_to_cp861_	   },

	/* CP862 and aliases */
	{"cp-862",				conv_cp862_to_uni_,		conv_uni_to_cp862_	   },
	{"862",					conv_cp862_to_uni_,		conv_uni_to_cp862_	   },
	{"csPC862LatinHebrew",	conv_cp862_to_uni_,		conv_uni_to_cp862_	   },
	{"IBM862",				conv_cp862_to_uni_,		conv_uni_to_cp862_	   },

	/* CP863 and aliases */
	{"cp-863",				conv_cp863_to_uni_,		conv_uni_to_cp863_	   },
	{"863",					conv_cp863_to_uni_,		conv_uni_to_cp863_	   },
	{"csIBM863",			conv_cp863_to_uni_,		conv_uni_to_cp863_	   },
	{"IBM863",				conv_cp863_to_uni_,		conv_uni_to_cp863_	   },

	/* CP864 and aliases */
	{"cp-864",				conv_cp864_to_uni_,		conv_uni_to_cp864_	   },
	{"csIBM864",			conv_cp864_to_uni_,		conv_uni_to_cp864_	   },
	{"IBM864",				conv_cp864_to_uni_,		conv_uni_to_cp864_	   },

	/* CP865 and aliases */
	{"cp-865",				conv_cp865_to_uni_,		conv_uni_to_cp865_	   },
	{"865",					conv_cp865_to_uni_,		conv_uni_to_cp865_	   },
	{"csIBM865",			conv_cp865_to_uni_,		conv_uni_to_cp865_	   },
	{"IBM865",				conv_cp865_to_uni_,		conv_uni_to_cp865_	   },

	/* CP866 and aliases */
	{"cp-866",				conv_cp866_to_uni_,		conv_uni_to_cp866_	   },
	{"866",					conv_cp866_to_uni_,		conv_uni_to_cp866_	   },
	{"csIBM866",			conv_cp866_to_uni_,		conv_uni_to_cp866_	   },
	{"IBM866",				conv_cp866_to_uni_,		conv_uni_to_cp866_	   },

	/* CP869 and aliases */
	{"cp-869",				conv_cp869_to_uni_,		conv_uni_to_cp869_	   },
	{"869",					conv_cp869_to_uni_,		conv_uni_to_cp869_	   },
	{"cp-gr",				conv_cp869_to_uni_,		conv_uni_to_cp869_	   },
	{"csIBM869",			conv_cp869_to_uni_,		conv_uni_to_cp869_	   },
	{"IBM869",				conv_cp869_to_uni_,		conv_uni_to_cp869_	   },

	/* CP874 and aliases */
	{"cp-874",				conv_cp874_to_uni_,		conv_uni_to_cp874_	   },
	{"windows-874",			conv_cp874_to_uni_,		conv_uni_to_cp874_	   },
	{"cswindows874",		conv_cp874_to_uni_,		conv_uni_to_cp874_	   },

	/* CP1026 and aliases */
	{"cp-1026",				conv_cp1026_to_uni_,	conv_uni_to_cp1026_	   },
	{"csIBM1026",			conv_cp1026_to_uni_,	conv_uni_to_cp1026_	   },
	{"IBM1026",				conv_cp1026_to_uni_,	conv_uni_to_cp1026_	   },

	/* CP1250 and aliases */
	{"cp-1250",				conv_cp1250_to_uni_,	conv_uni_to_cp1250_	   },
	{"windows-1250",		conv_cp1250_to_uni_,	conv_uni_to_cp1250_	   },
	{"cswindows1250",		conv_cp1250_to_uni_,	conv_uni_to_cp1250_	   },

	/* CP1251 and aliases */
	{"cp-1251",				conv_cp1251_to_uni_,	conv_uni_to_cp1251_	   },
	{"windows-1251",		conv_cp1251_to_uni_,	conv_uni_to_cp1251_	   },
	{"cswindows1251",		conv_cp1251_to_uni_,	conv_uni_to_cp1251_	   },

	/* CP1252 and aliases */
	{"cp-1252",				conv_cp1252_to_uni_,	conv_uni_to_cp1252_	   },
	{"windows-1252",		conv_cp1252_to_uni_,	conv_uni_to_cp1252_	   },
	{"cswindows1252",		conv_cp1252_to_uni_,	conv_uni_to_cp1252_	   },

	/* CP1253 and aliases */
	{"cp-1253",				conv_cp1253_to_uni_,	conv_uni_to_cp1253_	   },
	{"windows-1253",		conv_cp1253_to_uni_,	conv_uni_to_cp1253_	   },
	{"cswindows1253",		conv_cp1253_to_uni_,	conv_uni_to_cp1253_	   },

	/* CP1254 and aliases */
	{"cp-1254",				conv_cp1254_to_uni_,	conv_uni_to_cp1254_	   },
	{"windows-1254",		conv_cp1254_to_uni_,	conv_uni_to_cp1254_	   },
	{"cswindows1254",		conv_cp1254_to_uni_,	conv_uni_to_cp1254_	   },

	/* CP1255 and aliases */
	{"cp-1255",				conv_cp1255_to_uni_,	conv_uni_to_cp1255_	   },
	{"windows-1255",		conv_cp1255_to_uni_,	conv_uni_to_cp1255_	   },
	{"cswindows1255",		conv_cp1255_to_uni_,	conv_uni_to_cp1255_	   },

	/* CP1256 and aliases */
	{"cp-1256",				conv_cp1256_to_uni_,	conv_uni_to_cp1256_	   },
	{"windows-1256",		conv_cp1256_to_uni_,	conv_uni_to_cp1256_	   },
	{"cswindows1256",		conv_cp1256_to_uni_,	conv_uni_to_cp1256_	   },

	/* CP1257 and aliases */
	{"cp-1257",				conv_cp1257_to_uni_,	conv_uni_to_cp1257_	   },
	{"windows-1257",		conv_cp1257_to_uni_,	conv_uni_to_cp1257_	   },
	{"cswindows1257",		conv_cp1257_to_uni_,	conv_uni_to_cp1257_	   },

	/* CP1258 and aliases */
	{"cp-1258",				conv_cp1258_to_uni_,	conv_uni_to_cp1258_	   },
	{"windows-1258",		conv_cp1258_to_uni_,	conv_uni_to_cp1258_	   },
	{"cswindows1258",		conv_cp1258_to_uni_,	conv_uni_to_cp1258_	   },

	/* KOI8-R and aliases */
	{"koi8-r",				conv_koi8_r_to_uni_,	conv_uni_to_koi8_r_	   },
	{"csKOI8R",				conv_koi8_r_to_uni_,	conv_uni_to_koi8_r_	   },

	/* KOI8-U and aliases */
	{"koi8-u",				conv_koi8_u_to_uni_,	conv_uni_to_koi8_u_	   },
	{"csKOI8U",				conv_koi8_u_to_uni_,	conv_uni_to_koi8_u_	   },

	/* KZ1048 and aliases */
	{"kz-1048",				conv_kz1048_to_uni_,	conv_uni_to_kz1048_	   },
	{"STRK1048-2002",		conv_kz1048_to_uni_,	conv_uni_to_kz1048_	   },
	{"RK1048",				conv_kz1048_to_uni_,	conv_uni_to_kz1048_	   },
	{"csKZ1048",			conv_kz1048_to_uni_,	conv_uni_to_kz1048_	   },

	/* NEXTSTEP and aliases */
	{"nextstep",			conv_nextstep_to_uni_,	conv_uni_to_nextstep_  },

	/* CELTIC and aliases (MacCeltic) */
	{"mac-celtic",			conv_celtic_to_uni_,	conv_uni_to_celtic_	   },
	{"x-mac-celtic",		conv_celtic_to_uni_,	conv_uni_to_celtic_	   },

	/* CENTEURO and aliases (MacCentralEuropean) */
	{"mac-centeuro",		conv_centeuro_to_uni_,	conv_uni_to_centeuro_  },
	{"x-mac-centeuro",		conv_centeuro_to_uni_,	conv_uni_to_centeuro_  },

	/* CROATIAN and aliases (MacCroatian) */
	{"mac-croatian",		conv_croatian_to_uni_,	conv_uni_to_croatian_  },
	{"x-mac-croatian",		conv_croatian_to_uni_,	conv_uni_to_croatian_  },

	/* CYRILLIC and aliases (MacCyrillic) */
	{"mac-cyrillic",		conv_cyrillic_to_uni_,	conv_uni_to_cyrillic_  },
	{"x-mac-cyrillic",		conv_cyrillic_to_uni_,	conv_uni_to_cyrillic_  },

	/* GAELIC and aliases (MacGaelic) */
	{"mac-gaelic",			conv_gaelic_to_uni_,	conv_uni_to_gaelic_	   },
	{"x-mac-gaelic",		conv_gaelic_to_uni_,	conv_uni_to_gaelic_	   },

	/* GREEK and aliases (MacGreek) */
	{"mac-greek",			conv_greek_to_uni_,		conv_uni_to_greek_	   },
	{"x-mac-greek",			conv_greek_to_uni_,		conv_uni_to_greek_	   },

	/* ICELAND and aliases (MacIcelandic) */
	{"mac-icelandic",		conv_iceland_to_uni_,	conv_uni_to_iceland_   },
	{"x-mac-icelandic",		conv_iceland_to_uni_,	conv_uni_to_iceland_   },
	{"mac-iceland",			conv_iceland_to_uni_,	conv_uni_to_iceland_   },
	{"x-mac-iceland",		conv_iceland_to_uni_,	conv_uni_to_iceland_   },

	/* INUIT and aliases (MacInuit) */
	{"mac-inuit",			conv_inuit_to_uni_,		conv_uni_to_inuit_	   },
	{"x-mac-inuit",			conv_inuit_to_uni_,		conv_uni_to_inuit_	   },

	/* ROMAN and aliases (MacRoman) */
	{"mac-roman",			conv_roman_to_uni_,		conv_uni_to_roman_	   },
	{"x-mac-roman",			conv_roman_to_uni_,		conv_uni_to_roman_	   },

	/* ROMANIAN and aliases (MacRomanian) */
	{"mac-romanian",		conv_romanian_to_uni_,	conv_uni_to_romanian_  },
	{"x-mac-romanian",		conv_romanian_to_uni_,	conv_uni_to_romanian_  },

	/* TURKISH and aliases (MacTurkish) */
	{"mac-turkish",			conv_turkish_to_uni_,	conv_uni_to_turkish_   },
	{"x-mac-turkish",		conv_turkish_to_uni_,	conv_uni_to_turkish_   },

	/* UKRAINIAN and aliases (MacUkrainian) */
	{"mac-ukrainian",		conv_cyrillic_to_uni_,	conv_uni_to_cyrillic_  },
	{"x-mac-ukrainian",		conv_cyrillic_to_uni_,	conv_uni_to_cyrillic_  }

#endif /* LIBNDM_SBCS_SUPPORT */

};
static const size_t NDM_CONV_COUNT_ = NDM_ARRAY_SIZE(NDM_CONV_PAIRS_);

static ssize_t ndm_conv_find_(const char *const name)
{
	ssize_t i = 0;

	while (i < NDM_ARRAY_SIZE(NDM_CONV_PAIRS_)) {
		const char *p = name;
		const char *q = NDM_CONV_PAIRS_[i].name_;

		do {
			while (*p != '\0' && !isalnum(*p)) {
				++p;
			}

			while (*q != '\0' && !isalnum(*q)) {
				++q;
			}

			if (tolower(*p) != tolower(*q)) {
				/* Check next name. */
				break;
			}

			/* @c *p and @c *q are equal. */
			if (*p == '\0') {
				return i;
			}

			/* @c *p and @c *q are equal but not zero. */
			++p;
			++q;
		} while (true);

		++i;
	}

	return -1;
}

ndm_conv_t ndm_conv_open(
		const char *const to,
		const char *const from)
{
	const ssize_t to_index = ndm_conv_find_(to);
	const ssize_t from_index = ndm_conv_find_(from);

	if (from_index < 0 || to_index < 0) {
		errno = EINVAL;

		return NDM_CONV_INVALID_;
	}

	return
		(ndm_conv_t) (from_index << NDM_CONV_SHIFT_FROM_) |
		(ndm_conv_t) (to_index << NDM_CONV_SHIFT_TO_);
}

size_t ndm_conv(
		const ndm_conv_t cd,
		const char **inp,
		size_t *in_bytes_left,
		char **outp,
		size_t *out_bytes_left)
{
	if (inp == NULL || *inp == NULL) {
		/**
		 * Reset @c conv to its initial shift state
		 * for state-dependent encodings, @c outp value ignored.
		 * Here the state-dependent encondings are not supported
		 * and zero returned.
		 **/

		return 0;
	}

	assert (outp != NULL && *outp != NULL);

	/**
	 * "Returned errors:
	 * [EILSEQ] Input conversion stopped due to an input byte that
	 *          does not belong to the input codeset.
	 * [E2BIG]  Input conversion stopped due to lack of space
	 *          in the output buffer.
	 * [EINVAL] Input conversion stopped due to an incomplete character
	 *          or shift sequence at the end of the input buffer.
	 * [EBADF]  The cd argument is not a valid open conversion descriptor."
	 **/

	const size_t from_index = NDM_CONV_FROM_(cd);
	const size_t to_index = NDM_CONV_TO_(cd);
	size_t nonidentical_count = 0;

	if (from_index >= NDM_CONV_COUNT_ || to_index >= NDM_CONV_COUNT_) {
		errno = EBADF;

		return (size_t) -1;
	}

	to_uni_func_t_ to_uni = NDM_CONV_PAIRS_[from_index].to_uni_;
	uni_to_func_t_ uni_to = NDM_CONV_PAIRS_[to_index].uni_to_;

	while (*in_bytes_left > 0) {
		uint32_t cp;

		/* Called with nonzero input sequence. */
		const long d = to_uni((const uint8_t *) *inp, *in_bytes_left, &cp);

		if (d > 0) {
			assert ( cp <= NDM_CONV_UNICODE_MAX_ &&
					(cp < NDM_CONV_UNICODE_SUR_HIGH_START_ ||
					 cp > NDM_CONV_UNICODE_SUR_LOW_END_));

			const long e = uni_to(cp, (uint8_t *) *outp, *out_bytes_left);

			if (e > 0) {
				*inp += d;
				*in_bytes_left -= (size_t) d;
				*outp += e;
				*out_bytes_left -= (size_t) e;
			} else
			if (e < 0) {
				errno = E2BIG;

				return (size_t) -1;
			} else {
				/**
				 * A valid input Unicode code point can not be presented
				 * in an output charset.
				 * "If iconv() encounters a character in the input buffer
				 * that is valid, but for which an identical character
				 * does not exist in the target codeset, iconv() shall
				 * perform an implementation-defined conversion on
				 * this character."
				 **/

				if (*out_bytes_left < REPNC_SIZE_) {
					errno = E2BIG;

					return (size_t) -1;
				}

				/**
				 * @a REPNC_ should be a valid character
				 * for all output charsets.
				 **/
				**outp = REPNC_;

				*inp += d;
				*in_bytes_left -= (size_t) d;
				*outp += REPNC_SIZE_;
				*out_bytes_left -= REPNC_SIZE_;

				nonidentical_count++;
			}
		} else {
			errno = (d == 0) ? EINVAL : EILSEQ;

			return (size_t) -1;
		}
	}

	/**
	 * "The iconv() function shall update the variables pointed to by
	 * the arguments to reflect the extent of the conversion and return
	 * the number of non-identical conversions performed. If the entire
	 * string in the input buffer is converted, the value pointed to by
	 * @a inbytesleft shall be 0. If the input conversion is stopped due
	 * to any conditions mentioned above, the value pointed to by
	 * @a inbytesleft shall be non-zero and @a errno shall be set to
	 * indicate the condition. If an error occurs, @c iconv() shall
	 * return (size_t) - 1 and set @a errno to indicate the error."
	 **/

	return nonidentical_count;
}

int ndm_conv_close(
		ndm_conv_t cd)
{
	return 0;
}

