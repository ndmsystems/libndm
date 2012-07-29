#include <string.h>
#include <ndm/strvec.h>
#include "test.h"

static bool __array_is_valid(const struct ndm_strvec_t *v)
{
	const char **p = ndm_strvec_array(v);
	size_t s = ndm_strvec_size(v);
	size_t i = 0;

	while (i < s && strcmp(p[i], ndm_strvec_at(v, i)) == 0) {
		++i;
	}

	return (i == s && p[i] == NULL) ? true : false;
}

int main()
{
	struct ndm_strvec_t r;
	struct ndm_strvec_t v = NDM_STRVEC_INIT;
	const char *array[] = {"_test", "test0", "test1", "test2", NULL};

	NDM_TEST(ndm_strvec_is_empty(&v));

	ndm_strvec_init(&r);
	NDM_TEST(__array_is_valid(&v));

	NDM_TEST(ndm_strvec_is_empty(&r));
	NDM_TEST(__array_is_valid(&v));

	NDM_TEST(ndm_strvec_push_back(&v, "test0") != NULL);
	NDM_TEST(ndm_strvec_size(&v) == 1);
	NDM_TEST(__array_is_valid(&v));

	NDM_TEST(!ndm_strvec_is_empty(&v));

	NDM_TEST(ndm_strvec_insert_at(&v, 1, "test1") != NULL);
	NDM_TEST(ndm_strvec_size(&v) == 2);

	NDM_TEST(ndm_strvec_insert_at(&v, 2, "test2") != NULL);
	NDM_TEST(ndm_strvec_size(&v) == 3);

	NDM_TEST(ndm_strvec_find(&v, "test0") == 0);
	NDM_TEST(ndm_strvec_find(&v, "test1") == 1);
	NDM_TEST(ndm_strvec_find(&v, "test2") == 2);
	NDM_TEST(__array_is_valid(&v));

	NDM_TEST(strcmp("test0",
		ndm_strvec_at(&v, ndm_strvec_find(&v, "test0"))) == 0);
	NDM_TEST(strcmp("test1",
		ndm_strvec_at(&v, ndm_strvec_find(&v, "test1"))) == 0);
	NDM_TEST(strcmp("test2",
		ndm_strvec_at(&v, ndm_strvec_find(&v, "test2"))) == 0);

	NDM_TEST(strcmp("test0", ndm_strvec_front(&v)) == 0);
	NDM_TEST(strcmp("test2", ndm_strvec_back(&v)) == 0);
	NDM_TEST(__array_is_valid(&v));

	NDM_TEST(ndm_strvec_insert_at(&v, 1, "_test") != NULL);

	NDM_TEST(strcmp("test0", ndm_strvec_at(&v, 0)) == 0);
	NDM_TEST(strcmp("_test", ndm_strvec_at(&v, 1)) == 0);
	NDM_TEST(strcmp("test1", ndm_strvec_at(&v, 2)) == 0);
	NDM_TEST(strcmp("test2", ndm_strvec_at(&v, 3)) == 0);
	NDM_TEST(__array_is_valid(&v));

	ndm_strvec_sort_descending(&v);

	NDM_TEST(strcmp("test2", ndm_strvec_at(&v, 0)) == 0);
	NDM_TEST(strcmp("test1", ndm_strvec_at(&v, 1)) == 0);
	NDM_TEST(strcmp("test0", ndm_strvec_at(&v, 2)) == 0);
	NDM_TEST(strcmp("_test", ndm_strvec_at(&v, 3)) == 0);
	NDM_TEST(__array_is_valid(&v));

	ndm_strvec_sort_ascending(&v);

	NDM_TEST(strcmp("_test", ndm_strvec_at(&v, 0)) == 0);
	NDM_TEST(strcmp("test0", ndm_strvec_at(&v, 1)) == 0);
	NDM_TEST(strcmp("test1", ndm_strvec_at(&v, 2)) == 0);
	NDM_TEST(strcmp("test2", ndm_strvec_at(&v, 3)) == 0);
	NDM_TEST(__array_is_valid(&v));

	NDM_TEST_BREAK_IF(!ndm_strvec_assign(&r, &v));
	NDM_TEST(ndm_strvec_is_equal(&r, &v));

	ndm_strvec_remove_at(&v, 2);

	NDM_TEST(ndm_strvec_size(&v) == 3);
	NDM_TEST(strcmp("_test", ndm_strvec_at(&v, 0)) == 0);
	NDM_TEST(strcmp("test0", ndm_strvec_at(&v, 1)) == 0);
	NDM_TEST(strcmp("test2", ndm_strvec_at(&v, 2)) == 0);
	NDM_TEST(!ndm_strvec_is_equal(&r, &v));
	NDM_TEST(__array_is_valid(&v));

	ndm_strvec_pop_back(&v);

	NDM_TEST(ndm_strvec_size(&v) == 2);
	NDM_TEST(strcmp("_test", ndm_strvec_at(&v, 0)) == 0);
	NDM_TEST(strcmp("test0", ndm_strvec_at(&v, 1)) == 0);
	NDM_TEST(!ndm_strvec_is_equal(&r, &v));
	NDM_TEST(__array_is_valid(&v));

	ndm_strvec_pop_front(&v);

	NDM_TEST(!ndm_strvec_is_equal(&r, &v));
	NDM_TEST(ndm_strvec_size(&v) == 1);
	NDM_TEST(strcmp("test0", ndm_strvec_at(&v, 0)) == 0);
	NDM_TEST(__array_is_valid(&v));

	ndm_strvec_remove_at(&v, 0);

	NDM_TEST(__array_is_valid(&v));
	NDM_TEST(ndm_strvec_is_empty(&v));
	NDM_TEST(!ndm_strvec_is_equal(&r, &v));
	NDM_TEST_BREAK_IF(!ndm_strvec_assign_array(&v, array));
	NDM_TEST(__array_is_valid(&v));
	NDM_TEST(ndm_strvec_is_equal(&r, &v));

	NDM_TEST(ndm_strvec_append(&r, &v));
	NDM_TEST(!ndm_strvec_is_equal(&r, &v));
	NDM_TEST(ndm_strvec_size(&r) == 2*ndm_strvec_size(&v));

	ndm_strvec_truncate(&r, ndm_strvec_size(&v));
	NDM_TEST(ndm_strvec_is_equal(&r, &v));
	NDM_TEST(__array_is_valid(&v));

	ndm_strvec_truncate(&r, 2*ndm_strvec_size(&v));
	NDM_TEST(ndm_strvec_is_equal(&r, &v));
	NDM_TEST(__array_is_valid(&v));

	ndm_strvec_clear(&v);
	ndm_strvec_clear(&r);

	return NDM_TEST_RESULT;
}

