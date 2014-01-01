#ifndef __NDM_SRC_CHARSET_KOI8_U_H__
#define __NDM_SRC_CHARSET_KOI8_U_H__

/**
 * Automatically generated by @c convgen
 * from @c ./MAPPINGS/VENDORS/MISC/KOI8-U.TXT
 * Range merge code gap is 128.
 * See http://www.unicode.org/Public/MAPPINGS/ for
 * other charsets.
 **/

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

static const uint16_t KOI8_U_00000080_000000FF_TO_UNI_[] = {
	0x2500, 0x2502, 0x250c, 0x2510, 0x2514, 0x2518, 0x251c, 0x2524,
	0x252c, 0x2534, 0x253c, 0x2580, 0x2584, 0x2588, 0x258c, 0x2590,
	0x2591, 0x2592, 0x2593, 0x2320, 0x25a0, 0x2219, 0x221a, 0x2248,
	0x2264, 0x2265, 0x00a0, 0x2321, 0x00b0, 0x00b2, 0x00b7, 0x00f7,
	0x2550, 0x2551, 0x2552, 0x0451, 0x0454, 0x2554, 0x0456, 0x0457,
	0x2557, 0x2558, 0x2559, 0x255a, 0x255b, 0x0491, 0x255d, 0x255e,
	0x255f, 0x2560, 0x2561, 0x0401, 0x0404, 0x2563, 0x0406, 0x0407,
	0x2566, 0x2567, 0x2568, 0x2569, 0x256a, 0x0490, 0x256c, 0x00a9,
	0x044e, 0x0430, 0x0431, 0x0446, 0x0434, 0x0435, 0x0444, 0x0433,
	0x0445, 0x0438, 0x0439, 0x043a, 0x043b, 0x043c, 0x043d, 0x043e,
	0x043f, 0x044f, 0x0440, 0x0441, 0x0442, 0x0443, 0x0436, 0x0432,
	0x044c, 0x044b, 0x0437, 0x0448, 0x044d, 0x0449, 0x0447, 0x044a,
	0x042e, 0x0410, 0x0411, 0x0426, 0x0414, 0x0415, 0x0424, 0x0413,
	0x0425, 0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e,
	0x041f, 0x042f, 0x0420, 0x0421, 0x0422, 0x0423, 0x0416, 0x0412,
	0x042c, 0x042b, 0x0417, 0x0428, 0x042d, 0x0429, 0x0427, 0x042a
};

static inline long conv_koi8_u_to_uni_(
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
			KOI8_U_00000080_000000FF_TO_UNI_[b0 - 0x80];

		return 1;
	}

	return -1;
}

static const uint8_t UNI_000000A0_000000F7_TO_KOI8_U_[] = {
	0x9a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x9c, 0x00, 0x9d, 0x00, 0x00, 0x00, 0x00, 0x9e,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9f
};

static const uint8_t UNI_00000401_00000491_TO_KOI8_U_[] = {
	0xb3, 0x00, 0x00, 0xb4, 0x00, 0xb6, 0xb7, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe1,
	0xe2, 0xf7, 0xe7, 0xe4, 0xe5, 0xf6, 0xfa, 0xe9,
	0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf2,
	0xf3, 0xf4, 0xf5, 0xe6, 0xe8, 0xe3, 0xfe, 0xfb,
	0xfd, 0xff, 0xf9, 0xf8, 0xfc, 0xe0, 0xf1, 0xc1,
	0xc2, 0xd7, 0xc7, 0xc4, 0xc5, 0xd6, 0xda, 0xc9,
	0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd2,
	0xd3, 0xd4, 0xd5, 0xc6, 0xc8, 0xc3, 0xde, 0xdb,
	0xdd, 0xdf, 0xd9, 0xd8, 0xdc, 0xc0, 0xd1, 0x00,
	0xa3, 0x00, 0x00, 0xa4, 0x00, 0xa6, 0xa7, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbd,
	0xad
};

static const uint8_t UNI_00002219_00002265_TO_KOI8_U_[] = {
	0x95, 0x96, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x97,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x98, 0x99
};

static const uint8_t UNI_00002320_00002321_TO_KOI8_U_[] = {
	0x93, 0x9b
};

static const uint8_t UNI_00002500_000025A0_TO_KOI8_U_[] = {
	0x80, 0x00, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x82, 0x00, 0x00, 0x00,
	0x83, 0x00, 0x00, 0x00, 0x84, 0x00, 0x00, 0x00,
	0x85, 0x00, 0x00, 0x00, 0x86, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x87, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x89, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x8a, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xa0, 0xa1, 0xa2, 0x00, 0xa5, 0x00, 0x00, 0xa8,
	0xa9, 0xaa, 0xab, 0xac, 0x00, 0xae, 0xaf, 0xb0,
	0xb1, 0xb2, 0x00, 0xb5, 0x00, 0x00, 0xb8, 0xb9,
	0xba, 0xbb, 0xbc, 0x00, 0xbe, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x8b, 0x00, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x00,
	0x8d, 0x00, 0x00, 0x00, 0x8e, 0x00, 0x00, 0x00,
	0x8f, 0x90, 0x91, 0x92, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x94
};

static inline long conv_uni_to_koi8_u_(
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
			UNI_000000A0_000000F7_TO_KOI8_U_[cp - 0xa0];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	if (0x0401 <= cp && cp <= 0x0491) {
		const uint8_t code =
			UNI_00000401_00000491_TO_KOI8_U_[cp - 0x0401];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	if (0x2219 <= cp && cp <= 0x2265) {
		const uint8_t code =
			UNI_00002219_00002265_TO_KOI8_U_[cp - 0x2219];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	if (0x2320 <= cp && cp <= 0x2321) {
		*out =
			UNI_00002320_00002321_TO_KOI8_U_[cp - 0x2320];

		return 1;
	}

	if (0x2500 <= cp && cp <= 0x25a0) {
		const uint8_t code =
			UNI_00002500_000025A0_TO_KOI8_U_[cp - 0x2500];

		/* illegal code for this range */
		if (code == 0x00) {
			return 0;
		}

		*out = code;

		return 1;
	}

	return 0;
}

#endif /* __NDM_SRC_CHARSET_KOI8_U_H__ */
