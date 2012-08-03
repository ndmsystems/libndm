#include <string.h>
#include <ndm/log.h>
#include <ndm/macro.h>
#include "test.h"

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

	return NDM_TEST_RESULT;
}
