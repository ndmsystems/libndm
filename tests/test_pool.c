#include <stdlib.h>
#include <string.h>
#include <ndm/pool.h>
#include "test.h"

#define ALLOC_SIZE			1000
#define STATIC_SIZE			120
#define DYNAMIC_SIZE		30
#define TEST_STRING			"test_string_for_ndm_pool_strdup"
#define TEST_STRING2		"test_string_for_ndm_pool_strdup2"

int main()
{
	char *s = NULL;
	size_t i = 0;
	char buf[STATIC_SIZE];
	struct ndm_pool_t pool = NDM_POOL_INITIALIZER(
		buf, sizeof(buf), DYNAMIC_SIZE);

	NDM_TEST(ndm_pool_is_valid(&pool));
	NDM_TEST(ndm_pool_allocated(&pool) == 0);
	NDM_TEST(ndm_pool_total_dynamic_size(&pool) == 0);

	for (i = 0; i < ALLOC_SIZE; i++) {
		NDM_TEST_BREAK_IF(ndm_pool_malloc(&pool, 1) == NULL);
	}

	NDM_TEST(ndm_pool_allocated(&pool) == ALLOC_SIZE);
	NDM_TEST_BREAK_IF((s = ndm_pool_strdup(&pool, TEST_STRING)) == NULL);
	NDM_TEST(strcmp(s, TEST_STRING) == 0);

	NDM_TEST_BREAK_IF((s = ndm_pool_strdup(&pool, TEST_STRING2)) == NULL);
	NDM_TEST(strcmp(s, TEST_STRING2) == 0);

	ndm_pool_clear(&pool);
	NDM_TEST(ndm_pool_allocated(&pool) == 0);

	for (i = 0; i < 10000; i++) {
		const size_t n = (size_t) (rand() % (2*DYNAMIC_SIZE));
		void *p = ndm_pool_malloc(&pool, n);

		NDM_TEST_BREAK_IF(p == NULL);
		memset(p, (char) i, n);
	}

	ndm_pool_clear(&pool);
	NDM_TEST(ndm_pool_allocated(&pool) == 0);
	NDM_TEST(ndm_pool_total_dynamic_size(&pool) == 0);

	return NDM_TEST_RESULT;
}

