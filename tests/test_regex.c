#include <stdlib.h>
#include <string.h>
#include <ndm/regex.h>
#include "test.h"

int main(int argc, char* argv[])
{
	{
		struct ndm_regex_t *reg = ndm_regex_alloc("abcd", REG_EXTENDED);
		struct ndm_regex_matcher_t *m = ndm_regex_matcher_alloc("abcd");

		NDM_TEST_BREAK_IF(reg == NULL);
		NDM_TEST_BREAK_IF(m == NULL);

		NDM_TEST(ndm_regex_flags(reg) == REG_EXTENDED);
		NDM_TEST(strcmp(ndm_regex_pattern(reg), "abcd") == 0);

		NDM_TEST(ndm_regex_matcher_find(m, reg, 0));
		NDM_TEST(ndm_regex_matcher_group_count(m) == 1);

		char *g0 = ndm_regex_matcher_group_get(m, 0);

		NDM_TEST_BREAK_IF(g0 == NULL);
		NDM_TEST(strcmp(g0, "abcd") == 0);

		free(g0);

		ndm_regex_matcher_free(&m);
		ndm_regex_free(&reg);
	}

	{
		struct ndm_regex_t *reg = ndm_regex_alloc("^abcd", REG_EXTENDED);
		struct ndm_regex_matcher_t *m = ndm_regex_matcher_alloc("xabcd");

		NDM_TEST_BREAK_IF(reg == NULL);
		NDM_TEST_BREAK_IF(m == NULL);

		NDM_TEST(ndm_regex_flags(reg) == REG_EXTENDED);
		NDM_TEST(strcmp(ndm_regex_pattern(reg), "^abcd") == 0);

		NDM_TEST(!ndm_regex_matcher_find(m, reg, 0));
		NDM_TEST( ndm_regex_matcher_group_count(m) == 0);

		char *g0 = ndm_regex_matcher_group_get(m, 0);

		NDM_TEST_BREAK_IF(g0 != NULL);

		free(g0);

		ndm_regex_matcher_free(&m);
		ndm_regex_free(&reg);
	}

	{
		struct ndm_regex_t *reg =
			ndm_regex_alloc("(abcd|efgh)", REG_EXTENDED);
		struct ndm_regex_matcher_t *m =
			ndm_regex_matcher_alloc("xabcd");

		NDM_TEST_BREAK_IF(reg == NULL);
		NDM_TEST_BREAK_IF(m == NULL);

		NDM_TEST(ndm_regex_matcher_find(m, reg, 0));
		NDM_TEST(ndm_regex_matcher_group_count(m) == 2);

		char *g0 = ndm_regex_matcher_group_get(m, 0);
		char *g1 = ndm_regex_matcher_group_get(m, 1);

		NDM_TEST_BREAK_IF(g0 == NULL);
		NDM_TEST_BREAK_IF(g1 == NULL);

		NDM_TEST(strcmp(g0, "abcd") == 0);
		NDM_TEST(strcmp(g1, "abcd") == 0);

		free(g0);
		free(g1);

		ndm_regex_matcher_free(&m);
		ndm_regex_free(&reg);
	}

	{
		struct ndm_regex_t *reg =
			ndm_regex_alloc("((abcd)|(efgh))", REG_EXTENDED);
		struct ndm_regex_matcher_t *m =
			ndm_regex_matcher_alloc("efgh");

		NDM_TEST_BREAK_IF(reg == NULL);
		NDM_TEST_BREAK_IF(m == NULL);

		NDM_TEST(ndm_regex_matcher_find(m, reg, 0));
		NDM_TEST(ndm_regex_matcher_group_count(m) == 4);

		char *g0 = ndm_regex_matcher_group_get(m, 0);
		char *g1 = ndm_regex_matcher_group_get(m, 1);
		char *g2 = ndm_regex_matcher_group_get(m, 2);
		char *g3 = ndm_regex_matcher_group_get(m, 3);

		NDM_TEST_BREAK_IF(g0 == NULL);
		NDM_TEST_BREAK_IF(g1 == NULL);
		NDM_TEST_BREAK_IF(g2 == NULL);
		NDM_TEST_BREAK_IF(g3 == NULL);

		NDM_TEST(strcmp(g0, "efgh") == 0);
		NDM_TEST(strcmp(g1, "efgh") == 0);
		NDM_TEST(strcmp(g2, "") == 0);
		NDM_TEST(strcmp(g3, "efgh") == 0);

		free(g0);
		free(g1);
		free(g2);
		free(g3);

		ndm_regex_matcher_free(&m);
		ndm_regex_free(&reg);
	}

	return NDM_TEST_RESULT;
}
