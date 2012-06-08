#include "ndm_common.h"

extern char __error_message[ERROR_MSG_MAX_LENGTH + 1];

uint32_t ndm_get_random()
{
	srand(__get_monotonic_us());
	
	return rand();
}

char *ndm_strerror(const int error)
{
	const char *message = strerror(error);

	*__error_message = '\0';

	if (message == NULL) {
		snprintf(
			__error_message, sizeof(__error_message),
			"error %i occured", error);
	} else {
		snprintf(__error_message, sizeof(__error_message), "%s", message);

		if (strlen(message) > sizeof(__error_message) - 1) {
			snprintf(
				__error_message + sizeof(__error_message) - sizeof(DOTS) - 1,
				sizeof(DOTS), "%s", DOTS);
		}

		if (!isupper(__error_message[1])) {
			/* ignore abbreviations */

			*__error_message = tolower(*__error_message);
		}
	}

	return __error_message;
}

int ndm_file_exist(const char *const file_name, const int mode)
{
	int ret_code = 0;
	
	if (access(file_name,mode) == -1) {
		fprintf(stderr,"File \"%s\" invalid: %s.\n",
			file_name, ndm_strerror(errno));
	}
	else ret_code = 1;
	
	return ret_code;
}

uint32_t ndmUtil_calculate_checksum(
		const void *data,
		const unsigned long octet_count,
		uint32_t initial)
{
	uint32_t s = initial;
	uint16_t *words = (uint16_t *) data;
	unsigned long i = octet_count;

	while (i > 1) {
		s += *words++;
		i -= 2;
	}

	if (i == 1) {
		s += (*(uint8_t *) words);
	}

	while ((s & 0xffff0000) != 0) {
		s = (s & 0x0000ffff) + ((s >> 16) & 0x0000fffff);
	}

	return (uint16_t) ~s;
}

