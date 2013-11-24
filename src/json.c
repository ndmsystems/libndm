/**
 * Copyright (C) 2011 Milo Yip
 * Copyright (c) 2013 NDM Systems, Inc. http://www.ndmsystems.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <ndm/int.h>
#include <ndm/pool.h>
#include <ndm/json.h>
#include <ndm/macro.h>
#include <ndm/dlist.h>

#define NDM_JSON_PRINT_IDENT_STEP_			2	/* two spaces */
#define NDM_JSON_PRINT_BUFSIZE_				4096

NDM_BUILD_ASSERT(ndm_json_nonzero_buffer_size, NDM_JSON_PRINT_BUFSIZE_ > 0);

#define NDM_JSON_IS_CHAR_LESS_SHORT_		(UCHAR_MAX < USHRT_MAX)
#define NDM_JSON_IS_SHORT_LESS_INT_			(USHRT_MAX < INT_MAX  )
#define NDM_JSON_IS_INT_LESS_LONG_			(INT_MAX   < LONG_MAX )
#define NDM_JSON_IS_LONG_LESS_LLONG_		(LONG_MAX  < LLONG_MAX)

#define NDM_JSON_TYPE_NULL_					0x0000
#define NDM_JSON_TYPE_BOOLEAN_				0x0001
#define NDM_JSON_TYPE_STRING_				0x0002
#define NDM_JSON_TYPE_NUMBER_				0x0003
#define NDM_JSON_TYPE_ARRAY_				0x0004
#define NDM_JSON_TYPE_OBJECT_				0x0005
#define NDM_JSON_TYPE_MASK_					0x0007
#define NDM_JSON_TYPE_(flags)				\
	((flags) & NDM_JSON_TYPE_MASK_)

#define NDM_JSON_NUMBER_IS_CHAR_			0x0008
#define NDM_JSON_NUMBER_IS_UCHAR_			0x0010
#define NDM_JSON_NUMBER_IS_SHORT_			0x0020
#define NDM_JSON_NUMBER_IS_USHORT_			0x0040
#define NDM_JSON_NUMBER_IS_INT_				0x0080
#define NDM_JSON_NUMBER_IS_UINT_			0x0100
#define NDM_JSON_NUMBER_IS_LONG_			0x0200
#define NDM_JSON_NUMBER_IS_ULONG_			0x0400
#define NDM_JSON_NUMBER_IS_LLONG_			0x0800
#define NDM_JSON_NUMBER_IS_ULLONG_			0x1000
#define NDM_JSON_NUMBER_IS_DOUBLE_			0x2000

/**
 * Need bytes: "+/-", "integer and fractional" part size
 * is less than NDM_INT_MAX_BUFSIZE(double),
 * ".", "e", "+/-", "exponent" part size is less than
 * NDM_INT_MAX_BUFSIZE(int), null-terminator.
 **/

#define NDM_JSON_DOUBLE_BUFSIZE_			\
	1 +										\
	NDM_INT_MAX_BUFSIZE(double) +			\
	1 +										\
	1 +										\
	1 +										\
	NDM_INT_MAX_BUFSIZE(int) +				\
	1

struct ndm_json_array_t
{
	struct ndm_pool_t *pool_;
	struct ndm_dlist_entry_t elements_;
};

struct ndm_json_object_t
{
	struct ndm_pool_t *pool_;
	struct ndm_dlist_entry_t members_;
};

struct ndm_json_array_element_t
{
	/**
	 * No specific members for an array element.
	 **/
};

struct ndm_json_object_member_t
{
	const char *name_;
};

struct ndm_json_value_t
{
	unsigned int flags_;
	struct ndm_json_value_t *parent_;
	struct ndm_dlist_entry_t list_;
	union
	{
		struct ndm_json_array_element_t array_;
		struct ndm_json_object_member_t object_;
	} member_of_;
	union
	{
		bool boolean_;						//!< boolean value
		char *string_;						//!< string value
		long long llong_;					//!< signed integer value
		unsigned long long ullong_;			//!< unsigned integer value
		double double_;						//!< double value
		struct ndm_json_array_t array_;		//!< array value
		struct ndm_json_object_t object_;	//!< object value
	} data_;
};

struct ndm_json_value_t *ndm_json_value_parent(
		const struct ndm_json_value_t *const value)
{
	return value->parent_;
}

bool ndm_json_value_is_null(
		const struct ndm_json_value_t *const value)
{
	return NDM_JSON_TYPE_(value->flags_) == NDM_JSON_TYPE_NULL_;
}

bool ndm_json_value_is_false(
		const struct ndm_json_value_t *const value)
{
	return ndm_json_value_is_boolean(value) && !value->data_.boolean_;
}

bool ndm_json_value_is_true(
		const struct ndm_json_value_t *const value)
{
	return ndm_json_value_is_boolean(value) && value->data_.boolean_;
}

bool ndm_json_value_is_boolean(
		const struct ndm_json_value_t *const value)
{
	return NDM_JSON_TYPE_(value->flags_) == NDM_JSON_TYPE_BOOLEAN_;
}

bool ndm_json_value_is_string(
		const struct ndm_json_value_t *const value)
{
	return NDM_JSON_TYPE_(value->flags_) == NDM_JSON_TYPE_STRING_;
}

bool ndm_json_value_is_number(
		const struct ndm_json_value_t *const value)
{
	return NDM_JSON_TYPE_(value->flags_) == NDM_JSON_TYPE_NUMBER_;
}

bool ndm_json_value_is_char(
		const struct ndm_json_value_t *const value)
{
	return
		ndm_json_value_is_number(value) &&
		(value->flags_ & NDM_JSON_NUMBER_IS_CHAR_);
}

bool ndm_json_value_is_uchar(
		const struct ndm_json_value_t *const value)
{
	return
		ndm_json_value_is_number(value) &&
		(value->flags_ & NDM_JSON_NUMBER_IS_UCHAR_);
}

bool ndm_json_value_is_short(
		const struct ndm_json_value_t *const value)
{
	return
		ndm_json_value_is_number(value) &&
		(value->flags_ & NDM_JSON_NUMBER_IS_SHORT_);
}

bool ndm_json_value_is_ushort(
		const struct ndm_json_value_t *const value)
{
	return
		ndm_json_value_is_number(value) &&
		(value->flags_ & NDM_JSON_NUMBER_IS_USHORT_);
}

bool ndm_json_value_is_int(
		const struct ndm_json_value_t *const value)
{
	return
		ndm_json_value_is_number(value) &&
		(value->flags_ & NDM_JSON_NUMBER_IS_INT_);
}

bool ndm_json_value_is_uint(
		const struct ndm_json_value_t *const value)
{
	return
		ndm_json_value_is_number(value) &&
		(value->flags_ & NDM_JSON_NUMBER_IS_UINT_);
}

bool ndm_json_value_is_long(
		const struct ndm_json_value_t *const value)
{
	return
		ndm_json_value_is_number(value) &&
		(value->flags_ & NDM_JSON_NUMBER_IS_LONG_);
}

bool ndm_json_value_is_ulong(
		const struct ndm_json_value_t *const value)
{
	return
		ndm_json_value_is_number(value) &&
		(value->flags_ & NDM_JSON_NUMBER_IS_ULONG_);
}

bool ndm_json_value_is_llong(
		const struct ndm_json_value_t *const value)
{
	return
		ndm_json_value_is_number(value) &&
		(value->flags_ & NDM_JSON_NUMBER_IS_LLONG_);
}

bool ndm_json_value_is_ullong(
		const struct ndm_json_value_t *const value)
{
	return
		ndm_json_value_is_number(value) &&
		(value->flags_ & NDM_JSON_NUMBER_IS_ULLONG_);
}

bool ndm_json_value_is_double(
		const struct ndm_json_value_t *const value)
{
	return
		ndm_json_value_is_number(value) &&
		(value->flags_ & NDM_JSON_NUMBER_IS_DOUBLE_);
}

bool ndm_json_value_is_array(
		const struct ndm_json_value_t *const value)
{
	return NDM_JSON_TYPE_(value->flags_) == NDM_JSON_TYPE_ARRAY_;
}

bool ndm_json_value_is_object(
		const struct ndm_json_value_t *const value)
{
	return NDM_JSON_TYPE_(value->flags_) == NDM_JSON_TYPE_OBJECT_;
}

bool ndm_json_value_boolean(
		const struct ndm_json_value_t *const value)
{
	return value->data_.boolean_;
}

const char *ndm_json_value_string(
		const struct ndm_json_value_t *const value)
{
	return value->data_.string_;
}

char ndm_json_value_char(
		const struct ndm_json_value_t *const value)
{
	return (char) value->data_.llong_;
}

unsigned char ndm_json_value_uchar(
		const struct ndm_json_value_t *const value)
{
	return (unsigned char) value->data_.ullong_;
}

short ndm_json_value_short(
		const struct ndm_json_value_t *const value)
{
	return (short) value->data_.llong_;
}

unsigned short ndm_json_value_ushort(
		const struct ndm_json_value_t *const value)
{
	return (unsigned short) value->data_.ullong_;
}

int ndm_json_value_int(
		const struct ndm_json_value_t *const value)
{
	return (int) value->data_.llong_;
}

unsigned int ndm_json_value_uint(
		const struct ndm_json_value_t *const value)
{
	return (unsigned int) value->data_.ullong_;
}

long ndm_json_value_long(
		const struct ndm_json_value_t *const value)
{
	return (long) value->data_.llong_;
}

unsigned long ndm_json_value_ulong(
		const struct ndm_json_value_t *const value)
{
	return (unsigned long) value->data_.ullong_;
}

long long ndm_json_value_llong(
		const struct ndm_json_value_t *const value)
{
	return value->data_.llong_;
}

unsigned long long ndm_json_value_ullong(
		const struct ndm_json_value_t *const value)
{
	return value->data_.ullong_;
}

double ndm_json_value_double(
		const struct ndm_json_value_t *const value)
{
	return value->data_.double_;
}

struct ndm_json_array_t *ndm_json_value_array(
		const struct ndm_json_value_t *const value)
{
	return ndm_json_value_is_array(value) ?
		(struct ndm_json_array_t *) &value->data_.array_ : NULL;
}

struct ndm_json_object_t *ndm_json_value_object(
		const struct ndm_json_value_t *const value)
{
	return ndm_json_value_is_object(value) ?
		(struct ndm_json_object_t *) &value->data_.object_ : NULL;
}

static inline struct ndm_json_value_t *ndm_json_value_alloc_(
		struct ndm_pool_t *pool,
		const unsigned int type)
{
	struct ndm_json_value_t *v = (struct ndm_json_value_t *)
		ndm_pool_malloc(pool, sizeof(struct ndm_json_value_t));

	if (v == NULL) {
		return NULL;
	}

	/**
	 * Reset all flags, assign a type.
	 **/

	v->flags_ = NDM_JSON_TYPE_(type);
	v->parent_ = NULL;
	ndm_dlist_init(&v->list_);

	/**
	 * All fields within rest unions are not initialized.
	 **/

	return v;
}

static inline struct ndm_json_value_t *ndm_json_value_alloc_null_(
		struct ndm_pool_t *pool)
{
	return ndm_json_value_alloc_(pool, NDM_JSON_TYPE_NULL_);
}

static inline struct ndm_json_value_t *ndm_json_value_alloc_boolean_(
		struct ndm_pool_t *pool,
		const bool boolean)
{
	struct ndm_json_value_t *v =
		ndm_json_value_alloc_(pool, NDM_JSON_TYPE_BOOLEAN_);

	if (v == NULL) {
		return NULL;
	}

	v->data_.boolean_ = boolean;

	return v;
}

static inline struct ndm_json_value_t *ndm_json_value_alloc_string_(
		struct ndm_pool_t *pool,
		const char *const value)
{
	struct ndm_json_value_t *v =
		ndm_json_value_alloc_(pool, NDM_JSON_TYPE_STRING_);

	if (v == NULL) {
		return NULL;
	}

	v->data_.string_ = ndm_pool_strdup(pool, value);

	if (v->data_.string_ == NULL) {
		return NULL;
	}

	return v;
}

static void ndm_json_value_append_llong_flags_(
		struct ndm_json_value_t *value)
{
	const long long llong = value->data_.llong_;

	value->flags_ |= NDM_JSON_NUMBER_IS_LLONG_;

#if NDM_JSON_IS_LONG_LESS_LLONG_
	if (LONG_MIN <= llong && llong <= LONG_MAX)
#endif
	{
		value->flags_ |= NDM_JSON_NUMBER_IS_LONG_;

#if NDM_JSON_IS_INT_LESS_LONG_
		if (INT_MIN <= llong && llong <= INT_MAX)
#endif
		{
			value->flags_ |= NDM_JSON_NUMBER_IS_INT_;

#if NDM_JSON_IS_SHORT_LESS_INT_
			if (SHRT_MIN <= llong && llong <= SHRT_MAX)
#endif
			{
				value->flags_ |= NDM_JSON_NUMBER_IS_SHORT_;

#if NDM_JSON_IS_CHAR_LESS_SHORT_
				if (CHAR_MIN <= llong && llong <= CHAR_MAX)
#endif
				{
					value->flags_ |= NDM_JSON_NUMBER_IS_CHAR_;
				}
			}
		}
	}
}

static void ndm_json_value_append_ullong_flags_(
		struct ndm_json_value_t *value)
{
	const unsigned long long ullong = value->data_.ullong_;

	value->flags_ |= NDM_JSON_NUMBER_IS_ULLONG_;

#if NDM_JSON_IS_LONG_LESS_LLONG_
	if (ullong <= ULONG_MAX)
#endif
	{
		value->flags_ |= NDM_JSON_NUMBER_IS_ULONG_;

#if NDM_JSON_IS_INT_LESS_LONG_
		if (ullong <= UINT_MAX)
#endif
		{
			value->flags_ |= NDM_JSON_NUMBER_IS_UINT_;

#if NDM_JSON_IS_SHORT_LESS_INT_
			if (ullong <= USHRT_MAX)
#endif
			{
				value->flags_ |= NDM_JSON_NUMBER_IS_USHORT_;

#if NDM_JSON_IS_CHAR_LESS_SHORT_
				if (ullong <= UCHAR_MAX)
#endif
				{
					value->flags_ |= NDM_JSON_NUMBER_IS_UCHAR_;
				}
			}
		}
	}
}

static struct ndm_json_value_t *ndm_json_value_alloc_llong_(
		struct ndm_pool_t *pool,
		const long long llong)
{
	struct ndm_json_value_t *v =
		ndm_json_value_alloc_(pool, NDM_JSON_TYPE_NUMBER_);

	if (v == NULL) {
		return NULL;
	}

	v->data_.llong_ = llong;
	ndm_json_value_append_llong_flags_(v);

	if (llong >= 0) {
		ndm_json_value_append_ullong_flags_(v);
	}

	return v;
}

static struct ndm_json_value_t *ndm_json_value_alloc_ullong_(
		struct ndm_pool_t *pool,
		const unsigned long long ullong)
{
	struct ndm_json_value_t *v =
		ndm_json_value_alloc_(pool, NDM_JSON_TYPE_NUMBER_);

	if (v == NULL) {
		return NULL;
	}

	v->data_.ullong_ = ullong;
	ndm_json_value_append_ullong_flags_(v);

	if (ullong <= LLONG_MAX) {
		ndm_json_value_append_llong_flags_(v);
	}

	return v;
}

static inline struct ndm_json_value_t *ndm_json_value_alloc_double_(
		struct ndm_pool_t *pool,
		const double value)
{
	struct ndm_json_value_t *v =
		ndm_json_value_alloc_(pool, NDM_JSON_TYPE_NUMBER_);

	if (v == NULL) {
		return NULL;
	}

	v->data_.double_ = value;
	v->flags_ |= NDM_JSON_NUMBER_IS_DOUBLE_;

	return v;
}

static inline struct ndm_json_value_t *ndm_json_value_alloc_array_(
		struct ndm_pool_t *pool)
{
	struct ndm_json_value_t *v =
		ndm_json_value_alloc_(pool, NDM_JSON_TYPE_ARRAY_);

	if (v == NULL) {
		return NULL;
	}

	v->data_.array_.pool_ = pool;
	ndm_dlist_init(&v->data_.array_.elements_);

	return v;
}

static inline struct ndm_json_value_t *ndm_json_value_alloc_object_(
		struct ndm_pool_t *pool)
{
	struct ndm_json_value_t *v =
		ndm_json_value_alloc_(pool, NDM_JSON_TYPE_OBJECT_);

	if (v == NULL) {
		return NULL;
	}

	v->data_.object_.pool_ = pool;
	ndm_dlist_init(&v->data_.object_.members_);

	return v;
}

/**
 * Array functions.
 * The NULL @a array pointer not allowed.
 **/

struct ndm_pool_t *ndm_json_array_pool(
		const struct ndm_json_array_t *const array)
{
	return array->pool_;
}

struct ndm_json_value_t *ndm_json_array_value(
		const struct ndm_json_array_t *const array)
{
	return (struct ndm_json_value_t *)
		(((char *) array) -
			offsetof(struct ndm_json_value_t, data_.array_));
}

bool ndm_json_array_is_empty(
		const struct ndm_json_array_t *array)
{
	return ndm_dlist_is_empty(&array->elements_);
}

size_t ndm_json_array_size(
		const struct ndm_json_array_t *array)
{
	size_t size = 0;
	struct ndm_dlist_entry_t *e = array->elements_.next;

	while (e != &array->elements_) {
		e = e->next;
		size++;
	}

	return size;
}

static inline struct ndm_json_array_element_t *
ndm_json_array_element_from_list_(
		struct ndm_dlist_entry_t *list)
{
	return &ndm_dlist_entry(
		list,
		struct ndm_json_value_t,
		list_)->member_of_.array_;
}

struct ndm_json_array_element_t *ndm_json_array_element_first(
		const struct ndm_json_array_t *array)
{
	if (ndm_json_array_is_empty(array)) {
		return NULL;
	}

	return ndm_json_array_element_from_list_(array->elements_.next);
}

struct ndm_json_array_element_t *ndm_json_array_element_last(
		const struct ndm_json_array_t *array)
{
	if (ndm_json_array_is_empty(array)) {
		return NULL;
	}

	return ndm_json_array_element_from_list_(array->elements_.prev);
}

struct ndm_json_array_element_t *ndm_json_array_element_next(
		const struct ndm_json_array_element_t *element)
{
	struct ndm_json_value_t *v = ndm_json_array_element_value(element);

	if (&v->parent_->data_.array_.elements_ == v->list_.next) {
		return NULL;
	}

	return ndm_json_array_element_from_list_(v->list_.next);
}

struct ndm_json_array_element_t *ndm_json_array_element_prev(
		const struct ndm_json_array_element_t *element)
{
	struct ndm_json_value_t *v = ndm_json_array_element_value(element);

	if (&v->parent_->data_.array_.elements_ == v->list_.prev) {
		return NULL;
	}

	return ndm_json_array_element_from_list_(v->list_.prev);
}

struct ndm_json_value_t *ndm_json_array_element_value(
		const struct ndm_json_array_element_t *element)
{
	return (struct ndm_json_value_t *)
		(((char *) element) -
			offsetof(struct ndm_json_value_t, member_of_.array_));
}

static inline struct ndm_json_value_t *ndm_json_array_push_(
		struct ndm_json_array_t *array,
		struct ndm_json_value_t *value)
{
	if (array == NULL || value == NULL) {
		return NULL;
	}

	value->parent_ = ndm_json_array_value(array);
	ndm_dlist_insert_before(&array->elements_, &value->list_);

	return value;
}

struct ndm_json_value_t *ndm_json_array_push_null(
		struct ndm_json_array_t *array)
{
	return ndm_json_array_push_(
		array,
		array == NULL ?
			NULL :
			ndm_json_value_alloc_null_(array->pool_));
}

struct ndm_json_value_t *ndm_json_array_push_boolean(
		struct ndm_json_array_t *array,
		const bool value)
{
	return ndm_json_array_push_(
		array,
		array == NULL ?
			NULL :
			ndm_json_value_alloc_boolean_(array->pool_, value));
}

struct ndm_json_value_t *ndm_json_array_push_string(
		struct ndm_json_array_t *array,
		const char *const value)
{
	return ndm_json_array_push_(
		array,
		array == NULL ?
			NULL :
			ndm_json_value_alloc_string_(array->pool_, value));
}

struct ndm_json_value_t *ndm_json_array_push_char(
		struct ndm_json_array_t *array,
		const char value)
{
	return ndm_json_array_push_(
		array,
		array == NULL ?
			NULL :
			ndm_json_value_alloc_llong_(array->pool_, value));
}

struct ndm_json_value_t *ndm_json_array_push_uchar(
		struct ndm_json_array_t *array,
		const unsigned char value)
{
	return ndm_json_array_push_(
		array,
		array == NULL ?
			NULL :
			ndm_json_value_alloc_ullong_(array->pool_, value));
}

struct ndm_json_value_t *ndm_json_array_push_short(
		struct ndm_json_array_t *array,
		const short value)
{
	return ndm_json_array_push_(
		array,
		array == NULL ?
			NULL :
			ndm_json_value_alloc_llong_(array->pool_, value));
}

struct ndm_json_value_t *ndm_json_array_push_ushort(
		struct ndm_json_array_t *array,
		const unsigned short value)
{
	return ndm_json_array_push_(
		array,
		array == NULL ?
			NULL :
			ndm_json_value_alloc_ullong_(array->pool_, value));
}

struct ndm_json_value_t *ndm_json_array_push_int(
		struct ndm_json_array_t *array,
		const int value)
{
	return ndm_json_array_push_(
		array,
		array == NULL ?
			NULL :
			ndm_json_value_alloc_llong_(array->pool_, value));
}

struct ndm_json_value_t *ndm_json_array_push_uint(
		struct ndm_json_array_t *array,
		const unsigned int value)
{
	return ndm_json_array_push_(
		array,
		array == NULL ?
			NULL :
			ndm_json_value_alloc_ullong_(array->pool_, value));
}

struct ndm_json_value_t *ndm_json_array_push_long(
		struct ndm_json_array_t *array,
		const long value)
{
	return ndm_json_array_push_(
		array,
		array == NULL ?
			NULL :
			ndm_json_value_alloc_llong_(array->pool_, value));
}

struct ndm_json_value_t *ndm_json_array_push_ulong(
		struct ndm_json_array_t *array,
		const unsigned long value)
{
	return ndm_json_array_push_(
		array,
		array == NULL ?
			NULL :
			ndm_json_value_alloc_ullong_(array->pool_, value));
}

struct ndm_json_value_t *ndm_json_array_push_llong(
		struct ndm_json_array_t *array,
		const long long value)
{
	return ndm_json_array_push_(
		array,
		array == NULL ?
			NULL :
			ndm_json_value_alloc_llong_(array->pool_, value));
}

struct ndm_json_value_t *ndm_json_array_push_ullong(
		struct ndm_json_array_t *array,
		const unsigned long long value)
{
	return ndm_json_array_push_(
		array,
		array == NULL ?
			NULL :
			ndm_json_value_alloc_ullong_(array->pool_, value));
}

struct ndm_json_value_t *ndm_json_array_push_double(
		struct ndm_json_array_t *array,
		const double value)
{
	return ndm_json_array_push_(
		array,
		array == NULL ?
			NULL :
			ndm_json_value_alloc_double_(array->pool_, value));
}

struct ndm_json_array_t *ndm_json_array_push_array(
		struct ndm_json_array_t *array)
{
	struct ndm_json_value_t *v = ndm_json_array_push_(
		array,
		array == NULL ?
			NULL :
			ndm_json_value_alloc_array_(array->pool_));

	if (v == NULL) {
		return NULL;
	}

	return &v->data_.array_;
}

struct ndm_json_object_t *ndm_json_array_push_object(
		struct ndm_json_array_t *array)
{
	struct ndm_json_value_t *v = ndm_json_array_push_(
		array,
		array == NULL ?
			NULL :
			ndm_json_value_alloc_object_(array->pool_));

	if (v == NULL) {
		return NULL;
	}

	return &v->data_.object_;
}

/**
 * Object functions.
 * The NULL @a object pointer not allowed.
 **/

struct ndm_pool_t *ndm_json_object_pool(
		const struct ndm_json_object_t *const object)
{
	return object->pool_;
}

struct ndm_json_value_t *ndm_json_object_value(
		const struct ndm_json_object_t *const object)
{
	return (struct ndm_json_value_t *)
		(((char *) object) -
			offsetof(struct ndm_json_value_t, data_.object_));
}

bool ndm_json_object_is_empty(
		const struct ndm_json_object_t *object)
{
	return ndm_dlist_is_empty(&object->members_);
}

struct ndm_json_value_t *ndm_json_object_get(
		const struct ndm_json_object_t *object,
		const char *const name)
{
	struct ndm_json_value_t *e;

	ndm_dlist_foreach_entry(
			e,
			struct ndm_json_value_t,
			list_,
			&object->members_)
	{
		if (strcmp(e->member_of_.object_.name_, name) == 0) {
			return e;
		}
	}

	return NULL;
}

static inline struct ndm_json_object_member_t *
ndm_json_object_member_from_list_(
		struct ndm_dlist_entry_t *list)
{
	return &ndm_dlist_entry(
		list,
		struct ndm_json_value_t,
		list_)->member_of_.object_;
}

struct ndm_json_object_member_t *ndm_json_object_member_first(
		const struct ndm_json_object_t *object)
{
	if (ndm_json_object_is_empty(object)) {
		return NULL;
	}

	return ndm_json_object_member_from_list_(object->members_.next);
}

struct ndm_json_object_member_t *ndm_json_object_member_last(
		const struct ndm_json_object_t *object)
{
	if (ndm_json_object_is_empty(object)) {
		return NULL;
	}

	return ndm_json_object_member_from_list_(object->members_.prev);
}

struct ndm_json_object_member_t *ndm_json_object_member_next(
		const struct ndm_json_object_member_t *member)
{
	struct ndm_json_value_t *v = ndm_json_object_member_value(member);

	if (&v->parent_->data_.object_.members_ == v->list_.next) {
		return NULL;
	}

	return ndm_json_object_member_from_list_(v->list_.next);
}

struct ndm_json_object_member_t *ndm_json_object_member_prev(
		const struct ndm_json_object_member_t *member)
{
	struct ndm_json_value_t *v = ndm_json_object_member_value(member);

	if (&v->parent_->data_.object_.members_ == v->list_.prev) {
		return NULL;
	}

	return ndm_json_object_member_from_list_(v->list_.prev);
}

const char *ndm_json_object_member_name(
		const struct ndm_json_object_member_t *member)
{
	return member->name_;
}

struct ndm_json_value_t *ndm_json_object_member_value(
		const struct ndm_json_object_member_t *member)
{
	return (struct ndm_json_value_t *)
		(((char *) member) -
			offsetof(struct ndm_json_value_t, member_of_.object_));
}

static inline struct ndm_json_value_t *ndm_json_object_set_(
		struct ndm_json_object_t *object,
		const char *const name,
		struct ndm_json_value_t *value)
{
	if (object == NULL || value == NULL) {
		return NULL;
	}

	value->parent_ = ndm_json_object_value(object);
	value->member_of_.object_.name_ = ndm_pool_strdup(object->pool_, name);

	if (value->member_of_.object_.name_ == NULL) {
		return NULL;
	}

	ndm_dlist_insert_before(&object->members_, &value->list_);

	return value;
}

struct ndm_json_value_t *ndm_json_object_set_null(
		struct ndm_json_object_t *object,
		const char *const name)
{
	return ndm_json_object_set_(
		object,
		name,
		object == NULL ?
			NULL :
			ndm_json_value_alloc_null_(object->pool_));
}

struct ndm_json_value_t *ndm_json_object_set_boolean(
		struct ndm_json_object_t *object,
		const char *const name,
		const bool value)
{
	return ndm_json_object_set_(
		object,
		name,
		object == NULL ?
			NULL :
			ndm_json_value_alloc_boolean_(object->pool_, value));
}

struct ndm_json_value_t *ndm_json_object_set_string(
		struct ndm_json_object_t *object,
		const char *const name,
		const char *const value)
{
	return ndm_json_object_set_(
		object,
		name,
		object == NULL ?
			NULL :
			ndm_json_value_alloc_string_(object->pool_, value));
}

struct ndm_json_value_t *ndm_json_object_set_char(
		struct ndm_json_object_t *object,
		const char *const name,
		const char value)
{
	return ndm_json_object_set_(
		object,
		name,
		object == NULL ?
			NULL :
			ndm_json_value_alloc_llong_(object->pool_, value));
}

struct ndm_json_value_t *ndm_json_object_set_uchar(
		struct ndm_json_object_t *object,
		const char *const name,
		const unsigned char value)
{
	return ndm_json_object_set_(
		object,
		name,
		object == NULL ?
			NULL :
			ndm_json_value_alloc_ullong_(object->pool_, value));
}

struct ndm_json_value_t *ndm_json_object_set_short(
		struct ndm_json_object_t *object,
		const char *const name,
		const short value)
{
	return ndm_json_object_set_(
		object,
		name,
		object == NULL ?
			NULL :
			ndm_json_value_alloc_llong_(object->pool_, value));
}

struct ndm_json_value_t *ndm_json_object_set_ushort(
		struct ndm_json_object_t *object,
		const char *const name,
		const unsigned short value)
{
	return ndm_json_object_set_(
		object,
		name,
		object == NULL ?
			NULL :
			ndm_json_value_alloc_ullong_(object->pool_, value));
}

struct ndm_json_value_t *ndm_json_object_set_int(
		struct ndm_json_object_t *object,
		const char *const name,
		const int value)
{
	return ndm_json_object_set_(
		object,
		name,
		object == NULL ?
			NULL :
			ndm_json_value_alloc_llong_(object->pool_, value));
}

struct ndm_json_value_t *ndm_json_object_set_uint(
		struct ndm_json_object_t *object,
		const char *const name,
		const unsigned int value)
{
	return ndm_json_object_set_(
		object,
		name,
		object == NULL ?
			NULL :
			ndm_json_value_alloc_ullong_(object->pool_, value));
}

struct ndm_json_value_t *ndm_json_object_set_long(
		struct ndm_json_object_t *object,
		const char *const name,
		const long value)
{
	return ndm_json_object_set_(
		object,
		name,
		object == NULL ?
			NULL :
			ndm_json_value_alloc_llong_(object->pool_, value));
}

struct ndm_json_value_t *ndm_json_object_set_ulong(
		struct ndm_json_object_t *object,
		const char *const name,
		const unsigned long value)
{
	return ndm_json_object_set_(
		object,
		name,
		object == NULL ?
			NULL :
			ndm_json_value_alloc_ullong_(object->pool_, value));
}

struct ndm_json_value_t *ndm_json_object_set_llong(
		struct ndm_json_object_t *object,
		const char *const name,
		const long long value)
{
	return ndm_json_object_set_(
		object,
		name,
		object == NULL ?
			NULL :
			ndm_json_value_alloc_llong_(object->pool_, value));
}

struct ndm_json_value_t *ndm_json_object_set_ullong(
		struct ndm_json_object_t *object,
		const char *const name,
		const unsigned long long value)
{
	return ndm_json_object_set_(
		object,
		name,
		object == NULL ?
			NULL :
			ndm_json_value_alloc_ullong_(object->pool_, value));
}

struct ndm_json_value_t *ndm_json_object_set_double(
		struct ndm_json_object_t *object,
		const char *const name,
		const double value)
{
	return ndm_json_object_set_(
		object,
		name,
		object == NULL ?
			NULL :
			ndm_json_value_alloc_double_(object->pool_, value));
}

struct ndm_json_array_t *ndm_json_object_set_array(
		struct ndm_json_object_t *object,
		const char *const name)
{
	struct ndm_json_value_t *v = ndm_json_object_set_(
		object,
		name,
		object == NULL ?
			NULL :
			ndm_json_value_alloc_array_(object->pool_));

	if (v == NULL) {
		return NULL;
	}

	return &v->data_.array_;
}

struct ndm_json_object_t *ndm_json_object_set_object(
		struct ndm_json_object_t *object,
		const char *const name)
{
	struct ndm_json_value_t *v = ndm_json_object_set_(
		object,
		name,
		object == NULL ?
			NULL :
			ndm_json_value_alloc_object_(object->pool_));

	if (v == NULL) {
		return NULL;
	}

	return &v->data_.object_;
}

struct ndm_json_array_t *ndm_json_array_new(
		struct ndm_pool_t *pool)
{
	struct ndm_json_value_t *v = ndm_json_value_alloc_array_(pool);

	if (v == NULL) {
		return NULL;
	}

	return &v->data_.array_;
}

struct ndm_json_object_t *ndm_json_object_new(
		struct ndm_pool_t *pool)
{
	struct ndm_json_value_t *v = ndm_json_value_alloc_object_(pool);

	if (v == NULL) {
		return NULL;
	}

	return &v->data_.object_;
}

/**
 * JSON in situ parser.
 **/

struct ndm_json_parse_context_t_
{
	struct ndm_pool_t *pool;
	char *json;
};

static inline void ndm_json_parse_whitespaces_(
		struct ndm_json_parse_context_t_ *ctx)
{
	char *s = ctx->json;

	while (*s == ' ' || *s == '\n' || *s == '\r' || *s == '\t') {
		++s;
	}

	ctx->json = s;
}

/**
 * Helper function to parse four hexidecimal digits
 * in \uXXXX within @c ndm_json_parser_parse_string_().
 **/

static enum ndm_json_parse_error_t ndm_json_parse_hex4_(
		struct ndm_json_parse_context_t_ *ctx,
		uint32_t *codepoint)
{
	*codepoint = 0;

	for (unsigned long i = 0; i < 4; i++) {
		static const unsigned char HEX_DIGITS_[256] =
		{
			255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
			255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
			255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
			  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,255,255,255,255,255,255,
			255, 10, 11, 12, 13, 14, 15,255,255,255,255,255,255,255,255,255,
			255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
			255, 10, 11, 12, 13, 14, 15,255,255,255,255,255,255,255,255,255,
			255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
			255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
			255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
			255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
			255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
			255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
			255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
			255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
			255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
		};
		const unsigned char d = HEX_DIGITS_[(unsigned char) *ctx->json++];

		if (d > 0x0f) {
			/**
			 * Incorrect hex digit after \\u escape.
			 **/

			return NDM_JSON_PARSE_ERROR_INVALID_HEX;
		}

		*codepoint <<= 4;
		*codepoint += d;
	}

	return NDM_JSON_PARSE_ERROR_OK;
}

/**
 * Parse string, handling the prefix and suffix double quotes and escaping.
 *
 * string -> "" | " chars "
 * chars  -> char | char chars
 * char   -> any-Unicode-character-except-"-or-\-or-control-character |
 *           \" | \\ | \/ | \b | \f | \n | \r | \t | \u four-hex-digits
 **/

static enum ndm_json_parse_error_t ndm_json_parse_string_(
		struct ndm_json_parse_context_t_ *ctx,
		char **str)
{
	static const char ESCAPE_[256] =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

		0x00, 0x00, '\"', 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  '/',

		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, '\\', 0x00, 0x00, 0x00,

		0x00, 0x00, '\b', 0x00, 0x00, 0x00, '\f', 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '\n', 0x00,

		0x00, 0x00, '\r', 0x00, '\t', 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	assert (*ctx->json == '"');

	/* Skip '\"'. */

	ctx->json++;
	*str = NULL;

	char *start = ctx->json;
	char *out = start;

	do
	{
		const char c = *ctx->json++;

		if (c == '\\') {
			/* Escape. */

			const char e = *ctx->json++;
			const char escaped = ESCAPE_[(unsigned char) e];

			if (escaped != 0) {
				/**
				 * Put a replaced escaped character.
				 **/

				*out++ = escaped;
			} else
			if (e == 'u') {
				/* Unicode codepoint. */

				uint32_t codepoint;
				enum ndm_json_parse_error_t code =
					ndm_json_parse_hex4_(ctx, &codepoint);

				if (code != NDM_JSON_PARSE_ERROR_OK) {
					return code;
				}

				if (codepoint >= 0xd800 && codepoint <= 0xdbff) {
					/* UTF-16 surrogate pair. */

					if (*ctx->json++ != '\\' || *ctx->json++ != 'u') {
						/**
						 * Missing second \\u in a surrogate pair.
						 **/

						return NDM_JSON_PARSE_ERROR_CORRUPTED_SURROGATE_PAIR;
					}

					uint32_t codepoint2;

					code = ndm_json_parse_hex4_(ctx, &codepoint2);

					if (code != NDM_JSON_PARSE_ERROR_OK) {
						return code;
					}

					if (codepoint2 < 0xdc00 || codepoint2 > 0xdfff) {
						/**
						 * Second \\u in the surrogate pair is invalid.
						 **/

						return NDM_JSON_PARSE_ERROR_INVALID_SURROGATE_PAIR;
					}

					codepoint =
						(((codepoint  - 0xd800) << 10) |
						  (codepoint2 - 0xdc00)) + 0x10000;
				}

				/**
				 * Store an encoded UTF codepoint; it is always
				 * safe to replace \uXXXX by at least a four byte
				 * UTF-8 sequence.
				 **/

				if (codepoint < 0x80) {
					*out++ = (char) codepoint;
				} else
				if (codepoint < 0x800) {
					*out++ = (char) (0xc0 | ((codepoint >>  6) & 0xff));
					*out++ = (char) (0x80 | ((codepoint >>  0) & 0x3f));
				} else
				if (codepoint < 0x10000) {
					*out++ = (char) (0xe0 | ((codepoint >> 12) & 0xff));
					*out++ = (char) (0x80 | ((codepoint >>  6) & 0x3f));
					*out++ = (char) (0x80 | ((codepoint >>  0) & 0x3f));
				} else
				if (codepoint < 0x110000) {
					*out++ = (char) (0xf0 | ((codepoint >> 18) & 0xff));
					*out++ = (char) (0x80 | ((codepoint >> 12) & 0x3f));
					*out++ = (char) (0x80 | ((codepoint >>  6) & 0x3f));
					*out++ = (char) (0x80 | ((codepoint >>  0) & 0x3f));
				} else {
					/**
					 * Encode a replacement character
					 * for an invalid codepoint.
					 **/

					*out++ = '?';
				}
			} else {
				/* Unknown escape character. */

				return NDM_JSON_PARSE_ERROR_INVALID_ESCAPE_CHAR;
			}
		} else
		if (c == '"') {
			/**
			 * Closing double quote. Null-terminate the string.
			 **/

			*out = '\0';
			*str = start;

			return NDM_JSON_PARSE_ERROR_OK;
		} else
		if (c == '\0') {
			/**
			 * Ending quotation lacks before the end of a string.
			 **/

			return NDM_JSON_PARSE_ERROR_UNTERMINATED_STRING;
		} else
		if (((unsigned char) c) < 0x20) {
			/**
			 * RFC 4627: unescaped = %x20-21 / %x23-5B / %x5D-10FFFF.
			 * Incorrect unescaped character in string.
			 **/

			return NDM_JSON_PARSE_ERROR_UNESCAPED_CHAR;
		} else {
			/* Put a normal character. */

			*out++ = c;
		}
	} while (true);

	/* Never reached really. */

	return NDM_JSON_PARSE_ERROR_OK;
}

/**
 * Forward declarations.
 **/

static enum ndm_json_parse_error_t ndm_json_parse_array_(
		struct ndm_json_parse_context_t_ *ctx,
		struct ndm_json_array_t *array) NDM_ATTR_WUR;

static enum ndm_json_parse_error_t ndm_json_parse_object_(
		struct ndm_json_parse_context_t_ *ctx,
		struct ndm_json_object_t *object) NDM_ATTR_WUR;

/**
 * Parse any JSON value.
 *
 * value -> string | number | object | array | true | false | null
 **/

static enum ndm_json_parse_error_t ndm_json_parse_value_(
		struct ndm_json_parse_context_t_ *ctx,
		struct ndm_json_value_t **value)
{
	char *s = ctx->json;
	const char c = s[0];

	/* Parse "null". */

	if (c == 'n') {
		if (s[1] == 'u' &&
			s[2] == 'l' &&
			s[3] == 'l')
		{
			s += 4;
			ctx->json = s;

			if ((*value = ndm_json_value_alloc_null_(ctx->pool)) == NULL) {
				return NDM_JSON_PARSE_ERROR_OOM;
			}

			return NDM_JSON_PARSE_ERROR_OK;
		}

		/**
		 * Unknown value, null expected.
		 **/

		return NDM_JSON_PARSE_ERROR_NULL_EXPECTED;
	}

	/* Parse "true". */

	if (c == 't') {
		if (s[1] == 'r' &&
			s[2] == 'u' &&
			s[3] == 'e')
		{
			s += 4;
			ctx->json = s;

			if ((*value = ndm_json_value_alloc_boolean_(
					ctx->pool, true)) == NULL)
			{
				return NDM_JSON_PARSE_ERROR_OOM;
			}

			return NDM_JSON_PARSE_ERROR_OK;
		}

		/**
		 * Unknown value, true expected.
		 **/

		return NDM_JSON_PARSE_ERROR_TRUE_EXPECTED;
	}

	/* Parse "false". */

	if (c == 'f') {
		if (s[1] == 'a' &&
			s[2] == 'l' &&
			s[3] == 's' &&
			s[4] == 'e')
		{
			s += 5;
			ctx->json = s;

			if ((*value = ndm_json_value_alloc_boolean_(
					ctx->pool, false)) == NULL)
			{
				return NDM_JSON_PARSE_ERROR_OOM;
			}

			return NDM_JSON_PARSE_ERROR_OK;
		}

		/**
		 * Unknown value, false expected.
		 **/

		return NDM_JSON_PARSE_ERROR_FALSE_EXPECTED;
	}

	/* Parse an array. */

	if (c == '[') {
		if ((*value = ndm_json_value_alloc_array_(ctx->pool)) == NULL) {
			return NDM_JSON_PARSE_ERROR_OOM;
		}

		return ndm_json_parse_array_(ctx, ndm_json_value_array(*value));
	}

	/* Parse an object. */

	if (c == '{') {
		if ((*value = ndm_json_value_alloc_object_(ctx->pool)) == NULL ) {
			return NDM_JSON_PARSE_ERROR_OOM;
		}

		return ndm_json_parse_object_(ctx, ndm_json_value_object(*value));
	}

	/* Parse a string. */

	if (c == '"') {
		char *str;
		enum ndm_json_parse_error_t code = ndm_json_parse_string_(ctx, &str);

		if (code != NDM_JSON_PARSE_ERROR_OK) {
			return code;
		}

		/**
		 * Allocate a new string value, do not copy its contents.
		 **/

		if ((*value = ndm_json_value_alloc_(
				ctx->pool, NDM_JSON_TYPE_STRING_)) == NULL)
		{
			return NDM_JSON_PARSE_ERROR_OOM;
		}

		(*value)->data_.string_ = str;

		return NDM_JSON_PARSE_ERROR_OK;
	}

	if (c != '-' && !isdigit(c)) {
		/**
		 * All types tried to parse, only a number allowed here.
		 **/

		return NDM_JSON_PARSE_ERROR_UNKNOWN_TYPE;
	}

	/**
	 * Parse a decimal integer or float number.
	 * number -> int | int frac | int exp | int frac exp
	 * int    -> digit | digit1-9 digits | - digit | - digit1-9 digits
	 * frac   -> . digits
	 * exp    -> e digits
	 * digits -> digit | digit digits
	 * e      -> e | e+ | e- | E | E+ | E-
	 **/

	char *start = s;
	bool negative = false;

	if (*s == '-') {
		negative = true;
		s++;
	}

	/**
	 * Minus is allowed before single zero.
	 **/

	if (s[0] == '0' && isdigit(s[1])) {
		/**
		 * Special zero value, no leading zeroes allowed.
		 **/

		return NDM_JSON_PARSE_ERROR_NUMBER_LEADING_ZERO;
	}

	/**
	 * Nonzero value.
	 * Check a JSON number format before parsing.
	 **/

	char *digit_start = s;

	while (isdigit(*s)) {
		s++;
	}

	if (digit_start == s) {
		/* No digits. */

		ctx->json = s;

		return NDM_JSON_PARSE_ERROR_NUMBER_EXPECTED;
	}

	bool use_double = false;

	if (*s == '.') {
		/* It is a double value. */

		char *frac_start = ++s;

		use_double = true;

		while (isdigit(*s)) {
			s++;
		}

		if (frac_start == s) {
			/* No digits in a fractional part. */

			ctx->json = s;

			return NDM_JSON_PARSE_ERROR_NUMBER_FRAC_EXPECTED;
		}
	}

	if (*s == 'e' || *s == 'E') {
		use_double = true;
		s++;

		if (*s == '+' || *s == '-') {
			s++;
		}

		char *exp_start = s;

		while (isdigit(*s)) {
			s++;
		}

		if (exp_start == s) {
			/* No digits in an exponent part. */

			ctx->json = s;

			return NDM_JSON_PARSE_ERROR_NUMBER_EXP_EXPECTED;
		}
	}

	if (!use_double) {
		/* Try to parse as integer. */

		char *end;

		errno = 0;

		if (negative) {
			const long long ll = strtoll(start, &end, 10);

			if (errno != 0 || end != s) {
				/**
				 * A signed integer value is too big,
				 * try a double type.
				 **/

				use_double = true;
			} else
			if ((*value = ndm_json_value_alloc_llong_(
					ctx->pool, ll)) == NULL)
			{
				ctx->json = start;

				return NDM_JSON_PARSE_ERROR_OOM;
			}
		} else {
			const unsigned long long ull = strtoull(start, &end, 10);

			if (errno != 0 || end != s) {
				/**
				 * An unsigned integer value is too big,
				 * try a double type.
				 **/

				use_double = true;
			} else
			if ((*value = ndm_json_value_alloc_ullong_(
					ctx->pool, ull)) == NULL)
			{
				ctx->json = start;

				return NDM_JSON_PARSE_ERROR_OOM;
			}
		}
	}

	if (use_double) {
		errno = 0;

		char *end;
		const double d = strtod(start, &end);

		if (errno != 0 || end != s) {
			/**
			 * A number is out of a range for a double type.
			 **/

			ctx->json = start;

			return NDM_JSON_PARSE_ERROR_NUMBER_RANGE;
		}

		if ((*value = ndm_json_value_alloc_double_(ctx->pool, d)) == NULL) {
			ctx->json = start;

			return NDM_JSON_PARSE_ERROR_OOM;
		}
	}

	ctx->json = s;

	return NDM_JSON_PARSE_ERROR_OK;
}

/**
 * Parse array: [ value, ... ]
 *
 * array    -> [ ] | [ elements ]
 * elements -> value | value , elements
 **/

static enum ndm_json_parse_error_t ndm_json_parse_array_(
		struct ndm_json_parse_context_t_ *ctx,
		struct ndm_json_array_t *array)
{
	assert (*ctx->json == '[');

	/* Skip '['. */

	ctx->json++;
	ndm_json_parse_whitespaces_(ctx);

	if (*ctx->json == ']') {
		/* Empty array. */

		ctx->json++;

		return NDM_JSON_PARSE_ERROR_OK;
	}

	do {
		struct ndm_json_value_t *value = NULL;
		enum ndm_json_parse_error_t code =
			ndm_json_parse_value_(ctx, &value);

		if (code != NDM_JSON_PARSE_ERROR_OK) {
			if (code == NDM_JSON_PARSE_ERROR_UNKNOWN_TYPE &&
				*ctx->json == ']')
			{
				/* No symbols parsed, empty value. */

				return NDM_JSON_PARSE_ERROR_CORRUPTED_ARRAY;
			}

			return code;
		}

		if (ndm_json_array_push_(array, value) == NULL) {
			return NDM_JSON_PARSE_ERROR_OOM;
		}

		ndm_json_parse_whitespaces_(ctx);

		if (*ctx->json == ',') {
			ctx->json++;
			ndm_json_parse_whitespaces_(ctx);
		} else
		if (*ctx->json == ']') {
			ctx->json++;

			return NDM_JSON_PARSE_ERROR_OK;
		} else
		{
			/**
			 * Must be a comma or ']' after an array element.
			 **/

			return NDM_JSON_PARSE_ERROR_CORRUPTED_ARRAY;
		}
	} while (true);

	/**
	 * Never reached really.
	 **/

	return NDM_JSON_PARSE_ERROR_OK;
}

/**
 * Parse object: { string : value, ... }
 *
 * object  -> { } | { members }
 * members -> pair | pair , members
 * pair    -> string : value
 **/

static enum ndm_json_parse_error_t ndm_json_parse_object_(
		struct ndm_json_parse_context_t_ *ctx,
		struct ndm_json_object_t *object)
{
	assert (*ctx->json == '{');

	/* Skip '{'. */

	ctx->json++;
	ndm_json_parse_whitespaces_(ctx);

	if (*ctx->json == '}') {
		/* Empty object. */

		ctx->json++;

		return NDM_JSON_PARSE_ERROR_OK;
	}

	do {
		if (*ctx->json != '"') {
			/**
			 * Name of an object member must be a string.
			 **/

			return NDM_JSON_PARSE_ERROR_STRING_EXPECTED;
		}

		/**
		 * Get an object member name (in situ string parsing).
		 **/

		char *name = NULL;
		enum ndm_json_parse_error_t code =
			ndm_json_parse_string_(ctx, &name);

		if (code != NDM_JSON_PARSE_ERROR_OK) {
			return code;
		}

		ndm_json_parse_whitespaces_(ctx);

		if (*ctx->json != ':') {
			/**
			 * There must be a colon after the name of an object member.
			 **/

			return NDM_JSON_PARSE_ERROR_COLON_EXPECTED;
		}

		ctx->json++;
		ndm_json_parse_whitespaces_(ctx);

		/**
		 * Parse a value with a scanned name.
		 **/

		struct ndm_json_value_t *value = NULL;

		code = ndm_json_parse_value_(ctx, &value);

		if (code != NDM_JSON_PARSE_ERROR_OK) {
			if (code == NDM_JSON_PARSE_ERROR_UNKNOWN_TYPE &&
				*ctx->json == '}')
			{
				/**
				 * No symbols parsed, empty value.
				 **/

				return NDM_JSON_PARSE_ERROR_CORRUPTED_OBJECT;
			}

			return code;
		}

		/**
		 * Insert a new value to an object, do not copy a member name.
		 **/

		value->parent_ = ndm_json_object_value(object);
		value->member_of_.object_.name_ = name;
		ndm_dlist_insert_before(&object->members_, &value->list_);

		ndm_json_parse_whitespaces_(ctx);

		if (*ctx->json == ',') {
			ctx->json++;
			ndm_json_parse_whitespaces_(ctx);
		} else
		if (*ctx->json == '}') {
			ctx->json++;

			return NDM_JSON_PARSE_ERROR_OK;
		} else {
			/**
			 * Must be a comma or '}' after an object member.
			 **/

			return NDM_JSON_PARSE_ERROR_CORRUPTED_OBJECT;
		}
	} while (true);

	/**
	 * Never reached really.
	 **/

	return NDM_JSON_PARSE_ERROR_OK;
}

static enum ndm_json_parse_error_t ndm_json_parse_(
		const char start_char,
		const enum ndm_json_parse_error_t fail_code,
		struct ndm_pool_t *pool,
		char *json,
		struct ndm_json_value_t **value)
{
	assert (json != NULL);

	enum ndm_json_parse_error_t code = NDM_JSON_PARSE_ERROR_OK;
	struct ndm_json_parse_context_t_ ctx =
	{
		.pool = pool,
		.json = json
	};

	*value = NULL;

	ndm_json_parse_whitespaces_(&ctx);

	if (*ctx.json != start_char) {
		code = (*ctx.json == '\0') ?
			NDM_JSON_PARSE_ERROR_EMPTY_DOCUMENT :
			fail_code;
	} else {
		code = ndm_json_parse_value_(&ctx, value);

		if (code == NDM_JSON_PARSE_ERROR_OK) {
			ndm_json_parse_whitespaces_(&ctx);

			if (*ctx.json != '\0') {
				/**
				 * Nothing should follow the root object or array.
				 **/

				code = NDM_JSON_PARSE_ERROR_TRAILING_SYMBOLS;
			}
		}
	}

	return code;
}

enum ndm_json_parse_error_t ndm_json_array_parse(
		struct ndm_pool_t *pool,
		char *json,
		struct ndm_json_array_t **array)
{
	struct ndm_json_value_t *value = NULL;
	enum ndm_json_parse_error_t code = ndm_json_parse_(
		'[', NDM_JSON_PARSE_ERROR_ARRAY, pool, json, &value);

	*array = (code == NDM_JSON_PARSE_ERROR_OK) ?
		ndm_json_value_array(value) : NULL;

	return code;
}

enum ndm_json_parse_error_t ndm_json_object_parse(
		struct ndm_pool_t *pool,
		char *json,
		struct ndm_json_object_t **object)
{
	struct ndm_json_value_t *value = NULL;
	enum ndm_json_parse_error_t code = ndm_json_parse_(
		'{', NDM_JSON_PARSE_ERROR_OBJECT, pool, json, &value);

	*object = (code == NDM_JSON_PARSE_ERROR_OK) ?
		ndm_json_value_object(value) : NULL;

	return code;
}

/**
 * JSON printer.
 **/

struct ndm_json_print_context_t_
{
	enum ndm_json_print_flags_t flags;
	size_t ident;							//!< current ident
	char *json;								//!< current JSON string
	size_t json_size;						//!< without null-terminator
	char buffer[NDM_JSON_PRINT_BUFSIZE_];	//!< static print buffer
	char *pp;								//!< @c buffer put position
	char *pend;								//!< @c buffer end position
	bool ok;								//!< JSON printing state
};

static void ndm_json_print_init_(
		struct ndm_json_print_context_t_ *ctx,
		const enum ndm_json_print_flags_t flags)
{
	ctx->flags = flags;
	ctx->ident = 0;
	ctx->json = NULL;
	ctx->json_size = 0;
	ctx->pp = ctx->buffer;
	ctx->pend = ctx->buffer + sizeof(ctx->buffer);
	ctx->ok = true;
}

static void ndm_json_print_flush_(
		struct ndm_json_print_context_t_ *ctx)
{
	const size_t buffered = (size_t) (ctx->pp - ctx->buffer);
	char *json = realloc(ctx->json, ctx->json_size + buffered + 1);

	if (json == NULL) {
		ctx->ok = false;
	} else {
		ctx->json = json;
		memcpy(ctx->json + ctx->json_size, ctx->buffer, buffered);
		ctx->json_size += buffered;
		ctx->json[ctx->json_size] = '\0';
	}

	/**
	 * Always reset a pointer.
	 **/

	ctx->pp = ctx->buffer;
}

#define NDM_JSON_PRINT_CSTR_(ctx, cstr)		\
	ndm_json_print_data_(ctx, cstr, sizeof(cstr) - 1)

static void ndm_json_print_data_(
		struct ndm_json_print_context_t_ *ctx,
		const char *const data,
		const size_t data_size)
{
	/**
	 * @c ctx->ok is not checked in this function.
	 **/

	const size_t avail = (size_t) (ctx->pend - ctx->pp);

	/**
	 * Not "<=" to leave at least one free symbol.
	 **/

	if (data_size < avail) {
		memcpy(ctx->pp, data, data_size);
		ctx->pp += data_size;
	} else {
		/**
		 * Try to flush a buffer and print new data to a JSON.
		 **/

		const size_t buffered = (size_t) (ctx->pp - ctx->buffer);
		char *json = realloc(
			ctx->json, ctx->json_size + buffered + data_size + 1);

		if (json == NULL) {
			ctx->ok = false;
		} else {
			ctx->json = json;

			/* Copy the buffered data. */
			memcpy(ctx->json + ctx->json_size, ctx->buffer, buffered);
			ctx->json_size += buffered;

			/* Copy new data. */
			memcpy(ctx->json + ctx->json_size, data, data_size);
			ctx->json_size += data_size;

			ctx->json[ctx->json_size] = '\0';
		}

		/**
		 * Always reset a pointer on overflow.
		 **/

		ctx->pp = ctx->buffer;
	}
}

static void ndm_json_print_fill_(
		struct ndm_json_print_context_t_ *ctx,
		const char c,
		const size_t count)
{
	/**
	 * @c ctx->ok is not checked in this function.
	 **/

	const size_t avail = (size_t) (ctx->pend - ctx->pp);

	/**
	 * Not "<=" to leave at least one free symbol.
	 **/

	if (count < avail) {
		memset(ctx->pp, c, count);
		ctx->pp += count;
	} else {
		/**
		 * Try to flush a buffer and print new data to a JSON.
		 **/

		const size_t buffered = (size_t) (ctx->pp - ctx->buffer);
		char *json = realloc(
			ctx->json, ctx->json_size + buffered + count + 1);

		if (json == NULL) {
			ctx->ok = false;
		} else {
			ctx->json = json;

			/* Copy the buffered data. */
			memcpy(ctx->json + ctx->json_size, ctx->buffer, buffered);
			ctx->json_size += buffered;

			/* Fill new data. */
			memset(ctx->json + ctx->json_size, c, count);
			ctx->json_size += count;

			ctx->json[ctx->json_size] = '\0';
		}

		/**
		 * Always reset a pointer on overflow.
		 **/

		ctx->pp = ctx->buffer;
	}
}

static inline void ndm_json_print_char_(
		struct ndm_json_print_context_t_ *ctx,
		const char c)
{
	/**
	 * There is always at least the one free symbol in a buffer,
	 * no @c ctx->ok checks needed.
	 **/

	*ctx->pp++ = c;

	if (ctx->pp == ctx->pend) {
		ndm_json_print_flush_(ctx);
	}
}

static char *ndm_json_print_done_(
		struct ndm_json_print_context_t_ *ctx,
		size_t *json_size)
{
	ndm_json_print_flush_(ctx);

	if (json_size != NULL) {
		*json_size = ctx->ok ? ctx->json_size : 0;
	}

	if (ctx->ok) {
		return ctx->json;
	}

	free(ctx->json);

	return NULL;
}

static void ndm_json_print_ident_(
		struct ndm_json_print_context_t_ *ctx)
{
	if (!(ctx->flags & NDM_JSON_PRINT_FLAGS_COMPACT)) {
		if (ctx->flags & NDM_JSON_PRINT_FLAGS_CRLF) {
			NDM_JSON_PRINT_CSTR_(ctx, "\r\n");
		} else {
			ndm_json_print_char_(ctx, '\n');
		}

		ndm_json_print_fill_(ctx, ' ', ctx->ident);
	}
}

static void ndm_json_print_string_(
		struct ndm_json_print_context_t_ *ctx,
		const char *const value)
{
	static const char HEX_[16] = "0123456789ABCDEF";
	static const char ESCAPE_[256] =
	{
		'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u',
		'b', 't', 'n', 'u', 'f', 'r', 'u', 'u',
		'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u',
		'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u',
		  0,   0, '"',   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,'\\',   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
	};

	ndm_json_print_char_(ctx, '\"');

	const char *s = value;

	while (*s != '\0') {
		const unsigned char c = (unsigned char) *s++;
		const char e = ESCAPE_[c];

		if (e != 0) {
			/* Print an escaped character. */

			if (e != 'u') {
				char buffer[2] =
				{
					'\\',
					e
				};

				/* "\e" */
				ndm_json_print_data_(ctx, buffer, sizeof(buffer));
			} else {
				char buffer[6] =
				{
					'\\',
					'u',
					'0',
					'0',
					HEX_[(c >> 4) & 0x0f],
					HEX_[(c >> 0) & 0x0f]
				};

				/* "\u00xx" */
				ndm_json_print_data_(ctx, buffer, sizeof(buffer));
			}
		} else {
			ndm_json_print_char_(ctx, (char) c);
		}
	}

	ndm_json_print_char_(ctx, '\"');
}

/**
 * Forward declaration.
 **/

static void ndm_json_print_value_(
		struct ndm_json_print_context_t_ *ctx,
		const struct ndm_json_value_t *value);

static void ndm_json_print_array_(
		struct ndm_json_print_context_t_ *ctx,
		const struct ndm_json_array_t *array)
{
	ndm_json_print_char_(ctx, '[');
	ctx->ident += NDM_JSON_PRINT_IDENT_STEP_;

	const struct ndm_json_array_element_t *e =
		ndm_json_array_element_first(array);

	while (e != NULL && ctx->ok) {
		ndm_json_print_ident_(ctx);
		ndm_json_print_value_(ctx, ndm_json_array_element_value(e));

		e = ndm_json_array_element_next(e);

		if (e != NULL) {
			ndm_json_print_char_(ctx, ',');
		}
	}

	ctx->ident -= NDM_JSON_PRINT_IDENT_STEP_;
	ndm_json_print_ident_(ctx);
	ndm_json_print_char_(ctx, ']');
}

static void ndm_json_print_object_(
		struct ndm_json_print_context_t_ *ctx,
		const struct ndm_json_object_t *object)
{
	ndm_json_print_char_(ctx, '{');
	ctx->ident += NDM_JSON_PRINT_IDENT_STEP_;

	const struct ndm_json_object_member_t *m =
		ndm_json_object_member_first(object);

	while (m != NULL && ctx->ok) {
		ndm_json_print_ident_(ctx);
		ndm_json_print_string_(ctx, ndm_json_object_member_name(m));
		ndm_json_print_char_(ctx, ':');

		if (!(ctx->flags & NDM_JSON_PRINT_FLAGS_COMPACT)) {
			ndm_json_print_char_(ctx, ' ');
		}

		ndm_json_print_value_(ctx, ndm_json_object_member_value(m));

		m = ndm_json_object_member_next(m);

		if (m != NULL) {
			ndm_json_print_char_(ctx, ',');
		}
	}

	ctx->ident -= NDM_JSON_PRINT_IDENT_STEP_;
	ndm_json_print_ident_(ctx);
	ndm_json_print_char_(ctx, '}');
}

static void ndm_json_print_value_(
		struct ndm_json_print_context_t_ *ctx,
		const struct ndm_json_value_t *value)
{
	if (ndm_json_value_is_object(value)) {
		ndm_json_print_object_(ctx, ndm_json_value_object(value));
	} else
	if (ndm_json_value_is_array(value)) {
		ndm_json_print_array_(ctx, ndm_json_value_array(value));
	} else
	if (ndm_json_value_is_string(value)) {
		ndm_json_print_string_(ctx, ndm_json_value_string(value));
	} else
	if (ndm_json_value_is_null(value)) {
		NDM_JSON_PRINT_CSTR_(ctx, "null");
	} else
	if (ndm_json_value_is_true(value)) {
		NDM_JSON_PRINT_CSTR_(ctx, "true");
	} else
	if (ndm_json_value_is_false(value)) {
		NDM_JSON_PRINT_CSTR_(ctx, "false");
	} else
	if (ndm_json_value_is_llong(value)) {
		char buffer[NDM_INT_BUFSIZE];
		const int size = snprintf(
			buffer, sizeof(buffer),
			"%lli", ndm_json_value_llong(value));

		assert (0 < size && size < sizeof(buffer));

		ndm_json_print_data_(ctx, buffer, (size_t) size);
	} else
	if (ndm_json_value_is_ullong(value)) {
		char buffer[NDM_INT_BUFSIZE];
		const int size = snprintf(
			buffer, sizeof(buffer),
			"%llu", ndm_json_value_ullong(value));

		assert (0 < size && size < sizeof(buffer));

		ndm_json_print_data_(ctx, buffer, (size_t) size);
	} else {
		assert (ndm_json_value_is_double(value));

		char buffer[NDM_JSON_DOUBLE_BUFSIZE_];
		const int size = snprintf(
			buffer, sizeof(buffer),
			"%g", ndm_json_value_double(value));

		assert (0 < size && size < sizeof(buffer));

		ndm_json_print_data_(ctx, buffer, (size_t) size);
	}
}

char *ndm_json_array_print(
		const struct ndm_json_array_t *const array,
		const enum ndm_json_print_flags_t flags,
		size_t *json_size)
{
	struct ndm_json_print_context_t_ ctx;

	ndm_json_print_init_(&ctx, flags);
	ndm_json_print_array_(&ctx, array);

	return ndm_json_print_done_(&ctx, json_size);
}

char *ndm_json_object_print(
		const struct ndm_json_object_t *const object,
		const enum ndm_json_print_flags_t flags,
		size_t *json_size)
{
	struct ndm_json_print_context_t_ ctx;

	ndm_json_print_init_(&ctx, flags);
	ndm_json_print_object_(&ctx, object);

	return ndm_json_print_done_(&ctx, json_size);
}

