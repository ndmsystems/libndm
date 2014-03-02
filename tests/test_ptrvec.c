#include <ndm/ptrvec.h>
#include "test.h"

#include <stdio.h>

static int ptr_comp_(const void *l, const void *r)
{
	const void *lp = *((void **) l);
	const void *rp = *((void **) r);

	return
		lp > rp ?  1 :
		lp < rp ? -1 : 0;
}

int main()
{
	struct ndm_ptrvec_t v = NDM_PTRVEC_INITIALIZER;
	struct ndm_ptrvec_t u;

	ndm_ptrvec_init(&u);

	NDM_TEST(ndm_ptrvec_size(&u) == 0);
	NDM_TEST(ndm_ptrvec_is_empty(&u));
	NDM_TEST(ndm_ptrvec_ptr(&u) == NULL);

	NDM_TEST(ndm_ptrvec_size(&v) == 0);
	NDM_TEST(ndm_ptrvec_is_empty(&v));
	NDM_TEST(ndm_ptrvec_ptr(&v) == NULL);

	NDM_TEST_BREAK_IF(!ndm_ptrvec_push_back(&v, NULL));

	NDM_TEST(ndm_ptrvec_size(&v) == 1);
	NDM_TEST(!ndm_ptrvec_is_empty(&v));
	NDM_TEST(ndm_ptrvec_ptr(&v) != NULL);
	NDM_TEST(ndm_ptrvec_at(&v, 0) == NULL);

	ndm_ptrvec_set(&v, 0, (void *) 1);
	NDM_TEST(ndm_ptrvec_at(&v, 0) == (void *) 1);

	NDM_TEST_BREAK_IF(!ndm_ptrvec_push_front(&v, (void *) 2));
	NDM_TEST(ndm_ptrvec_size(&v) == 2);
	NDM_TEST(ndm_ptrvec_at(&v, 0) == (void *) 2);
	NDM_TEST(ndm_ptrvec_at(&v, 1) == (void *) 1);

	NDM_TEST_BREAK_IF(!ndm_ptrvec_insert(&v, 1, (void *) 3));
	NDM_TEST(ndm_ptrvec_at(&v, 0) == (void *) 2);
	NDM_TEST(ndm_ptrvec_at(&v, 1) == (void *) 3);
	NDM_TEST(ndm_ptrvec_at(&v, 2) == (void *) 1);

	NDM_TEST_BREAK_IF(!ndm_ptrvec_push_back(&v, (void *) 4));
	NDM_TEST(ndm_ptrvec_size(&v) == 4);
	NDM_TEST(ndm_ptrvec_at(&v, 0) == (void *) 2);
	NDM_TEST(ndm_ptrvec_at(&v, 1) == (void *) 3);
	NDM_TEST(ndm_ptrvec_at(&v, 2) == (void *) 1);
	NDM_TEST(ndm_ptrvec_at(&v, 3) == (void *) 4);

	ndm_ptrvec_sort(&v, ptr_comp_);

	NDM_TEST(ndm_ptrvec_at(&v, 0) == (void *) 1);
	NDM_TEST(ndm_ptrvec_at(&v, 1) == (void *) 2);
	NDM_TEST(ndm_ptrvec_at(&v, 2) == (void *) 3);
	NDM_TEST(ndm_ptrvec_at(&v, 3) == (void *) 4);

	ndm_ptrvec_set(&v, 0, (void *) 4);
	ndm_ptrvec_set(&v, 3, (void *) 1);

	ndm_ptrvec_bubble_sort(&v, ptr_comp_);

	NDM_TEST(ndm_ptrvec_at(&v, 0) == (void *) 1);
	NDM_TEST(ndm_ptrvec_at(&v, 1) == (void *) 2);
	NDM_TEST(ndm_ptrvec_at(&v, 2) == (void *) 3);
	NDM_TEST(ndm_ptrvec_at(&v, 3) == (void *) 4);

	NDM_TEST(ndm_ptrvec_find(&v, (void *) 1) == 0);
	NDM_TEST(ndm_ptrvec_find(&v, (void *) 2) == 1);
	NDM_TEST(ndm_ptrvec_find(&v, (void *) 3) == 2);
	NDM_TEST(ndm_ptrvec_find(&v, (void *) 4) == 3);
	NDM_TEST(ndm_ptrvec_find(&v, (void *) 0) == 4);

	ndm_ptrvec_rotate_left(&v);
	NDM_TEST(ndm_ptrvec_at(&v, 0) == (void *) 2);
	NDM_TEST(ndm_ptrvec_at(&v, 1) == (void *) 3);
	NDM_TEST(ndm_ptrvec_at(&v, 2) == (void *) 4);
	NDM_TEST(ndm_ptrvec_at(&v, 3) == (void *) 1);

	NDM_TEST_BREAK_IF(!ndm_ptrvec_push_back(&u, NULL));
	NDM_TEST(ndm_ptrvec_at(&u, 0) == NULL);

	NDM_TEST_BREAK_IF(!ndm_ptrvec_assign(&u, &v));
	NDM_TEST(ndm_ptrvec_size(&u) == 4);
	NDM_TEST(ndm_ptrvec_at(&u, 0) == (void *) 2);
	NDM_TEST(ndm_ptrvec_at(&u, 1) == (void *) 3);
	NDM_TEST(ndm_ptrvec_at(&u, 2) == (void *) 4);
	NDM_TEST(ndm_ptrvec_at(&u, 3) == (void *) 1);

	ndm_ptrvec_pop_back(&v);
	NDM_TEST(ndm_ptrvec_size(&v) == 3);
	NDM_TEST(ndm_ptrvec_at(&v, 0) == (void *) 2);
	NDM_TEST(ndm_ptrvec_at(&v, 1) == (void *) 3);
	NDM_TEST(ndm_ptrvec_at(&v, 2) == (void *) 4);

	ndm_ptrvec_pop_front(&v);
	NDM_TEST(ndm_ptrvec_size(&v) == 2);
	NDM_TEST(ndm_ptrvec_at(&v, 0) == (void *) 3);
	NDM_TEST(ndm_ptrvec_at(&v, 1) == (void *) 4);

	ndm_ptrvec_swap(&u, &v);
	NDM_TEST(ndm_ptrvec_size(&u) == 2);
	NDM_TEST(ndm_ptrvec_at(&u, 0) == (void *) 3);
	NDM_TEST(ndm_ptrvec_at(&u, 1) == (void *) 4);
	NDM_TEST(ndm_ptrvec_size(&v) == 4);
	NDM_TEST(ndm_ptrvec_at(&v, 0) == (void *) 2);
	NDM_TEST(ndm_ptrvec_at(&v, 1) == (void *) 3);
	NDM_TEST(ndm_ptrvec_at(&v, 2) == (void *) 4);
	NDM_TEST(ndm_ptrvec_at(&v, 3) == (void *) 1);

	ndm_ptrvec_clear(&v);

	NDM_TEST(ndm_ptrvec_size(&v) == 0);
	NDM_TEST(ndm_ptrvec_is_empty(&v));
	NDM_TEST(ndm_ptrvec_ptr(&v) == NULL);

	ndm_ptrvec_clear(&u);

	NDM_TEST(ndm_ptrvec_size(&u) == 0);
	NDM_TEST(ndm_ptrvec_is_empty(&u));
	NDM_TEST(ndm_ptrvec_ptr(&u) == NULL);

	return NDM_TEST_RESULT;
}
