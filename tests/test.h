#ifndef __NDM_TEST_H__
#define __NDM_TEST_H__

#include <stdbool.h>

void __ndm_test__(
		const char *const expr_name,
		const bool expr_value,
		const char *const expr_text,
		const char *const file,
		const int line,
		const bool do_break);

int __ndm_test_result__();

#define NDM_TEST(expr)												\
	__ndm_test__("NDM_TEST", expr, #expr, __FILE__, __LINE__, false)

#define NDM_TEST_BREAK_IF(expr)										\
	__ndm_test__("NDM_TEST_BREAK_IF", expr, #expr, __FILE__, __LINE__, true)

#define NDM_TEST_RESULT												\
	__ndm_test_result__()

#endif	/* __NDM_TEST_H__ */

