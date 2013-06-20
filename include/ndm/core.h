#ifndef __NDM_CORE_H__
#define __NDM_CORE_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "code.h"
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
	NDM_CORE_RESPONSE_ERROR_SYNTAX,		/* invalid path node syntax 	*/
	NDM_CORE_RESPONSE_ERROR_FORMAT,		/* invalid node value format	*/
	NDM_CORE_RESPONSE_ERROR_NOT_FOUND,	/* node or attribute not found 	*/
	NDM_CORE_RESPONSE_ERROR_BUFFER_SIZE,/* buffer size is too small		*/
	NDM_CORE_RESPONSE_ERROR_MESSAGE,	/* core error message received	*/
	NDM_CORE_RESPONSE_ERROR_SYSTEM		/* see errno for details		*/
};

enum ndm_core_cache_mode_t
{
	NDM_CORE_MODE_CACHE,				/* cache a response				*/
	NDM_CORE_MODE_NO_CACHE				/* do not cache a response		*/
};

struct ndm_core_t;
struct ndm_core_response_t;

struct ndm_core_event_t;
struct ndm_core_event_connection_t;

/**
 * Core event connection functions.
 */

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
 */

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
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(5, 6);

struct ndm_core_response_t *ndm_core_get_help(
		struct ndm_core_t *core,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const command);

struct ndm_core_response_t *ndm_core_continue(
		struct ndm_core_t *core) NDM_ATTR_WUR;

struct ndm_core_response_t *ndm_core_break(
		struct ndm_core_t *core) NDM_ATTR_WUR;

bool ndm_core_last_message_received(
		struct ndm_core_t *core) NDM_ATTR_WUR;

enum ndm_core_response_type_t ndm_core_last_message_type(
		struct ndm_core_t *core) NDM_ATTR_WUR;

const char *ndm_core_last_message_string(
		struct ndm_core_t *core) NDM_ATTR_WUR;

const char *ndm_core_last_message_ident(
		struct ndm_core_t *core) NDM_ATTR_WUR;

const char *ndm_core_last_message_source(
		struct ndm_core_t *core) NDM_ATTR_WUR;

ndm_code_t ndm_core_last_message_code(
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
 */

enum ndm_core_response_error_t ndm_core_response_first_node(
		const struct ndm_xml_node_t *node,
		const struct ndm_xml_node_t **value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(3, 4);

enum ndm_core_response_error_t ndm_core_response_first_str(
		const struct ndm_xml_node_t *node,
		const char **value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(3, 4);

enum ndm_core_response_error_t ndm_core_response_first_int(
		const struct ndm_xml_node_t *node,
		int *value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(3, 4);

enum ndm_core_response_error_t ndm_core_response_first_uint(
		const struct ndm_xml_node_t *node,
		unsigned int *value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(3, 4);

enum ndm_core_response_error_t ndm_core_response_first_bool(
		const struct ndm_xml_node_t *node,
		const bool parse_value,
		bool *value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(4, 5);

/**
 * The highest level core functions.
 *
 * Some function prototypes contain a value_path argument to address
 * a specific first node or attribute using the following syntax:
 * "child_name1/child_name2/.../child_nameN@attr_name".
 * The path is relative name that starts addressing from
 * the root "response" node of a response XML document.
 * Any child name can be an empty string that means a "current" node.
 * For example a "@name" path addresses a "name" attribute of the root
 * "response" node. So "interface/mac", "/interface/mac", and
 * "//interface//mac///" are the same paths to a child node "mac" of a
 * child "interface" node of the root "response" node.
 * All space characters of the path considered as parts of node names.
 */

enum ndm_core_response_error_t ndm_core_request_break(
		struct ndm_core_t *core) NDM_ATTR_WUR;

enum ndm_core_response_error_t ndm_core_request_send(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const char *const command_args[],
		const char *const command_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(4, 5);

/**
 * Functions with command formatting.
 */

enum ndm_core_response_error_t ndm_core_request_first_str_alloc_cf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		char **value,
		const char *const value_path,
		const char *const command_args[],
		const char *const command_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(7, 8);

enum ndm_core_response_error_t ndm_core_request_first_str_buffer_cf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		char *value,
		const size_t value_buffer_size,
		size_t *value_size,
		const char *const value_path,
		const char *const command_args[],
		const char *const command_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(9, 10);

enum ndm_core_response_error_t ndm_core_request_first_int_cf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		int *value,
		const char *const value_path,
		const char *const command_args[],
		const char *const command_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(7, 8);

enum ndm_core_response_error_t ndm_core_request_first_uint_cf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		unsigned int *value,
		const char *const value_path,
		const char *const command_args[],
		const char *const command_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(7, 8);

enum ndm_core_response_error_t ndm_core_request_first_bool_cf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		const bool parse_value,
		bool *value,
		const char *const value_path,
		const char *const command_args[],
		const char *const command_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(8, 9);

/**
 * Functions with path formatting.
 */

enum ndm_core_response_error_t ndm_core_request_first_str_alloc_pf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const command_args[],
		const char *const command,
		char **value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(7, 8);

enum ndm_core_response_error_t ndm_core_request_first_str_buffer_pf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const command_args[],
		const char *const command,
		char *value,
		const size_t value_buffer_size,
		size_t *value_size,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(9, 10);

enum ndm_core_response_error_t ndm_core_request_first_int_pf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const command_args[],
		const char *const command,
		int *value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(7, 8);

enum ndm_core_response_error_t ndm_core_request_first_uint_pf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const command_args[],
		const char *const command,
		unsigned int *value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(7, 8);

enum ndm_core_response_error_t ndm_core_request_first_bool_pf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const command_args[],
		const char *const command,
		const bool parse_value,
		bool *value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(8, 9);

#endif	/* __NDM_CORE_H__ */

