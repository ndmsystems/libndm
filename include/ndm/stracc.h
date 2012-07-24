#ifndef __NDM_STRACC_H__
#define __NDM_STRACC_H__

#include <stdbool.h>
#include "attr.h"

#define NDM_STRACC_INIT						NULL

struct ndm_stracc_t;

struct ndm_stracc_t *ndm_stracc_alloc() NDM_ATTR_WUR;

void ndm_stracc_append(
		struct ndm_stracc_t *acc,
		const char *const format,
		...) NDM_ATTR_PRINTF(2, 3);

bool ndm_stracc_is_valid(
		struct ndm_stracc_t *acc) NDM_ATTR_WUR;

const char *ndm_stracc_value(
		struct ndm_stracc_t *acc) NDM_ATTR_WUR;

size_t ndm_stracc_size(
		struct ndm_stracc_t *acc) NDM_ATTR_WUR;

void ndm_stracc_free(
		struct ndm_stracc_t **acc);

#endif	/* __NDM_STRACC_H__ */

