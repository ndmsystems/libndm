#include <string.h>
#include <ndm/stracc.h>
#include "test.h"

int main()
{
	struct ndm_stracc_t *acc = ndm_stracc_alloc();

	NDM_TEST_BREAK_IF(acc == NULL);

	ndm_stracc_append(acc, "test %i\n", 1);
	NDM_TEST(ndm_stracc_is_valid(acc));
	NDM_TEST(ndm_stracc_size(acc) == 7);
	NDM_TEST(strcmp(ndm_stracc_value(acc), "test 1\n") == 0);

	ndm_stracc_append(acc, "test %i\n", 2);
	NDM_TEST(ndm_stracc_is_valid(acc));
	NDM_TEST(ndm_stracc_size(acc) == 14);
	NDM_TEST(strcmp(ndm_stracc_value(acc), "test 1\ntest 2\n") == 0);

	ndm_stracc_free(&acc);

	NDM_TEST_BREAK_IF((acc = ndm_stracc_alloc()) == NULL);

	ndm_stracc_append(acc, "test%c", '\0');
	NDM_TEST(ndm_stracc_is_valid(acc));
	NDM_TEST(ndm_stracc_size(acc) == 5);

	ndm_stracc_append(acc, "test%c", '\0');
	NDM_TEST(ndm_stracc_is_valid(acc));
	NDM_TEST(ndm_stracc_size(acc) == 10);

	ndm_stracc_free(&acc);

	return NDM_TEST_RESULT;
}
