#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ndm/core.h>
#include <ndm/poll.h>
#include <ndm/time.h>
#include <ndm/macro.h>
#include "test.h"

int main()
{
	struct ndm_core_t *core = ndm_core_open("test", 0, 0, 0);
	struct ndm_core_response_t *r = NULL;

	NDM_TEST_BREAK_IF(core == NULL);

	r = ndm_core_parse(core, NDM_CORE_MODE_NO_CACHE, "no service http");

	NDM_TEST(r != NULL);

	if (r != NULL) {
		ndm_core_response_free(&r);
	}

	ndm_core_close(&core);

	return NDM_TEST_RESULT;
}
