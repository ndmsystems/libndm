#ifndef __NDM_REGEX_H__
#define __NDM_REGEX_H__

#include <regex.h>
#include <stdbool.h>
#include "attr.h"

struct ndm_regex_t;
struct ndm_regex_matcher_t;

struct ndm_regex_t *ndm_regex_alloc(
		const char *const pattern,
		const int flags) NDM_ATTR_WUR;

void ndm_regex_free(
		struct ndm_regex_t **re);

int ndm_regex_flags(
		const struct ndm_regex_t *re) NDM_ATTR_WUR;

const char *ndm_regex_pattern(
		const struct ndm_regex_t *re) NDM_ATTR_WUR;

struct ndm_regex_matcher_t *ndm_regex_matcher_alloc(
		const char *const seq) NDM_ATTR_WUR;

void ndm_regex_matcher_free(
		struct ndm_regex_matcher_t **matcher);

bool ndm_regex_matcher_find(
		struct ndm_regex_matcher_t *matcher,
		const struct ndm_regex_t *re,
		const int eflags) NDM_ATTR_WUR;

char *ndm_regex_matcher_group_get(
		const struct ndm_regex_matcher_t *matcher,
		const size_t index) NDM_ATTR_WUR;

size_t ndm_regex_matcher_group_count(
		const struct ndm_regex_matcher_t *matcher) NDM_ATTR_WUR;

size_t ndm_regex_matcher_start(
		const struct ndm_regex_matcher_t *matcher) NDM_ATTR_WUR;

size_t ndm_regex_matcher_end(
		const struct ndm_regex_matcher_t *matcher) NDM_ATTR_WUR;

void ndm_regex_matcher_reset(
		struct ndm_regex_matcher_t *matcher);

#endif /* __NDM_REGEX_H__ */

