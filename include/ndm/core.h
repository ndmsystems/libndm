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
 * instance is set to @c NULL), @c false — otherwise (@a errno contains error
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
 * @returns @c true if the connection contains data (core events), @c false —
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
 * @c NULL — otherwise.
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
 * Open a connection to the NDM core, allocate memory for  the @b ndm_core_t
 * structure.
 *
 * @param agent Agent name to identify which application last modified
 * the configuration of the system.
 * @param cache_ttl_msec Caching time for the core responses in milliseconds.
 * @param cache_max_size Maximum size of the cache in bytes.
 *
 * @returns Pointer to the instance of @b ndm_core_t structure in case of
 * successful connection, @c NULL — otherwise, @a errno stores an error code.
 */

struct ndm_core_t *ndm_core_open(
		const char *const agent,
		const int cache_ttl_msec,
		const size_t cache_max_size) NDM_ATTR_WUR;

/**
 * Close the core connection if it was opened and clear cache. Release
 * the memory allocated for the @b ndm_core_t structure if the pointer is
 * not equal to @c NULL.
 *
 * @param core Pointer to the core connection instance.
 *
 * @returns @c true if the core connection is closed (a pointer to the core
 * connection instance is set to @c NULL), @c false — otherwise (@a errno
 * contains error code).
 *
 * @par Example
 * @snippet core_open_close.c open and close
 */

bool ndm_core_close(
		struct ndm_core_t **core);

/**
 * Get the descriptor of the core connection.
 *
 * @param core Pointer to the core connection instance.
 *
 * @returns The core connection descriptor.
 */

int ndm_core_fd(
		const struct ndm_core_t *core) NDM_ATTR_WUR;

/**
 * Forcibly clear the cache of the core connection.
 *
 * @param core Pointer to the core connection instance.
 * @param remove_all If @c true — all content is to be removed, if @c false —
 * only obsolete data is to be removed.
 */

void ndm_core_cache_clear(
		struct ndm_core_t *core,
		const bool remove_all);

/**
 * Set the timeout of data exchange through the connection.
 *
 * @param core Pointer to the core connection instance.
 * @param timeout The maximum waiting time for the request sending or
 * the response receiving in milliseconds.
 */

void ndm_core_set_timeout(
		struct ndm_core_t *core,
		const int timeout);

/**
 * Get the timeout of the core connection.
 *
 * @param core Pointer to the core connection instance.
 *
 * @returns Current value of the timeout.
 */

int ndm_core_get_timeout(
		const struct ndm_core_t *core) NDM_ATTR_WUR;

/**
 * ...
 *
 * @param core
 *
 * @returns
 */

const char *ndm_core_agent(
		const struct ndm_core_t *core) NDM_ATTR_WUR;

/**
 * Send a special request to authenticate a user-specified name and password,
 * according to the current user database which is stored in the core. Function
 * should be used in applications requiring access control at the user level.
 *
 * @param core Pointer to the core connection instance.
 * @param user The specified user name.
 * @param password The specified password.
 * @param realm Realm name of the account.
 * @param tag Tag, which a user must have for authentication success.
 * @param authenticated A pointer to a variable that contains the authentication
 * result.
 *
 * @returns @c true if completed successfully and @a authenticated contains
 * correct value, @c false — otherwise (@a errno contains error code).
 */

bool ndm_core_authenticate(
		struct ndm_core_t *core,
		const char *const user,
		const char *const password,
		const char *const realm,
		const char *const tag,
		bool *authenticated) NDM_ATTR_WUR;

/**
 * Send request to the core.
 *
 * @param core Pointer to the core connection instance.
 * @param request_type Type of request. There are three possible values ​​—
 * NDM_CORE_REQUEST_CONFIG, NDM_CORE_REQUEST_EXECUTE or NDM_CORE_REQUEST_PARSE.
 * @param cache_mode Cache mode of the response.
 * @param command_args An array of command arguments (ignored if request type is
 * NDM_CORE_REQUEST_PARSE).
 * @param command_format String template of command.
 *
 * @returns Pointer to the received response (@b ndm_core_response_t structure)
 * in case of a successful exchange with the core, @c NULL — otherwise
 * (@a errno contains error code).
 */

struct ndm_core_response_t *ndm_core_request(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const command_args[],
		const char *const command_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(5, 6);

/**
 * Get additional description for the command.
 * The request of the following form is sent to the core:
 * @code
 * <request id='5'>
 *   <help>ser</help>
 * </request>
 * @endcode
 * where 'help' node contains the command name for which an additional
 * description is needed.
 * The core response will have the following form:
 * @code
 * <response id="5">
 *   <help>
 *     <completion>vice</completion>
 *     <hint/>
 *     <command name="service">
 *       <description>manage services</description>
 *     </command>
 *   </help>
 *   <prompt>(config)</prompt>
 * </response>
 * @endcode
 *
 * @param core Pointer to the core connection instance.
 * @param cache_mode Cache mode of the response.
 * @param command The command name or a part of it.
 *
 * @returns Pointer to the received response (@b ndm_core_response_t structure)
 * in case of a successful exchange with the core, @c NULL — otherwise
 * (@a errno contains error code).
 */

struct ndm_core_response_t *ndm_core_get_help(
		struct ndm_core_t *core,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const command);

/**
 * Send 'continue' request to the core to get a current state of an active
 * background command.
 * The initial request:
 * @code
 * <request id="0">
 *   <parse>tools ping 127.0.0.1 count 2</parse>
 * </request>
 * @endcode
 * The first response:
 * @code
 * <packet>
 *   <response>
 *     <message>Sending ICMP ECHO request to 127.0.0.1</message>
 *     <message>PING 127.0.0.1 (127.0.0.1) 56 (84) bytes of data.</message>
 *     <message>84 bytes from 127.0.0.1: icmp_req=1, ttl=64, time=1.48 ms.</message>
 *     <continued/>
 *   </response>
 * </packet>
 * @endcode
 * The 'continue' request:
 * @code
 * <request id="1">
 *   <continue/>
 * </request>
 * @endcode
 * The second response:
 * @code
 * <packet>
 *   <response id="1">
 *     <message>84 bytes from 127.0.0.1: icmp_req=2, ttl=64, time=0.54 ms.</message>
 *     <message/>
 *     <message>--- 127.0.0.1 ping statistics -</message>
 *     <message>2 packets transmitted, 2 packets received, 0% packet loss,</message>
 *     <message>0 duplicate(s), time 1000.64 ms.</message>
 *     <message>Round-trip min/avg/max = 0.54/1.15/1.76 ms.</message>
 *     <continued/>
 *   </response>
 * </packet>
 * @endcode
 * The 'continue' request:
 * @code
 * <request id="2">
 *   <continue/>
 * </request>
 * @endcode
 * The final response:
 * @code
 * <packet>
 *   <response id="2">
 *     <message>Process terminated.</message>
 *     <prompt>(config)</prompt>
 *  </response>
 * </packet>
 * @endcode
 *
 * @param core Pointer to the core connection instance.
 *
 * @returns Pointer to the received response (@b ndm_core_response_t structure)
 * in case of a successful exchange with the core, @c NULL — otherwise
 * (@a errno contains error code).
 */

struct ndm_core_response_t *ndm_core_continue(
		struct ndm_core_t *core) NDM_ATTR_WUR;

/**
 * Send 'break' request to the core to stop the execution of an active
 * background command.
 * The initial request:
 * @code
 * <request id="0">
 *   <parse>tools ping 127.0.0.1</parse>
 * </request>
 * @endcode
 * The first response:
 * @code
 * <packet>
 *   <response>
 *     <message>Sending ICMP ECHO request to 127.0.0.1</message>
 *     <message>PING 127.0.0.1 (127.0.0.1) 56 (84) bytes of data.</message>
 *     <message>84 bytes from 127.0.0.1: icmp_req=1, ttl=64, time=1.80 ms.</message>
 *     <message>84 bytes from 127.0.0.1: icmp_req=2, ttl=64, time=0.53 ms.</message>
 *     <message>84 bytes from 127.0.0.1: icmp_req=3, ttl=64, time=0.53 ms.</message>
 *     <message>84 bytes from 127.0.0.1: icmp_req=4, ttl=64, time=0.54 ms.</message>
 *     <message>84 bytes from 127.0.0.1: icmp_req=5, ttl=64, time=0.54 ms.</message>
 *     <message>84 bytes from 127.0.0.1: icmp_req=6, ttl=64, time=0.54 ms.</message>
 *     <continued/>
 *   </response>
 * </packet>
 * @endcode
 * The 'break' request:
 * @code
 * <request id="1">
 *   <break/>
 * </request>
 * @endcode
 * The second response:
 * @code
 * <packet>
 *   <response id="1">
 *     <prompt>(config)</prompt>
 *  </response>
 * </packet>
 * @endcode
 *
 * @param core Pointer to the core connection instance.
 *
 * @returns Pointer to the received response (@b ndm_core_response_t structure)
 * in case of a successful exchange with the core, @c NULL — otherwise
 * (@a errno contains error code).
 */

struct ndm_core_response_t *ndm_core_break(
		struct ndm_core_t *core) NDM_ATTR_WUR;

/**
 * Check if the last core response contains 'message' node.
 *
 * @param core Pointer to the core connection instance.
 *
 * @returns @c true if contains, @c false — otherwise.
 */

bool ndm_core_last_message_received(
		struct ndm_core_t *core) NDM_ATTR_WUR;

/**
 * Get the type of the last message of core response. It makes sense if
 * ndm_core_last_message_received() returned @c true.
 *
 * @param core Pointer to the core connection instance.
 *
 * @returns The type of last message.
 */

enum ndm_core_response_type_t ndm_core_last_message_type(
		struct ndm_core_t *core) NDM_ATTR_WUR;

/**
 * Get the content of the last message of core response. It makes sense if
 * ndm_core_last_message_received() returned @c true.
 *
 * @param core Pointer to the core connection instance.
 *
 * @returns Pointer to a string that contains the message.
 */

const char *ndm_core_last_message_string(
		struct ndm_core_t *core) NDM_ATTR_WUR;

/**
 * Get the source-class of the last message of core response. It makes sense if
 * ndm_core_last_message_received() returned @c true.
 *
 * @param core Pointer to the core connection instance.
 *
 * @returns Pointer to the source-class string.
 */

const char *ndm_core_last_message_ident(
		struct ndm_core_t *core) NDM_ATTR_WUR;

/**
 * Get the source-class instance name of the last message of core response.
 * It makes sense if ndm_core_last_message_received() returned @c true.
 *
 * @param core Pointer to the core connection instance.
 *
 * @returns Pointer to the source-class instance name string.
 */

const char *ndm_core_last_message_source(
		struct ndm_core_t *core) NDM_ATTR_WUR;

/**
 * Get the code of the last message of core response.
 * It makes sense if ndm_core_last_message_received() returned @c true.
 *
 * @param core Pointer to the core connection instance.
 *
 * @returns The last message code.
 */

ndm_code_t ndm_core_last_message_code(
		struct ndm_core_t *core) NDM_ATTR_WUR;

/**
 * Release the memory that response instance occupies. After completion assigns
 * @c NULL to @a response.
 *
 * @param response Pointer to the response instance. The value can be @c NULL
 * as well as NULL-pointer to response instance.
 */

void ndm_core_response_free(
		struct ndm_core_response_t **response);

/**
 * Check if the core response contains message code which corresponds to
 * the result type @c INFO or @c WARNING.
 *
 * @param response Pointer to the response instance.
 *
 * @returns @c true if result type is @c INFO or @c WARNING, @c false —
 * otherwise.
 */

bool ndm_core_response_is_ok(
		const struct ndm_core_response_t *response) NDM_ATTR_WUR;

/**
 * Get the type of response reference to which contains in response structure.
 *
 * @param response Pointer to the response instance.
 *
 * @returns Type of response.
 */

enum ndm_core_response_type_t ndm_core_response_type(
		const struct ndm_core_response_t *response) NDM_ATTR_WUR;

/**
 * Check if the response contains 'continue' node.
 *
 * @param response Pointer to the response instance.
 *
 * @returns @c true if contains, @c false — otherwise.
 */

bool ndm_core_response_is_continued(
		const struct ndm_core_response_t *response) NDM_ATTR_WUR;

/**
 * Get the pointer to the root node of response XML structure.
 *
 * @param response Pointer to the response instance.
 *
 * @returns Pointer to the root node. @c NULL is never returned.
 */

const struct ndm_xml_node_t *ndm_core_response_root(
		const struct ndm_core_response_t *response) NDM_ATTR_WUR;

/**
 * Get the pointer to the response first node that matches the specified
 * criteria.
 *
 * @param node Root node, relative to which a request is made.
 * @param[out] value Reference to the result node.
 * @param value_path_format Path format for the node searching.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

enum ndm_core_response_error_t ndm_core_response_first_node(
		const struct ndm_xml_node_t *node,
		const struct ndm_xml_node_t **value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(3, 4);

/**
 * Get the pointer to the response first node that contains string value.
 *
 * @param node Root node, relative to which a request is made.
 * @param[out] value Pointer to a resulting string pointer.
 * @param value_path_format Path format for the node searching.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

enum ndm_core_response_error_t ndm_core_response_first_str(
		const struct ndm_xml_node_t *node,
		const char **value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(3, 4);

/**
 * Get the pointer to the response first node that contains
 * char integer value.
 *
 * @param node Root node, relative to which a request is made.
 * @param[out] value Pointer to a resulting char integer.
 * @param value_path_format Path format for the node searching.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

enum ndm_core_response_error_t ndm_core_response_first_char(
		const struct ndm_xml_node_t *node,
		char *value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(3, 4);

/**
 * Get the pointer to the response first node that contains
 * unsigned char integer value.
 *
 * @param node Root node, relative to which a request is made.
 * @param[out] value Pointer to a resulting unsigned char interger.
 * @param value_path_format Path format for the node searching.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

enum ndm_core_response_error_t ndm_core_response_first_uchar(
		const struct ndm_xml_node_t *node,
		unsigned char *value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(3, 4);

/**
 * Get the pointer to the response first node that contains
 * short integer value.
 *
 * @param node Root node, relative to which a request is made.
 * @param[out] value Pointer to a resulting short integer.
 * @param value_path_format Path format for the node searching.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

enum ndm_core_response_error_t ndm_core_response_first_short(
		const struct ndm_xml_node_t *node,
		short *value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(3, 4);

/**
 * Get the pointer to the response first node that contains
 * unsigned short integer value.
 *
 * @param node Root node, relative to which a request is made.
 * @param[out] value Pointer to a resulting unsigned short interger.
 * @param value_path_format Path format for the node searching.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

enum ndm_core_response_error_t ndm_core_response_first_ushort(
		const struct ndm_xml_node_t *node,
		unsigned short *value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(3, 4);

/**
 * Get the pointer to the response first node that contains integer value.
 *
 * @param node Root node, relative to which a request is made.
 * @param[out] value Pointer to a resulting integer.
 * @param value_path_format Path format for the node searching.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

enum ndm_core_response_error_t ndm_core_response_first_int(
		const struct ndm_xml_node_t *node,
		int *value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(3, 4);

/**
 * Get the pointer to the response first node that contains
 * unsigned integer value.
 *
 * @param node Root node, relative to which a request is made.
 * @param[out] value Pointer to a resulting unsigned interger.
 * @param value_path_format Path format for the node searching.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

enum ndm_core_response_error_t ndm_core_response_first_uint(
		const struct ndm_xml_node_t *node,
		unsigned int *value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(3, 4);

/**
 * Get the pointer to the response first node that contains
 * long integer value.
 *
 * @param node Root node, relative to which a request is made.
 * @param[out] value Pointer to a resulting long integer.
 * @param value_path_format Path format for the node searching.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

enum ndm_core_response_error_t ndm_core_response_first_long(
		const struct ndm_xml_node_t *node,
		long *value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(3, 4);

/**
 * Get the pointer to the response first node that contains
 * unsigned long integer value.
 *
 * @param node Root node, relative to which a request is made.
 * @param[out] value Pointer to a resulting unsigned long integer.
 * @param value_path_format Path format for the node searching.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

enum ndm_core_response_error_t ndm_core_response_first_ulong(
		const struct ndm_xml_node_t *node,
		unsigned long *value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(3, 4);

/**
 * Get the pointer to the response first node that contains
 * long long integer value.
 *
 * @param node Root node, relative to which a request is made.
 * @param[out] value Pointer to a resulting long long integer.
 * @param value_path_format Path format for the node searching.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

enum ndm_core_response_error_t ndm_core_response_first_llong(
		const struct ndm_xml_node_t *node,
		long long *value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(3, 4);

/**
 * Get the pointer to the response first node that contains
 * unsigned long long integer value.
 *
 * @param node Root node, relative to which a request is made.
 * @param[out] value Pointer to a resulting unsigned long long integer.
 * @param value_path_format Path format for the node searching.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

enum ndm_core_response_error_t ndm_core_response_first_ullong(
		const struct ndm_xml_node_t *node,
		unsigned long long *value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(3, 4);

/**
 * Get the pointer to the response first node that contains boolean value.
 * The following values are considered as positive: non-zero integer, strings
 * @a yes, @a true, @a up and @a on. As negative: @c 0, @a no, @a false,
 * @a down and @a off. All string values ​​are case-insensitive.
 *
 * @param node Root node, relative to which a request is made.
 * @param parse_value @c true if standard function processing is needed with
 * appropriate error handling, @c false if you need to know about the presence
 * or absence of the required value only.
 * @param[out] value @c true if the required value is found, @c false if not.
 * @param value_path_format Path format for the node searching.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

enum ndm_core_response_error_t ndm_core_response_first_bool(
		const struct ndm_xml_node_t *node,
		const bool parse_value,
		bool *value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(4, 5);

/**
 * Send 'break' request (see ndm_core_break()) followed by analysis of the core
 * response.
 *
 * @param core Pointer to the core connection instance.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

enum ndm_core_response_error_t ndm_core_request_break(
		struct ndm_core_t *core) NDM_ATTR_WUR;

/**
 * Send request (see ndm_core_request()) followed by analysis of the core
 * response. Is used in the simplest cases, when enough to know was or was not
 * correctly processed the request.
 *
 * @param core Pointer to the core connection instance.
 * @param request_type Type of request.
 * @param command_args An array of command arguments (is used for
 * @c NDM_CORE_REQUEST_CONFIG or @c NDM_CORE_REQUEST_EXECUTE types of request
 * only).
 * @param command_format Format of command.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

enum ndm_core_response_error_t ndm_core_request_send(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const char *const command_args[],
		const char *const command_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(4, 5);

/**
 * Send request and return the string value of the required node with memory
 * allocation.
 *
 * @param core Pointer to the core connection instance.
 * @param request_type Type of request.
 * @param cache_mode Cache mode of the response.
 * @param value Pointer to the allocated memory with the stored value.
 * @param value_path Path to the node/attribute value of which is needed.
 * @param command_args An array of command arguments (is used for
 * @c NDM_CORE_REQUEST_CONFIG or @c NDM_CORE_REQUEST_EXECUTE types of request
 * only).
 * @param command_format Format of command.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
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

/**
 * Send request and return the string value of the required node to the buffer.
 *
 * @param core Pointer to the core connection instance.
 * @param request_type Type of request.
 * @param cache_mode Cache mode of the response.
 * @param value Pointer to the buffer with the stored value.
 * @param value_buffer_size Size of the buffer.
 * @param value_size Actual size of the data to store the value (see
 * @c NDM_CORE_RESPONSE_ERROR_BUFFER_SIZE).
 * @param value_path Path to the node/attribute value of which is needed.
 * @param command_args An array of command arguments (is used for
 * @c NDM_CORE_REQUEST_CONFIG or @c NDM_CORE_REQUEST_EXECUTE types of request
 * only).
 * @param command_format Format of command.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

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

/**
 * Send request and return the integer value of the required node to the buffer.
 *
 * @param core Pointer to the core connection instance.
 * @param request_type Type of request.
 * @param cache_mode Cache mode of the response.
 * @param value Pointer to the buffer with the stored value.
 * @param value_path Path to the node/attribute value of which is needed.
 * @param command_args An array of command arguments (is used for
 * @c NDM_CORE_REQUEST_CONFIG or @c NDM_CORE_REQUEST_EXECUTE types of request
 * only).
 * @param command_format Format of command.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

enum ndm_core_response_error_t ndm_core_request_first_int_cf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		int *value,
		const char *const value_path,
		const char *const command_args[],
		const char *const command_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(7, 8);

/**
 * Send request and return the unsigned integer value of the required node
 * to the buffer.
 *
 * @param core Pointer to the core connection instance.
 * @param request_type Type of request.
 * @param cache_mode Cache mode of the response.
 * @param value Pointer to the buffer with the stored value.
 * @param value_path Path to the node/attribute value of which is needed.
 * @param command_args An array of command arguments (is used for
 * @c NDM_CORE_REQUEST_CONFIG or @c NDM_CORE_REQUEST_EXECUTE types of request
 * only).
 * @param command_format Format of command.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

enum ndm_core_response_error_t ndm_core_request_first_uint_cf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		unsigned int *value,
		const char *const value_path,
		const char *const command_args[],
		const char *const command_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(7, 8);

/**
 * Send request and return the boolean value of the required node to the buffer.
 * The following values are considered as positive: non-zero integer, strings
 * @a yes, @a true, @a up and @a on. As negative: @c 0, @a no, @a false,
 * @a down and @a off. All string values ​​are case-insensitive.
 *
 * @param core Pointer to the core connection instance.
 * @param request_type Type of request.
 * @param cache_mode Cache mode of the response.
 * @param parse_value @c true if standard function processing is needed with
 * appropriate error handling, @c false if you need to know about the presence
 * or absence of the required value only.
 * @param value @c true if the required value is found, @c false if not.
 * @param value_path Path to the node/attribute value of which is needed.
 * @param command_args An array of command arguments (is used for
 * @c NDM_CORE_REQUEST_CONFIG or @c NDM_CORE_REQUEST_EXECUTE types of request
 * only).
 * @param command_format Format of command.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

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
 * Send request and return the string value of the required node with memory
 * allocation.
 *
 * @param core Pointer to the core connection instance.
 * @param request_type Type of request.
 * @param cache_mode Cache mode of the response.
 * @param command_args An array of command arguments (is used for
 * @c NDM_CORE_REQUEST_CONFIG or @c NDM_CORE_REQUEST_EXECUTE types of request
 * only).
 * @param command Command name.
 * @param value Pointer to the allocated memory with the stored value.
 * @param value_path_format Format of path to the node/attribute value of which
 * is needed.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
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

/**
 * Send request and return the string value of the required node to the buffer.
 *
 * @param core Pointer to the core connection instance.
 * @param request_type Type of request.
 * @param cache_mode Cache mode of the response.
 * @param command_args An array of command arguments (is used for
 * @c NDM_CORE_REQUEST_CONFIG or @c NDM_CORE_REQUEST_EXECUTE types of request
 * only).
 * @param command Command name.
 * @param value Pointer to the buffer with the stored value.
 * @param value_buffer_size Size of the buffer.
 * @param value_size Actual size of the data to store the value (see
 * @c NDM_CORE_RESPONSE_ERROR_BUFFER_SIZE).
 * @param value_path_format Format of path to the node/attribute value of which
 * is needed.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

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

/**
 * Send request and return the integer value of the required node to the buffer.
 *
 * @param core Pointer to the core connection instance.
 * @param request_type Type of request.
 * @param cache_mode Cache mode of the response.
 * @param command_args An array of command arguments (is used for
 * @c NDM_CORE_REQUEST_CONFIG or @c NDM_CORE_REQUEST_EXECUTE types of request
 * only).
 * @param command Command name.
 * @param value Pointer to the buffer with the stored value.
 * @param value_path_format Format of path to the node/attribute value of which
 * is needed.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

enum ndm_core_response_error_t ndm_core_request_first_int_pf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const command_args[],
		const char *const command,
		int *value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(7, 8);

/**
 * Send request and return the unsigned integer value of the required node
 * to the buffer.
 *
 * @param core Pointer to the core connection instance.
 * @param request_type Type of request.
 * @param cache_mode Cache mode of the response.
 * @param command_args An array of command arguments (is used for
 * @c NDM_CORE_REQUEST_CONFIG or @c NDM_CORE_REQUEST_EXECUTE types of request
 * only).
 * @param command Command name.
 * @param value Pointer to the buffer with the stored value.
 * @param value_path_format Format of path to the node/attribute value of which
 * is needed.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

enum ndm_core_response_error_t ndm_core_request_first_uint_pf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const command_args[],
		const char *const command,
		unsigned int *value,
		const char *const value_path_format,
		...) NDM_ATTR_WUR NDM_ATTR_PRINTF(7, 8);

/**
 * Send request and return the boolean value of the required node to the buffer.
 * The following values are considered as positive: non-zero integer, strings
 * @a yes, @a true, @a up and @a on. As negative: @c 0, @a no, @a false,
 * @a down and @a off. All string values ​​are case-insensitive.
 *
 * @param core Pointer to the core connection instance.
 * @param request_type Type of request.
 * @param cache_mode Cache mode of the response.
 * @param command_args An array of command arguments (is used for
 * @c NDM_CORE_REQUEST_CONFIG or @c NDM_CORE_REQUEST_EXECUTE types of request
 * only).
 * @param command Command name.
 * @param parse_value @c true if standard function processing is needed with
 * appropriate error handling, @c false if you need to know about the presence
 * or absence of the required value only.
 * @param value @c true if the required value is found, @c false if not.
 * @param value_path_format Format of path to the node/attribute value of which
 * is needed.
 *
 * @returns Result of a function of @b ndm_core_response_error_t type.
 */

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

