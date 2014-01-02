#include <stdlib.h>
#include <string.h>
#include <ndm/dlist.h>
#include <ndm/macro.h>
#include "test.h"

struct entry_t
{
	struct ndm_dlist_entry_t list;
	int value;
};

int main()
{
	static NDM_DLIST_HEAD(shead);
	struct ndm_dlist_entry_t head = NDM_DLIST_INITIALIZER(head);
	struct ndm_dlist_entry_t h;
	struct entry_t *e, *p;

	ndm_dlist_init(&h);

	NDM_TEST(ndm_dlist_is_empty(&h));
	NDM_TEST(ndm_dlist_is_empty(&head));
	NDM_TEST(ndm_dlist_is_empty(&shead));
	NDM_TEST(ndm_dlist_size(&h) == 0);

	NDM_TEST_BREAK_IF((e = malloc(sizeof(*e))) == NULL);

	e->value = 0;

	ndm_dlist_insert_after(&h, &e->list);
	NDM_TEST(!ndm_dlist_is_empty(&h));

	NDM_TEST_BREAK_IF((p = malloc(sizeof(*e))) == NULL);

	p->value = 1;

	ndm_dlist_insert_after(&e->list, &p->list);
	NDM_TEST(!ndm_dlist_is_empty(&h));

	e = p;

	NDM_TEST_BREAK_IF((p = malloc(sizeof(*e))) == NULL);

	p->value = 2;

	ndm_dlist_insert_after(&e->list, &p->list);
	NDM_TEST(!ndm_dlist_is_empty(&h));
	NDM_TEST(ndm_dlist_size(&h) == 3);

	int i = 0;

	ndm_dlist_foreach_entry(e, struct entry_t, list, &h) {
		NDM_TEST(e->value == i++);
	}

	ndm_dlist_foreach_entry_safe(e, struct entry_t, list, &h, p) {
		ndm_dlist_remove(&e->list);
		free(e);
	}

	NDM_TEST(ndm_dlist_is_empty(&h));

	i = 0;

	NDM_TEST_BREAK_IF((e = malloc(sizeof(*e))) == NULL);

	e->value = i++;

	ndm_dlist_insert_before(&h, &e->list);
	NDM_TEST(!ndm_dlist_is_empty(&h));

	NDM_TEST_BREAK_IF((p = malloc(sizeof(*e))) == NULL);

	p->value = i++;

	ndm_dlist_insert_before(&e->list, &p->list);
	NDM_TEST(!ndm_dlist_is_empty(&h));

	e = p;

	NDM_TEST_BREAK_IF((p = malloc(sizeof(*e))) == NULL);

	p->value = i++;

	ndm_dlist_insert_before(&e->list, &p->list);
	NDM_TEST(!ndm_dlist_is_empty(&h));

	NDM_TEST(p == ndm_dlist_entry(&p->list, struct entry_t, list));

	ndm_dlist_foreach_entry(e, struct entry_t, list, &h) {
		NDM_TEST(e->value == --i);
	}

	ndm_dlist_foreach_entry_safe(e, struct entry_t, list, &h, p) {
		ndm_dlist_remove(&e->list);
		free(e);
	}

	NDM_TEST(ndm_dlist_is_empty(&h));

	return NDM_TEST_RESULT;
}

