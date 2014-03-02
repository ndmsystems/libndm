#include <stddef.h>
#include <string.h>
#include <ndm/md5.h>
#include "test.h"

int main()
{
	struct ndm_md5_t md5 = NDM_MD5_INITIALIZER;
	const char* const MD5_TESTS_[] =	// see RFC1321
	{
		"",	"d41d8cd98f00b204e9800998ecf8427e",
		"a", "0cc175b9c0f1b6a831c399e269772661",
		"abc", "900150983cd24fb0d6963f7d28e17f72",
		"message digest", "f96b697d7cb7938d525a2f31aaf161d0",
		"abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b",
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
			"d174ab98d277d9f5a5611c2c9f419d9f",
		"123456789012345678901234567890123456789012345678901234567890" \
		"12345678901234567890", "57edf4a22be3c955ac49da2e2107b67a",
		NULL
	};
	const char *const *tp = MD5_TESTS_;

	while (*tp != NULL) {
		char text[NDM_MD5_TEXT_BUFFER_SIZE];

		ndm_md5_init(&md5);
		ndm_md5_update(&md5, *tp, strlen(*tp));
		ndm_md5_text_digest(&md5, text);

		++tp;

		NDM_TEST(strcmp(*tp, text) == 0);

		++tp;
	}

	return NDM_TEST_RESULT;
}
