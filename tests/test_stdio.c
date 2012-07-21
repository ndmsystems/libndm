#include <stdlib.h>
#include <string.h>
#include <ndm/stdio.h>
#include "test.h"

int main()
{
	char *p;

	NDM_TEST_BREAK_IF(ndm_asprintf(&p, "%s", "test") <= 0 || p == NULL);
	NDM_TEST(strcmp("test", p) == 0);
	free(p);

	NDM_TEST_BREAK_IF(ndm_asprintf(&p,
		"%s %i", "test", 100) <= 0 || p == NULL);
	NDM_TEST(strcmp("test 100", p) == 0);
	free(p);

	NDM_TEST_BREAK_IF(ndm_asprintf(&p,
		"%s %i %c", "test", 100, 'a') <= 0 || p == NULL);
	NDM_TEST(strcmp("test 100 a", p) == 0);
	free(p);

	return NDM_TEST_RESULT;
}

