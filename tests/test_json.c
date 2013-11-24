#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <ndm/pool.h>
#include <ndm/json.h>
#include "test.h"

#define NDM_TEST_POOL_STATIC_SIZE_				512
#define NDM_TEST_POOL_DYNAMIC_SIZE_				1024

#undef  UCHAR_MIN
#define UCHAR_MIN								0

#undef  USHRT_MIN
#define USHRT_MIN								0

#undef  UINT_MIN
#define UINT_MIN								0

#undef  ULONG_MIN
#define ULONG_MIN								0

#undef  ULLONG_MIN
#define ULLONG_MIN								0

#define NDM_TEST_NUMBER_COMMON_(										\
		e, parent,														\
		m, op, bound,													\
		c_type, j_type,													\
		add, item, get)													\
do {																	\
	const struct ndm_json_value_t *jval = ndm_json_##item##_value(e);	\
																		\
	NDM_TEST(!ndm_json_value_is_null(jval));							\
	NDM_TEST(!ndm_json_value_is_boolean(jval));							\
	NDM_TEST(!ndm_json_value_is_object(jval));							\
	NDM_TEST(!ndm_json_value_is_array(jval));							\
	NDM_TEST(!ndm_json_value_is_string(jval));							\
	NDM_TEST( ndm_json_value_is_number(jval));							\
	NDM_TEST((CHAR_##m   op bound) == ndm_json_value_is_char(jval));	\
	NDM_TEST((UCHAR_##m  op bound) == ndm_json_value_is_uchar(jval));	\
	NDM_TEST((SHRT_##m   op bound) == ndm_json_value_is_short(jval));	\
	NDM_TEST((USHRT_##m  op bound) == ndm_json_value_is_ushort(jval));	\
	NDM_TEST((INT_##m    op bound) == ndm_json_value_is_int(jval));		\
	NDM_TEST((UINT_##m   op bound) == ndm_json_value_is_uint(jval));	\
	NDM_TEST((LONG_##m   op bound) == ndm_json_value_is_long(jval));	\
	NDM_TEST((ULONG_##m  op bound) == ndm_json_value_is_ulong(jval));	\
	NDM_TEST((LLONG_##m  op bound) == ndm_json_value_is_llong(jval));	\
	NDM_TEST((ULLONG_##m op bound) == ndm_json_value_is_ullong(jval));	\
	NDM_TEST(!ndm_json_value_is_double(jval));							\
	NDM_TEST( ndm_json_value_##j_type(jval) == val);					\
	NDM_TEST( ndm_json_##item##_next(e) == NULL);						\
	NDM_TEST( ndm_json_##item##_prev(e) != NULL);						\
	NDM_TEST( ndm_json_##item##_last(r) == e);							\
	NDM_TEST( ndm_json_value_parent(jval) == ndm_json_##get(parent));	\
} while (0)

#define NDM_TEST_NUMBER_RANGE_ARRAY_(									\
		e, parent,														\
		m, op, bound,													\
		c_type, j_type,													\
		add, item, get)													\
do {																	\
	const c_type val = bound;											\
																		\
	NDM_TEST_BREAK_IF(ndm_json_##add##_##j_type(parent, val) == NULL);	\
	NDM_TEST_BREAK_IF((e = ndm_json_##item##_next(e)) == NULL);			\
																		\
	NDM_TEST_NUMBER_COMMON_(											\
		e, parent,														\
		m, op, bound,													\
		c_type, j_type,													\
		add, item, get);												\
} while (0)

#define NDM_TEST_NUMBER_RANGE_OBJECT_(									\
		e, parent, name,												\
		m, op, bound,													\
		c_type, j_type,													\
		add, item, get)													\
do {																	\
	const c_type val = bound;											\
																		\
	NDM_TEST_BREAK_IF(													\
		ndm_json_##add##_##j_type(parent, name, val) == NULL);			\
	NDM_TEST_BREAK_IF((e = ndm_json_##item##_next(e)) == NULL);			\
	NDM_TEST( strcmp(ndm_json_##item##_name(e), name) == 0);			\
																		\
	NDM_TEST_NUMBER_COMMON_(											\
		e, parent,														\
		m, op, bound,													\
		c_type, j_type,													\
		add, item, get);												\
} while (0)

#define NDM_TEST_NUMBER_ARRAY_(											\
		e, parent, c_type, j_type, c_name)								\
do {																	\
	NDM_TEST_NUMBER_RANGE_ARRAY_(										\
		e, parent,														\
		MIN, <=, c_name##_MIN,											\
		c_type, j_type,													\
		array_push, array_element, array_value);						\
																		\
	NDM_TEST_NUMBER_RANGE_ARRAY_(										\
		e, parent,														\
		MAX, >=, c_name##_MAX,											\
		c_type, j_type,													\
		array_push, array_element, array_value);						\
																		\
	NDM_TEST_NUMBER_RANGE_ARRAY_(										\
		e, parent,														\
		MAX, >=, U##c_name##_MAX,										\
		unsigned c_type, u##j_type,										\
		array_push, array_element, array_value);						\
} while (0)

#define NDM_TEST_NUMBER_OBJECT_(										\
		e, parent, c_type, j_type, c_name,								\
		name_min, name_max, name_umax)									\
do {																	\
	NDM_TEST_NUMBER_RANGE_OBJECT_(										\
		e, parent, name_min,											\
		MIN, <=, c_name##_MIN,											\
		c_type, j_type,													\
		object_set, object_member, object_value);						\
																		\
	NDM_TEST_NUMBER_RANGE_OBJECT_(										\
		e, parent, name_max,											\
		MAX, >=, c_name##_MAX,											\
		c_type, j_type,													\
		object_set, object_member, object_value);						\
																		\
	NDM_TEST_NUMBER_RANGE_OBJECT_(										\
		e, parent, name_umax,											\
		MAX, >=, U##c_name##_MAX,										\
		unsigned c_type, u##j_type,										\
		object_set, object_member, object_value);						\
} while (0);

static void test_array_()
{
	char buf[NDM_TEST_POOL_STATIC_SIZE_];
	struct ndm_pool_t pool = NDM_POOL_INITIALIZER(
		buf, sizeof(buf), NDM_TEST_POOL_DYNAMIC_SIZE_);
	struct ndm_json_array_t *r = ndm_json_array_new(&pool);
	struct ndm_json_value_t *av = ndm_json_array_value(r);

	NDM_TEST(ndm_json_array_value(r) != NULL);
	NDM_TEST(ndm_json_array_element_first(r) == NULL);
	NDM_TEST(ndm_json_array_element_last(r) == NULL);

	NDM_TEST(!ndm_json_value_is_null(av));
	NDM_TEST(!ndm_json_value_is_boolean(av));
	NDM_TEST(!ndm_json_value_is_object(av));
	NDM_TEST( ndm_json_value_is_array(av));
	NDM_TEST(!ndm_json_value_is_string(av));
	NDM_TEST(!ndm_json_value_is_number(av));
	NDM_TEST(!ndm_json_value_is_char(av));
	NDM_TEST(!ndm_json_value_is_uchar(av));
	NDM_TEST(!ndm_json_value_is_short(av));
	NDM_TEST(!ndm_json_value_is_ushort(av));
	NDM_TEST(!ndm_json_value_is_int(av));
	NDM_TEST(!ndm_json_value_is_uint(av));
	NDM_TEST(!ndm_json_value_is_long(av));
	NDM_TEST(!ndm_json_value_is_ulong(av));
	NDM_TEST(!ndm_json_value_is_llong(av));
	NDM_TEST(!ndm_json_value_is_ullong(av));
	NDM_TEST(!ndm_json_value_is_double(av));
	NDM_TEST(ndm_json_value_parent(av) == NULL);

	const struct ndm_json_value_t *n = ndm_json_array_push_null(r);
	struct ndm_json_array_element_t *e = ndm_json_array_element_first(r);

	NDM_TEST_BREAK_IF(n == NULL);
	NDM_TEST_BREAK_IF(e == NULL);

	NDM_TEST( ndm_json_value_is_null(n));
	NDM_TEST(!ndm_json_value_is_boolean(n));
	NDM_TEST(!ndm_json_value_is_object(n));
	NDM_TEST(!ndm_json_value_is_array(n));
	NDM_TEST(!ndm_json_value_is_string(n));
	NDM_TEST(!ndm_json_value_is_number(n));
	NDM_TEST(!ndm_json_value_is_char(n));
	NDM_TEST(!ndm_json_value_is_uchar(n));
	NDM_TEST(!ndm_json_value_is_short(n));
	NDM_TEST(!ndm_json_value_is_ushort(n));
	NDM_TEST(!ndm_json_value_is_int(n));
	NDM_TEST(!ndm_json_value_is_uint(n));
	NDM_TEST(!ndm_json_value_is_long(n));
	NDM_TEST(!ndm_json_value_is_ulong(n));
	NDM_TEST(!ndm_json_value_is_llong(n));
	NDM_TEST(!ndm_json_value_is_ullong(n));
	NDM_TEST(!ndm_json_value_is_double(n));
	NDM_TEST( ndm_json_array_element_next(e) == NULL);
	NDM_TEST( ndm_json_array_element_prev(e) == NULL);
	NDM_TEST( ndm_json_array_element_last(r) == e);
	NDM_TEST( ndm_json_value_parent(n) == ndm_json_array_value(r));

	NDM_TEST_BREAK_IF(ndm_json_array_push_array(r) == NULL);
	NDM_TEST_BREAK_IF((e = ndm_json_array_element_next(e)) == NULL);

	struct ndm_json_value_t *a = ndm_json_array_element_value(e);

	NDM_TEST( ndm_json_value_array(a) != NULL);
	NDM_TEST( ndm_json_value_object(a) == NULL);
	NDM_TEST(!ndm_json_value_is_null(a));
	NDM_TEST(!ndm_json_value_is_boolean(a));
	NDM_TEST(!ndm_json_value_is_object(a));
	NDM_TEST( ndm_json_value_is_array(a));
	NDM_TEST(!ndm_json_value_is_string(a));
	NDM_TEST(!ndm_json_value_is_number(a));
	NDM_TEST(!ndm_json_value_is_char(a));
	NDM_TEST(!ndm_json_value_is_uchar(a));
	NDM_TEST(!ndm_json_value_is_short(a));
	NDM_TEST(!ndm_json_value_is_ushort(a));
	NDM_TEST(!ndm_json_value_is_int(a));
	NDM_TEST(!ndm_json_value_is_uint(a));
	NDM_TEST(!ndm_json_value_is_long(a));
	NDM_TEST(!ndm_json_value_is_ulong(a));
	NDM_TEST(!ndm_json_value_is_llong(a));
	NDM_TEST(!ndm_json_value_is_ullong(a));
	NDM_TEST(!ndm_json_value_is_double(a));
	NDM_TEST( ndm_json_value_parent(n) == ndm_json_array_value(r));
	NDM_TEST( ndm_json_array_element_next(e) == NULL);
	NDM_TEST( ndm_json_array_element_prev(e) != NULL);
	NDM_TEST( ndm_json_array_element_last(r) == e);

	NDM_TEST_BREAK_IF(ndm_json_array_push_object(r) == NULL);
	NDM_TEST_BREAK_IF((e = ndm_json_array_element_next(e)) == NULL);

	const struct ndm_json_value_t *o = ndm_json_array_element_value(e);

	NDM_TEST( ndm_json_value_array(o) == NULL);
	NDM_TEST( ndm_json_value_object(o) != NULL);
	NDM_TEST(!ndm_json_value_is_null(o));
	NDM_TEST(!ndm_json_value_is_boolean(o));
	NDM_TEST( ndm_json_value_is_object(o));
	NDM_TEST(!ndm_json_value_is_array(o));
	NDM_TEST(!ndm_json_value_is_string(o));
	NDM_TEST(!ndm_json_value_is_number(o));
	NDM_TEST(!ndm_json_value_is_char(o));
	NDM_TEST(!ndm_json_value_is_uchar(o));
	NDM_TEST(!ndm_json_value_is_short(o));
	NDM_TEST(!ndm_json_value_is_ushort(o));
	NDM_TEST(!ndm_json_value_is_int(o));
	NDM_TEST(!ndm_json_value_is_uint(o));
	NDM_TEST(!ndm_json_value_is_long(o));
	NDM_TEST(!ndm_json_value_is_ulong(o));
	NDM_TEST(!ndm_json_value_is_llong(o));
	NDM_TEST(!ndm_json_value_is_ullong(o));
	NDM_TEST(!ndm_json_value_is_double(o));
	NDM_TEST( ndm_json_value_parent(o) == ndm_json_array_value(r));
	NDM_TEST( ndm_json_array_element_next(e) == NULL);
	NDM_TEST( ndm_json_array_element_prev(e) != NULL);
	NDM_TEST( ndm_json_array_element_last(r) == e);

	NDM_TEST_BREAK_IF(ndm_json_array_push_boolean(r, false) == NULL);
	NDM_TEST_BREAK_IF((e = ndm_json_array_element_next(e)) == NULL);

	const struct ndm_json_value_t *f = ndm_json_array_element_value(e);

	NDM_TEST(!ndm_json_value_is_null(f));
	NDM_TEST( ndm_json_value_is_boolean(f));
	NDM_TEST(!ndm_json_value_is_object(f));
	NDM_TEST(!ndm_json_value_is_array(f));
	NDM_TEST(!ndm_json_value_is_string(f));
	NDM_TEST(!ndm_json_value_is_number(f));
	NDM_TEST(!ndm_json_value_is_char(f));
	NDM_TEST(!ndm_json_value_is_uchar(f));
	NDM_TEST(!ndm_json_value_is_short(f));
	NDM_TEST(!ndm_json_value_is_ushort(f));
	NDM_TEST(!ndm_json_value_is_int(f));
	NDM_TEST(!ndm_json_value_is_uint(f));
	NDM_TEST(!ndm_json_value_is_long(f));
	NDM_TEST(!ndm_json_value_is_ulong(f));
	NDM_TEST(!ndm_json_value_is_llong(f));
	NDM_TEST(!ndm_json_value_is_ullong(f));
	NDM_TEST(!ndm_json_value_is_double(f));
	NDM_TEST( ndm_json_value_boolean(f) == false);
	NDM_TEST( ndm_json_array_element_next(e) == NULL);
	NDM_TEST( ndm_json_array_element_prev(e) != NULL);
	NDM_TEST( ndm_json_array_element_last(r) == e);
	NDM_TEST( ndm_json_value_parent(f) == ndm_json_array_value(r));

	NDM_TEST_BREAK_IF(ndm_json_array_push_boolean(r, true) == NULL);
	NDM_TEST_BREAK_IF((e = ndm_json_array_element_next(e)) == NULL);

	const struct ndm_json_value_t *t = ndm_json_array_element_value(e);

	NDM_TEST(!ndm_json_value_is_null(t));
	NDM_TEST( ndm_json_value_is_boolean(t));
	NDM_TEST(!ndm_json_value_is_object(t));
	NDM_TEST(!ndm_json_value_is_array(t));
	NDM_TEST(!ndm_json_value_is_string(t));
	NDM_TEST(!ndm_json_value_is_number(t));
	NDM_TEST(!ndm_json_value_is_char(t));
	NDM_TEST(!ndm_json_value_is_uchar(t));
	NDM_TEST(!ndm_json_value_is_short(t));
	NDM_TEST(!ndm_json_value_is_ushort(t));
	NDM_TEST(!ndm_json_value_is_int(t));
	NDM_TEST(!ndm_json_value_is_uint(t));
	NDM_TEST(!ndm_json_value_is_long(t));
	NDM_TEST(!ndm_json_value_is_ulong(t));
	NDM_TEST(!ndm_json_value_is_llong(t));
	NDM_TEST(!ndm_json_value_is_ullong(t));
	NDM_TEST(!ndm_json_value_is_double(t));
	NDM_TEST( ndm_json_value_boolean(t) == true);
	NDM_TEST( ndm_json_array_element_next(e) == NULL);
	NDM_TEST( ndm_json_array_element_prev(e) != NULL);
	NDM_TEST( ndm_json_array_element_last(r) == e);
	NDM_TEST( ndm_json_value_parent(t) == ndm_json_array_value(r));

	NDM_TEST_BREAK_IF(ndm_json_array_push_string(r, "test string") == NULL);
	NDM_TEST_BREAK_IF((e = ndm_json_array_element_next(e)) == NULL);

	const struct ndm_json_value_t *s = ndm_json_array_element_value(e);

	NDM_TEST(!ndm_json_value_is_null(s));
	NDM_TEST(!ndm_json_value_is_boolean(s));
	NDM_TEST(!ndm_json_value_is_object(s));
	NDM_TEST(!ndm_json_value_is_array(s));
	NDM_TEST( ndm_json_value_is_string(s));
	NDM_TEST(!ndm_json_value_is_number(s));
	NDM_TEST(!ndm_json_value_is_char(s));
	NDM_TEST(!ndm_json_value_is_uchar(s));
	NDM_TEST(!ndm_json_value_is_short(s));
	NDM_TEST(!ndm_json_value_is_ushort(s));
	NDM_TEST(!ndm_json_value_is_int(s));
	NDM_TEST(!ndm_json_value_is_uint(s));
	NDM_TEST(!ndm_json_value_is_long(s));
	NDM_TEST(!ndm_json_value_is_ulong(s));
	NDM_TEST(!ndm_json_value_is_llong(s));
	NDM_TEST(!ndm_json_value_is_ullong(s));
	NDM_TEST(!ndm_json_value_is_double(s));
	NDM_TEST( strcmp(ndm_json_value_string(s), "test string") == 0);
	NDM_TEST( ndm_json_array_element_next(e) == NULL);
	NDM_TEST( ndm_json_array_element_prev(e) != NULL);
	NDM_TEST( ndm_json_array_element_last(r) == e);
	NDM_TEST( ndm_json_value_parent(s) == ndm_json_array_value(r));

	NDM_TEST_BREAK_IF(
		ndm_json_array_push_string(r, "test string 2") == NULL);
	NDM_TEST_BREAK_IF((e = ndm_json_array_element_next(e)) == NULL);

	const struct ndm_json_value_t *c = ndm_json_array_element_value(e);

	NDM_TEST(!ndm_json_value_is_null(c));
	NDM_TEST(!ndm_json_value_is_boolean(c));
	NDM_TEST(!ndm_json_value_is_object(c));
	NDM_TEST(!ndm_json_value_is_array(c));
	NDM_TEST( ndm_json_value_is_string(c));
	NDM_TEST(!ndm_json_value_is_number(c));
	NDM_TEST(!ndm_json_value_is_char(c));
	NDM_TEST(!ndm_json_value_is_uchar(c));
	NDM_TEST(!ndm_json_value_is_short(c));
	NDM_TEST(!ndm_json_value_is_ushort(c));
	NDM_TEST(!ndm_json_value_is_int(c));
	NDM_TEST(!ndm_json_value_is_uint(c));
	NDM_TEST(!ndm_json_value_is_long(c));
	NDM_TEST(!ndm_json_value_is_ulong(c));
	NDM_TEST(!ndm_json_value_is_llong(c));
	NDM_TEST(!ndm_json_value_is_ullong(c));
	NDM_TEST(!ndm_json_value_is_double(c));
	NDM_TEST( strcmp(ndm_json_value_string(c), "test string 2") == 0);
	NDM_TEST( ndm_json_array_element_next(e) == NULL);
	NDM_TEST( ndm_json_array_element_prev(e) != NULL);
	NDM_TEST( ndm_json_array_element_last(r) == e);
	NDM_TEST( ndm_json_value_parent(c) == ndm_json_array_value(r));

	NDM_TEST_NUMBER_ARRAY_(
		e, r,
		char, char, CHAR);

	NDM_TEST_NUMBER_ARRAY_(
		e, r,
		short, short, SHRT);

	NDM_TEST_NUMBER_ARRAY_(
		e, r,
		int, int, INT);

	NDM_TEST_NUMBER_ARRAY_(
		e, r,
		long, long, LONG);

	NDM_TEST_NUMBER_ARRAY_(
		e, r,
		long long, llong, LLONG);

	const double nd = 1.0;

	NDM_TEST_BREAK_IF(ndm_json_array_push_double(r, nd) == NULL);
	NDM_TEST_BREAK_IF((e = ndm_json_array_element_next(e)) == NULL);

	const struct ndm_json_value_t *dn = ndm_json_array_element_value(e);

	NDM_TEST(!ndm_json_value_is_null(dn));
	NDM_TEST(!ndm_json_value_is_boolean(dn));
	NDM_TEST(!ndm_json_value_is_object(dn));
	NDM_TEST(!ndm_json_value_is_array(dn));
	NDM_TEST(!ndm_json_value_is_string(dn));
	NDM_TEST( ndm_json_value_is_number(dn));
	NDM_TEST(!ndm_json_value_is_char(dn));
	NDM_TEST(!ndm_json_value_is_uchar(dn));
	NDM_TEST(!ndm_json_value_is_short(dn));
	NDM_TEST(!ndm_json_value_is_ushort(dn));
	NDM_TEST(!ndm_json_value_is_int(dn));
	NDM_TEST(!ndm_json_value_is_uint(dn));
	NDM_TEST(!ndm_json_value_is_long(dn));
	NDM_TEST(!ndm_json_value_is_ulong(dn));
	NDM_TEST(!ndm_json_value_is_llong(dn));
	NDM_TEST(!ndm_json_value_is_ullong(dn));
	NDM_TEST( ndm_json_value_is_double(dn));
	NDM_TEST( ndm_json_value_double(dn) == nd);
	NDM_TEST( ndm_json_array_element_next(e) == NULL);
	NDM_TEST( ndm_json_array_element_prev(e) != NULL);
	NDM_TEST( ndm_json_array_element_last(r) == e);
	NDM_TEST( ndm_json_value_parent(dn) == ndm_json_array_value(r));

#define CHECK_JSON_ELEMENT_TYPE_(e, pred)								\
do {																	\
	NDM_TEST_BREAK_IF(e == NULL);										\
	NDM_TEST(pred(ndm_json_array_element_value(e)));					\
																		\
	e = ndm_json_array_element_next(e);									\
} while (false)

	e = ndm_json_array_element_first(r);

	CHECK_JSON_ELEMENT_TYPE_(e, ndm_json_value_is_null);
	CHECK_JSON_ELEMENT_TYPE_(e, ndm_json_value_is_array);
	CHECK_JSON_ELEMENT_TYPE_(e, ndm_json_value_is_object);
	CHECK_JSON_ELEMENT_TYPE_(e, ndm_json_value_is_boolean);
	CHECK_JSON_ELEMENT_TYPE_(e, ndm_json_value_is_boolean);
	CHECK_JSON_ELEMENT_TYPE_(e, ndm_json_value_is_string);
	CHECK_JSON_ELEMENT_TYPE_(e, ndm_json_value_is_string);
	CHECK_JSON_ELEMENT_TYPE_(e, ndm_json_value_is_number);
	CHECK_JSON_ELEMENT_TYPE_(e, ndm_json_value_is_number);
	CHECK_JSON_ELEMENT_TYPE_(e, ndm_json_value_is_number);
	CHECK_JSON_ELEMENT_TYPE_(e, ndm_json_value_is_number);
	CHECK_JSON_ELEMENT_TYPE_(e, ndm_json_value_is_number);
	CHECK_JSON_ELEMENT_TYPE_(e, ndm_json_value_is_number);
	CHECK_JSON_ELEMENT_TYPE_(e, ndm_json_value_is_number);
	CHECK_JSON_ELEMENT_TYPE_(e, ndm_json_value_is_number);
	CHECK_JSON_ELEMENT_TYPE_(e, ndm_json_value_is_number);
	CHECK_JSON_ELEMENT_TYPE_(e, ndm_json_value_is_number);
	CHECK_JSON_ELEMENT_TYPE_(e, ndm_json_value_is_number);

	ndm_pool_clear(&pool);

	/* NULL-safe operations */
	NDM_TEST(ndm_json_array_push_null(NULL) == NULL);
	NDM_TEST(ndm_json_array_push_boolean(NULL, true) == NULL);
	NDM_TEST(ndm_json_array_push_boolean(NULL, false) == NULL);
	NDM_TEST(ndm_json_array_push_string(NULL, NULL) == NULL);
	NDM_TEST(ndm_json_array_push_llong(NULL, 0) == NULL);
	NDM_TEST(ndm_json_array_push_ullong(NULL, 0) == NULL);
	NDM_TEST(ndm_json_array_push_double(NULL, 0.0) == NULL);
	NDM_TEST(ndm_json_array_push_array(NULL) == NULL);
	NDM_TEST(ndm_json_array_push_object(NULL) == NULL);
}

static void test_object_()
{
	char buf[NDM_TEST_POOL_STATIC_SIZE_];
	struct ndm_pool_t pool = NDM_POOL_INITIALIZER(
		buf, sizeof(buf), NDM_TEST_POOL_DYNAMIC_SIZE_);
	struct ndm_json_object_t *r = ndm_json_object_new(&pool);
	struct ndm_json_value_t *ov = ndm_json_object_value(r);

	NDM_TEST(ndm_json_object_value(r) != NULL);
	NDM_TEST(ndm_json_object_member_first(r) == NULL);
	NDM_TEST(ndm_json_object_member_last(r) == NULL);

	NDM_TEST(!ndm_json_value_is_null(ov));
	NDM_TEST(!ndm_json_value_is_boolean(ov));
	NDM_TEST( ndm_json_value_is_object(ov));
	NDM_TEST(!ndm_json_value_is_array(ov));
	NDM_TEST(!ndm_json_value_is_string(ov));
	NDM_TEST(!ndm_json_value_is_number(ov));
	NDM_TEST(!ndm_json_value_is_char(ov));
	NDM_TEST(!ndm_json_value_is_uchar(ov));
	NDM_TEST(!ndm_json_value_is_short(ov));
	NDM_TEST(!ndm_json_value_is_ushort(ov));
	NDM_TEST(!ndm_json_value_is_int(ov));
	NDM_TEST(!ndm_json_value_is_uint(ov));
	NDM_TEST(!ndm_json_value_is_long(ov));
	NDM_TEST(!ndm_json_value_is_ulong(ov));
	NDM_TEST(!ndm_json_value_is_llong(ov));
	NDM_TEST(!ndm_json_value_is_ullong(ov));
	NDM_TEST(!ndm_json_value_is_double(ov));
	NDM_TEST(ndm_json_value_parent(ov) == NULL);

	const struct ndm_json_value_t *n = ndm_json_object_set_null(r, "first");
	struct ndm_json_object_member_t *m = ndm_json_object_member_first(r);

	NDM_TEST_BREAK_IF(n == NULL);
	NDM_TEST_BREAK_IF(m == NULL);

	NDM_TEST(strcmp(ndm_json_object_member_name(m), "first") == 0);

	NDM_TEST( ndm_json_value_is_null(n));
	NDM_TEST(!ndm_json_value_is_boolean(n));
	NDM_TEST(!ndm_json_value_is_object(n));
	NDM_TEST(!ndm_json_value_is_array(n));
	NDM_TEST(!ndm_json_value_is_string(n));
	NDM_TEST(!ndm_json_value_is_number(n));
	NDM_TEST(!ndm_json_value_is_char(n));
	NDM_TEST(!ndm_json_value_is_uchar(n));
	NDM_TEST(!ndm_json_value_is_short(n));
	NDM_TEST(!ndm_json_value_is_ushort(n));
	NDM_TEST(!ndm_json_value_is_int(n));
	NDM_TEST(!ndm_json_value_is_uint(n));
	NDM_TEST(!ndm_json_value_is_long(n));
	NDM_TEST(!ndm_json_value_is_ulong(n));
	NDM_TEST(!ndm_json_value_is_llong(n));
	NDM_TEST(!ndm_json_value_is_ullong(n));
	NDM_TEST(!ndm_json_value_is_double(n));
	NDM_TEST( ndm_json_object_member_next(m) == NULL);
	NDM_TEST( ndm_json_object_member_prev(m) == NULL);
	NDM_TEST( ndm_json_object_member_last(r) == m);
	NDM_TEST( ndm_json_value_parent(n) == ndm_json_object_value(r));

	NDM_TEST_BREAK_IF(ndm_json_object_set_array(r, "second") == NULL);
	NDM_TEST_BREAK_IF((m = ndm_json_object_member_next(m)) == NULL);

	NDM_TEST( strcmp(ndm_json_object_member_name(m), "second") == 0);

	struct ndm_json_value_t *a = ndm_json_object_member_value(m);

	NDM_TEST( ndm_json_value_array(a) != NULL);
	NDM_TEST( ndm_json_value_object(a) == NULL);

	NDM_TEST(!ndm_json_value_is_null(a));
	NDM_TEST(!ndm_json_value_is_boolean(a));
	NDM_TEST(!ndm_json_value_is_object(a));
	NDM_TEST( ndm_json_value_is_array(a));
	NDM_TEST(!ndm_json_value_is_string(a));
	NDM_TEST(!ndm_json_value_is_number(a));
	NDM_TEST(!ndm_json_value_is_char(a));
	NDM_TEST(!ndm_json_value_is_uchar(a));
	NDM_TEST(!ndm_json_value_is_short(a));
	NDM_TEST(!ndm_json_value_is_ushort(a));
	NDM_TEST(!ndm_json_value_is_int(a));
	NDM_TEST(!ndm_json_value_is_uint(a));
	NDM_TEST(!ndm_json_value_is_long(a));
	NDM_TEST(!ndm_json_value_is_ulong(a));
	NDM_TEST(!ndm_json_value_is_llong(a));
	NDM_TEST(!ndm_json_value_is_ullong(a));
	NDM_TEST(!ndm_json_value_is_double(a));
	NDM_TEST( ndm_json_value_parent(n) == ndm_json_object_value(r));
	NDM_TEST( ndm_json_object_member_next(m) == NULL);
	NDM_TEST( ndm_json_object_member_prev(m) != NULL);
	NDM_TEST( ndm_json_object_member_last(r) == m);

	NDM_TEST_BREAK_IF(ndm_json_object_set_object(r, "third") == NULL);
	NDM_TEST_BREAK_IF((m = ndm_json_object_member_next(m)) == NULL);

	const struct ndm_json_value_t *o = ndm_json_object_member_value(m);

	NDM_TEST( strcmp(ndm_json_object_member_name(m), "third") == 0);
	NDM_TEST( ndm_json_value_array(o) == NULL);
	NDM_TEST( ndm_json_value_object(o) != NULL);

	NDM_TEST(!ndm_json_value_is_null(o));
	NDM_TEST(!ndm_json_value_is_boolean(o));
	NDM_TEST( ndm_json_value_is_object(o));
	NDM_TEST(!ndm_json_value_is_array(o));
	NDM_TEST(!ndm_json_value_is_string(o));
	NDM_TEST(!ndm_json_value_is_number(o));
	NDM_TEST(!ndm_json_value_is_char(o));
	NDM_TEST(!ndm_json_value_is_uchar(o));
	NDM_TEST(!ndm_json_value_is_short(o));
	NDM_TEST(!ndm_json_value_is_ushort(o));
	NDM_TEST(!ndm_json_value_is_int(o));
	NDM_TEST(!ndm_json_value_is_uint(o));
	NDM_TEST(!ndm_json_value_is_long(o));
	NDM_TEST(!ndm_json_value_is_ulong(o));
	NDM_TEST(!ndm_json_value_is_llong(o));
	NDM_TEST(!ndm_json_value_is_ullong(o));
	NDM_TEST(!ndm_json_value_is_double(o));
	NDM_TEST( ndm_json_value_parent(o) == ndm_json_object_value(r));
	NDM_TEST( ndm_json_object_member_next(m) == NULL);
	NDM_TEST( ndm_json_object_member_prev(m) != NULL);
	NDM_TEST( ndm_json_object_member_last(r) == m);

	NDM_TEST_BREAK_IF(
		ndm_json_object_set_boolean(r, "fourth", false) == NULL);
	NDM_TEST_BREAK_IF((m = ndm_json_object_member_next(m)) == NULL);

	const struct ndm_json_value_t *f = ndm_json_object_member_value(m);

	NDM_TEST( strcmp(ndm_json_object_member_name(m), "fourth") == 0);

	NDM_TEST(!ndm_json_value_is_null(f));
	NDM_TEST( ndm_json_value_is_boolean(f));
	NDM_TEST(!ndm_json_value_is_object(f));
	NDM_TEST(!ndm_json_value_is_array(f));
	NDM_TEST(!ndm_json_value_is_string(f));
	NDM_TEST(!ndm_json_value_is_number(f));
	NDM_TEST(!ndm_json_value_is_char(f));
	NDM_TEST(!ndm_json_value_is_uchar(f));
	NDM_TEST(!ndm_json_value_is_short(f));
	NDM_TEST(!ndm_json_value_is_ushort(f));
	NDM_TEST(!ndm_json_value_is_int(f));
	NDM_TEST(!ndm_json_value_is_uint(f));
	NDM_TEST(!ndm_json_value_is_long(f));
	NDM_TEST(!ndm_json_value_is_ulong(f));
	NDM_TEST(!ndm_json_value_is_llong(f));
	NDM_TEST(!ndm_json_value_is_ullong(f));
	NDM_TEST(!ndm_json_value_is_double(f));
	NDM_TEST( ndm_json_value_boolean(f) == false);
	NDM_TEST( ndm_json_object_member_next(m) == NULL);
	NDM_TEST( ndm_json_object_member_prev(m) != NULL);
	NDM_TEST( ndm_json_object_member_last(r) == m);
	NDM_TEST( ndm_json_value_parent(f) == ndm_json_object_value(r));

	NDM_TEST_BREAK_IF(ndm_json_object_set_boolean(r, "fifth", true) == NULL);
	NDM_TEST_BREAK_IF((m = ndm_json_object_member_next(m)) == NULL);

	const struct ndm_json_value_t *t = ndm_json_object_member_value(m);

	NDM_TEST( strcmp(ndm_json_object_member_name(m), "fifth") == 0);

	NDM_TEST(!ndm_json_value_is_null(t));
	NDM_TEST( ndm_json_value_is_boolean(t));
	NDM_TEST(!ndm_json_value_is_object(t));
	NDM_TEST(!ndm_json_value_is_array(t));
	NDM_TEST(!ndm_json_value_is_string(t));
	NDM_TEST(!ndm_json_value_is_number(t));
	NDM_TEST(!ndm_json_value_is_char(t));
	NDM_TEST(!ndm_json_value_is_uchar(t));
	NDM_TEST(!ndm_json_value_is_short(t));
	NDM_TEST(!ndm_json_value_is_ushort(t));
	NDM_TEST(!ndm_json_value_is_int(t));
	NDM_TEST(!ndm_json_value_is_uint(t));
	NDM_TEST(!ndm_json_value_is_long(t));
	NDM_TEST(!ndm_json_value_is_ulong(t));
	NDM_TEST(!ndm_json_value_is_llong(t));
	NDM_TEST(!ndm_json_value_is_ullong(t));
	NDM_TEST(!ndm_json_value_is_double(t));
	NDM_TEST( ndm_json_value_boolean(t) == true);
	NDM_TEST( ndm_json_object_member_next(m) == NULL);
	NDM_TEST( ndm_json_object_member_prev(m) != NULL);
	NDM_TEST( ndm_json_object_member_last(r) == m);
	NDM_TEST( ndm_json_value_parent(t) == ndm_json_object_value(r));

	NDM_TEST_BREAK_IF(
		ndm_json_object_set_string(r, "sixth", "test string") == NULL);
	NDM_TEST_BREAK_IF((m = ndm_json_object_member_next(m)) == NULL);

	const struct ndm_json_value_t *s = ndm_json_object_member_value(m);

	NDM_TEST( strcmp(ndm_json_object_member_name(m), "sixth") == 0);

	NDM_TEST(!ndm_json_value_is_null(s));
	NDM_TEST(!ndm_json_value_is_boolean(s));
	NDM_TEST(!ndm_json_value_is_object(s));
	NDM_TEST(!ndm_json_value_is_array(s));
	NDM_TEST( ndm_json_value_is_string(s));
	NDM_TEST(!ndm_json_value_is_number(s));
	NDM_TEST(!ndm_json_value_is_char(s));
	NDM_TEST(!ndm_json_value_is_uchar(s));
	NDM_TEST(!ndm_json_value_is_short(s));
	NDM_TEST(!ndm_json_value_is_ushort(s));
	NDM_TEST(!ndm_json_value_is_int(s));
	NDM_TEST(!ndm_json_value_is_uint(s));
	NDM_TEST(!ndm_json_value_is_long(s));
	NDM_TEST(!ndm_json_value_is_ulong(s));
	NDM_TEST(!ndm_json_value_is_llong(s));
	NDM_TEST(!ndm_json_value_is_ullong(s));
	NDM_TEST(!ndm_json_value_is_double(s));
	NDM_TEST( strcmp(ndm_json_value_string(s), "test string") == 0);
	NDM_TEST( ndm_json_object_member_next(m) == NULL);
	NDM_TEST( ndm_json_object_member_prev(m) != NULL);
	NDM_TEST( ndm_json_object_member_last(r) == m);
	NDM_TEST( ndm_json_value_parent(s) == ndm_json_object_value(r));

	NDM_TEST_BREAK_IF(
		ndm_json_object_set_string(r, "seventh", "test string 2") == NULL);
	NDM_TEST_BREAK_IF((m = ndm_json_object_member_next(m)) == NULL);

	const struct ndm_json_value_t *c = ndm_json_object_member_value(m);

	NDM_TEST( strcmp(ndm_json_object_member_name(m), "seventh") == 0);

	NDM_TEST(!ndm_json_value_is_null(c));
	NDM_TEST(!ndm_json_value_is_boolean(c));
	NDM_TEST(!ndm_json_value_is_object(c));
	NDM_TEST(!ndm_json_value_is_array(c));
	NDM_TEST( ndm_json_value_is_string(c));
	NDM_TEST(!ndm_json_value_is_number(c));
	NDM_TEST(!ndm_json_value_is_char(c));
	NDM_TEST(!ndm_json_value_is_uchar(c));
	NDM_TEST(!ndm_json_value_is_short(c));
	NDM_TEST(!ndm_json_value_is_ushort(c));
	NDM_TEST(!ndm_json_value_is_int(c));
	NDM_TEST(!ndm_json_value_is_uint(c));
	NDM_TEST(!ndm_json_value_is_long(c));
	NDM_TEST(!ndm_json_value_is_ulong(c));
	NDM_TEST(!ndm_json_value_is_llong(c));
	NDM_TEST(!ndm_json_value_is_ullong(c));
	NDM_TEST(!ndm_json_value_is_double(c));
	NDM_TEST( strcmp(ndm_json_value_string(c), "test string 2") == 0);
	NDM_TEST( ndm_json_object_member_next(m) == NULL);
	NDM_TEST( ndm_json_object_member_prev(m) != NULL);
	NDM_TEST( ndm_json_object_member_last(r) == m);
	NDM_TEST( ndm_json_value_parent(c) == ndm_json_object_value(r));

	NDM_TEST_NUMBER_OBJECT_(
		m, r,
		char, char, CHAR,
		"eigth", "nineth", "tenth");

	NDM_TEST_NUMBER_OBJECT_(
		m, r,
		short, short, SHRT,
		"eleventh", "twelfth", "thirteenth");

	NDM_TEST_NUMBER_OBJECT_(
		m, r,
		int, int, INT,
		"fourteenth", "fifteenth", "sixteenth");

	NDM_TEST_NUMBER_OBJECT_(
		m, r,
		long, long, LONG,
		"seventeenth", "eighteenth", "nineteenth");

	NDM_TEST_NUMBER_OBJECT_(
		m, r,
		long long, llong, LLONG,
		"twentieth", "twenty-first", "twenty-second");

	const double nd = 1.0;

	NDM_TEST_BREAK_IF(
		ndm_json_object_set_double(r, "twenty-third", nd) == NULL);
	NDM_TEST_BREAK_IF((m = ndm_json_object_member_next(m)) == NULL);

	const struct ndm_json_value_t *dn = ndm_json_object_member_value(m);

	NDM_TEST( strcmp(ndm_json_object_member_name(m), "twenty-third") == 0);

	NDM_TEST(!ndm_json_value_is_null(dn));
	NDM_TEST(!ndm_json_value_is_boolean(dn));
	NDM_TEST(!ndm_json_value_is_object(dn));
	NDM_TEST(!ndm_json_value_is_array(dn));
	NDM_TEST(!ndm_json_value_is_string(dn));
	NDM_TEST( ndm_json_value_is_number(dn));
	NDM_TEST(!ndm_json_value_is_char(dn));
	NDM_TEST(!ndm_json_value_is_uchar(dn));
	NDM_TEST(!ndm_json_value_is_short(dn));
	NDM_TEST(!ndm_json_value_is_ushort(dn));
	NDM_TEST(!ndm_json_value_is_int(dn));
	NDM_TEST(!ndm_json_value_is_uint(dn));
	NDM_TEST(!ndm_json_value_is_long(dn));
	NDM_TEST(!ndm_json_value_is_ulong(dn));
	NDM_TEST(!ndm_json_value_is_llong(dn));
	NDM_TEST(!ndm_json_value_is_ullong(dn));
	NDM_TEST( ndm_json_value_is_double(dn));
	NDM_TEST( ndm_json_value_double(dn) == nd);
	NDM_TEST( ndm_json_object_member_next(m) == NULL);
	NDM_TEST( ndm_json_object_member_prev(m) != NULL);
	NDM_TEST( ndm_json_object_member_last(r) == m);
	NDM_TEST( ndm_json_value_parent(dn) == ndm_json_object_value(r));

#define CHECK_JSON_MEMBER_TYPE_(o, name, pred)							\
do {																	\
	struct ndm_json_value_t *oval = ndm_json_object_get(o, name);		\
																		\
	NDM_TEST_BREAK_IF(oval == NULL);									\
	NDM_TEST(pred(oval));												\
} while (false)

	CHECK_JSON_MEMBER_TYPE_(r, "first", ndm_json_value_is_null);
	CHECK_JSON_MEMBER_TYPE_(r, "second", ndm_json_value_is_array);
	CHECK_JSON_MEMBER_TYPE_(r, "third", ndm_json_value_is_object);
	CHECK_JSON_MEMBER_TYPE_(r, "fourth", ndm_json_value_is_boolean);
	CHECK_JSON_MEMBER_TYPE_(r, "fifth", ndm_json_value_is_boolean);
	CHECK_JSON_MEMBER_TYPE_(r, "sixth", ndm_json_value_is_string);
	CHECK_JSON_MEMBER_TYPE_(r, "seventh", ndm_json_value_is_string);
	CHECK_JSON_MEMBER_TYPE_(r, "nineth", ndm_json_value_is_number);
	CHECK_JSON_MEMBER_TYPE_(r, "tenth", ndm_json_value_is_number);
	CHECK_JSON_MEMBER_TYPE_(r, "eleventh", ndm_json_value_is_number);
	CHECK_JSON_MEMBER_TYPE_(r, "twelfth", ndm_json_value_is_number);
	CHECK_JSON_MEMBER_TYPE_(r, "thirteenth", ndm_json_value_is_number);
	CHECK_JSON_MEMBER_TYPE_(r, "fourteenth", ndm_json_value_is_number);
	CHECK_JSON_MEMBER_TYPE_(r, "fifteenth", ndm_json_value_is_number);
	CHECK_JSON_MEMBER_TYPE_(r, "sixteenth", ndm_json_value_is_number);
	CHECK_JSON_MEMBER_TYPE_(r, "eighteenth", ndm_json_value_is_number);
	CHECK_JSON_MEMBER_TYPE_(r, "nineteenth", ndm_json_value_is_number);
	CHECK_JSON_MEMBER_TYPE_(r, "twentieth", ndm_json_value_is_number);
	CHECK_JSON_MEMBER_TYPE_(r, "twenty-first", ndm_json_value_is_number);
	CHECK_JSON_MEMBER_TYPE_(r, "twenty-second", ndm_json_value_is_number);
	CHECK_JSON_MEMBER_TYPE_(r, "twenty-third", ndm_json_value_is_number);

	NDM_TEST(ndm_json_object_member_value(
		ndm_json_object_member_first(r)) ==
			ndm_json_object_get(r, "first"));

	NDM_TEST(ndm_json_object_member_value(
		ndm_json_object_member_last(r)) ==
			ndm_json_object_get(r, "twenty-third"));

	ndm_pool_clear(&pool);

	/* NULL-safe operations */
	NDM_TEST(ndm_json_object_set_null(NULL, "null") == NULL);
	NDM_TEST(ndm_json_object_set_boolean(NULL, "true",  true) == NULL);
	NDM_TEST(ndm_json_object_set_boolean(NULL, "false", false) == NULL);
	NDM_TEST(ndm_json_object_set_string(NULL, "string", NULL) == NULL);
	NDM_TEST(ndm_json_object_set_llong(NULL, "llong", 0) == NULL);
	NDM_TEST(ndm_json_object_set_ullong(NULL, "ullong", 0) == NULL);
	NDM_TEST(ndm_json_object_set_double(NULL, "double", 0.0) == NULL);
	NDM_TEST(ndm_json_object_set_array(NULL, "array") == NULL);
	NDM_TEST(ndm_json_object_set_object(NULL, "object") == NULL);
}

static void test_parser_()
{
	char buf[NDM_TEST_POOL_STATIC_SIZE_];
	struct ndm_pool_t p = NDM_POOL_INITIALIZER(
		buf, sizeof(buf), NDM_TEST_POOL_DYNAMIC_SIZE_);
	struct ndm_json_array_t *a = NULL;
	struct ndm_json_object_t *o = NULL;

#define CHECK_JSON_PARSER_(a, o, pool, text, a_code, o_code)			\
{																		\
	do {																\
		char *a_json = ndm_pool_strdup(&pool, text);					\
		char *o_json = ndm_pool_strdup(&pool, text);					\
																		\
		NDM_TEST_BREAK_IF(a_json == NULL);								\
		NDM_TEST_BREAK_IF(o_json == NULL);								\
																		\
		NDM_TEST( ndm_json_array_parse(&pool, a_json, &a) == a_code);	\
		NDM_TEST(ndm_json_object_parse(&pool, o_json, &o) == o_code);	\
	} while (false);													\
}

	CHECK_JSON_PARSER_(a, o, p, "",
		NDM_JSON_PARSE_ERROR_EMPTY_DOCUMENT,
		NDM_JSON_PARSE_ERROR_EMPTY_DOCUMENT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, "  \t  \n\r  ",
		NDM_JSON_PARSE_ERROR_EMPTY_DOCUMENT,
		NDM_JSON_PARSE_ERROR_EMPTY_DOCUMENT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " true ",
		NDM_JSON_PARSE_ERROR_ARRAY,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [] ",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ \n ] ",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ {}, []",
		NDM_JSON_PARSE_ERROR_CORRUPTED_ARRAY,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " { \"a\":[], \"b\":{}",
		NDM_JSON_PARSE_ERROR_ARRAY,
		NDM_JSON_PARSE_ERROR_CORRUPTED_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [] a ",
		NDM_JSON_PARSE_ERROR_TRAILING_SYMBOLS,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " { \n\n } b   ",
		NDM_JSON_PARSE_ERROR_ARRAY,
		NDM_JSON_PARSE_ERROR_TRAILING_SYMBOLS);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, "1",
		NDM_JSON_PARSE_ERROR_ARRAY,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, "1.0",
		NDM_JSON_PARSE_ERROR_ARRAY,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, "null",
		NDM_JSON_PARSE_ERROR_ARRAY,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, "  [  a  \n ] \n  ",
		NDM_JSON_PARSE_ERROR_UNKNOWN_TYPE,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ nall ] ",
		NDM_JSON_PARSE_ERROR_NULL_EXPECTED,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ fals ] ",
		NDM_JSON_PARSE_ERROR_FALSE_EXPECTED,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ tru- ] ",
		NDM_JSON_PARSE_ERROR_TRUE_EXPECTED,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ \"тест\" ] ",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ \"\x1a\" ] ",
		NDM_JSON_PARSE_ERROR_UNESCAPED_CHAR,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " \n  [ \"test ] ",
		NDM_JSON_PARSE_ERROR_UNTERMINATED_STRING,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ \"test\" ] ",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, "  [ \" t\\u65est\" ] ",
		NDM_JSON_PARSE_ERROR_INVALID_HEX,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ \"  t\\u5est\"  ] ",
		NDM_JSON_PARSE_ERROR_INVALID_HEX,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, "  [ \" t\\u567st\" ] ",
		NDM_JSON_PARSE_ERROR_INVALID_HEX,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, "   [ \"t  \\u567\"   ] ",
		NDM_JSON_PARSE_ERROR_INVALID_HEX,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, "   [ \"t  \\u0567\"   ] ",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, "   [ \"t  \\u567e\"   ] ",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, "[\"\\u0020\\u0020\\u0020\"]",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);
	NDM_TEST_BREAK_IF(ndm_json_array_element_first(a) == NULL);
	NDM_TEST(ndm_json_value_is_string(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))));
	NDM_TEST(strcmp(ndm_json_value_string(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))), "   ") == 0);

	CHECK_JSON_PARSER_(a, o, p, "[\"\\ud832\"]",
		NDM_JSON_PARSE_ERROR_CORRUPTED_SURROGATE_PAIR,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, "[\"\\ud832\\udc10\"]",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, "[\"\\ud832 \\udc10\"]",
		NDM_JSON_PARSE_ERROR_CORRUPTED_SURROGATE_PAIR,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, "[\"\\ud832\\uda10\"]",
		NDM_JSON_PARSE_ERROR_INVALID_SURROGATE_PAIR,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, "[\"\\k\"]",
		NDM_JSON_PARSE_ERROR_INVALID_ESCAPE_CHAR,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ 0 ] ",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);
	NDM_TEST_BREAK_IF(ndm_json_array_element_first(a) == NULL);
	NDM_TEST(ndm_json_value_is_char(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))));
	NDM_TEST(ndm_json_value_char(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))) == 0);

	CHECK_JSON_PARSER_(a, o, p, " [ -0 ] ",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);
	NDM_TEST_BREAK_IF(ndm_json_array_element_first(a) == NULL);
	NDM_TEST(ndm_json_value_is_char(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))));
	NDM_TEST(ndm_json_value_char(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))) == 0);

	CHECK_JSON_PARSER_(a, o, p, " [ -1 ] ",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);
	NDM_TEST_BREAK_IF(ndm_json_array_element_first(a) == NULL);
	NDM_TEST(ndm_json_value_is_char(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))));
	NDM_TEST(ndm_json_value_char(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))) == -1);

	CHECK_JSON_PARSER_(a, o, p, " [ 100 ] ",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);
	NDM_TEST_BREAK_IF(ndm_json_array_element_first(a) == NULL);
	NDM_TEST(ndm_json_value_is_char(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))));
	NDM_TEST(ndm_json_value_char(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))) == 100);

	CHECK_JSON_PARSER_(a, o, p, " [ - ] ",
		NDM_JSON_PARSE_ERROR_NUMBER_EXPECTED,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ + ] ",
		NDM_JSON_PARSE_ERROR_UNKNOWN_TYPE,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ 0. ] ",
		NDM_JSON_PARSE_ERROR_NUMBER_FRAC_EXPECTED,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ 00 ] ",
		NDM_JSON_PARSE_ERROR_NUMBER_LEADING_ZERO,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ 01 ] ",
		NDM_JSON_PARSE_ERROR_NUMBER_LEADING_ZERO,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ 99999999999999999999999999999999 ] ",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p,
		" [ 999999999999999999999999999999999999999999999999999999999" \
		"999999999999999999999999999999999999999999999999999999999999" \
		"999999999999999999999999999999999999999999999999999999999999" \
		"999999999999999999999999999999999999999999999999999999999999" \
		"999999999999999999999999999999999999999999999999999999999999" \
		"999999999999999999999999999999999999999999999999999999999999" \
		"9999999999999999999999999999999999999999999 ] ",
		NDM_JSON_PARSE_ERROR_NUMBER_RANGE,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ 0.1 ] ",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);
	NDM_TEST_BREAK_IF(ndm_json_array_element_first(a) == NULL);
	NDM_TEST(ndm_json_value_is_double(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))));
	NDM_TEST(ndm_json_value_double(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))) == 0.1);

	CHECK_JSON_PARSER_(a, o, p, " [ 123.456 ] ",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);
	NDM_TEST_BREAK_IF(ndm_json_array_element_first(a) == NULL);
	NDM_TEST(ndm_json_value_is_double(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))));
	NDM_TEST(ndm_json_value_double(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))) == 123.456);

	CHECK_JSON_PARSER_(a, o, p, " [ 0.e ] ",
		NDM_JSON_PARSE_ERROR_NUMBER_FRAC_EXPECTED,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ 0.E ] ",
		NDM_JSON_PARSE_ERROR_NUMBER_FRAC_EXPECTED,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ E ] ",
		NDM_JSON_PARSE_ERROR_UNKNOWN_TYPE,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ 0.1e ] ",
		NDM_JSON_PARSE_ERROR_NUMBER_EXP_EXPECTED,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ 0.1e+ ] ",
		NDM_JSON_PARSE_ERROR_NUMBER_EXP_EXPECTED,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ 0.1e- ] ",
		NDM_JSON_PARSE_ERROR_NUMBER_EXP_EXPECTED,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ 0.1e0 ] ",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);
	NDM_TEST_BREAK_IF(ndm_json_array_element_first(a) == NULL);
	NDM_TEST(ndm_json_value_is_double(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))));
	NDM_TEST(ndm_json_value_double(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))) == 0.1e0);

	CHECK_JSON_PARSER_(a, o, p, " [ 0.1e+0 ] ",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);
	NDM_TEST_BREAK_IF(ndm_json_array_element_first(a) == NULL);
	NDM_TEST(ndm_json_value_is_double(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))));
	NDM_TEST(ndm_json_value_double(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))) == 0.1e+0);

	CHECK_JSON_PARSER_(a, o, p, " [ 0.1e-0 ] ",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);
	NDM_TEST_BREAK_IF(ndm_json_array_element_first(a) == NULL);
	NDM_TEST(ndm_json_value_is_double(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))));
	NDM_TEST(ndm_json_value_double(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))) == 0.1e-0);

	CHECK_JSON_PARSER_(a, o, p, " [ 0.1e3 ] ",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);
	NDM_TEST_BREAK_IF(ndm_json_array_element_first(a) == NULL);
	NDM_TEST(ndm_json_value_is_double(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))));
	NDM_TEST(ndm_json_value_double(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))) == 0.1e3);

	CHECK_JSON_PARSER_(a, o, p, " [ 0.1e-3 ] ",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);
	NDM_TEST_BREAK_IF(ndm_json_array_element_first(a) == NULL);
	NDM_TEST(ndm_json_value_is_double(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))));
	NDM_TEST(ndm_json_value_double(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))) == 0.1e-3);

	CHECK_JSON_PARSER_(a, o, p, " [ 0.1e+3 ] ",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);
	NDM_TEST_BREAK_IF(ndm_json_array_element_first(a) == NULL);
	NDM_TEST(ndm_json_value_is_double(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))));
	NDM_TEST(ndm_json_value_double(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))) == 0.1e+3);

	CHECK_JSON_PARSER_(a, o, p, " [ \"test\" ] ",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);
	NDM_TEST_BREAK_IF(ndm_json_array_element_first(a) == NULL);
	NDM_TEST_BREAK_IF(!ndm_json_value_is_string(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))));
	NDM_TEST(strcmp(ndm_json_value_string(
		ndm_json_array_element_value(
			ndm_json_array_element_first(a))), "test") == 0);

	CHECK_JSON_PARSER_(a, o, p, " [ null , ] ",
		NDM_JSON_PARSE_ERROR_CORRUPTED_ARRAY,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " { test  } ",
		NDM_JSON_PARSE_ERROR_ARRAY,
		NDM_JSON_PARSE_ERROR_STRING_EXPECTED);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " { \"test\"  } ",
		NDM_JSON_PARSE_ERROR_ARRAY,
		NDM_JSON_PARSE_ERROR_COLON_EXPECTED);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " { \"test\" :  } ",
		NDM_JSON_PARSE_ERROR_ARRAY,
		NDM_JSON_PARSE_ERROR_CORRUPTED_OBJECT);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	CHECK_JSON_PARSER_(a, o, p, " { \"test\" : 1 } ",
		NDM_JSON_PARSE_ERROR_ARRAY,
		NDM_JSON_PARSE_ERROR_OK);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o == NULL);
	NDM_TEST_BREAK_IF(ndm_json_object_member_first(o) == NULL);
	NDM_TEST_BREAK_IF(!ndm_json_value_is_char(
		ndm_json_object_member_value(
			ndm_json_object_member_first(o))));
	NDM_TEST(ndm_json_value_char(
		ndm_json_object_member_value(
			ndm_json_object_member_first(o))) == 1);
	NDM_TEST(ndm_json_value_char(ndm_json_object_get(o, "test")) == 1);

	CHECK_JSON_PARSER_(a, o, p, " { \"test\" : \"string\" } ",
		NDM_JSON_PARSE_ERROR_ARRAY,
		NDM_JSON_PARSE_ERROR_OK);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o == NULL);
	NDM_TEST_BREAK_IF(ndm_json_object_member_first(o) == NULL);
	NDM_TEST_BREAK_IF(!ndm_json_value_is_string(
		ndm_json_object_member_value(
			ndm_json_object_member_first(o))));
	NDM_TEST(strcmp(ndm_json_value_string(
		ndm_json_object_member_value(
			ndm_json_object_member_first(o))), "string") == 0);
	NDM_TEST(strcmp(ndm_json_value_string(
		ndm_json_object_get(o, "test")), "string") == 0);

	CHECK_JSON_PARSER_(a, o, p, " { \"test\" : 23.34 } ",
		NDM_JSON_PARSE_ERROR_ARRAY,
		NDM_JSON_PARSE_ERROR_OK);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o == NULL);
	NDM_TEST_BREAK_IF(ndm_json_object_member_first(o) == NULL);
	NDM_TEST_BREAK_IF(!ndm_json_value_is_double(
		ndm_json_object_member_value(
			ndm_json_object_member_first(o))));
	NDM_TEST(ndm_json_value_double(
		ndm_json_object_member_value(
			ndm_json_object_member_first(o))) == 23.34);
	NDM_TEST(ndm_json_value_double(
		ndm_json_object_get(o, "test")) == 23.34);

	CHECK_JSON_PARSER_(a, o, p, " { \"test\" : null } ",
		NDM_JSON_PARSE_ERROR_ARRAY,
		NDM_JSON_PARSE_ERROR_OK);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o == NULL);
	NDM_TEST_BREAK_IF(ndm_json_object_member_first(o) == NULL);
	NDM_TEST_BREAK_IF(!ndm_json_value_is_null(
		ndm_json_object_member_value(
			ndm_json_object_member_first(o))));
	NDM_TEST(ndm_json_value_is_null(
		ndm_json_object_get(o, "test")));

	CHECK_JSON_PARSER_(a, o, p, " { \"test\" : true } ",
		NDM_JSON_PARSE_ERROR_ARRAY,
		NDM_JSON_PARSE_ERROR_OK);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o == NULL);
	NDM_TEST_BREAK_IF(ndm_json_object_member_first(o) == NULL);
	NDM_TEST_BREAK_IF(!ndm_json_value_is_boolean(
		ndm_json_object_member_value(
			ndm_json_object_member_first(o))));
	NDM_TEST(ndm_json_value_boolean(
		ndm_json_object_get(o, "test")) == true);

	CHECK_JSON_PARSER_(a, o, p, " { \"test\" : false } ",
		NDM_JSON_PARSE_ERROR_ARRAY,
		NDM_JSON_PARSE_ERROR_OK);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o == NULL);
	NDM_TEST_BREAK_IF(ndm_json_object_member_first(o) == NULL);
	NDM_TEST_BREAK_IF(!ndm_json_value_is_boolean(
		ndm_json_object_member_value(
			ndm_json_object_member_first(o))));
	NDM_TEST(ndm_json_value_boolean(
		ndm_json_object_get(o, "test")) == false);

	CHECK_JSON_PARSER_(a, o, p, " { \"test\" : [ null , true ,false ] } ",
		NDM_JSON_PARSE_ERROR_ARRAY,
		NDM_JSON_PARSE_ERROR_OK);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o == NULL);

	NDM_TEST_BREAK_IF(ndm_json_object_member_first(o) == NULL);
	NDM_TEST_BREAK_IF(!ndm_json_value_is_array(
		ndm_json_object_member_value(
			ndm_json_object_member_first(o))));
	NDM_TEST_BREAK_IF((a = ndm_json_value_array(
		ndm_json_object_get(o, "test"))) == NULL);

	struct ndm_json_array_element_t *e = ndm_json_array_element_first(a);

	NDM_TEST_BREAK_IF(e == NULL);
	NDM_TEST(ndm_json_value_is_null(ndm_json_array_element_value(e)));

	NDM_TEST_BREAK_IF((e = ndm_json_array_element_next(e)) == NULL);
	NDM_TEST_BREAK_IF(
		!ndm_json_value_is_boolean(
			ndm_json_array_element_value(e)));
	NDM_TEST(
		ndm_json_value_boolean(
			ndm_json_array_element_value(e)) == true);

	NDM_TEST_BREAK_IF((e = ndm_json_array_element_next(e)) == NULL);
	NDM_TEST_BREAK_IF(
		!ndm_json_value_is_boolean(
			ndm_json_array_element_value(e)));
	NDM_TEST(
		ndm_json_value_boolean(
			ndm_json_array_element_value(e)) == false);

	NDM_TEST_BREAK_IF((e = ndm_json_array_element_next(e)) != NULL);

	CHECK_JSON_PARSER_(a, o, p, " [ { }, 1, { \"test\" : -5e-3 } ] ",
		NDM_JSON_PARSE_ERROR_OK,
		NDM_JSON_PARSE_ERROR_OBJECT);
	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST_BREAK_IF(o != NULL);

	e = ndm_json_array_element_first(a);

	NDM_TEST_BREAK_IF(e == NULL);
	NDM_TEST_BREAK_IF(!ndm_json_value_is_object(
		ndm_json_array_element_value(e)));
	NDM_TEST_BREAK_IF((o = ndm_json_value_object(
		ndm_json_array_element_value(e))) == NULL);
	NDM_TEST(ndm_json_object_is_empty(o));

	e = ndm_json_array_element_next(e);

	NDM_TEST_BREAK_IF(e == NULL);
	NDM_TEST_BREAK_IF(!ndm_json_value_is_char(
		ndm_json_array_element_value(e)));
	NDM_TEST(ndm_json_value_char(ndm_json_array_element_value(e)) == 1);

	e = ndm_json_array_element_next(e);

	NDM_TEST_BREAK_IF(e == NULL);
	NDM_TEST_BREAK_IF(!ndm_json_value_is_object(
		ndm_json_array_element_value(e)));
	NDM_TEST_BREAK_IF((o = ndm_json_value_object(
		ndm_json_array_element_value(e))) == NULL);
	NDM_TEST(!ndm_json_object_is_empty(o));

	NDM_TEST_BREAK_IF(!ndm_json_value_is_double(
		ndm_json_object_get(o, "test")));

	NDM_TEST(ndm_json_value_double(
		ndm_json_object_get(o, "test")) == -5e-3);

	e = ndm_json_array_element_next(e);

	NDM_TEST_BREAK_IF(e != NULL);

	CHECK_JSON_PARSER_(a, o, p,
		"{\n"
		"  \"array\": [ \"a1\", true, false, { \"obj\": -50} ],\n"
		"  \"null\":null,\n"
		"  \"object\": {\"inner\": \"value\"}\n"
		"}",
		NDM_JSON_PARSE_ERROR_ARRAY,
		NDM_JSON_PARSE_ERROR_OK);
	NDM_TEST_BREAK_IF(a != NULL);
	NDM_TEST_BREAK_IF(o == NULL);

	struct ndm_json_object_member_t *m = ndm_json_object_member_first(o);

	NDM_TEST_BREAK_IF(!ndm_json_value_is_array(
		ndm_json_object_member_value(m)));

	a = ndm_json_value_array(ndm_json_object_member_value(m));

	NDM_TEST_BREAK_IF(a == NULL);
	NDM_TEST(ndm_json_array_size(a) == 4);

	e = ndm_json_array_element_first(a);

	NDM_TEST_BREAK_IF(e == NULL);
	NDM_TEST(ndm_json_value_is_string(ndm_json_array_element_value(e)));
	NDM_TEST(strcmp(ndm_json_value_string(
		ndm_json_array_element_value(e)), "a1") == 0);

	e = ndm_json_array_element_next(e);

	NDM_TEST_BREAK_IF(e == NULL);
	NDM_TEST(ndm_json_value_is_boolean(ndm_json_array_element_value(e)));
	NDM_TEST(ndm_json_value_boolean(
		ndm_json_array_element_value(e)) == true);

	e = ndm_json_array_element_next(e);

	NDM_TEST_BREAK_IF(e == NULL);
	NDM_TEST(ndm_json_value_is_boolean(ndm_json_array_element_value(e)));
	NDM_TEST(ndm_json_value_boolean(
		ndm_json_array_element_value(e)) == false);

	e = ndm_json_array_element_next(e);

	NDM_TEST_BREAK_IF(e == NULL);
	NDM_TEST(ndm_json_value_is_object(ndm_json_array_element_value(e)));

	struct ndm_json_object_t *io =
		ndm_json_value_object(ndm_json_array_element_value(e));

	NDM_TEST_BREAK_IF(ndm_json_object_is_empty(io));

	struct ndm_json_object_member_t *om = ndm_json_object_member_first(io);

	NDM_TEST_BREAK_IF(om == NULL);
	NDM_TEST_BREAK_IF(!ndm_json_value_is_char(
		ndm_json_object_member_value(om)));
	NDM_TEST(ndm_json_value_char(ndm_json_object_member_value(om)) == -50);
	NDM_TEST(strcmp(ndm_json_object_member_name(om), "obj") == 0);

	om = ndm_json_object_member_next(om);

	NDM_TEST_BREAK_IF(om != NULL);

	m = ndm_json_object_member_next(m);

	NDM_TEST_BREAK_IF(m == NULL);
	NDM_TEST_BREAK_IF(!ndm_json_value_is_null(
		ndm_json_object_member_value(m)));
	NDM_TEST(strcmp(ndm_json_object_member_name(m), "null") == 0);

	m = ndm_json_object_member_next(m);

	NDM_TEST_BREAK_IF(m == NULL);
	NDM_TEST_BREAK_IF(!ndm_json_value_is_object(
		ndm_json_object_member_value(m)));
	NDM_TEST(strcmp(ndm_json_object_member_name(m), "object") == 0);

	o = ndm_json_value_object(ndm_json_object_member_value(m));

	m = ndm_json_object_member_next(m);

	NDM_TEST_BREAK_IF(m != NULL);

	NDM_TEST_BREAK_IF(ndm_json_object_is_empty(o));

	om = ndm_json_object_member_first(o);

	NDM_TEST_BREAK_IF(om == NULL);
	NDM_TEST_BREAK_IF(!ndm_json_value_is_string(
		ndm_json_object_member_value(om)));
	NDM_TEST(strcmp(ndm_json_value_string(
		ndm_json_object_member_value(om)), "value") == 0);
	NDM_TEST(strcmp(ndm_json_object_member_name(om), "inner") == 0);

	om = ndm_json_object_member_next(om);

	NDM_TEST_BREAK_IF(om != NULL);

	ndm_pool_clear(&p);
}

static void test_printer_()
{
	char buf[NDM_TEST_POOL_STATIC_SIZE_];
	struct ndm_pool_t p = NDM_POOL_INITIALIZER(
		buf, sizeof(buf), NDM_TEST_POOL_DYNAMIC_SIZE_);
	struct ndm_json_object_t *o = NULL;

	static const char in[] =
		"{\"array\":[\"a1\",true,false,{\"int\":-50,\"float\":123.456}],"
		"\"null\":null,\"object\":{\"inner\":\"value\"}}";

	char *in_compact = ndm_pool_strdup(&p, in);

	NDM_TEST_BREAK_IF(in_compact == NULL);
	NDM_TEST_BREAK_IF(
		ndm_json_object_parse(&p, in_compact, &o) !=
			NDM_JSON_PARSE_ERROR_OK);

	char *out_compact = ndm_json_object_print(
		o, NDM_JSON_PRINT_FLAGS_COMPACT, NULL);

	NDM_TEST_BREAK_IF(out_compact == NULL);
	NDM_TEST(strcmp(out_compact, in) == 0);

	free(out_compact);

	static const char out[] =
		"{\n"
		"  \"array\": [\n"
		"    \"a1\",\n"
		"    true,\n"
		"    false,\n"
		"    {\n"
		"      \"int\": -50,\n"
		"      \"float\": 123.456\n"
		"    }\n"
		"  ],\n"
		"  \"null\": null,\n"
		"  \"object\": {\n"
		"    \"inner\": \"value\"\n"
		"  }\n"
		"}";

	char *out_expanded = ndm_json_object_print(o, 0, NULL);

	NDM_TEST_BREAK_IF(out_expanded == NULL);
	NDM_TEST(strcmp(out_expanded, out) == 0);

	free(out_expanded);

	static const char out_crlf[] =
		"{\r\n"
		"  \"array\": [\r\n"
		"    \"a1\",\r\n"
		"    true,\r\n"
		"    false,\r\n"
		"    {\r\n"
		"      \"int\": -50,\r\n"
		"      \"float\": 123.456\r\n"
		"    }\r\n"
		"  ],\r\n"
		"  \"null\": null,\r\n"
		"  \"object\": {\r\n"
		"    \"inner\": \"value\"\r\n"
		"  }\r\n"
		"}";

	size_t out_size = 0;
	char *out_crlf_expanded =
		ndm_json_object_print(o, NDM_JSON_PRINT_FLAGS_CRLF, &out_size);

	NDM_TEST_BREAK_IF(out_crlf_expanded == NULL);
	NDM_TEST(out_size == strlen(out_crlf_expanded));
	NDM_TEST(strcmp(out_crlf_expanded, out_crlf) == 0);

	free(out_crlf_expanded);

	ndm_pool_clear(&p);
}

int main()
{
	test_array_();
	test_object_();
	test_parser_();
	test_printer_();

	return NDM_TEST_RESULT;
}

