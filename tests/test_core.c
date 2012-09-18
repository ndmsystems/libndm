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
	struct ndm_core_t *core = ndm_core_open("test",
		CACHE_TTL_MS, NDM_CORE_DEFAULT_CACHE_MAX_SIZE);
	struct ndm_core_response_t *r = NULL;
	bool authenticated = false;

	NDM_TEST_BREAK_IF(core == NULL);

	r = ndm_core_parse(core, NDM_CORE_MODE_NO_CACHE, "no service http");

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

	r = ndm_core_get_config(
		core, NDM_CORE_MODE_NO_CACHE,
		"service http", NULL);

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

	r = ndm_core_execute(core, NDM_CORE_MODE_NO_CACHE, "service http", NULL);

	NDM_TEST(r != NULL);

	if (r != NULL) {
		NDM_TEST(ndm_core_response_is_ok(r));
		ndm_core_response_free(&r);
	}

	r = ndm_core_get_config(
		core, NDM_CORE_MODE_NO_CACHE,
		"service http", NULL);

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
		r = ndm_core_get_config(
			core, NDM_CORE_MODE_CACHE,
			"show interface", NULL);

		NDM_TEST(r != NULL);

		ndm_core_response_free(&r);
	} while (0);

	do {
		r = ndm_core_get_config(
			core, NDM_CORE_MODE_CACHE,
			"show interface", NULL);

		NDM_TEST(r != NULL);

		ndm_core_response_free(&r);
	} while (0);

	do {
		const char *args[] =
		{
			"name", "WifiStation0",
			NULL
		};

		r = ndm_core_get_config(
			core, NDM_CORE_MODE_CACHE,
			"show interface", args);

		NDM_TEST(r != NULL);

		ndm_core_response_free(&r);
	} while (0);

	do {
		r = ndm_core_get_config(
			core, NDM_CORE_MODE_CACHE,
			"show interface", NULL);

		NDM_TEST(r != NULL);

		ndm_core_response_free(&r);
	} while (0);

	do {
		r = ndm_core_get_config(
			core, NDM_CORE_MODE_CACHE,
			"show running-config", NULL);

		NDM_TEST(r != NULL);

		ndm_core_response_free(&r);
	} while (0);

	do {
		r = ndm_core_execute(
			core, NDM_CORE_MODE_CACHE,
			"show log", NULL);

		NDM_TEST(r != NULL);
		NDM_TEST(ndm_core_response_is_continued(r));

		ndm_core_response_free(&r);
	} while (0);

	do {
		r = ndm_core_get_config(
			core, NDM_CORE_MODE_CACHE,
			"show interface", NULL);

		NDM_TEST(r != NULL);

		ndm_core_response_free(&r);
	} while (0);

	ndm_core_close(&core);

	return NDM_TEST_RESULT;
}

