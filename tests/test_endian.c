#include <string.h>
#include <stdbool.h>
#include <ndm/int.h>
#include <ndm/endian.h>
#include "test.h"

int main()
{
	NDM_TEST(NDM_INT_MIN(char) == CHAR_MIN);
	NDM_TEST(NDM_INT_MAX(char) == CHAR_MAX);

	NDM_TEST(
		( ndm_endian_is_le() &&
			ndm_endian_letoh16(0x1234) ==
				0x1234) ||
		(!ndm_endian_is_le() &&
			ndm_endian_letoh16(0x1234) ==
				0x3412));

	NDM_TEST(
		  ndm_endian_letoh16(
			ndm_endian_htole16(0x1234)) ==
				0x1234);

	NDM_TEST(
		( ndm_endian_is_le() &&
			ndm_endian_letoh32(0x12345678) ==
				0x12345678) ||
		(!ndm_endian_is_le() &&
			ndm_endian_letoh32(0x12345678) ==
				0x78563412));

	NDM_TEST(
		  ndm_endian_letoh32(
			ndm_endian_htole32(0x12345678)) ==
				0x12345678);

	NDM_TEST(
		( ndm_endian_is_le() &&
			ndm_endian_letoh64(0x1234567812345678ULL) ==
				0x1234567812345678ULL) ||
		(!ndm_endian_is_le() &&
			ndm_endian_letoh64(0x1234567812345678ULL) ==
				0x7856341278563412ULL));

	NDM_TEST(
		 ndm_endian_letoh64(
			ndm_endian_htole64(0x1234567812345678ULL)) ==
				0x1234567812345678ULL);

	NDM_TEST(
		( ndm_endian_is_le() &&
			ndm_endian_betoh16(0x1234) ==
				0x3412) ||
		(!ndm_endian_is_le() &&
			ndm_endian_betoh16(0x1234) ==
				0x1234));

	NDM_TEST(
		  ndm_endian_betoh16(
			ndm_endian_htobe16(0x1234)) ==
				0x1234);

	NDM_TEST(
		( ndm_endian_is_le() &&
			ndm_endian_betoh32(0x12345678) ==
				0x78563412) ||
		(!ndm_endian_is_le() &&
			ndm_endian_betoh32(0x12345678) ==
				0x12345678));

	NDM_TEST(
		  ndm_endian_betoh32(
			 ndm_endian_htobe32(0x12345678)) ==
				0x12345678);

	NDM_TEST(
		( ndm_endian_is_le() &&
			ndm_endian_betoh64(0x1234567812345678ULL) ==
				0x7856341278563412ULL) ||
		(!ndm_endian_is_le() &&
			ndm_endian_betoh64(0x1234567812345678ULL) ==
				0x1234567812345678ULL));

	NDM_TEST(
		  ndm_endian_betoh64(
			ndm_endian_htobe64(0x1234567812345678ULL)) ==
				0x1234567812345678ULL);

	return NDM_TEST_RESULT;
}

