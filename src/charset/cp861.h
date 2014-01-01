#ifndef __NDM_SRC_CHARSET_CP861_H__
#define __NDM_SRC_CHARSET_CP861_H__

/**
 * Automatically generated by @c convgen
 * from @c ./MAPPINGS/VENDORS/MICSFT/PC/CP861.TXT
 * Range merge code gap is 128.
 * See http://www.unicode.org/Public/MAPPINGS/ for
 * other charsets.
 **/

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

static const uint16_t CP861_00000080_000000FF_TO_UNI_[] = {
	0x00c7, 0x00fc, 0x00e9, 0x00e2, 0x00e4, 0x00e0, 0x00e5, 0x00e7,
	0x00ea, 0x00eb, 0x00e8, 0x00d0, 0x00f0, 0x00de, 0x00c4, 0x00c5,
	0x00c9, 0x00e6, 0x00c6, 0x00f4, 0x00f6, 0x00fe, 0x00fb, 0x00dd,
	0x00fd, 0x00d6, 0x00dc, 0x00f8, 0x00a3, 0x00d8, 0x20a7, 0x0192,
	0x00e1, 0x00ed, 0x00f3, 0x00fa, 0x00c1, 0x00cd, 0x00d3, 0x00da,
	0x00bf, 0x2310, 0x00ac, 0x00bd, 0x00bc, 0x00a1, 0x00ab, 0x00bb,
	0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
	0x2555, 0x2563, 0x2551, 0x2557, 0x255d, 0x255c, 0x255b, 0x2510,
	0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c, 0x255e, 0x255f,
	0x255a, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256c, 0x2567,
	0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256b,
	0x256a, 0x2518, 0x250c, 0x2588, 0x2584, 0x258c, 0x2590, 0x2580,
	0x03b1, 0x00df, 0x0393, 0x03c0, 0x03a3, 0x03c3, 0x00b5, 0x03c4,
	0x03a6, 0x0398, 0x03a9, 0x03b4, 0x221e, 0x03c6, 0x03b5, 0x2229,
	0x2261, 0x00b1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00f7, 0x2248,
	0x00b0, 0x2219, 0x00b7, 0x221a, 0x207f, 0x00b2, 0x25a0, 0x00a0
};

static inline long conv_cp861_to_uni_(
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
			CP861_00000080_000000FF_TO_UNI_[b0 - 0x80];

		return 1;
	}

	return -1;
}

static const uint8_t UNI_000000A0_000000FE_TO_CP861_[] = {
	0xff, 0xad, 0x00, 0x9c, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xae, 0xaa, 0x00, 0x00, 0x00,
	0xf8, 0xf1, 0xfd, 0x00, 0x00, 0xe6, 0x00, 0xfa,
	0x00, 0x00, 0x00, 0xaf, 0xac, 0xab, 0x00, 0xa8,
	0x00, 0xa4, 0x00, 0x00, 0x8e, 0x8f, 0x92, 0x80,
	0x00, 0x90, 0x00, 0x00, 0x00, 0xa5, 0x00, 0x00,
	0x8b, 0x00, 0x00, 0xa6, 0x00, 0x00, 0x99, 0x00,
	0x9d, 0x00, 0xa7, 0x00, 0x9a, 0x97, 0x8d, 0xe1,
	0x85, 0xa0, 0x83, 0x00, 0x84, 0x86, 0x91, 0x87,
	0x8a, 0x82, 0x88, 0x89, 0x00, 0xa1, 0x00, 0x00,
	0x8c, 0x00, 0x00, 0xa2, 0x93, 0x00, 0x94, 0xf6,
	0x9b, 0x00, 0xa3, 0x96, 0x81, 0x98, 0x95
};

static const uint8_t UNI_00000393_000003C6_TO_CP861_[] = {
	0xe2, 0x00, 0x00, 0x00, 0x00, 0xe9, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xe4, 0x00, 0x00, 0xe8, 0x00, 0x00, 0xea, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00,
	0x00, 0xeb, 0xee, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0xe3, 0x00, 0x00,
	0xe5, 0xe7, 0x00, 0xed
};

static const uint8_t UNI_0000207F_000020A7_TO_CP861_[] = {
	0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x9e
};

static const uint8_t UNI_00002219_00002265_TO_CP861_[] = {
	0xf9, 0xfb, 0x00, 0x00, 0x00, 0xec, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xef, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf7,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xf0, 0x00, 0x00, 0xf3, 0xf2
};

static const uint8_t UNI_00002310_00002321_TO_CP861_[] = {
	0xa9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xf4, 0xf5
};

static const uint8_t UNI_00002500_000025A0_TO_CP861_[] = {
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
	0xcd, 0xba, 0xd5, 0xd6, 0xc9, 0xb8, 0xb7, 0xbb,
	0xd4, 0xd3, 0xc8, 0xbe, 0xbd, 0xbc, 0xc6, 0xc7,
	0xcc, 0xb5, 0xb6, 0xb9, 0xd1, 0xd2, 0xcb, 0xcf,
	0xd0, 0xca, 0xd8, 0xd7, 0xce, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xdf, 0x00, 0x00, 0x00, 0xdc, 0x00, 0x00, 0x00,
	0xdb, 0x00, 0x00, 0x00, 0xdd, 0x00, 0x00, 0x00,
	0xde, 0xb0, 0xb1, 0xb2, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xfe
};

static inline long conv_uni_to_cp861_(
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

	if (0xa0 <= cp && cp <= 0xfe) {
		const uint8_t code =
			UNI_000000A0_000000FE_TO_CP861_[cp - 0xa0];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	if (cp == 0x0192) {
		*out = 0x9f;

		return 1;
	}

	if (0x0393 <= cp && cp <= 0x03c6) {
		const uint8_t code =
			UNI_00000393_000003C6_TO_CP861_[cp - 0x0393];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	if (0x207f <= cp && cp <= 0x20a7) {
		const uint8_t code =
			UNI_0000207F_000020A7_TO_CP861_[cp - 0x207f];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	if (0x2219 <= cp && cp <= 0x2265) {
		const uint8_t code =
			UNI_00002219_00002265_TO_CP861_[cp - 0x2219];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	if (0x2310 <= cp && cp <= 0x2321) {
		const uint8_t code =
			UNI_00002310_00002321_TO_CP861_[cp - 0x2310];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	if (0x2500 <= cp && cp <= 0x25a0) {
		const uint8_t code =
			UNI_00002500_000025A0_TO_CP861_[cp - 0x2500];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	return 0;
}

#endif /* __NDM_SRC_CHARSET_CP861_H__ */
