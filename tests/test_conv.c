#include <stdlib.h>
#include <string.h>
#include <ndm/conv.h>
#include <ndm/endian.h>
#include "test.h"

#define NDM_CONV_TEST_(													\
		from, to, flags,												\
		in, in_bytes, in_stop,											\
		out_stop, out,													\
		res)															\
do {																	\
	ndm_conv_t encoder_ = ndm_conv_open(from, to, flags);				\
	size_t size_ = 0;													\
	char *ba_;															\
																		\
	NDM_TEST_BREAK_IF(encoder_ < 0);									\
	NDM_TEST_BREAK_IF((ba_ = malloc(in_bytes)) == NULL);				\
																		\
	memcpy(ba_, in, in_bytes);											\
																		\
	const char *inp_ = ba_;												\
																		\
	NDM_TEST(ndm_conv(													\
		&encoder_,														\
		(const void **) &inp_, in_bytes,								\
		NULL, 0, &size_) == res);										\
	NDM_TEST(inp_ == ba_ + in_stop);									\
	NDM_TEST(size_ == out_stop);										\
																		\
	char *output_ = malloc(size_);										\
																		\
	NDM_TEST_BREAK_IF(output_ == NULL);									\
																		\
	char *outp_ = output_;												\
																		\
	inp_ = ba_;															\
																		\
	NDM_TEST(ndm_conv(													\
		&encoder_,														\
		(const void **) &inp_, in_bytes,								\
		(void **) &outp_, size_, NULL) == res);							\
	NDM_TEST(outp_ == output_ + out_stop);								\
	NDM_TEST(memcmp(output_, out, size_) == 0);							\
																		\
	free(ba_);															\
	free(output_);														\
																		\
	ndm_conv_close(&encoder_);											\
} while (0)

static void test_conv_utf16_(
		uint16_t *in,
		const size_t in_bytes,
		const char *const from,
		uint16_t (*from_host)(const uint16_t x))
{
	for (unsigned long i = 0; i < in_bytes/2; i++) {
		in[i] = from_host(in[i]);
	}

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_STRICTLY,
		in, in_bytes,
		in_bytes, in_bytes / 2,
		"test UTF-16", NDM_CONV_ERROR_OK);

	/**
	 * Surrogate pairs:
	 * high word [0xd800--0xdbff],
	 * low word  [0xdc00--0xdfff].
	 **/

	/* Hi word only. */

	in[5] = from_host(0xd800);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_STRICTLY,
		in, in_bytes,
		5 * 2, 5,
		"test ", NDM_CONV_ERROR_INPUT_ILLEGAL);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_ILLEGAL,
		in, in_bytes,
		in_bytes, in_bytes / 2,
		"test ?TF-16", NDM_CONV_ERROR_OK);

	/* Low word only. */

	in[5] = from_host(0xdcff);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_STRICTLY,
		in, in_bytes,
		5 * 2, 5,
		"test ", NDM_CONV_ERROR_INPUT_ILLEGAL);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_ILLEGAL,
		in, in_bytes,
		in_bytes, in_bytes / 2,
		"test ?TF-16", NDM_CONV_ERROR_OK);

	/* Valid surrogate pair. */

	in[5] = from_host(0xd800);
	in[6] = from_host(0xdcff);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_STRICTLY,
		in, in_bytes,
		5 * 2, 5,
		"test ", NDM_CONV_ERROR_INPUT_NON_MAPPED);

	NDM_CONV_TEST_(
		from, "UTF-8",
		NDM_CONV_FLAGS_ENCODE_STRICTLY,
		in, in_bytes,
		in_bytes, in_bytes / 2 + 2,
		"test \xf0\x90\x83\xbf""F-16",
		NDM_CONV_ERROR_OK);

	in[5] = from_host('U');
	in[6] = from_host('T');

	/* Truncated code. */

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_STRICTLY,
		in, in_bytes - 1,
		in_bytes - 2, in_bytes / 2 - 1,
		"test UTF-16", NDM_CONV_ERROR_INPUT_TRUNCATED);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_TRUNCATED,
		in, in_bytes - 1,
		in_bytes - 1, in_bytes / 2,
		"test UTF-1?", NDM_CONV_ERROR_OK);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_STRICTLY,
		in, 1,
		0, 0,
		"", NDM_CONV_ERROR_INPUT_TRUNCATED);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_TRUNCATED,
		in, 1,
		1, 1,
		"?", NDM_CONV_ERROR_OK);

	/* Last high surrogate. */

	in[10] = from_host(0xd8ff);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_STRICTLY,
		in, in_bytes,
		in_bytes - 2, in_bytes / 2 - 1,
		"test UTF-1", NDM_CONV_ERROR_INPUT_TRUNCATED);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_TRUNCATED,
		in, in_bytes,
		in_bytes, in_bytes / 2,
		"test UTF-1?", NDM_CONV_ERROR_OK);

	/* Last high and truncated low surrogates. */

	in[9] = from_host(0xd8ff);
	in[10]= from_host(0xdcff);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_STRICTLY,
		in, in_bytes - 1,
		in_bytes - 4, in_bytes / 2 - 2,
		"test UTF-", NDM_CONV_ERROR_INPUT_TRUNCATED);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_TRUNCATED,
		in, in_bytes - 1,
		in_bytes - 1, in_bytes / 2 - 1,
		"test UTF-?", NDM_CONV_ERROR_OK);

	/* Several surrogate starts. */

	in[7] = from_host(0xd880);
	in[8] = from_host(0xd880);
	in[9] = from_host(0xd880);
	in[10]= from_host(0xd880);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_STRICTLY,
		in, in_bytes,
		7 * 2, 7,
		"test UT", NDM_CONV_ERROR_INPUT_ILLEGAL);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_ILLEGAL,
		in, in_bytes,
		in_bytes - 2, in_bytes / 2 - 1,
		"test UT""?""?""?", NDM_CONV_ERROR_INPUT_TRUNCATED);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_ILLEGAL  |
		NDM_CONV_FLAGS_ENCODE_TRUNCATED,
		in, in_bytes,
		in_bytes, in_bytes / 2,
		"test UT?""?""?""?", NDM_CONV_ERROR_OK);

	/* Several surrogate ends. */

	in[7] = from_host(0xdc80);
	in[8] = from_host(0xdc80);
	in[9] = from_host(0xdc80);
	in[10]= from_host(0xdc80);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_STRICTLY,
		in, in_bytes,
		7 * 2, 7,
		"test UT", NDM_CONV_ERROR_INPUT_ILLEGAL);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_ILLEGAL,
		in, in_bytes,
		in_bytes, in_bytes / 2 - 3,
		"test UT?", NDM_CONV_ERROR_OK);
}

static void test_conv_utf32_(
		uint32_t *in,
		const size_t in_bytes,
		const char *const from,
		uint32_t (*from_host)(const uint32_t x))
{
	for (unsigned long i = 0; i < in_bytes/4; i++) {
		in[i] = from_host(in[i]);
	}

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_STRICTLY,
		in, in_bytes,
		in_bytes, in_bytes / 4,
		"test UTF-32", NDM_CONV_ERROR_OK);

	/* Illegal code point. */

	in[5] = from_host(0x200000);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_STRICTLY,
		in, in_bytes,
		5 * 4, 5,
		"test ", NDM_CONV_ERROR_INPUT_ILLEGAL);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_ILLEGAL,
		in, in_bytes,
		in_bytes, in_bytes / 4,
		"test ?TF-32", NDM_CONV_ERROR_OK);

	/* Non-mapped code point. */

	in[5] = from_host(0x20000);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_STRICTLY,
		in, in_bytes,
		5 * 4, 5,
		"test ", NDM_CONV_ERROR_INPUT_NON_MAPPED);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_ILLEGAL   |
		NDM_CONV_FLAGS_ENCODE_NON_MAPPED,
		in, in_bytes,
		in_bytes, in_bytes / 4,
		"test ?TF-32", NDM_CONV_ERROR_OK);

	in[5] = from_host('U');

	/* Truncated, only 1 byte presented. */

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_STRICTLY,
		in, in_bytes - 3,
		10 * 4, 10,
		"test UTF-3", NDM_CONV_ERROR_INPUT_TRUNCATED);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_TRUNCATED,
		in, in_bytes - 3,
		in_bytes - 3, in_bytes / 4,
		"test UTF-3?", NDM_CONV_ERROR_OK);

	/* Truncated, only 2 bytes presented. */

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_STRICTLY,
		in, in_bytes - 2,
		10 * 4, 10,
		"test UTF-3", NDM_CONV_ERROR_INPUT_TRUNCATED);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_TRUNCATED,
		in, in_bytes - 2,
		in_bytes - 2, in_bytes / 4,
		"test UTF-3?", NDM_CONV_ERROR_OK);

	/* Truncated, only 3 bytes presented. */

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_STRICTLY,
		in, in_bytes - 3,
		10 * 4, 10,
		"test UTF-3", NDM_CONV_ERROR_INPUT_TRUNCATED);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_TRUNCATED,
		in, in_bytes - 3,
		in_bytes - 3, in_bytes / 4,
		"test UTF-3?", NDM_CONV_ERROR_OK);

	/* Several invalid code points (from a surrogate range). */

	in[7] = from_host(0xd800);
	in[8] = from_host(0xdbff);
	in[9] = from_host(0xdc00);
	in[10]= from_host(0xdfff);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_STRICTLY,
		in, in_bytes,
		7 * 4, 7,
		"test UT", NDM_CONV_ERROR_INPUT_ILLEGAL);

	NDM_CONV_TEST_(
		from, "ISO-8859-1",
		NDM_CONV_FLAGS_ENCODE_ILLEGAL,
		in, in_bytes,
		in_bytes, in_bytes / 4 - 3,
		"test UT?", NDM_CONV_ERROR_OK);
}

static void test_conv_utf_range_(
		const char *const to,
		const bool is_le,
		const uint32_t start,
		const uint32_t end)
{
	const char *const from = is_le ? "UTF-32LE" : "UTF-32BE";
	uint32_t (*from_host)(const uint32_t x) =
		is_le ? ndm_endian_htole32 : ndm_endian_htobe32;

	ndm_conv_t dcoder = ndm_conv_open(
		from, to, NDM_CONV_FLAGS_ENCODE_STRICTLY);

	ndm_conv_t icoder = ndm_conv_open(
		to, from, NDM_CONV_FLAGS_ENCODE_STRICTLY);

	NDM_TEST(dcoder >= 0);
	NDM_TEST(icoder >= 0);

	if (dcoder >= 0 && icoder >= 0) {
		for (uint32_t i = start; i < end; i++) {
			uint32_t enc = 0;
			uint32_t dst = 0;
			size_t size = 0;
			uint32_t src = from_host(i);
			const char *in = (const char *) &src;
			char *out = (char *) &enc;

			NDM_TEST(ndm_conv(
				&dcoder,
				(const void **) &in, sizeof(src),
				(void **) &out, sizeof(enc), &size) == NDM_CONV_ERROR_OK);

			in = (const char *) &enc;
			out = (char *) &dst;

			NDM_TEST(ndm_conv(
				&icoder,
				(const void **) &in, size,
				(void **) &out, sizeof(dst), NULL) == NDM_CONV_ERROR_OK);

			NDM_TEST(src == dst);
		}
	}

	ndm_conv_close(&dcoder);
	ndm_conv_close(&icoder);
}

int main()
{
	{
		/* ISO 8859-1 */
		/*			 012345678901234 */
		char in[] = "test ISO-8859-1";

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			in, strlen(in) + 1,
			strlen(in) + 1, strlen(in) + 1,
			"test ISO-8859-1",
			NDM_CONV_ERROR_OK);

		in[5] = '\x80';

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			in, strlen(in) + 1,
			5, 5,
			"test ",
			NDM_CONV_ERROR_INPUT_ILLEGAL);

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			NDM_CONV_FLAGS_ENCODE_NON_STRICTLY,
			in, strlen(in) + 1,
			strlen(in) + 1, strlen(in) + 1,
			"test ?SO-8859-1",
			NDM_CONV_ERROR_OK);

		char iin[sizeof("тест")] = "тест";

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			iin, strlen(iin) + 1,
			0, 0,
			"",
			NDM_CONV_ERROR_INPUT_NON_MAPPED);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_NON_STRICTLY,
			iin, strlen(iin) + 1,
			strlen(iin) + 1, 5,
			"????",
			NDM_CONV_ERROR_OK);
	}

	{
		/* UTF-8 */
		/*           0123456789 */
		char in[] = "test UTF-8";

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			in, strlen(in) + 1,
			strlen(in) + 1, strlen(in) + 1,
			"test UTF-8",
			NDM_CONV_ERROR_OK);

		in[4] = '\x80';

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			in, strlen(in) + 1,
			4, 4,
			"test",
			NDM_CONV_ERROR_INPUT_ILLEGAL);

		/* Illegal character. */

		in[4] = ' ';
		in[5] = '\xd0';

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			in, strlen(in) + 1,
			5, 5,
			"test ",
			NDM_CONV_ERROR_INPUT_ILLEGAL);

		/* 1-byte illegal sequence. */

		in[5] = '\xc1';

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			in, strlen(in) + 1,
			5, 5,
			"test ",
			NDM_CONV_ERROR_INPUT_ILLEGAL);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_ILLEGAL,
			in, strlen(in) + 1,
			strlen(in) + 1, strlen(in) + 1,
			"test ?TF-8",
			NDM_CONV_ERROR_OK);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_NON_STRICTLY,
			in, strlen(in) + 1,
			strlen(in) + 1, strlen(in) + 1,
			"test ?TF-8",
			NDM_CONV_ERROR_OK);

		/* 2-byte non mapped character. */

		in[5] = '\xd0';
		in[6] = '\x90'; /* 2-byte Cyrillic 'A' */

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			in, strlen(in) + 1,
			5, 5,
			"test ",
			NDM_CONV_ERROR_INPUT_NON_MAPPED);

		/* 2-byte illegal sequence. */

		in[5] = '\xd0';
		in[6] = '\xf3';

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			in, strlen(in) + 1,
			5, 5,
			"test ",
			NDM_CONV_ERROR_INPUT_ILLEGAL);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_ILLEGAL,
			in, strlen(in) + 1,
			11, 11,
			"test ??F-8",
			NDM_CONV_ERROR_OK);

		in[6] = '\xf5';	/* 2-byte skip. */

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_ILLEGAL,
			in, strlen(in) + 1,
			11, 10,
			"test ?F-8",
			NDM_CONV_ERROR_OK);

		/* 2-byte truncated sequence. */

		in[5] = 'U';
		in[6] = 'T';
		in[7] = 'F';
		in[10] = '\xc2';

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			in, 11,
			10, 10,
			"test UTF-8",
			NDM_CONV_ERROR_INPUT_TRUNCATED);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_TRUNCATED,
			in, 11,
			11, 11,
			"test UTF-8?",
			NDM_CONV_ERROR_OK);

		in[10] = '\0';

		/* 3-byte illegal sequence, 2nd error byte. */

		in[5] = '\xe0';
		in[6] = '\x9f';
		in[7] = '\x81';

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			in, strlen(in) + 1,
			5, 5,
			"test ",
			NDM_CONV_ERROR_INPUT_ILLEGAL);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_ILLEGAL,
			in, strlen(in) + 1,
			11, 9,
			"test ?-8",
			NDM_CONV_ERROR_OK);

		/* 3-byte illegal sequence, 3rd error byte. */

		in[5] = '\xe0';
		in[6] = '\xa0';	/* 5 and 6 replaced by one symbol. */
		in[7] = '\xe2';

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_ILLEGAL,
			in, strlen(in) + 1,
			11, 10,
			"test ?""?-8",
			NDM_CONV_ERROR_OK);

		/* 3-byte non mapped sequence. */

		in[5] = '\xe0';
		in[6] = '\xa0';
		in[7] = '\xbf';

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			in, strlen(in) + 1,
			5, 5,
			"test ",
			NDM_CONV_ERROR_INPUT_NON_MAPPED);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_ILLEGAL,
			in, strlen(in) + 1,
			5, 5,
			"test ",
			NDM_CONV_ERROR_INPUT_NON_MAPPED);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_NON_MAPPED,
			in, strlen(in) + 1,
			11, 9,
			"test ?-8",
			NDM_CONV_ERROR_OK);

		/* 3-byte truncated sequence, only 1 byte presented. */

		in[5] = 'U';
		in[6] = 'T';
		in[7] = 'F';
		in[10]= '\xe1';

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			in, 11,
			10, 10,
			"test UTF-8",
			NDM_CONV_ERROR_INPUT_TRUNCATED);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_TRUNCATED,
			in, 11,
			11, 11,
			"test UTF-8?",
			NDM_CONV_ERROR_OK);

		in[10] = '\0';

		/* 3-byte truncated sequence, only 2 bytes presented. */

		in[5] = 'U';
		in[6] = 'T';
		in[7] = 'F';
		in[9] = '\xe1';
		in[10]= '\xbf';

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			in, 11,
			9, 9,
			"test UTF-",
			NDM_CONV_ERROR_INPUT_TRUNCATED);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_TRUNCATED,
			in, 11,
			11, 10,
			"test UTF-?",
			NDM_CONV_ERROR_OK);

		in[9] = '8';
		in[10]= '\0';

		/* 4-byte illegal sequence, 2nd error byte. */

		in[5] = '\xf0';
		in[6] = '\x8f';
		in[7] = 'F';

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			in, strlen(in) + 1,
			5, 5,
			"test ",
			NDM_CONV_ERROR_INPUT_ILLEGAL);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_ILLEGAL,
			in, strlen(in) + 1,
			11, 10,
			"test ?F-8",
			NDM_CONV_ERROR_OK);

		/* 4-byte illegal sequence, 3rd error byte. */

		in[5] = '\xf0';
		in[6] = '\x90';
		in[7] = '\xc0';

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			in, strlen(in) + 1,
			5, 5,
			"test ",
			NDM_CONV_ERROR_INPUT_ILLEGAL);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_ILLEGAL,
			in, strlen(in) + 1,
			11, 9,
			"test ?-8",
			NDM_CONV_ERROR_OK);

		/* 4-byte illegal sequence, 4th error byte. */

		in[5] = '\xf0';
		in[6] = '\x90';
		in[7] = '\xbf';
		in[8] = '\xc0';

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			in, strlen(in) + 1,
			5, 5,
			"test ",
			NDM_CONV_ERROR_INPUT_ILLEGAL);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_ILLEGAL,
			in, strlen(in) + 1,
			11, 8,
			"test ?8",
			NDM_CONV_ERROR_OK);

		/* 4-byte non mapped sequence. */

		in[5] = '\xf0';
		in[6] = '\x90';
		in[7] = '\xbf';
		in[8] = '\x80';

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			in, strlen(in) + 1,
			5, 5,
			"test ",
			NDM_CONV_ERROR_INPUT_NON_MAPPED);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_ILLEGAL,
			in, strlen(in) + 1,
			5, 5,
			"test ",
			NDM_CONV_ERROR_INPUT_NON_MAPPED);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_NON_MAPPED,
			in, strlen(in) + 1,
			11, 8,
			"test ?8",
			NDM_CONV_ERROR_OK);

		/* 4-byte truncated sequence, only 1 byte presented. */

		in[5] = 'U';
		in[6] = 'T';
		in[7] = 'F';
		in[8] = '-';
		in[9] = '8';
		in[10]= '\xf4';

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			in, 11,
			10, 10,
			"test UTF-8",
			NDM_CONV_ERROR_INPUT_TRUNCATED);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_TRUNCATED,
			in, 11,
			11, 11,
			"test UTF-8?",
			NDM_CONV_ERROR_OK);

		/* 4-byte truncated sequence, only 2 bytes presented. */

		in[9] = '\xf4';
		in[10]= '\x8f';

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			in, 11,
			9, 9,
			"test UTF-",
			NDM_CONV_ERROR_INPUT_TRUNCATED);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_TRUNCATED,
			in, 11,
			11, 10,
			"test UTF-?",
			NDM_CONV_ERROR_OK);

		/* 4-byte truncated sequence, only 3 bytes presented. */

		in[8] = '\xf4';
		in[9] = '\x8f';
		in[10]= '\xbf';

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			in, 11,
			8, 8,
			"test UTF",
			NDM_CONV_ERROR_INPUT_TRUNCATED);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_TRUNCATED,
			in, 11,
			11, 9,
			"test UTF?",
			NDM_CONV_ERROR_OK);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			"\xc1\xc1\xc1\xc1\xc1\xc1", 6,
			0, 0,
			"", NDM_CONV_ERROR_INPUT_ILLEGAL);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_ILLEGAL,
			"\xc1\xc1\xc1\xc1\xc1\xc1", 6,
			6, 1,
			"?", NDM_CONV_ERROR_OK);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_STRICTLY,
			"\xc2\xc2\xc2\xc2\xc2\xc2", 6,
			0, 0,
			"", NDM_CONV_ERROR_INPUT_ILLEGAL);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_ILLEGAL,
			"\xc2\xc2\xc2\xc2\xc2\xc2", 6,
			5, 5,
			"?????", NDM_CONV_ERROR_INPUT_TRUNCATED);

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			NDM_CONV_FLAGS_ENCODE_ILLEGAL |
			NDM_CONV_FLAGS_ENCODE_TRUNCATED,
			"\xc2\xc2\xc2\xc2\xc2\xc2", 6,
			6, 6,
			"??????", NDM_CONV_ERROR_OK);
	}

	{
		/* UTF-16 */
		uint16_t in_le[] =
			/*0    1    2    3    4    5    6    7    8    9   10*/
			{'t', 'e', 's', 't', ' ', 'U', 'T', 'F', '-', '1', '6'};

		test_conv_utf16_(in_le, sizeof(in_le),
			"UTF-16LE", ndm_endian_htole16);

		uint16_t in_be[] =
			//0    1    2    3    4    5    6    7    8    9   10
			{'t', 'e', 's', 't', ' ', 'U', 'T', 'F', '-', '1', '6'};

		test_conv_utf16_(in_be, sizeof(in_be),
			"UTF-16BE", ndm_endian_htobe16);
	}

	{
		/* UTF-32 */
		uint32_t in_le[] =
			/*0    1    2    3    4    5    6    7    8    9   10*/
			{'t', 'e', 's', 't', ' ', 'U', 'T', 'F', '-', '3', '2'};

		test_conv_utf32_(in_le, sizeof(in_le),
			"UTF-32LE", ndm_endian_htole32);

		uint32_t in_be[] =
			/*0    1    2    3    4    5    6    7    8    9   10*/
			{'t', 'e', 's', 't', ' ', 'U', 'T', 'F', '-', '3', '2'};

		test_conv_utf32_(in_be, sizeof(in_be),
			"UTF-32BE", ndm_endian_htobe32);
	}

	test_conv_utf_range_("ISO-8859-1", true, 0x0000, 0x80);
	test_conv_utf_range_("ISO-8859-1", false, 0x0000, 0x80);

	test_conv_utf_range_("UTF-8", true, 0x0000, 0xd800);
	test_conv_utf_range_("UTF-8", true, 0xe000, 0x110000);
	test_conv_utf_range_("UTF-8", false, 0x0000, 0xd800);
	test_conv_utf_range_("UTF-8", false, 0xe000, 0x110000);

	test_conv_utf_range_("UTF-16LE", true, 0x0000, 0xd800);
	test_conv_utf_range_("UTF-16LE", true, 0xe000, 0x110000);
	test_conv_utf_range_("UTF-16LE", false, 0x0000, 0xd800);
	test_conv_utf_range_("UTF-16LE", false, 0xe000, 0x110000);

	test_conv_utf_range_("UTF-16BE", true, 0x0000, 0xd800);
	test_conv_utf_range_("UTF-16BE", true, 0xe000, 0x110000);
	test_conv_utf_range_("UTF-16BE", false, 0x0000, 0xd800);
	test_conv_utf_range_("UTF-16BE", false, 0xe000, 0x110000);

	return NDM_TEST_RESULT;
}

