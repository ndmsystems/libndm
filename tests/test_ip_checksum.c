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

int main()
{
	uint8_t test_array1[] = {
		0x45,
		0x00,
		0x01,
		0x42,
		0x00,
		0x00,
		0x00,
		0x00,
		0x40,
		0x11,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0xff,
		0xff,
		0xff,
		0xff
	};
	uint8_t test_array2[] = {
		0x45,
		0x00,
		0x01,
		0x36,
		0x00,
		0x00,
		0x00,
		0x00,
		0x40,
		0x11,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0xff,
		0xff,
		0xff,
		0xff
	};

	uint32_t sum;

	sum = ndm_ip_checksum(test_array1,  sizeof(test_array1));
	NDM_TEST(sum == 0x0000ac79);

	sum = ndm_ip_checksum(test_array2,  sizeof(test_array2));
	NDM_TEST(sum == 0x0000b879);

	return NDM_TEST_RESULT;
}

