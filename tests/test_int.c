#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <ndm/int.h>
#include <ndm/macro.h>
#include "test.h"

#define TEST_INT_STRLEN(t) 											\
	do {															\
		char buf[sizeof(t)*CHAR_BIT];								\
		const int size = (NDM_INT_IS_SIGNED(t)) ?					\
			snprintf(buf, sizeof(buf), "%lli",						\
				(long long int) NDM_INT_MIN(t)) :					\
			snprintf(buf, sizeof(buf), "%llu",						\
				(unsigned long long int) NDM_INT_MIN(t));			\
																	\
		NDM_TEST_BREAK_IF(size < 0);								\
		NDM_TEST(NDM_INT_MAX_STRLEN(t) >= size);					\
	} while (false);

int main()
{
	char lb[NDM_INT_BUFSIZE];
	long l;
	unsigned long ul;

	NDM_TEST(NDM_INT_MIN(char) == CHAR_MIN);
	NDM_TEST(NDM_INT_MAX(char) == CHAR_MAX);

	NDM_TEST(NDM_INT_MIN(long) == LONG_MIN);
	NDM_TEST(NDM_INT_MAX(long) == LONG_MAX);

	NDM_TEST(
		NDM_INT_IS_SIGNED(time_t) ?
			(NDM_INT_MAX(time_t) <= LLONG_MAX) :
			(NDM_INT_MAX(time_t) <= ULLONG_MAX));

	TEST_INT_STRLEN(char);
	TEST_INT_STRLEN(unsigned char);
	TEST_INT_STRLEN(short);
	TEST_INT_STRLEN(unsigned short);
	TEST_INT_STRLEN(int);
	TEST_INT_STRLEN(unsigned int);
	TEST_INT_STRLEN(long int);
	TEST_INT_STRLEN(unsigned long int);

	TEST_INT_STRLEN(int64_t);
	TEST_INT_STRLEN(uint64_t);
	TEST_INT_STRLEN(intmax_t);
	TEST_INT_STRLEN(uintmax_t);

	NDM_TEST(!ndm_int_parse_long("", &l));
	NDM_TEST(!ndm_int_parse_long(" ", &l));
	NDM_TEST(!ndm_int_parse_long("0 ", &l));
	NDM_TEST(!ndm_int_parse_long(" 0", &l));
	NDM_TEST(!ndm_int_parse_long("999999999999999999999999999999999", &l));

	NDM_TEST(ndm_int_parse_long("1", &l) && l == 1);
	NDM_TEST(ndm_int_parse_long("-1", &l) && l == -1);

	NDM_TEST_BREAK_IF(snprintf(lb, sizeof(lb), "%li", LONG_MAX) <= 0);
	NDM_TEST(ndm_int_parse_long(lb, &l) && l == LONG_MAX);

	NDM_TEST_BREAK_IF(snprintf(lb, sizeof(lb), "%li", LONG_MIN) <= 0);
	NDM_TEST(ndm_int_parse_long(lb, &l) && l == LONG_MIN);

	NDM_TEST(!ndm_int_parse_ulong("", &ul));
	NDM_TEST(!ndm_int_parse_ulong(" ", &ul));
	NDM_TEST(!ndm_int_parse_ulong("0 ", &ul));
	NDM_TEST(!ndm_int_parse_ulong(" 0", &ul));
	NDM_TEST(!ndm_int_parse_ulong("999999999999999999999999999999999", &ul));
	NDM_TEST(!ndm_int_parse_ulong("-1", &ul));

	NDM_TEST(ndm_int_parse_ulong("1", &ul) && ul == 1);

	NDM_TEST_BREAK_IF(snprintf(lb, sizeof(lb), "%lu", ULONG_MAX) <= 0);
	NDM_TEST(ndm_int_parse_ulong(lb, &ul) && ul == ULONG_MAX);

	return NDM_TEST_RESULT;
}

