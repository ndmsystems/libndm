#include <stdio.h>
#include <ndm/core.h>
#include <ndm/macro.h>

int main()
{
	struct ndm_core_t *core =
		ndm_core_open(
				"test/ci",
				1000,
				NDM_CORE_DEFAULT_CACHE_MAX_SIZE);
	struct ndm_core_response_t *r = NULL;

	if (core != NULL) {
		r = ndm_core_request(
				core,
				NDM_CORE_REQUEST_PARSE,
				NDM_CORE_MODE_NO_CACHE,
				NULL,
				"no more");

		if( ndm_core_last_message_received(core) ) {

			/*! [decimal code printf] */
			printf("last message code: %" NDM_CODE_PRIu " (decimal)\n",
					ndm_core_last_message_code(core));
			/* last message code: 1179668 (decimal) */
			/*! [decimal code printf] */

			/*! [hex code printf] */
			printf("last message code: 0x%" NDM_CODE_PRIx " (hex)\n",
					ndm_core_last_message_code(core));
			/* last message code: 0x120014 (hex) */
			/*! [hex code printf] */

			/*! [octal code printf] */
			printf("last message code: 0%" NDM_CODE_PRIo " (octal)\n",
					ndm_core_last_message_code(core));
			/* last message code: 04400024 (octal) */
			/*! [octal code printf] */
		}
	}

	if (r != NULL) {
		ndm_core_response_free(&r);
	}

	ndm_core_close(&core);

	return 0;
}

