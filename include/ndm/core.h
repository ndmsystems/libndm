#ifndef __NDM_CORE_H__
#define __NDM_CORE_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "attr.h"

#define NDM_CORE_DEFAULT_CACHE_TTL					200

#define NDM_CORE_DEFAULT_CACHE_MAX_SIZE				65536

#define NDM_CORE_DEFAULT_TIMEOUT					15000

#define NDM_CORE_INITIALIZER						NULL
#define NDM_CORE_RESPONSE_INITIALIZER				NULL

struct ndm_xml_node_t;

enum ndm_core_request_type_t
{
	NDM_CORE_REQUEST_CONFIG,
	NDM_CORE_REQUEST_EXECUTE,
	NDM_CORE_REQUEST_PARSE
};

enum ndm_core_response_type_t
{
	NDM_CORE_INFO,
	NDM_CORE_WARNING,
	NDM_CORE_ERROR,
	NDM_CORE_CRITICAL
};

enum ndm_core_response_error_t
{
	NDM_CORE_RESPONSE_ERROR_OK,
	NDM_CORE_RESPONSE_ERROR_SYNTAX,
	NDM_CORE_RESPONSE_ERROR_NOT_FOUND,
	NDM_CORE_RESPONSE_ERROR_FORMAT,
	NDM_CORE_RESPONSE_ERROR_SYSTEM
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

bool ndm_core_event_connection_has_events(
		struct ndm_core_event_connection_t *connection) NDM_ATTR_WUR;

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
		const int cache_ttl_msec,
		const size_t cache_max_size) NDM_ATTR_WUR;

bool ndm_core_close(
		struct ndm_core_t **core);

int ndm_core_fd(
		const struct ndm_core_t *core) NDM_ATTR_WUR;

void ndm_core_cache_clear(
		struct ndm_core_t *core,
		const bool remove_all);

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
		const char *const tag,
		bool *authenticated) NDM_ATTR_WUR;

struct ndm_core_response_t *ndm_core_request(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const command_args[],
		const char *const command_format,
		...) NDM_ATTR_WUR;

struct ndm_core_response_t *ndm_core_get_help(
		struct ndm_core_t *core,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const command);

struct ndm_core_response_t *ndm_core_continue(
		struct ndm_core_t *core) NDM_ATTR_WUR;

struct ndm_core_response_t *ndm_core_break(
		struct ndm_core_t *core) NDM_ATTR_WUR;

const char *ndm_core_last_message_string(
		struct ndm_core_t *core) NDM_ATTR_WUR;

const char *ndm_core_last_message_ident(
		struct ndm_core_t *core) NDM_ATTR_WUR;

const char *ndm_core_last_message_source(
		struct ndm_core_t *core) NDM_ATTR_WUR;

uint32_t ndm_core_last_message_code(
		struct ndm_core_t *code) NDM_ATTR_WUR;

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

/**
 * Core response node functions.
 **/

enum ndm_core_response_error_t ndm_core_response_first_node(
		const struct ndm_xml_node_t *node,
		const struct ndm_xml_node_t **value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR;

enum ndm_core_response_error_t ndm_core_response_first_str(
		const struct ndm_xml_node_t *node,
		const char **value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR;

enum ndm_core_response_error_t ndm_core_response_first_int(
		const struct ndm_xml_node_t *node,
		int *value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR;

enum ndm_core_response_error_t ndm_core_response_first_uint(
		const struct ndm_xml_node_t *node,
		unsigned int *value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR;

enum ndm_core_response_error_t ndm_core_response_first_bool(
		const struct ndm_xml_node_t *node,
		bool *value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR;

/**
 * The highest level core functions.
 **/

enum ndm_core_response_error_t ndm_core_request_break(
		struct ndm_core_t *core) NDM_ATTR_WUR;

enum ndm_core_response_error_t ndm_core_request_first_str_alloc(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		char **value,
		const char *const value_path,
		const char *const command_args[],
		const char *const command_format,
		...) NDM_ATTR_WUR;

enum ndm_core_response_error_t ndm_core_request_first_str_buffer(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		char *value,
		const size_t value_buffer_size,
		size_t *value_size,
		const char *const value_path,
		const char *const command_args[],
		const char *const command_format,
		...) NDM_ATTR_WUR;

enum ndm_core_response_error_t ndm_core_request_first_int(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		int *value,
		const char *const value_path,
		const char *const command_args[],
		const char *const command_format,
		...) NDM_ATTR_WUR;

enum ndm_core_response_error_t ndm_core_request_first_uint(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		unsigned int *value,
		const char *const value_path,
		const char *const command_args[],
		const char *const command_format,
		...) NDM_ATTR_WUR;

enum ndm_core_response_error_t ndm_core_request_first_bool(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		bool *value,
		const char *const value_path,
		const char *const command_args[],
		const char *const command_format,
		...) NDM_ATTR_WUR;

#endif	/* __NDM_CORE_H__ */

