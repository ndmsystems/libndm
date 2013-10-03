#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ndm/macro.h>
#include <ndm/ip_checksum.h>
#include "test.h"

#define ARRAY_CHECKSUM_TEST(a, c)	\
	NDM_TEST(htons(ndm_ip_checksum(a, sizeof(a))) == c)

int main(int argc, char *argv[])
{
	static const uint8_t TEST_ARRAY1[] = {
		0x45, 0x00, 0x01, 0x42, 0x00, 0x00, 0x00, 0x00,
		0x40, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xff, 0xff, 0xff, 0xff
	};
	static const uint8_t TEST_ARRAY2[] = {
		0x45, 0x00, 0x01, 0x36, 0x00, 0x00, 0x00, 0x00,
		0x40, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xff, 0xff, 0xff, 0xff
	};
	static const uint8_t TEST_ARRAY3[] = {
		0x01, 0x02, 0x01
	};

	ARRAY_CHECKSUM_TEST(TEST_ARRAY1, 0x79ac);
	ARRAY_CHECKSUM_TEST(TEST_ARRAY2, 0x79b8);
	ARRAY_CHECKSUM_TEST(TEST_ARRAY3, 0xfdfd);

	return NDM_TEST_RESULT;
}

