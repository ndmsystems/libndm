#include <string.h>
#include <ndm/stracc.h>
#include "test.h"

int main()
{
	struct ndm_stracc_t a = NDM_STRACC_INITIALIZER;
	struct ndm_stracc_t b;
	const char *s = NULL;

	ndm_stracc_init(&b);
	NDM_TEST(ndm_stracc_is_empty(&b));

	NDM_TEST(ndm_stracc_is_empty(&a));
	ndm_stracc_append(&a, "test %i\n", 1);
	NDM_TEST(!ndm_stracc_is_equal(&a, &b));
	NDM_TEST(!ndm_stracc_is_empty(&a));
	NDM_TEST(ndm_stracc_is_valid(&a));
	NDM_TEST(ndm_stracc_is_valid(&b));
	NDM_TEST(ndm_stracc_size(&a) == 7);
	NDM_TEST(ndm_stracc_size(&b) == 0);
	NDM_TEST(strcmp(ndm_stracc_value(&a), "test 1\n") == 0);

	ndm_stracc_append(&a, "test %i\n", 2);
	NDM_TEST(ndm_stracc_is_valid(&a));
	NDM_TEST(ndm_stracc_size(&a) == 14);
	NDM_TEST(strcmp(ndm_stracc_value(&a), "test 1\ntest 2\n") == 0);

	NDM_TEST(ndm_stracc_assign(&b, &a));
	NDM_TEST(ndm_stracc_is_valid(&b));
	NDM_TEST(ndm_stracc_is_equal(&a, &b));

	ndm_stracc_clear(&a);
	NDM_TEST(ndm_stracc_is_valid(&a));
	NDM_TEST(!ndm_stracc_is_equal(&a, &b));

	ndm_stracc_append(&a, "test0%c", '\0');
	NDM_TEST(ndm_stracc_is_valid(&a));
	NDM_TEST(ndm_stracc_size(&a) == 6);

	ndm_stracc_append(&a, "test1%c", '\0');
	NDM_TEST(ndm_stracc_is_valid(&a));
	NDM_TEST(ndm_stracc_size(&a) == 12);

	ndm_stracc_swap(&a, &b);
	NDM_TEST(ndm_stracc_is_valid(&a));
	NDM_TEST(ndm_stracc_is_valid(&b));
	NDM_TEST(!ndm_stracc_is_equal(&a, &b));

	s = ndm_stracc_value(&a);
	NDM_TEST(ndm_stracc_next_cstr(&a, &s) == NULL);

	s = ndm_stracc_value(&b);
	NDM_TEST(strcmp(s, "test0") == 0);
	NDM_TEST_BREAK_IF(ndm_stracc_next_cstr(&b, &s) == NULL);
	NDM_TEST(strcmp(s, "test1") == 0);
	NDM_TEST(ndm_stracc_next_cstr(&b, &s) == NULL);

	ndm_stracc_clear(&a);

	ndm_stracc_append(&a, "test0%c", '\0');
	NDM_TEST(ndm_stracc_is_valid(&a));
	NDM_TEST(ndm_stracc_size(&a) == 6);

	ndm_stracc_append(&a, "test1");
	NDM_TEST(ndm_stracc_is_valid(&a));
	NDM_TEST(ndm_stracc_size(&a) == 11);

	s = ndm_stracc_value(&a);
	NDM_TEST(strcmp(s, "test0") == 0);
	NDM_TEST_BREAK_IF(ndm_stracc_next_cstr(&a, &s) == NULL);
	NDM_TEST(strcmp(s, "test1") == 0);
	NDM_TEST(ndm_stracc_next_cstr(&a, &s) == NULL);

	ndm_stracc_clear(&a);
	ndm_stracc_clear(&b);

	return NDM_TEST_RESULT;
}

