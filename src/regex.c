#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ndm/regex.h>
#include <ndm/string.h>

struct ndm_regex_t
{
	regex_t reg;
	int flags;
	char pattern[];
};

struct ndm_regex_matcher_t
{
	regmatch_t *match;
	size_t nmatch;
	size_t start;
	size_t end;
	char seq[];
};

struct ndm_regex_t *ndm_regex_alloc(
		const char *const pattern,
		const int flags)
{
	const size_t plen = strlen(pattern);
	struct ndm_regex_t *re = malloc(sizeof(*re) + plen + 1);

	if (re == NULL) {
		return NULL;
	}

	memcpy(re->pattern, pattern, plen + 1);
	re->flags = flags;

	if (regcomp(&re->reg, pattern, flags) != 0) {
		/* failed to compile a regular expression */
		errno = EINVAL;
		free(re);

		return NULL;
	}

	return re;
}

void ndm_regex_free(
		struct ndm_regex_t **re)
{
	if (re != NULL && *re != NULL) {
		struct ndm_regex_t *r = *re;

		regfree(&r->reg);
		free(r);

		*re = NULL;
	}
}

int ndm_regex_flags(
		const struct ndm_regex_t *re)
{
	return re->flags;
}

const char *ndm_regex_pattern(
		const struct ndm_regex_t *re)
{
	return re->pattern;
}

struct ndm_regex_matcher_t *ndm_regex_matcher_alloc(
		const char *const seq)
{
	const size_t slen = strlen(seq);
	struct ndm_regex_matcher_t *matcher =
		malloc(sizeof(*matcher) + slen + 1);

	if (matcher == NULL) {
		return NULL;
	}

	memcpy(matcher->seq, seq, slen + 1);
	matcher->match = NULL;
	matcher->nmatch = 0;
	matcher->start = 0;
	matcher->end = 0;

	return matcher;
}

void ndm_regex_matcher_free(
		struct ndm_regex_matcher_t **matcher)
{
	if (matcher != NULL && *matcher != NULL) {
		struct ndm_regex_matcher_t *m = *matcher;

		ndm_regex_matcher_reset(m);
		free(m);

		*matcher = NULL;
	}
}

bool ndm_regex_matcher_find(
		struct ndm_regex_matcher_t *matcher,
		const struct ndm_regex_t *re,
		const int eflags)
{
	const size_t nmatch = (size_t) re->reg.re_nsub + 1;

	if (nmatch > SIZE_MAX / sizeof(regmatch_t)) {
		errno = ENOMEM;

		return false;
	}

	regmatch_t* match = malloc(sizeof(regmatch_t) * nmatch);

	if (match == NULL) {
		return false;
	}

	free(matcher->match);
	matcher->match = NULL;
	matcher->nmatch = 0;

	if (regexec(
			&re->reg,
			matcher->seq + matcher->end,
			nmatch,
			match,
			eflags) != 0)
	{
		free(match);
		errno = ENOENT;

		return false;
	}

	matcher->start = matcher->end;
	matcher->end += (size_t) match[0].rm_eo;

	matcher->match = match;
	matcher->nmatch = nmatch;

	return true;
}

char *ndm_regex_matcher_group_get(
		const struct ndm_regex_matcher_t *matcher,
		const size_t index)
{
	if (index >= matcher->nmatch) {
		errno = EINVAL;

		return NULL;
	}

	const regmatch_t *rm = &matcher->match[index];

	if (rm->rm_so < 0) {
		return ndm_string_dup("");
	}

	const size_t s = (size_t) rm->rm_so;
	const size_t e = (size_t) rm->rm_eo;

	return ndm_string_ndup(matcher->seq + matcher->start + s, e - s);
}

size_t ndm_regex_matcher_group_count(
		const struct ndm_regex_matcher_t *matcher)
{
	return matcher->nmatch;
}

size_t ndm_regex_matcher_start(
		const struct ndm_regex_matcher_t *matcher)
{
	return matcher->start;
}

size_t ndm_regex_matcher_end(
		const struct ndm_regex_matcher_t *matcher)
{
	return matcher->end;
}

void ndm_regex_matcher_reset(
		struct ndm_regex_matcher_t *matcher)
{
	free(matcher->match);
	matcher->match = NULL;

	matcher->nmatch = 0;
	matcher->start = 0;
	matcher->end = 0;
}
