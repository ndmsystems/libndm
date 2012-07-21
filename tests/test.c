#include <stdio.h>
#include <stdlib.h>
#include "test.h"

static unsigned long __total = 0;
static unsigned long __fails = 0;

static volatile bool __registered = false;

static void __done()
{
	if (__fails == 0) {
		fprintf(stdout, "\n\033[32m*** ALL %lu TESTS " \
			"COMPLETED SUCCESSFULLY ***\033[0m\n", __total);
	} else {
		fprintf(stderr, "\n\033[1;31m*** FAILED %lu/%lu ***\033[0m\n",
			__fails, __total);
	}
}

void __ndm_test__(
		const char *const expr_name,
		const bool expr_value,
		const char *const expr_text,
		const char *const file,
		const int line,
		const bool do_break)
{
	++__total;

	if (!__registered && atexit(__done) == 0) {
		__registered = true;
	}

	if ((do_break && expr_value) || (!do_break && !expr_value)) {
		fprintf(stderr, "%s(%s) failed at %s:%i.\n",
			expr_name, expr_text, file, line);
		++__fails;

		if (do_break) {
			exit(EXIT_FAILURE);
		}
	}
}

int __ndm_test_result__()
{
	return (__fails == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

