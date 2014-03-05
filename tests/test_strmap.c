#include <ndm/strmap.h>
#include "test.h"

int main()
{
	struct ndm_strmap_t map = NDM_STRMAP_INITIALIZER_DEFAULT;
	struct ndm_strmap_t imap =
		NDM_STRMAP_INITIALIZER(
			NDM_STRMAP_FLAGS_CASE_INSENSITIVE);

	NDM_TEST(ndm_strmap_flags(&map) == NDM_STRMAP_FLAGS_DEFAULT);
	NDM_TEST(ndm_strmap_size(&map) == 0);
	NDM_TEST(ndm_strmap_is_empty(&map));

	NDM_TEST(ndm_strmap_set(&map, "key", "value"));
	NDM_TEST(ndm_strmap_size(&map) == 1);
	NDM_TEST(!ndm_strmap_is_empty(&map));

	NDM_TEST(strcmp(ndm_strmap_get_by_index(&map, 0), "value") == 0);
	NDM_TEST(ndm_strmap_find(&map, "key") == 0);

	NDM_TEST(strcmp(ndm_strmap_get(&map, "key"), "value") == 0);
	NDM_TEST(strcmp(ndm_strmap_get(&map, "key"), "") != 0);
	NDM_TEST(strcmp(ndm_strmap_get(&map, "key_"), "value") != 0);
	NDM_TEST(strcmp(ndm_strmap_get(&map, "key_"), "") == 0);

	NDM_TEST(ndm_strmap_has(&map, "key"));
	NDM_TEST(!ndm_strmap_has(&map, "key_"));

	NDM_TEST(ndm_strmap_nhas(&map, "key_", 3));
	NDM_TEST(!ndm_strmap_nhas(&map, "key", 2));
	NDM_TEST(!ndm_strmap_nhas(&map, "ke", 1));

	NDM_TEST(ndm_strmap_set(&map, "key", "value1"));
	NDM_TEST(ndm_strmap_set(&map, "key2", "value2"));
	NDM_TEST(ndm_strmap_set(&map, "key3", "value3"));
	NDM_TEST(strcmp(ndm_strmap_get(&map, "key"), "value1") == 0);
	NDM_TEST(strcmp(ndm_strmap_get(&map, "key2"), "value2") == 0);
	NDM_TEST(strcmp(ndm_strmap_get(&map, "key3"), "value3") == 0);
	NDM_TEST(ndm_strmap_nfind(&map, "key2", 4) == 1);
	NDM_TEST(ndm_strmap_nfind(&map, "kez2", 3) == 3);

	NDM_TEST(strcmp(ndm_strmap_get_key(&map, 0), "key") == 0);
	NDM_TEST(strcmp(ndm_strmap_get_key(&map, 1), "key2") == 0);
	NDM_TEST(strcmp(ndm_strmap_get_key(&map, 2), "key3") == 0);

	NDM_TEST(ndm_strmap_set(&imap, "Key", "Value"));
	NDM_TEST(ndm_strmap_has(&imap, "Key"));
	NDM_TEST(ndm_strmap_has(&imap, "key"));
	NDM_TEST(ndm_strmap_has(&imap, "keY"));
	NDM_TEST(ndm_strmap_has(&imap, "KEY"));
	NDM_TEST(ndm_strmap_set(&imap, "K2", "Value2"));
	NDM_TEST(ndm_strmap_set(&imap, "K3", "Value3"));
	NDM_TEST(ndm_strmap_nhas(&imap, "KEY2", 3));
	NDM_TEST(ndm_strmap_nhas(&imap, "KEy2", 3));
	NDM_TEST(ndm_strmap_nhas(&imap, "KeY2", 3));
	NDM_TEST(!ndm_strmap_nhas(&imap, "KE2", 3));
	NDM_TEST(!ndm_strmap_nhas(&imap, "KE2", 1));

	ndm_strmap_swap(&map, &imap);
	NDM_TEST(ndm_strmap_size(&map) == 3);
	NDM_TEST(strcmp(ndm_strmap_get(&map, "key"), "Value") == 0);
	NDM_TEST(strcmp(ndm_strmap_get(&map, "K2"), "Value2") == 0);
	NDM_TEST(strcmp(ndm_strmap_get(&map, "K3"), "Value3") == 0);
	NDM_TEST(ndm_strmap_flags(&map) == NDM_STRMAP_FLAGS_CASE_INSENSITIVE);
	NDM_TEST(ndm_strmap_size(&imap) == 3);
	NDM_TEST(strcmp(ndm_strmap_get(&imap, "key"), "value1") == 0);
	NDM_TEST(strcmp(ndm_strmap_get(&imap, "key2"), "value2") == 0);
	NDM_TEST(strcmp(ndm_strmap_get(&imap, "key3"), "value3") == 0);
	NDM_TEST(ndm_strmap_flags(&imap) == NDM_STRMAP_FLAGS_DEFAULT);
	ndm_strmap_swap(&map, &imap);

	NDM_TEST(ndm_strmap_remove(&imap, "K2"));
	NDM_TEST(ndm_strmap_size(&imap) == 2);
	NDM_TEST(strcmp(ndm_strmap_get(&imap, "key"), "Value") == 0);
	NDM_TEST(strcmp(ndm_strmap_get(&imap, "K3"), "Value3") == 0);

	ndm_strmap_remove_by_index(&imap, 1);

	NDM_TEST(ndm_strmap_size(&imap) == 1);
	NDM_TEST(strcmp(ndm_strmap_get(&imap, "key"), "Value") == 0);

	NDM_TEST(ndm_strmap_assign(&map, &imap));
	NDM_TEST_BREAK_IF(ndm_strmap_size(&map) != ndm_strmap_size(&imap));

	NDM_TEST(ndm_strmap_size(&map) == 1);
	NDM_TEST(strcmp(ndm_strmap_get(&map, "key"), "Value") == 0);

	size_t i = 0;

	while (i < ndm_strmap_size(&map)) {
		NDM_TEST(strcmp(
			ndm_strmap_get_by_index(&map, i),
			ndm_strmap_get_by_index(&imap, i)) == 0);
		++i;
	}

	ndm_strmap_clear(&map);
	NDM_TEST(ndm_strmap_flags(&imap) == NDM_STRMAP_FLAGS_CASE_INSENSITIVE);
	NDM_TEST(ndm_strmap_size(&map) == 0);
	NDM_TEST(ndm_strmap_is_empty(&map));

	ndm_strmap_clear(&imap);
	NDM_TEST(ndm_strmap_flags(&imap) == NDM_STRMAP_FLAGS_CASE_INSENSITIVE);
	NDM_TEST(ndm_strmap_size(&imap) == 0);
	NDM_TEST(ndm_strmap_is_empty(&imap));

	ndm_strmap_init(&map, NDM_STRMAP_FLAGS_DEFAULT);
	NDM_TEST(ndm_strmap_flags(&map) == NDM_STRMAP_FLAGS_DEFAULT);
	NDM_TEST(ndm_strmap_size(&map) == 0);
	NDM_TEST(ndm_strmap_is_empty(&map));

	ndm_strmap_init(&imap, NDM_STRMAP_FLAGS_DEFAULT);
	NDM_TEST(ndm_strmap_flags(&imap) == NDM_STRMAP_FLAGS_DEFAULT);
	NDM_TEST(ndm_strmap_size(&imap) == 0);
	NDM_TEST(ndm_strmap_is_empty(&imap));

	return NDM_TEST_RESULT;
}
