#ifndef __NDM_SRC_CHARSET_CP775_H__
#define __NDM_SRC_CHARSET_CP775_H__

/**
 * Automatically generated by @c convgen
 * from @c ./MAPPINGS/VENDORS/MICSFT/PC/CP775.TXT
 * Range merge code gap is 128.
 * See http://www.unicode.org/Public/MAPPINGS/ for
 * other charsets.
 **/

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

static const uint16_t CP775_00000080_000000FF_TO_UNI_[] = {
	0x0106, 0x00fc, 0x00e9, 0x0101, 0x00e4, 0x0123, 0x00e5, 0x0107,
	0x0142, 0x0113, 0x0156, 0x0157, 0x012b, 0x0179, 0x00c4, 0x00c5,
	0x00c9, 0x00e6, 0x00c6, 0x014d, 0x00f6, 0x0122, 0x00a2, 0x015a,
	0x015b, 0x00d6, 0x00dc, 0x00f8, 0x00a3, 0x00d8, 0x00d7, 0x00a4,
	0x0100, 0x012a, 0x00f3, 0x017b, 0x017c, 0x017a, 0x201d, 0x00a6,
	0x00a9, 0x00ae, 0x00ac, 0x00bd, 0x00bc, 0x0141, 0x00ab, 0x00bb,
	0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x0104, 0x010c, 0x0118,
	0x0116, 0x2563, 0x2551, 0x2557, 0x255d, 0x012e, 0x0160, 0x2510,
	0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c, 0x0172, 0x016a,
	0x255a, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256c, 0x017d,
	0x0105, 0x010d, 0x0119, 0x0117, 0x012f, 0x0161, 0x0173, 0x016b,
	0x017e, 0x2518, 0x250c, 0x2588, 0x2584, 0x258c, 0x2590, 0x2580,
	0x00d3, 0x00df, 0x014c, 0x0143, 0x00f5, 0x00d5, 0x00b5, 0x0144,
	0x0136, 0x0137, 0x013b, 0x013c, 0x0146, 0x0112, 0x0145, 0x2019,
	0x00ad, 0x00b1, 0x201c, 0x00be, 0x00b6, 0x00a7, 0x00f7, 0x201e,
	0x00b0, 0x2219, 0x00b7, 0x00b9, 0x00b3, 0x00b2, 0x25a0, 0x00a0
};

static inline long conv_cp775_to_uni_(
		const uint8_t *const in,
		const size_t in_bytes,
		uint32_t *cp)
{
	assert (in_bytes > 0);

	const uint8_t b0 = *in;

	if (b0 <= 0x7f) {
		*cp = b0;

		return 1;
	}

	if (0x80 <= b0) {
		*cp =
			CP775_00000080_000000FF_TO_UNI_[b0 - 0x80];

		return 1;
	}

	return -1;
}

static const uint8_t UNI_000000A0_0000017E_TO_CP775_[] = {
	0xff, 0x00, 0x96, 0x9c, 0x9f, 0x00, 0xa7, 0xf5,
	0x00, 0xa8, 0x00, 0xae, 0xaa, 0xf0, 0xa9, 0x00,
	0xf8, 0xf1, 0xfd, 0xfc, 0x00, 0xe6, 0xf4, 0xfa,
	0x00, 0xfb, 0x00, 0xaf, 0xac, 0xab, 0xf3, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x8e, 0x8f, 0x92, 0x00,
	0x00, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xe0, 0x00, 0xe5, 0x99, 0x9e,
	0x9d, 0x00, 0x00, 0x00, 0x9a, 0x00, 0x00, 0xe1,
	0x00, 0x00, 0x00, 0x00, 0x84, 0x86, 0x91, 0x00,
	0x00, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xa2, 0x00, 0xe4, 0x94, 0xf6,
	0x9b, 0x00, 0x00, 0x00, 0x81, 0x00, 0x00, 0x00,
	0xa0, 0x83, 0x00, 0x00, 0xb5, 0xd0, 0x80, 0x87,
	0x00, 0x00, 0x00, 0x00, 0xb6, 0xd1, 0x00, 0x00,
	0x00, 0x00, 0xed, 0x89, 0x00, 0x00, 0xb8, 0xd3,
	0xb7, 0xd2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x95, 0x85, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xa1, 0x8c, 0x00, 0x00, 0xbd, 0xd4,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe8, 0xe9,
	0x00, 0x00, 0x00, 0xea, 0xeb, 0x00, 0x00, 0x00,
	0x00, 0xad, 0x88, 0xe3, 0xe7, 0xee, 0xec, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xe2, 0x93, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8a, 0x8b,
	0x00, 0x00, 0x97, 0x98, 0x00, 0x00, 0x00, 0x00,
	0xbe, 0xd5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xc7, 0xd7, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xc6, 0xd6, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x8d, 0xa5, 0xa3, 0xa4, 0xcf, 0xd8
};

static const uint8_t UNI_00002019_0000201E_TO_CP775_[] = {
	0xef, 0x00, 0x00, 0xf2, 0xa6, 0xf7
};

static const uint8_t UNI_00002500_000025A0_TO_CP775_[] = {
	0xc4, 0x00, 0xb3, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xda, 0x00, 0x00, 0x00,
	0xbf, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00,
	0xd9, 0x00, 0x00, 0x00, 0xc3, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xb4, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xc2, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xc1, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xc5, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xcd, 0xba, 0x00, 0x00, 0xc9, 0x00, 0x00, 0xbb,
	0x00, 0x00, 0xc8, 0x00, 0x00, 0xbc, 0x00, 0x00,
	0xcc, 0x00, 0x00, 0xb9, 0x00, 0x00, 0xcb, 0x00,
	0x00, 0xca, 0x00, 0x00, 0xce, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xdf, 0x00, 0x00, 0x00, 0xdc, 0x00, 0x00, 0x00,
	0xdb, 0x00, 0x00, 0x00, 0xdd, 0x00, 0x00, 0x00,
	0xde, 0xb0, 0xb1, 0xb2, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xfe
};

static inline long conv_uni_to_cp775_(
		uint32_t cp,
		uint8_t *out,
		const size_t out_bytes)
{
	if (out_bytes == 0) {
		return -1;
	}

	if (cp <= 0x7f) {
		*out = (uint8_t) cp;

		return 1;
	}

	if (0xa0 <= cp && cp <= 0x17e) {
		const uint8_t code =
			UNI_000000A0_0000017E_TO_CP775_[cp - 0xa0];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	if (0x2019 <= cp && cp <= 0x201e) {
		const uint8_t code =
			UNI_00002019_0000201E_TO_CP775_[cp - 0x2019];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	if (cp == 0x2219) {
		*out = 0xf9;

		return 1;
	}

	if (0x2500 <= cp && cp <= 0x25a0) {
		const uint8_t code =
			UNI_00002500_000025A0_TO_CP775_[cp - 0x2500];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	return 0;
}

#endif /* __NDM_SRC_CHARSET_CP775_H__ */
