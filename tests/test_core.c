#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ndm/xml.h>
#include <ndm/core.h>
#include <ndm/poll.h>
#include <ndm/time.h>
#include <ndm/macro.h>
#include "test.h"

#define CACHE_TTL_MS			1000

int main()
{
	struct ndm_core_t *core = ndm_core_open("test/ci",
		CACHE_TTL_MS, NDM_CORE_DEFAULT_CACHE_MAX_SIZE);
	struct ndm_core_response_t *r = NULL;
	bool authenticated = false;

	NDM_TEST_BREAK_IF(core == NULL);

	r = ndm_core_request(core, NDM_CORE_REQUEST_PARSE,
		NDM_CORE_MODE_NO_CACHE, NULL, "no service http");

	NDM_TEST(r != NULL);

	if (r != NULL) {
		ndm_core_response_free(&r);
	}

	NDM_TEST(ndm_core_authenticate(
		core, "admin", "1",
		"Undefined realm", "cli", &authenticated));
	NDM_TEST(!authenticated);

	NDM_TEST(ndm_core_authenticate(
		core, "admin", "1234",
		"Undefined realm", "cli", &authenticated));
	NDM_TEST(authenticated);

	r = ndm_core_get_help(core, NDM_CORE_MODE_NO_CACHE, "no service ht");

	NDM_TEST(r != NULL);

	if (r != NULL) {
		NDM_TEST(ndm_xml_node_first_child(
			ndm_core_response_root(r), "help") != NULL);

		ndm_core_response_free(&r);
	}

	r = ndm_core_request(core, NDM_CORE_REQUEST_CONFIG,
		NDM_CORE_MODE_NO_CACHE, NULL, "service http");

	NDM_TEST(r != NULL);

	if (r != NULL) {
		const struct ndm_xml_node_t *root = ndm_core_response_root(r);
		const struct ndm_xml_node_t *config =
			ndm_xml_node_first_child(root, "config");

		NDM_TEST(
			config == NULL ||
			ndm_xml_node_first_child(config, "enabled") == NULL);

		ndm_core_response_free(&r);
	}

	r = ndm_core_request(core, NDM_CORE_REQUEST_EXECUTE,
		NDM_CORE_MODE_NO_CACHE, NULL, "service http");

	NDM_TEST(r != NULL);

	if (r != NULL) {
		NDM_TEST(ndm_core_response_is_ok(r));
		ndm_core_response_free(&r);
	}

	r = ndm_core_request(core, NDM_CORE_REQUEST_CONFIG,
		NDM_CORE_MODE_NO_CACHE, NULL, "service http");

	NDM_TEST(r != NULL);

	if (r != NULL) {
		const struct ndm_xml_node_t *root = ndm_core_response_root(r);
		const struct ndm_xml_node_t *config =
			ndm_xml_node_first_child(root, "config");

		NDM_TEST(config != NULL);

		if (config != NULL) {
			NDM_TEST(ndm_xml_node_first_child(config, "enabled") != NULL);
		}

		ndm_core_response_free(&r);
	}

	r = ndm_core_continue(core);

	NDM_TEST(r != NULL);

	if (r != NULL) {
		NDM_TEST(ndm_core_response_is_ok(r));

		ndm_core_response_free(&r);
	}

	do {
		r = ndm_core_request(core, NDM_CORE_REQUEST_CONFIG,
			NDM_CORE_MODE_CACHE, NULL, "show interface");

		NDM_TEST(r != NULL);

		ndm_core_response_free(&r);
	} while (0);

	do {
		r = ndm_core_request(core, NDM_CORE_REQUEST_CONFIG,
			NDM_CORE_MODE_CACHE, NULL, "show interface");

		NDM_TEST(r != NULL);

		ndm_core_response_free(&r);
	} while (0);

	do {
		const char *args[] =
		{
			"name", "WifiStation0",
			NULL
		};

		r = ndm_core_request(core, NDM_CORE_REQUEST_CONFIG,
			NDM_CORE_MODE_CACHE, args, "show interface");

		NDM_TEST(r != NULL);

		ndm_core_response_free(&r);
	} while (0);

	do {
		r = ndm_core_request(core, NDM_CORE_REQUEST_CONFIG,
			NDM_CORE_MODE_CACHE, NULL, "show interface");

		NDM_TEST(r != NULL);

		ndm_core_response_free(&r);
	} while (0);

	do {
		r = ndm_core_request(core, NDM_CORE_REQUEST_PARSE,
			NDM_CORE_MODE_CACHE, NULL, "show running-config");

		NDM_TEST(r != NULL);

		ndm_core_response_free(&r);
	} while (0);

	do {
		r = ndm_core_request(core, NDM_CORE_REQUEST_EXECUTE,
			NDM_CORE_MODE_CACHE, NULL, "show log");

		NDM_TEST(r != NULL);
		NDM_TEST(ndm_core_response_is_continued(r));

		ndm_core_response_free(&r);
	} while (0);

	do {
		r = ndm_core_request(core, NDM_CORE_REQUEST_PARSE,
			NDM_CORE_MODE_CACHE, NULL, "show interface");

		NDM_TEST(r != NULL);

		ndm_core_response_free(&r);
	} while (0);

	r = ndm_core_request(core,
		NDM_CORE_REQUEST_PARSE,
		NDM_CORE_MODE_CACHE,
		NULL, "show interface");

	NDM_TEST(r != NULL);

	if (r != NULL) {
		int i = 0;
		unsigned int u = 0;
		bool b = false;
		const struct ndm_xml_node_t *n = ndm_core_response_root(r);
		const char *s = NULL;

		NDM_TEST(ndm_core_response_first_str(n, &s, "") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		NDM_TEST(s != NULL && *s == '\0');	/* <response> value */

		NDM_TEST(ndm_core_response_first_str(n, &s, "/") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		NDM_TEST(s != NULL && *s == '\0');	/* <response> value */

		NDM_TEST(ndm_core_response_first_str(n, &s, "//") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		NDM_TEST(s != NULL && *s == '\0');	/* <response> value */

		NDM_TEST(ndm_core_response_first_str(n, &s, "///") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		NDM_TEST(s != NULL && *s == '\0');	/* <response> value */

		NDM_TEST(ndm_core_response_first_str(n, &s, "interface") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		NDM_TEST(s != NULL && *s == '\0');	/* <response/interface> value */

		NDM_TEST(ndm_core_response_first_str(n, &s, "//interface") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		NDM_TEST(s != NULL && *s == '\0');	/* <response/interface> value */

		NDM_TEST(ndm_core_response_first_str(n, &s, "///interface") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		NDM_TEST(s != NULL && *s == '\0');	/* <response/interface> value */

		NDM_TEST(ndm_core_response_first_str(n, &s, "interface/") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		NDM_TEST(s != NULL && *s == '\0');	/* <response/interface> value */

		NDM_TEST(ndm_core_response_first_str(n, &s, "interface//") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		NDM_TEST(s != NULL && *s == '\0');	/* <response/interface> value */

		NDM_TEST(ndm_core_response_first_str(n, &s, "interface///") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		NDM_TEST(s != NULL && *s == '\0');	/* <response/interface> value */

		NDM_TEST(ndm_core_response_first_str(n, &s, "/interface/") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		NDM_TEST(s != NULL && *s == '\0');	/* <response/interface> value */

		NDM_TEST(ndm_core_response_first_str(n, &s, "//interface//") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		NDM_TEST(s != NULL && *s == '\0');	/* <response/interface> value */

		NDM_TEST(ndm_core_response_first_str(n, &s, "///interface///") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		NDM_TEST(s != NULL && *s == '\0');	/* <response/interface> value */

		NDM_TEST(ndm_core_response_first_str(
			n, &s, "//interface///mtu//") == NDM_CORE_RESPONSE_ERROR_OK);
		/* <response/interface/mtu> value */
		NDM_TEST(s != NULL && strcmp(s, "1500") == 0);

		NDM_TEST(ndm_core_response_first_str(n, &s, "interface@name") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		/* <response/interface@name> value */
		NDM_TEST(s != NULL && *s != '\0');

		NDM_TEST(ndm_core_response_first_int(n, &i, "interface/mtu") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		/* <response/interface/mtu> value */
		NDM_TEST(i == 1500);

		NDM_TEST(ndm_core_response_first_uint(n, &u, "interface/mtu") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		/* <response/interface/mtu> value */
		NDM_TEST(u == 1500);

		NDM_TEST(ndm_core_response_first_bool(
			n, &b, "interface/global") == NDM_CORE_RESPONSE_ERROR_OK);

		i = 0;

		NDM_TEST(
			ndm_core_request_first_int(core,
				NDM_CORE_REQUEST_PARSE,
				NDM_CORE_MODE_CACHE,
				&i, "interface/mtu",
				NULL, "show interface") == NDM_CORE_RESPONSE_ERROR_OK);
		NDM_TEST(i == 1500);

		NDM_TEST(
			ndm_core_request_first_bool(core,
				NDM_CORE_REQUEST_PARSE,
				NDM_CORE_MODE_CACHE,
				&b, "config/enabled",
				NULL, "service aaa") != NDM_CORE_RESPONSE_ERROR_OK);
		printf("\"%s\" [ident = \"%s\",source = \"%s\", code = \"%lu\"]\n",
			ndm_core_last_message_string(core),
			ndm_core_last_message_source(core),
			ndm_core_last_message_ident(core),
			(unsigned long) ndm_core_last_message_code(core));
	}

	ndm_core_response_free(&r);

	ndm_core_close(&core);

	return NDM_TEST_RESULT;
}

