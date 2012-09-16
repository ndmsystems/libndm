#ifndef __NDM_CORE_H__
#define __NDM_CORE_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "attr.h"

#define NDM_CORE_DEFAULT_CACHE_TTL					200

#define NDM_CORE_DEFAULT_CACHE_MAX_SIZE				65536
#define NDM_CORE_DEFAULT_CACHE_DYNAMIC_BLOCK_SIZE	4096

#define NDM_CORE_DEFAULT_TIMEOUT					15000

#define NDM_CORE_INITIALIZER						NULL
#define NDM_CORE_RESPONSE_INITIALIZER				NULL

struct ndm_xml_node_t;

enum ndm_core_response_type_t
{
	NDM_CORE_INFO,
	NDM_CORE_WARNING,
	NDM_CORE_ERROR,
	NDM_CORE_CRITICAL
};

enum ndm_core_cache_mode_t
{
	NDM_CORE_MODE_CACHE,
	NDM_CORE_MODE_NO_CACHE
};

struct ndm_core_t;
struct ndm_core_response_t;

struct ndm_core_event_t;
struct ndm_core_event_connection_t;

/**
 * Core event connection functions.
 **/

struct ndm_core_event_connection_t *ndm_core_event_connection_open(
		const int timeout) NDM_ATTR_WUR;

bool ndm_core_event_connection_close(
		struct ndm_core_event_connection_t **connection) NDM_ATTR_WUR;

int ndm_core_event_connection_fd(
		const struct ndm_core_event_connection_t *connection) NDM_ATTR_WUR;

struct ndm_core_event_t *ndm_core_event_connection_get(
		struct ndm_core_event_connection_t *connection) NDM_ATTR_WUR;

const struct ndm_xml_node_t *ndm_core_event_root(
		const struct ndm_core_event_t *event) NDM_ATTR_WUR;

const char *ndm_core_event_type(
		const struct ndm_core_event_t *event) NDM_ATTR_WUR;

struct timespec ndm_core_event_raise_time(
		const struct ndm_core_event_t *event) NDM_ATTR_WUR;

void ndm_core_event_free(
		struct ndm_core_event_t **event);

/**
 * Core connection functions.
 **/

struct ndm_core_t *ndm_core_open(
		const char *const agent,
		const int cache_ttl_ms,
		const size_t cache_max_size,
		const size_t cache_dynamic_block_size) NDM_ATTR_WUR;

bool ndm_core_close(
		struct ndm_core_t **core);

int ndm_core_fd(
		const struct ndm_core_t *core) NDM_ATTR_WUR;

void ndm_core_cache_clear(
		struct ndm_core_t *core);

void ndm_core_set_timeout(
		struct ndm_core_t *core,
		const int timeout);

int ndm_core_get_timeout(
		const struct ndm_core_t *core) NDM_ATTR_WUR;

const char *ndm_core_agent(
		const struct ndm_core_t *core) NDM_ATTR_WUR;

bool ndm_core_authenticate(
		struct ndm_core_t *core,
		const char *const user,
		const char *const password,
		const char *const realm,
		bool *authenticated) NDM_ATTR_WUR;

struct ndm_core_response_t *ndm_core_get_config(
		struct ndm_core_t *core,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const command,
		const char *const args[]) NDM_ATTR_WUR;

struct ndm_core_response_t *ndm_core_execute(
		struct ndm_core_t *core,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const command,
		const char *const args[]) NDM_ATTR_WUR;

struct ndm_core_response_t *ndm_core_get_help(
		struct ndm_core_t *core,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const format,
		...) NDM_ATTR_WUR;

struct ndm_core_response_t *ndm_core_parse(
		struct ndm_core_t *core,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const format,
		...) NDM_ATTR_WUR;

struct ndm_core_response_t *ndm_core_continue(
		struct ndm_core_t *core) NDM_ATTR_WUR;

struct ndm_core_response_t *ndm_core_break(
		struct ndm_core_t *core) NDM_ATTR_WUR;

void ndm_core_response_free(
		struct ndm_core_response_t **response);

bool ndm_core_response_is_ok(
		const struct ndm_core_response_t *response) NDM_ATTR_WUR;

enum ndm_core_response_type_t ndm_core_response_type(
		const struct ndm_core_response_t *response) NDM_ATTR_WUR;

bool ndm_core_response_is_continued(
		const struct ndm_core_response_t *response) NDM_ATTR_WUR;

const struct ndm_xml_node_t *ndm_core_response_root(
		const struct ndm_core_response_t *response) NDM_ATTR_WUR;

#endif	/* __NDM_CORE_H__ */

