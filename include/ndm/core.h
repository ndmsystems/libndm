#ifndef __NDM_CORE_H__
#define __NDM_CORE_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "code.h"
#include "attr.h"

/**
 * Period of cached data relevance in milliseconds.
 */

#define NDM_CORE_DEFAULT_CACHE_TTL					200

/**
 * The maximum size of the cache in bytes.
 */

#define NDM_CORE_DEFAULT_CACHE_MAX_SIZE				65536

/**
 * Maximum waiting time for the completion of the network I/O operations
 * in milliseconds.
 */

#define NDM_CORE_DEFAULT_TIMEOUT					15000

/**
 * Macro to initialize a variable of @a ndm_core_t type.
 * @code
 * srtuct ndm_core_t *core = NDM_CORE_INITIALIZER;
 * @endcode
 */

#define NDM_CORE_INITIALIZER						NULL

/**
 * Macro to initialize a variable of @a ndm_core_response_t type.
 * @code
 * srtuct ndm_core_response_t *response = NDM_CORE_RESPONSE_INITIALIZER;
 * @endcode
 */

#define NDM_CORE_RESPONSE_INITIALIZER				NULL

struct ndm_xml_node_t;

enum ndm_core_request_type_t
{
	NDM_CORE_REQUEST_CONFIG,	//!< Request for a command configuration
	NDM_CORE_REQUEST_EXECUTE,	//!< Request for a command execution
	NDM_CORE_REQUEST_PARSE		//!< Request for execution of a command in CLI
								//!  form
};

enum ndm_core_response_type_t
{
	NDM_CORE_INFO,		//!< Response type is @a info
	NDM_CORE_WARNING,	//!< Response type is @a warning
	NDM_CORE_ERROR,		//!< Response type is @a error
	NDM_CORE_CRITICAL	//!< Response type is @a critical
};

enum ndm_core_response_error_t
{
	NDM_CORE_RESPONSE_ERROR_OK,				//!< No errors occured
	NDM_CORE_RESPONSE_ERROR_SYNTAX,			//!< Invalid path node syntax
	NDM_CORE_RESPONSE_ERROR_FORMAT,			//!< Invalid node value format
	NDM_CORE_RESPONSE_ERROR_NOT_FOUND,		//!< Node or attribute is not found
	NDM_CORE_RESPONSE_ERROR_BUFFER_SIZE,	//!< Buffer size is too small
	NDM_CORE_RESPONSE_ERROR_MESSAGE,		//!< Core error message is received
	NDM_CORE_RESPONSE_ERROR_SYSTEM			//!< See @a errno for details
};

enum ndm_core_cache_mode_t
{
	NDM_CORE_MODE_CACHE,				//!< Cache a response
	NDM_CORE_MODE_NO_CACHE				//!< Do not cache a response
};

struct ndm_core_t;
struct ndm_core_response_t;

struct ndm_core_event_t;
struct ndm_core_event_connection_t;

/**
 * Open the event connection to the core.
 *
 * @param timeout The maximum waiting time for the event reading
 * (see ndm_core_event_connection_get()) in milliseconds.
 *
 * @returns A pointer to a structure that describes an open connection, or
 * @c NULL if an error.
 */

struct ndm_core_event_connection_t *ndm_core_event_connection_open(
		const int timeout) NDM_ATTR_WUR;

/**
 * Close the event connection if it was opened.
 *
 * @param connection Pointer to the connection instance. The value can be
 * @c NULL as well as NULL-pointer to connection instance.
 *
 * @returns @c true if the connection is closed (a pointer to the connection
 * instance is set to @c NULL), @c false - otherwise (@a errno contains error
 * code).
 */

bool ndm_core_event_connection_close(
		struct ndm_core_event_connection_t **connection) NDM_ATTR_WUR;

/**
 * Get the descriptor of the event connection.
 *
 * @param connection Pointer to the connection instance.
 *
 * @returns The event connection descriptor.
 */

int ndm_core_event_connection_fd(
		const struct ndm_core_event_connection_t *connection) NDM_ATTR_WUR;

/**
 * Check for a core events.
 *
 * @param connection Pointer to the connection instance.
 *
 * @returns @c true if the connection contains data (core events), @c false -
 * otherwise.
 */

bool ndm_core_event_connection_has_events(
		struct ndm_core_event_connection_t *connection) NDM_ATTR_WUR;

/**
 * Get the instance of core event.
 *
 * @param connection Pointer to the connection instance.
 *
 * @returns Pointer to the received instance of core event if successful,
 * @c NULL - otherwise.
 */

struct ndm_core_event_t *ndm_core_event_connection_get(
		struct ndm_core_event_connection_t *connection) NDM_ATTR_WUR;

/**
 * Get the 'event' root node of XML-description.
 *
 * @param event Pointer to the event instance.
 *
 * @returns Pointer to the root node. @c NULL is never returned.
 */

const struct ndm_xml_node_t *ndm_core_event_root(
		const struct ndm_core_event_t *event) NDM_ATTR_WUR;

/**
 * Get the 'class' value of the event root node.
 *
 * @param event Pointer to the event instance.
 *
 * @returns Pointer to the string that contains event type. @c NULL is never
 * returned.
 */

const char *ndm_core_event_type(
		const struct ndm_core_event_t *event) NDM_ATTR_WUR;

/**
 * Get the 'raise_time' value of the event root node.
 *
 * @param event Pointer to the event instance.
 *
 * @returns Event time distribution in the form of @a timespec structure.
 */

struct timespec ndm_core_event_raise_time(
		const struct ndm_core_event_t *event) NDM_ATTR_WUR;

/**
 * Release the memory that event instance occupies. After completion assigns
 * @c NULL to @a event.
 *
 * @param event Pointer to the event instance. The value can be @c NULL
 * as well as NULL-pointer to event instance.
 */

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

