#include <stdlib.h>
#include <string.h>
#include <ndm/stdio.h>
#include "test.h"

#define BUFFER_SIZE		4

int main()
{
	char *p;
	char buffer[BUFFER_SIZE];

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

	NDM_TEST(ndm_absprintf(
		buffer, sizeof(buffer), &p, "%i", 100) == 3);
	NDM_TEST(p == buffer);
	NDM_TEST(strcmp(p, "100") == 0);

	NDM_TEST(ndm_absprintf(
		buffer, sizeof(buffer), &p, "%i", 1000) == 4);
	NDM_TEST(p != buffer);
	NDM_TEST(strcmp(p, "1000") == 0);
	free(p);

	return NDM_TEST_RESULT;
}

