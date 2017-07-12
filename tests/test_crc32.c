#include <stddef.h>
#include <string.h>
#include <ndm/crc32.h>
#include "test.h"

int main()
{
	static const struct
	{
		const char *const text;
		const uint32_t digest;
	} CRC32_TESTS_[] =
	{
		{"An Arbitrary String", 0x6fbeaae7},
		{"ZYXWVUTSRQPONMLKJIHGFEDBCA", 0x99cdfdb2},
		{"123456789", 0xcbf43926},
		{NULL, 0}
	};
	struct ndm_crc32_t crc32 = NDM_CRC32_INITIALIZER;
	size_t i = 0;

	while (CRC32_TESTS_[i].text != NULL) {
		const char *const text = CRC32_TESTS_[i].text;

		ndm_crc32_init(&crc32);
		ndm_crc32_update(&crc32, text, strlen(text));

		NDM_TEST(ndm_crc32_digest(&crc32) == CRC32_TESTS_[i].digest);

		++i;
	}

	return NDM_TEST_RESULT;
}
