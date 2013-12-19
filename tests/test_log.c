#include <string.h>
#include <ndm/log.h>
#include <ndm/macro.h>
#include "test.h"

void test_vlog(
		enum level_t level,
		const char *const format,
		...)
{
	va_list ap;

	va_start(ap, format);
	ndm_vlog(level, format, ap);
	va_end(ap);
}

int main(int argc, char *argv[])
{
	int i = 0;

	NDM_TEST_BREAK_IF(
		!ndm_log_init(
			ndm_log_get_ident(argv), "source", true, false));

	NDM_LOG_INFO("info %i", i++);
	NDM_LOG_WARNING("warning %i", i++);
	NDM_LOG_ERROR("error %i", i++);
	NDM_LOG_CRITICAL("critical %i", i++);

	/* default debug level is LDEBUG_ALL if NDEBUG is undefined */
	NDM_TEST(NDM_LOG_DEBUG("debug [default] %i", i++));
	NDM_TEST(NDM_LOG_DEBUG_1("debug [level 1] %i", i++));
	NDM_TEST(NDM_LOG_DEBUG_2("debug [level 2] %i", i++));
	NDM_TEST(NDM_LOG_DEBUG_3("debug [level 3] %i", i++));

	ndm_log_set_debug(LDEBUG_1);
	NDM_TEST(NDM_LOG_DEBUG("debug [default] %i", i++));
	NDM_TEST(NDM_LOG_DEBUG_1("debug [level 1] %i", i++));
	NDM_TEST(!NDM_LOG_DEBUG_2("debug [level 2] %i", i++));
	NDM_TEST(!NDM_LOG_DEBUG_3("debug [level 3] %i", i++));

	ndm_log_set_debug(LDEBUG_2);
	NDM_TEST(NDM_LOG_DEBUG("debug [default] %i", i++));
	NDM_TEST(NDM_LOG_DEBUG_1("debug [level 1] %i", i++));
	NDM_TEST(NDM_LOG_DEBUG_2("debug [level 2] %i", i++));
	NDM_TEST(!NDM_LOG_DEBUG_3("debug [level 3] %i", i++));

	ndm_log_set_debug(LDEBUG_3);
	NDM_TEST(NDM_LOG_DEBUG("debug [default] %i", i++));
	NDM_TEST(NDM_LOG_DEBUG_1("debug [level 1] %i", i++));
	NDM_TEST(NDM_LOG_DEBUG_2("debug [level 2] %i", i++));
	NDM_TEST(NDM_LOG_DEBUG_3("debug [level 3] %i", i++));

	ndm_log_set_debug(LDEBUG_OFF);
	NDM_TEST(!NDM_LOG_DEBUG("debug [default] %i", i++));
	NDM_TEST(!NDM_LOG_DEBUG_1("debug [level 1] %i", i++));
	NDM_TEST(!NDM_LOG_DEBUG_2("debug [level 2] %i", i++));
	NDM_TEST(!NDM_LOG_DEBUG_3("debug [level 3] %i", i++));

	test_vlog(LINFO, "info %i", i++);
	test_vlog(LWARNING, "warning %i", i++);
	test_vlog(LERROR, "error %i", i++);
	test_vlog(LCRITICAL, "critical %i", i++);

	return NDM_TEST_RESULT;
}
