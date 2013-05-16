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
	NDM_LOG_DEBUG("debug %i", i++);

	test_vlog(LINFO, "info %i", i++);
	test_vlog(LWARNING, "warning %i", i++);
	test_vlog(LERROR, "error %i", i++);
	test_vlog(LCRITICAL, "critical %i", i++);

	return NDM_TEST_RESULT;
}
