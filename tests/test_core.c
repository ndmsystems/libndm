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
	bool found = false;

	NDM_TEST_BREAK_IF(core == NULL);

	NDM_TEST(ndm_core_find_command(core, "service htt", &found));
	NDM_TEST(!found);

	NDM_TEST(ndm_core_find_command(core, "service http", &found));
	NDM_TEST(found);

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
		core, "admin", "",
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
		char c = 0;
		unsigned char uc = 0;
		short sh = 0;
		unsigned short ush = 0;
		int i = 0;
		unsigned int ui = 0;
		long l = 0;
		unsigned long ul = 0;
		long long ll = 0;
		unsigned long long ull = 0;
		bool b = false;
		const struct ndm_xml_node_t *n = ndm_core_response_root(r);
		const char *s = NULL;

		NDM_TEST(ndm_core_response_first_str(n, &s, "%s", "") ==
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

		NDM_TEST(ndm_core_response_first_char(n, &c, "interface/index") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		/* <response/interface/mtu> value */
		NDM_TEST(c == 0);

		NDM_TEST(ndm_core_response_first_uchar(n, &uc, "interface/index") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		/* <response/interface/mtu> value */
		NDM_TEST(uc == 0);

		NDM_TEST(ndm_core_response_first_short(n, &sh, "interface/mtu") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		/* <response/interface/mtu> value */
		NDM_TEST(sh == 1500);

		NDM_TEST(ndm_core_response_first_ushort(n, &ush, "interface/mtu") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		/* <response/interface/mtu> value */
		NDM_TEST(ush == 1500);

		NDM_TEST(ndm_core_response_first_int(n, &i, "interface/mtu") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		/* <response/interface/mtu> value */
		NDM_TEST(i == 1500);

		NDM_TEST(ndm_core_response_first_uint(n, &ui, "interface/mtu") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		/* <response/interface/mtu> value */
		NDM_TEST(ui == 1500);

		NDM_TEST(ndm_core_response_first_long(n, &l, "interface/mtu") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		/* <response/interface/mtu> value */
		NDM_TEST(l == 1500);

		NDM_TEST(ndm_core_response_first_ulong(n, &ul, "interface/mtu") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		/* <response/interface/mtu> value */
		NDM_TEST(ul == 1500);

		NDM_TEST(ndm_core_response_first_llong(n, &ll, "interface/mtu") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		/* <response/interface/mtu> value */
		NDM_TEST(ll == 1500);

		NDM_TEST(ndm_core_response_first_ullong(n, &ull, "interface/mtu") ==
			NDM_CORE_RESPONSE_ERROR_OK);
		/* <response/interface/mtu> value */
		NDM_TEST(ull == 1500);

		NDM_TEST(ndm_core_response_first_bool(
			n, true, &b, "interface/global") == NDM_CORE_RESPONSE_ERROR_OK);

		i = 0;

		NDM_TEST(
			ndm_core_request_first_int_cf(core,
				NDM_CORE_REQUEST_PARSE,
				NDM_CORE_MODE_CACHE,
				&i, "interface/mtu",
				NULL, "show interface") == NDM_CORE_RESPONSE_ERROR_OK);
		NDM_TEST(i == 1500);

		NDM_TEST(ndm_core_request_break(core) == NDM_CORE_RESPONSE_ERROR_OK);

		NDM_TEST(
			ndm_core_request_first_bool_cf(core,
				NDM_CORE_REQUEST_PARSE,
				NDM_CORE_MODE_CACHE,
				false, &b, "config/enabled",
				NULL, "service aaa") != NDM_CORE_RESPONSE_ERROR_OK);

		NDM_TEST(
			ndm_core_request_first_bool_pf(core,
				NDM_CORE_REQUEST_PARSE,
				NDM_CORE_MODE_CACHE,
				NULL, "service aaa",
				false, &b, "config/%s", "enabled") !=
					NDM_CORE_RESPONSE_ERROR_OK);

		NDM_TEST(ndm_core_last_message_received(core));

		NDM_TEST(strcmp(
			ndm_core_last_message_string(core),
			"no such command: aaa") == 0);
		NDM_TEST(ndm_core_last_message_code(core) == 0x80710020);
		NDM_TEST(*ndm_core_last_message_source(core) == '\0');
		NDM_TEST(strcmp(
			ndm_core_last_message_ident(core),
			"Command::Base") == 0);
	}

	ndm_core_response_free(&r);

	ndm_core_close(&core);

	return NDM_TEST_RESULT;
}

