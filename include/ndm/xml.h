#ifndef __NDM_XML__
#define __NDM_XML__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "attr.h"
#include "pool.h"

enum ndm_xml_node_type_t
{
	NDM_XML_NODE_TYPE_DOCUMENT,
	NDM_XML_NODE_TYPE_ELEMENT,
	NDM_XML_NODE_TYPE_DATA,
	NDM_XML_NODE_TYPE_CDATA,
	NDM_XML_NODE_TYPE_COMMENT,
	NDM_XML_NODE_TYPE_DECLARATION,
	NDM_XML_NODE_TYPE_DOCTYPE,
	NDM_XML_NODE_TYPE_PI
};

enum ndm_xml_document_parse_flags_t
{
	NDM_XML_DOCUMENT_PARSE_FLAGS_EMPTY					= 0x0000,
	NDM_XML_DOCUMENT_PARSE_FLAGS_NO_DATA_NODES			= 0x0001,
	NDM_XML_DOCUMENT_PARSE_FLAGS_NO_ELEMENT_VALUES		= 0x0002,
	NDM_XML_DOCUMENT_PARSE_FLAGS_NO_ENTITY_TRANSLATION	= 0x0008,
	NDM_XML_DOCUMENT_PARSE_FLAGS_NO_UTF8				= 0x0010,
	NDM_XML_DOCUMENT_PARSE_FLAGS_PARSE_DECLARATION_NODE	= 0x0020,
	NDM_XML_DOCUMENT_PARSE_FLAGS_PARSE_COMMENT_NODES	= 0x0040,
	NDM_XML_DOCUMENT_PARSE_FLAGS_PARSE_DOCTYPE_NODE		= 0x0080,
	NDM_XML_DOCUMENT_PARSE_FLAGS_PARSE_PI_NODES			= 0x0100,
	NDM_XML_DOCUMENT_PARSE_FLAGS_CHECK_CLOSING_TAGS		= 0x0200,
	NDM_XML_DOCUMENT_PARSE_FLAGS_TRIM_WHITESPACE		= 0x0400,
	NDM_XML_DOCUMENT_PARSE_FLAGS_NORMALIZE_WHITESPACE	= 0x0800,

	NDM_XML_DOCUMENT_PARSE_FLAGS_DEFAULT				=
		NDM_XML_DOCUMENT_PARSE_FLAGS_PARSE_DECLARATION_NODE	|
		NDM_XML_DOCUMENT_PARSE_FLAGS_PARSE_COMMENT_NODES	|
		NDM_XML_DOCUMENT_PARSE_FLAGS_PARSE_DOCTYPE_NODE		|
		NDM_XML_DOCUMENT_PARSE_FLAGS_PARSE_PI_NODES			|
		NDM_XML_DOCUMENT_PARSE_FLAGS_CHECK_CLOSING_TAGS
};

enum ndm_xml_document_parse_error_t
{
	NDM_XML_DOCUMENT_PARSE_ERROR_OK,
	NDM_XML_DOCUMENT_PARSE_ERROR_OOM,
	NDM_XML_DOCUMENT_PARSE_ERROR_INVALID_CHAR_ENTITY,
	NDM_XML_DOCUMENT_PARSE_ERROR_UNEXPECTED_STREAM_END,
	NDM_XML_DOCUMENT_PARSE_ERROR_UNEXPECTED_CDATA_END,
	NDM_XML_DOCUMENT_PARSE_ERROR_CLOSING_TAG,
	NDM_XML_DOCUMENT_PARSE_ERROR_NO_SEMICOLON,
	NDM_XML_DOCUMENT_PARSE_ERROR_ATTR_NAME_EXPECTED,
	NDM_XML_DOCUMENT_PARSE_ERROR_EQUAL_SIGN_EXPECTED,
	NDM_XML_DOCUMENT_PARSE_ERROR_QUOTE_EXPECTED,
	NDM_XML_DOCUMENT_PARSE_ERROR_DECL_END_EXPECTED,
	NDM_XML_DOCUMENT_PARSE_ERROR_PI_TARGET_EXPECTED,
	NDM_XML_DOCUMENT_PARSE_ERROR_NODE_NAME_EXPECTED,
	NDM_XML_DOCUMENT_PARSE_ERROR_RBRACKET_EXPECTED,
	NDM_XML_DOCUMENT_PARSE_ERROR_LBRACKET_EXPECTED
};

struct ndm_xml_node_t;
struct ndm_xml_attr_t;

struct ndm_xml_document_t
{
	struct ndm_xml_node_t *__root;
	struct ndm_pool_t __pool;
};

#define NDM_XML_DOCUMENT_INITIALIZER(			\
		static_buffer,							\
		static_buffer_size,						\
		dynamic_buffer_size)					\
	{											\
		.__root = NULL,							\
		.__pool = NDM_POOL_INITIALIZER(			\
			static_buffer,						\
			static_buffer_size,					\
			dynamic_buffer_size),				\
	}

/**
 * XML document functions.
 */

void ndm_xml_document_init(
		struct ndm_xml_document_t *doc,
		void *static_buffer,
		const size_t static_buffer_size,
		const size_t dynamic_buffer_size);

bool ndm_xml_document_copy(
		struct ndm_xml_document_t *dest,
		const struct ndm_xml_document_t *source) NDM_ATTR_WUR;

bool ndm_xml_document_is_equal(
		const struct ndm_xml_document_t *doc,
		const struct ndm_xml_document_t *other) NDM_ATTR_WUR;

enum ndm_xml_document_parse_error_t ndm_xml_document_parse(
		struct ndm_xml_document_t *doc,
		char *text,
		const enum ndm_xml_document_parse_flags_t flags) NDM_ATTR_WUR;

void *ndm_xml_document_alloc(
		struct ndm_xml_document_t *doc,
		const size_t size) NDM_ATTR_WUR;

struct ndm_xml_node_t *ndm_xml_document_alloc_root(
		struct ndm_xml_document_t *doc) NDM_ATTR_WUR;

struct ndm_xml_node_t *ndm_xml_document_alloc_node(
		struct ndm_xml_document_t *doc,
		const enum ndm_xml_node_type_t type,
		const char *const name,
		const char *const value) NDM_ATTR_WUR;

struct ndm_xml_attr_t *ndm_xml_document_alloc_attr(
		struct ndm_xml_document_t *doc,
		const char *const name,
		const char *const value) NDM_ATTR_WUR;

char *ndm_xml_document_alloc_str(
		struct ndm_xml_document_t *doc,
		const char *const s) NDM_ATTR_WUR;

char *ndm_xml_document_alloc_strn(
		struct ndm_xml_document_t *doc,
		const char *const s,
		const size_t size) NDM_ATTR_WUR;

void ndm_xml_document_clear(
		struct ndm_xml_document_t *doc);

bool ndm_xml_document_is_valid(
		const struct ndm_xml_document_t *doc) NDM_ATTR_WUR;

bool ndm_xml_document_is_empty(
		const struct ndm_xml_document_t *doc) NDM_ATTR_WUR;

size_t ndm_xml_document_size(
		const struct ndm_xml_document_t *doc) NDM_ATTR_WUR;

size_t ndm_xml_document_allocated_size(
		const struct ndm_xml_document_t *doc) NDM_ATTR_WUR;

struct ndm_xml_node_t *ndm_xml_document_root(
		const struct ndm_xml_document_t *doc) NDM_ATTR_WUR;

/**
 * XML node functions.
 */

enum ndm_xml_node_type_t ndm_xml_node_type(
		const struct ndm_xml_node_t *node) NDM_ATTR_WUR;

const char *ndm_xml_node_name(
		const struct ndm_xml_node_t *node) NDM_ATTR_WUR;

size_t ndm_xml_node_name_size(
		const struct ndm_xml_node_t *node) NDM_ATTR_WUR;

const char *ndm_xml_node_value(
		const struct ndm_xml_node_t *node) NDM_ATTR_WUR;

size_t ndm_xml_node_value_size(
		const struct ndm_xml_node_t *node) NDM_ATTR_WUR;

struct ndm_xml_node_t *ndm_xml_node_parent(
		const struct ndm_xml_node_t *node) NDM_ATTR_WUR;

void ndm_xml_node_set_name(
		struct ndm_xml_node_t *node,
		const char *const value);

void ndm_xml_node_set_value(
		struct ndm_xml_node_t *node,
		const char *const value);

struct ndm_xml_document_t *ndm_xml_node_document(
		const struct ndm_xml_node_t *node) NDM_ATTR_WUR;

struct ndm_xml_node_t *ndm_xml_node_first_child(
		const struct ndm_xml_node_t *node,
		const char *const name) NDM_ATTR_WUR;

struct ndm_xml_node_t *ndm_xml_node_last_child(
		const struct ndm_xml_node_t *node,
		const char *const name) NDM_ATTR_WUR;

struct ndm_xml_node_t *ndm_xml_node_next_sibling(
		const struct ndm_xml_node_t *node,
		const char *const name) NDM_ATTR_WUR;

struct ndm_xml_node_t *ndm_xml_node_prev_sibling(
		const struct ndm_xml_node_t *node,
		const char *const name) NDM_ATTR_WUR;

struct ndm_xml_attr_t *ndm_xml_node_first_attr(
		const struct ndm_xml_node_t *node,
		const char *const name) NDM_ATTR_WUR;

struct ndm_xml_attr_t *ndm_xml_node_last_attr(
		const struct ndm_xml_node_t *node,
		const char *const name) NDM_ATTR_WUR;

void ndm_xml_node_prepend_child(
		struct ndm_xml_node_t *node,
		struct ndm_xml_node_t *child);

void ndm_xml_node_append_child(
		struct ndm_xml_node_t *node,
		struct ndm_xml_node_t *child);

struct ndm_xml_node_t *ndm_xml_node_append_child_str(
		struct ndm_xml_node_t *node,
		const char *const name,
		const char *const value);

struct ndm_xml_node_t *ndm_xml_node_append_child_int(
		struct ndm_xml_node_t *node,
		const char *const name,
		const intmax_t value);

struct ndm_xml_node_t *ndm_xml_node_append_child_uint(
		struct ndm_xml_node_t *node,
		const char *const name,
		const uintmax_t value);

struct ndm_xml_attr_t *ndm_xml_node_append_attr_str(
		struct ndm_xml_node_t *node,
		const char *const name,
		const char *const value);

struct ndm_xml_attr_t *ndm_xml_node_append_attr_int(
		struct ndm_xml_node_t *node,
		const char *const name,
		const intmax_t value);

struct ndm_xml_attr_t *ndm_xml_node_append_attr_uint(
		struct ndm_xml_node_t *node,
		const char *const name,
		const uintmax_t value);

void ndm_xml_node_insert_child(
		struct ndm_xml_node_t *node,
		struct ndm_xml_node_t *where,
		struct ndm_xml_node_t *child);

void ndm_xml_node_remove_first_child(
		struct ndm_xml_node_t *node);

void ndm_xml_node_remove_last_child(
		struct ndm_xml_node_t *node);

void ndm_xml_node_remove_child(
		struct ndm_xml_node_t *node,
		struct ndm_xml_node_t *child);

void ndm_xml_node_remove_all_children(
		struct ndm_xml_node_t *node,
		struct ndm_xml_node_t *start_child);

void ndm_xml_node_prepend_attr(
		struct ndm_xml_node_t *node,
		struct ndm_xml_attr_t *attr);

void ndm_xml_node_append_attr(
		struct ndm_xml_node_t *node,
		struct ndm_xml_attr_t *attr);

void ndm_xml_node_insert_attr(
		struct ndm_xml_node_t *node,
		struct ndm_xml_attr_t *where,
		struct ndm_xml_attr_t *attr);

void ndm_xml_node_remove_first_attr(
		struct ndm_xml_node_t *node);

void ndm_xml_node_remove_last_attr(
		struct ndm_xml_node_t *node);

void ndm_xml_node_remove_attr(
		struct ndm_xml_node_t *node,
		struct ndm_xml_attr_t *where);

void ndm_xml_node_remove_all_attr(
		struct ndm_xml_node_t *node);

/**
 * XML attribute functions.
 */

const char *ndm_xml_attr_name(
		const struct ndm_xml_attr_t *attr) NDM_ATTR_WUR;

size_t ndm_xml_attr_name_size(
		const struct ndm_xml_attr_t *attr) NDM_ATTR_WUR;

const char *ndm_xml_attr_value(
		const struct ndm_xml_attr_t *attr) NDM_ATTR_WUR;

size_t ndm_xml_attr_value_size(
		const struct ndm_xml_attr_t *attr) NDM_ATTR_WUR;

void ndm_xml_attr_set_name(
		struct ndm_xml_attr_t *attr,
		const char *const name);

void ndm_xml_attr_set_value(
		struct ndm_xml_attr_t *attr,
		const char *const value);

struct ndm_xml_node_t *ndm_xml_attr_node(
		const struct ndm_xml_attr_t *attr) NDM_ATTR_WUR;

struct ndm_xml_document_t *ndm_xml_attr_document(
		const struct ndm_xml_attr_t *attr) NDM_ATTR_WUR;

struct ndm_xml_attr_t *ndm_xml_attr_next(
		const struct ndm_xml_attr_t *attr,
		const char *const name) NDM_ATTR_WUR;

struct ndm_xml_attr_t *ndm_xml_attr_prev(
		const struct ndm_xml_attr_t *attr,
		const char *const name) NDM_ATTR_WUR;

#endif	/* __NDM_XML__ */

