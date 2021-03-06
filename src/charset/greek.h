#ifndef __NDM_SRC_CHARSET_GREEK_H__
#define __NDM_SRC_CHARSET_GREEK_H__

/**
 * Automatically generated by @c convgen
 * from @c ./MAPPINGS/VENDORS/APPLE/GREEK.TXT
 * Range merge code gap is 128.
 * See http://www.unicode.org/Public/MAPPINGS/ for
 * other charsets.
 **/

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

static const uint16_t GREEK_00000020_000000FF_TO_UNI_[] = {
	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
	0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
	0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
	0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
	0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
	0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
	0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
	0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
	0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0xd800,
	0x00c4, 0x00b9, 0x00b2, 0x00c9, 0x00b3, 0x00d6, 0x00dc, 0x0385,
	0x00e0, 0x00e2, 0x00e4, 0x0384, 0x00a8, 0x00e7, 0x00e9, 0x00e8,
	0x00ea, 0x00eb, 0x00a3, 0x2122, 0x00ee, 0x00ef, 0x2022, 0x00bd,
	0x2030, 0x00f4, 0x00f6, 0x00a6, 0x20ac, 0x00f9, 0x00fb, 0x00fc,
	0x2020, 0x0393, 0x0394, 0x0398, 0x039b, 0x039e, 0x03a0, 0x00df,
	0x00ae, 0x00a9, 0x03a3, 0x03aa, 0x00a7, 0x2260, 0x00b0, 0x00b7,
	0x0391, 0x00b1, 0x2264, 0x2265, 0x00a5, 0x0392, 0x0395, 0x0396,
	0x0397, 0x0399, 0x039a, 0x039c, 0x03a6, 0x03ab, 0x03a8, 0x03a9,
	0x03ac, 0x039d, 0x00ac, 0x039f, 0x03a1, 0x2248, 0x03a4, 0x00ab,
	0x00bb, 0x2026, 0x00a0, 0x03a5, 0x03a7, 0x0386, 0x0388, 0x0153,
	0x2013, 0x2015, 0x201c, 0x201d, 0x2018, 0x2019, 0x00f7, 0x0389,
	0x038a, 0x038c, 0x038e, 0x03ad, 0x03ae, 0x03af, 0x03cc, 0x038f,
	0x03cd, 0x03b1, 0x03b2, 0x03c8, 0x03b4, 0x03b5, 0x03c6, 0x03b3,
	0x03b7, 0x03b9, 0x03be, 0x03ba, 0x03bb, 0x03bc, 0x03bd, 0x03bf,
	0x03c0, 0x03ce, 0x03c1, 0x03c3, 0x03c4, 0x03b8, 0x03c9, 0x03c2,
	0x03c7, 0x03c5, 0x03b6, 0x03ca, 0x03cb, 0x0390, 0x03b0, 0x00ad
};

static inline long conv_greek_to_uni_(
		const uint8_t *const in,
		const size_t in_bytes,
		uint32_t *cp)
{
	assert (in_bytes > 0);

	const uint8_t b0 = *in;

	if (0x20 <= b0) {
		const uint16_t code =
			GREEK_00000020_000000FF_TO_UNI_[b0 - 0x20];

		/* illegal code for this range */
		if (code == 0xd800) {
			return -1;
		}

		*cp = code;

		return 1;
	}

	return -1;
}

static const uint8_t UNI_00000020_000000FC_TO_GREEK_[] = {
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
	0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xca, 0x00, 0x00, 0x92, 0x00, 0xb4, 0x9b, 0xac,
	0x8c, 0xa9, 0x00, 0xc7, 0xc2, 0xff, 0xa8, 0x00,
	0xae, 0xb1, 0x82, 0x84, 0x00, 0x00, 0x00, 0xaf,
	0x00, 0x81, 0x00, 0xc8, 0x00, 0x97, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
	0x00, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x85, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x86, 0x00, 0x00, 0xa7,
	0x88, 0x00, 0x89, 0x00, 0x8a, 0x00, 0x00, 0x8d,
	0x8f, 0x8e, 0x90, 0x91, 0x00, 0x00, 0x94, 0x95,
	0x00, 0x00, 0x00, 0x00, 0x99, 0x00, 0x9a, 0xd6,
	0x00, 0x9d, 0x00, 0x9e, 0x9f
};

static const uint8_t UNI_00000384_000003CE_TO_GREEK_[] = {
	0x8b, 0x87, 0xcd, 0x00, 0xce, 0xd7, 0xd8, 0x00,
	0xd9, 0x00, 0xda, 0xdf, 0xfd, 0xb0, 0xb5, 0xa1,
	0xa2, 0xb6, 0xb7, 0xb8, 0xa3, 0xb9, 0xba, 0xa4,
	0xbb, 0xc1, 0xa5, 0xc3, 0xa6, 0xc4, 0x00, 0xaa,
	0xc6, 0xcb, 0xbc, 0xcc, 0xbe, 0xbf, 0xab, 0xbd,
	0xc0, 0xdb, 0xdc, 0xdd, 0xfe, 0xe1, 0xe2, 0xe7,
	0xe4, 0xe5, 0xfa, 0xe8, 0xf5, 0xe9, 0xeb, 0xec,
	0xed, 0xee, 0xea, 0xef, 0xf0, 0xf2, 0xf7, 0xf3,
	0xf4, 0xf9, 0xe6, 0xf8, 0xe3, 0xf6, 0xfb, 0xfc,
	0xde, 0xe0, 0xf1
};

static const uint8_t UNI_00002013_000020AC_TO_GREEK_[] = {
	0xd0, 0x00, 0xd1, 0x00, 0x00, 0xd4, 0xd5, 0x00,
	0x00, 0xd2, 0xd3, 0x00, 0x00, 0xa0, 0x00, 0x96,
	0x00, 0x00, 0x00, 0xc9, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x98, 0x00, 0x00,
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
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x9c
};

static const uint8_t UNI_00002248_00002265_TO_GREEK_[] = {
	0xc5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xad, 0x00, 0x00, 0x00, 0xb2, 0xb3
};

static inline long conv_uni_to_greek_(
		uint32_t cp,
		uint8_t *out,
		const size_t out_bytes)
{
	if (out_bytes == 0) {
		return -1;
	}

	if (0x20 <= cp && cp <= 0xfc) {
		const uint8_t code =
			UNI_00000020_000000FC_TO_GREEK_[cp - 0x20];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	if (cp == 0x0153) {
		*out = 0xcf;

		return 1;
	}

	if (0x0384 <= cp && cp <= 0x03ce) {
		const uint8_t code =
			UNI_00000384_000003CE_TO_GREEK_[cp - 0x0384];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	if (0x2013 <= cp && cp <= 0x20ac) {
		const uint8_t code =
			UNI_00002013_000020AC_TO_GREEK_[cp - 0x2013];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	if (cp == 0x2122) {
		*out = 0x93;

		return 1;
	}

	if (0x2248 <= cp && cp <= 0x2265) {
		const uint8_t code =
			UNI_00002248_00002265_TO_GREEK_[cp - 0x2248];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	return 0;
}

#endif /* __NDM_SRC_CHARSET_GREEK_H__ */
