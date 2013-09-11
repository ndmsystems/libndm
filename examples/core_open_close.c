#include <ndm/core.h>

int main()
{
	/*! [open and close] */
	struct ndm_core_t *core =
		ndm_core_open(
				"test/ci",
				1000,
				NDM_CORE_DEFAULT_CACHE_MAX_SIZE);

	/* ... */

	ndm_core_close(&core);
	/*! [open and close] */

	return 0;
}

