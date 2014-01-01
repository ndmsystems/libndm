#ifndef __NDM_SRC_CHARSET_CP1255_H__
#define __NDM_SRC_CHARSET_CP1255_H__

/**
 * Automatically generated by @c convgen
 * from @c ./MAPPINGS/VENDORS/MICSFT/WINDOWS/CP1255.TXT
 * Range merge code gap is 128.
 * See http://www.unicode.org/Public/MAPPINGS/ for
 * other charsets.
 **/

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

static const uint16_t CP1255_00000080_000000FE_TO_UNI_[] = {
	0x20ac, 0xd800, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
	0x02c6, 0x2030, 0xd800, 0x2039, 0xd800, 0xd800, 0xd800, 0xd800,
	0xd800, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
	0x02dc, 0x2122, 0xd800, 0x203a, 0xd800, 0xd800, 0xd800, 0xd800,
	0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x20aa, 0x00a5, 0x00a6, 0x00a7,
	0x00a8, 0x00a9, 0x00d7, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
	0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
	0x00b8, 0x00b9, 0x00f7, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
	0x05b0, 0x05b1, 0x05b2, 0x05b3, 0x05b4, 0x05b5, 0x05b6, 0x05b7,
	0x05b8, 0x05b9, 0xd800, 0x05bb, 0x05bc, 0x05bd, 0x05be, 0x05bf,
	0x05c0, 0x05c1, 0x05c2, 0x05c3, 0x05f0, 0x05f1, 0x05f2, 0x05f3,
	0x05f4, 0xd800, 0xd800, 0xd800, 0xd800, 0xd800, 0xd800, 0xd800,
	0x05d0, 0x05d1, 0x05d2, 0x05d3, 0x05d4, 0x05d5, 0x05d6, 0x05d7,
	0x05d8, 0x05d9, 0x05da, 0x05db, 0x05dc, 0x05dd, 0x05de, 0x05df,
	0x05e0, 0x05e1, 0x05e2, 0x05e3, 0x05e4, 0x05e5, 0x05e6, 0x05e7,
	0x05e8, 0x05e9, 0x05ea, 0xd800, 0xd800, 0x200e, 0x200f
};

static inline long conv_cp1255_to_uni_(
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

	if (0x80 <= b0 && b0 <= 0xfe) {
		const uint16_t code =
			CP1255_00000080_000000FE_TO_UNI_[b0 - 0x80];

		/* illegal code for this range */
		if (code == 0xd800) {
			return -1;
		}

		*cp = code;

		return 1;
	}

	return -1;
}

static const uint8_t UNI_000000A0_000000F7_TO_CP1255_[] = {
	0xa0, 0xa1, 0xa2, 0xa3, 0x00, 0xa5, 0xa6, 0xa7,
	0xa8, 0xa9, 0x00, 0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
	0xb8, 0xb9, 0x00, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xaa,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xba
};

static const uint8_t UNI_000002C6_000002DC_TO_CP1255_[] = {
	0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x98
};

static const uint8_t UNI_000005B0_000005F4_TO_CP1255_[] = {
	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
	0xc8, 0xc9, 0x00, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
	0xd0, 0xd1, 0xd2, 0xd3, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
	0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
	0xf8, 0xf9, 0xfa, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xd4, 0xd5, 0xd6, 0xd7, 0xd8
};

static const uint8_t UNI_0000200E_000020AC_TO_CP1255_[] = {
	0xfd, 0xfe, 0x00, 0x00, 0x00, 0x96, 0x97, 0x00,
	0x00, 0x00, 0x91, 0x92, 0x82, 0x00, 0x93, 0x94,
	0x84, 0x00, 0x86, 0x87, 0x95, 0x00, 0x00, 0x00,
	0x85, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x89, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x8b, 0x9b, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xa4, 0x00, 0x80
};

static inline long conv_uni_to_cp1255_(
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

	if (0xa0 <= cp && cp <= 0xf7) {
		const uint8_t code =
			UNI_000000A0_000000F7_TO_CP1255_[cp - 0xa0];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	if (cp == 0x0192) {
		*out = 0x83;

		return 1;
	}

	if (0x02c6 <= cp && cp <= 0x02dc) {
		const uint8_t code =
			UNI_000002C6_000002DC_TO_CP1255_[cp - 0x02c6];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	if (0x05b0 <= cp && cp <= 0x05f4) {
		const uint8_t code =
			UNI_000005B0_000005F4_TO_CP1255_[cp - 0x05b0];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	if (0x200e <= cp && cp <= 0x20ac) {
		const uint8_t code =
			UNI_0000200E_000020AC_TO_CP1255_[cp - 0x200e];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	if (cp == 0x2122) {
		*out = 0x99;

		return 1;
	}

	return 0;
}

#endif /* __NDM_SRC_CHARSET_CP1255_H__ */
