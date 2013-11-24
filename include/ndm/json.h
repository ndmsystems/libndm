#ifndef __NDM_JSON_H__
#define __NDM_JSON_H__

#include <stdbool.h>
#include "attr.h"

enum ndm_json_print_flags_t
{
	NDM_JSON_PRINT_FLAGS_COMPACT						= 0x0001,
	NDM_JSON_PRINT_FLAGS_CRLF							= 0x0002,

	NDM_JSON_PRINT_FLAGS_DEFAULT						=
		NDM_JSON_PRINT_FLAGS_COMPACT
};

enum ndm_json_parse_error_t
{
	NDM_JSON_PARSE_ERROR_OK,
	NDM_JSON_PARSE_ERROR_ARRAY,
	NDM_JSON_PARSE_ERROR_OBJECT,
	NDM_JSON_PARSE_ERROR_OOM,
	NDM_JSON_PARSE_ERROR_NULL_EXPECTED,
	NDM_JSON_PARSE_ERROR_FALSE_EXPECTED,
	NDM_JSON_PARSE_ERROR_TRUE_EXPECTED,
	NDM_JSON_PARSE_ERROR_INVALID_HEX,
	NDM_JSON_PARSE_ERROR_CORRUPTED_SURROGATE_PAIR,
	NDM_JSON_PARSE_ERROR_INVALID_SURROGATE_PAIR,
	NDM_JSON_PARSE_ERROR_INVALID_ESCAPE_CHAR,
	NDM_JSON_PARSE_ERROR_UNTERMINATED_STRING,
	NDM_JSON_PARSE_ERROR_UNESCAPED_CHAR,
	NDM_JSON_PARSE_ERROR_NUMBER_LEADING_ZERO,
	NDM_JSON_PARSE_ERROR_NUMBER_EXPECTED,
	NDM_JSON_PARSE_ERROR_NUMBER_FRAC_EXPECTED,
	NDM_JSON_PARSE_ERROR_NUMBER_EXP_EXPECTED,
	NDM_JSON_PARSE_ERROR_NUMBER_RANGE,
	NDM_JSON_PARSE_ERROR_CORRUPTED_ARRAY,
	NDM_JSON_PARSE_ERROR_STRING_EXPECTED,
	NDM_JSON_PARSE_ERROR_COLON_EXPECTED,
	NDM_JSON_PARSE_ERROR_CORRUPTED_OBJECT,
	NDM_JSON_PARSE_ERROR_EMPTY_DOCUMENT,
	NDM_JSON_PARSE_ERROR_TRAILING_SYMBOLS,
	NDM_JSON_PARSE_ERROR_UNKNOWN_TYPE
};

struct ndm_pool_t;

struct ndm_json_value_t;			//!< generic JSON value
struct ndm_json_array_t;			//!< JSON array
struct ndm_json_object_t;			//!< JSON object
struct ndm_json_array_element_t;	//!< JSON array element
struct ndm_json_object_member_t;	//!< JSON object member

/**
 * Returns a parent for a given @c value.
 * If the value is root one, @a NULL returned.
 * The @a NULL @a value not allowed.
 **/

struct ndm_json_value_t *ndm_json_value_parent(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

/**
 * Value predicates.
 * @c ndm_json_value_is_*() group of functions returns @c true
 * if a @a value has a type defined by a specific name of a function.
 * The @a NULL @a value pointer not allowed.
 **/

bool ndm_json_value_is_null(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

bool ndm_json_value_is_boolean(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

bool ndm_json_value_is_string(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

bool ndm_json_value_is_number(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

bool ndm_json_value_is_char(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

bool ndm_json_value_is_uchar(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

bool ndm_json_value_is_short(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

bool ndm_json_value_is_ushort(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

bool ndm_json_value_is_int(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

bool ndm_json_value_is_uint(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

bool ndm_json_value_is_long(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

bool ndm_json_value_is_ulong(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

bool ndm_json_value_is_llong(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

bool ndm_json_value_is_ullong(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

bool ndm_json_value_is_double(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

bool ndm_json_value_is_array(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

bool ndm_json_value_is_object(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

/**
 * @c ndm_json_value_*() group of functions returns corresponding values
 * if an input JSON @a value has a type defined by a specific name
 * of a function. If the @a value has a different type returned value
 * unspecified.
 * The @a NULL @a value pointer not allowed.
 **/

bool ndm_json_value_boolean(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

const char *ndm_json_value_string(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

char ndm_json_value_char(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

unsigned char ndm_json_value_uchar(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

short ndm_json_value_short(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

unsigned short ndm_json_value_ushort(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

int ndm_json_value_int(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

unsigned int ndm_json_value_uint(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

long ndm_json_value_long(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

unsigned long ndm_json_value_ulong(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

long long ndm_json_value_llong(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

unsigned long long ndm_json_value_ullong(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

double ndm_json_value_double(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

struct ndm_json_array_t *ndm_json_value_array(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

struct ndm_json_object_t *ndm_json_value_object(
		const struct ndm_json_value_t *const value) NDM_ATTR_WUR;

/**
 * Array functions.
 **/

/**
 * @c ndm_json_array_new() returns a pointer to a newly allocated JSON
 * array using a given @c pool.
 * On memory error @a NULL returned.
 * The @a NULL @a pool pointer not allowed.
 **/

struct ndm_json_array_t *ndm_json_array_new(
		struct ndm_pool_t *pool) NDM_ATTR_WUR;

/**
 * Returns a pointer to a pool where an @c array allocated.
 * The @a NULL @a array not allowed.
 */

struct ndm_pool_t *ndm_json_array_pool(
		const struct ndm_json_array_t *const array) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_array_value(
		const struct ndm_json_array_t *const array) NDM_ATTR_WUR;

/**
 * The @a NULL @a array pointer not allowed.
 **/

bool ndm_json_array_is_empty(
		const struct ndm_json_array_t *array) NDM_ATTR_WUR;

size_t ndm_json_array_size(
		const struct ndm_json_array_t *array) NDM_ATTR_WUR;

struct ndm_json_array_element_t *ndm_json_array_element_first(
		const struct ndm_json_array_t *array) NDM_ATTR_WUR;

struct ndm_json_array_element_t *ndm_json_array_element_last(
		const struct ndm_json_array_t *array) NDM_ATTR_WUR;

struct ndm_json_array_element_t *ndm_json_array_element_next(
		const struct ndm_json_array_element_t *element) NDM_ATTR_WUR;

struct ndm_json_array_element_t *ndm_json_array_element_prev(
		const struct ndm_json_array_element_t *element) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_array_element_value(
		const struct ndm_json_array_element_t *element) NDM_ATTR_WUR;

/**
 * @c ndm_json_array_push_*() group of functions adds a new array
 * @a value. Each function allocates memory from a pool of an @c array.
 * Returns @c true on success and @c false if the input @a array
 * is @a NULL or there is not enough memory.
 **/

struct ndm_json_value_t *ndm_json_array_push_null(
		struct ndm_json_array_t *array) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_array_push_boolean(
		struct ndm_json_array_t *array,
		const bool value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_array_push_string(
		struct ndm_json_array_t *array,
		const char *const value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_array_push_char(
		struct ndm_json_array_t *array,
		const char value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_array_push_uchar(
		struct ndm_json_array_t *array,
		const unsigned char value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_array_push_short(
		struct ndm_json_array_t *array,
		const short value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_array_push_ushort(
		struct ndm_json_array_t *array,
		const unsigned short value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_array_push_int(
		struct ndm_json_array_t *array,
		const int value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_array_push_uint(
		struct ndm_json_array_t *array,
		const unsigned int value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_array_push_long(
		struct ndm_json_array_t *array,
		const long value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_array_push_ulong(
		struct ndm_json_array_t *array,
		const unsigned long value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_array_push_llong(
		struct ndm_json_array_t *array,
		const long long value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_array_push_ullong(
		struct ndm_json_array_t *array,
		const unsigned long long value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_array_push_double(
		struct ndm_json_array_t *array,
		const double value) NDM_ATTR_WUR;

struct ndm_json_array_t *ndm_json_array_push_array(
		struct ndm_json_array_t *array) NDM_ATTR_WUR;

struct ndm_json_object_t *ndm_json_array_push_object(
		struct ndm_json_array_t *array) NDM_ATTR_WUR;

/**
 * @c ndm_json_array_print() returns a string representation
 * of a JSON @a array allocated in the heap.
 * The @a array can not be @a NULL.
 * On success @a json_size contains a pointer to the JSON string length
 * (without a null-terminator), if it was not @a NULL.
 * Returns @a NULL on memory error.
 **/

char *ndm_json_array_print(
		const struct ndm_json_array_t *const array,
		const enum ndm_json_print_flags_t flags,
		size_t *json_size) NDM_ATTR_WUR;

/**
 * @c ndm_json_array_parse() tries to do in situ parsing
 * of a @a json string to a JSON @a array using a @a pool.
 * On success @a *array points to a newly allocated root JSON array.
 * Returns an error of type @c ndm_json_parse_error_t.
 * All string values of the JSON array and all its children
 * are stored in the input @a json string.
 **/

enum ndm_json_parse_error_t ndm_json_array_parse(
		struct ndm_pool_t *pool,
		char *json,
		struct ndm_json_array_t **array) NDM_ATTR_WUR;

/**
 * Object functions.
 **/

/**
 * @c ndm_json_object_new() returns a pointer to a newly allocated JSON
 * object using a given @c pool.
 * On memory error @a NULL returned.
 * The @a NULL @a pool pointer not allowed.
 **/

struct ndm_json_object_t *ndm_json_object_new(
		struct ndm_pool_t *pool) NDM_ATTR_WUR;

struct ndm_pool_t *ndm_json_object_pool(
		const struct ndm_json_object_t *const object) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_object_value(
		const struct ndm_json_object_t *const object) NDM_ATTR_WUR;

/**
 * Returns a pointer to a pool where an @c object allocated.
 * The @a NULL @a object not allowed.
 */

bool ndm_json_object_is_empty(
		const struct ndm_json_object_t *object) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_object_get(
		const struct ndm_json_object_t *object,
		const char *const name) NDM_ATTR_WUR;

struct ndm_json_object_member_t *ndm_json_object_member_first(
		const struct ndm_json_object_t *object) NDM_ATTR_WUR;

struct ndm_json_object_member_t *ndm_json_object_member_last(
		const struct ndm_json_object_t *object) NDM_ATTR_WUR;

struct ndm_json_object_member_t *ndm_json_object_member_next(
		const struct ndm_json_object_member_t *member) NDM_ATTR_WUR;

struct ndm_json_object_member_t *ndm_json_object_member_prev(
		const struct ndm_json_object_member_t *member) NDM_ATTR_WUR;

const char *ndm_json_object_member_name(
		const struct ndm_json_object_member_t *member) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_object_member_value(
		const struct ndm_json_object_member_t *member) NDM_ATTR_WUR;

/**
 * @c ndm_json_object_set_*() group of functions adds a new object
 * @a name / @a value pair. Each function copies a value name and
 * allocates memory from a pool of an @c object.
 * Returns @c true on success and @c false if the input @a object
 * or @a value is @a NULL or there is not enough memory.
 **/

struct ndm_json_value_t *ndm_json_object_set_null(
		struct ndm_json_object_t *object,
		const char *const name) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_object_set_boolean(
		struct ndm_json_object_t *object,
		const char *const name,
		const bool value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_object_set_string(
		struct ndm_json_object_t *object,
		const char *const name,
		const char *const value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_object_set_char(
		struct ndm_json_object_t *object,
		const char *const name,
		const char value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_object_set_uchar(
		struct ndm_json_object_t *object,
		const char *const name,
		const unsigned char value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_object_set_short(
		struct ndm_json_object_t *object,
		const char *const name,
		const short value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_object_set_ushort(
		struct ndm_json_object_t *object,
		const char *const name,
		const unsigned short value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_object_set_int(
		struct ndm_json_object_t *object,
		const char *const name,
		const int value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_object_set_uint(
		struct ndm_json_object_t *object,
		const char *const name,
		const unsigned int value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_object_set_long(
		struct ndm_json_object_t *object,
		const char *const name,
		const long value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_object_set_ulong(
		struct ndm_json_object_t *object,
		const char *const name,
		const unsigned long value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_object_set_llong(
		struct ndm_json_object_t *object,
		const char *const name,
		const long long value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_object_set_ullong(
		struct ndm_json_object_t *object,
		const char *const name,
		const unsigned long long value) NDM_ATTR_WUR;

struct ndm_json_value_t *ndm_json_object_set_double(
		struct ndm_json_object_t *object,
		const char *const name,
		const double value) NDM_ATTR_WUR;

struct ndm_json_array_t *ndm_json_object_set_array(
		struct ndm_json_object_t *object,
		const char *const name) NDM_ATTR_WUR;

struct ndm_json_object_t *ndm_json_object_set_object(
		struct ndm_json_object_t *object,
		const char *const name) NDM_ATTR_WUR;

/**
 * @c ndm_json_object_print() returns a string representation
 * of a JSON @a object allocated in the heap.
 * The @a object can not be @a NULL.
 * On success @a json_size contains a pointer to the JSON string length
 * (without a null-terminator), if it was not @a NULL.
 * Returns @a NULL on memory error.
 **/

char *ndm_json_object_print(
		const struct ndm_json_object_t *const object,
		const enum ndm_json_print_flags_t flags,
		size_t *json_size) NDM_ATTR_WUR;

/**
 * @c ndm_json_object_parse() tries to do in situ parsing
 * of a @a json string to a JSON @a object using a @a pool.
 * On success @a *object points to a newly allocated root JSON object.
 * Returns an error of type @c ndm_json_parse_error_t.
 * All string values of the object and all its children
 * are stored in the input @a json string.
 **/

enum ndm_json_parse_error_t ndm_json_object_parse(
		struct ndm_pool_t *pool,
		char *json,
		struct ndm_json_object_t **object) NDM_ATTR_WUR;

#endif /* __NDM_JSON_H__ */

