#ifndef __NDM_SRC_CHARSET_8859_13_H__
#define __NDM_SRC_CHARSET_8859_13_H__

/**
 * Automatically generated by @c convgen
 * from @c ./MAPPINGS/ISO8859/8859-13.TXT
 * Range merge code gap is 128.
 * See http://www.unicode.org/Public/MAPPINGS/ for
 * other charsets.
 **/

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

static const uint16_t ISO_8859_13_000000A1_000000FF_TO_UNI_[] = {
	0x201d, 0x00a2, 0x00a3, 0x00a4, 0x201e, 0x00a6, 0x00a7, 0x00d8,
	0x00a9, 0x0156, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00c6, 0x00b0,
	0x00b1, 0x00b2, 0x00b3, 0x201c, 0x00b5, 0x00b6, 0x00b7, 0x00f8,
	0x00b9, 0x0157, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00e6, 0x0104,
	0x012e, 0x0100, 0x0106, 0x00c4, 0x00c5, 0x0118, 0x0112, 0x010c,
	0x00c9, 0x0179, 0x0116, 0x0122, 0x0136, 0x012a, 0x013b, 0x0160,
	0x0143, 0x0145, 0x00d3, 0x014c, 0x00d5, 0x00d6, 0x00d7, 0x0172,
	0x0141, 0x015a, 0x016a, 0x00dc, 0x017b, 0x017d, 0x00df, 0x0105,
	0x012f, 0x0101, 0x0107, 0x00e4, 0x00e5, 0x0119, 0x0113, 0x010d,
	0x00e9, 0x017a, 0x0117, 0x0123, 0x0137, 0x012b, 0x013c, 0x0161,
	0x0144, 0x0146, 0x00f3, 0x014d, 0x00f5, 0x00f6, 0x00f7, 0x0173,
	0x0142, 0x015b, 0x016b, 0x00fc, 0x017c, 0x017e, 0x2019
};

static inline long conv_8859_13_to_uni_(
		const uint8_t *const in,
		const size_t in_bytes,
		uint32_t *cp)
{
	assert (in_bytes > 0);

	const uint8_t b0 = *in;

	if (b0 <= 0xa0) {
		*cp = b0;

		return 1;
	}

	if (0xa1 <= b0) {
		*cp =
			ISO_8859_13_000000A1_000000FF_TO_UNI_[b0 - 0xa1];

		return 1;
	}

	return -1;
}

static const uint8_t UNI_000000A2_0000017E_TO_ISO_8859_13_[] = {
	0xa2, 0xa3, 0xa4, 0x00, 0xa6, 0xa7, 0x00, 0xa9,
	0x00, 0xab, 0xac, 0xad, 0xae, 0x00, 0xb0, 0xb1,
	0xb2, 0xb3, 0x00, 0xb5, 0xb6, 0xb7, 0x00, 0xb9,
	0x00, 0xbb, 0xbc, 0xbd, 0xbe, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xc4, 0xc5, 0xaf, 0x00, 0x00, 0xc9,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xd3, 0x00, 0xd5, 0xd6, 0xd7, 0xa8, 0x00,
	0x00, 0x00, 0xdc, 0x00, 0x00, 0xdf, 0x00, 0x00,
	0x00, 0x00, 0xe4, 0xe5, 0xbf, 0x00, 0x00, 0xe9,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xf3, 0x00, 0xf5, 0xf6, 0xf7, 0xb8, 0x00,
	0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0xc2, 0xe2,
	0x00, 0x00, 0xc0, 0xe0, 0xc3, 0xe3, 0x00, 0x00,
	0x00, 0x00, 0xc8, 0xe8, 0x00, 0x00, 0x00, 0x00,
	0xc7, 0xe7, 0x00, 0x00, 0xcb, 0xeb, 0xc6, 0xe6,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xcc, 0xec, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xce, 0xee, 0x00, 0x00, 0xc1, 0xe1, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xcd, 0xed, 0x00, 0x00,
	0x00, 0xcf, 0xef, 0x00, 0x00, 0x00, 0x00, 0xd9,
	0xf9, 0xd1, 0xf1, 0xd2, 0xf2, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xd4, 0xf4, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xaa, 0xba, 0x00, 0x00,
	0xda, 0xfa, 0x00, 0x00, 0x00, 0x00, 0xd0, 0xf0,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xdb, 0xfb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xd8, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0xca,
	0xea, 0xdd, 0xfd, 0xde, 0xfe
};

static const uint8_t UNI_00002019_0000201E_TO_ISO_8859_13_[] = {
	0xff, 0x00, 0x00, 0xb4, 0xa1, 0xa5
};

static inline long conv_uni_to_8859_13_(
		uint32_t cp,
		uint8_t *out,
		const size_t out_bytes)
{
	if (out_bytes == 0) {
		return -1;
	}

	if (cp <= 0xa0) {
		*out = (uint8_t) cp;

		return 1;
	}

	if (0xa2 <= cp && cp <= 0x17e) {
		const uint8_t code =
			UNI_000000A2_0000017E_TO_ISO_8859_13_[cp - 0xa2];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	if (0x2019 <= cp && cp <= 0x201e) {
		const uint8_t code =
			UNI_00002019_0000201E_TO_ISO_8859_13_[cp - 0x2019];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	return 0;
}

#endif /* __NDM_SRC_CHARSET_8859_13_H__ */
