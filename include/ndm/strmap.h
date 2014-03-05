#ifndef __NDM_STRMAP_H__
#define __NDM_STRMAP_H__

#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include "attr.h"
#include "ptrvec.h"

#define NDM_STRMAP_INITIALIZER_DEFAULT								\
	{																\
		.vec_ = NDM_PTRVEC_INITIALIZER,								\
		.flags_ = NDM_STRMAP_FLAGS_DEFAULT							\
	}

#define NDM_STRMAP_INITIALIZER(flags)								\
	{																\
		.vec_ = NDM_PTRVEC_INITIALIZER,								\
		.flags_ = flags												\
	}

enum ndm_strmap_flags_t
{
	NDM_STRMAP_FLAGS_DEFAULT				= 0x00,
	NDM_STRMAP_FLAGS_CASE_INSENSITIVE		= 0x01
};

struct ndm_strmap_t
{
	struct ndm_ptrvec_t vec_;
	enum ndm_strmap_flags_t flags_;
};

static inline void ndm_strmap_init(
		struct ndm_strmap_t *map,
		enum ndm_strmap_flags_t flags)
{
	ndm_ptrvec_init(&map->vec_);
	map->flags_ = flags;
}

static enum ndm_strmap_flags_t ndm_strmap_flags(
		struct ndm_strmap_t *map) NDM_ATTR_WUR;

static inline enum ndm_strmap_flags_t ndm_strmap_flags(
		struct ndm_strmap_t *map)
{
	return map->flags_;
}

static size_t ndm_strmap_size(
		const struct ndm_strmap_t *map) NDM_ATTR_WUR;

static inline size_t ndm_strmap_size(
		const struct ndm_strmap_t *map)
{
	return ndm_ptrvec_size(&map->vec_);
}

static bool ndm_strmap_is_empty(
		const struct ndm_strmap_t *map) NDM_ATTR_WUR;

static inline bool ndm_strmap_is_empty(
		const struct ndm_strmap_t *map)
{
	return ndm_strmap_size(map) == 0;
}

void ndm_strmap_clear(
		struct ndm_strmap_t *map);

bool ndm_strmap_nset(
		struct ndm_strmap_t *map,
		const char *const key,
		const size_t key_size,
		const char *const value,
		const size_t value_size) NDM_ATTR_WUR;

static bool ndm_strmap_set(
		struct ndm_strmap_t *map,
		const char *const key,
		const char *const value) NDM_ATTR_WUR;

static inline bool ndm_strmap_set(
		struct ndm_strmap_t *map,
		const char *const key,
		const char *const value)
{
	return ndm_strmap_nset(map, key, strlen(key), value, strlen(value));
}

const char *ndm_strmap_nget(
		struct ndm_strmap_t *map,
		const char *const key,
		const size_t key_size) NDM_ATTR_WUR;

static const char *ndm_strmap_get(
		struct ndm_strmap_t *map,
		const char *const key) NDM_ATTR_WUR;

static inline const char *ndm_strmap_get(
		struct ndm_strmap_t *map,
		const char *const key)
{
	return ndm_strmap_nget(map, key, strlen(key));
}

const char *ndm_strmap_get_by_index(
		struct ndm_strmap_t *map,
		const size_t index) NDM_ATTR_WUR;

bool ndm_strmap_nremove(
		struct ndm_strmap_t *map,
		const char *const key,
		const size_t key_size);

static inline bool ndm_strmap_remove(
		struct ndm_strmap_t *map,
		const char *const key)
{
	return ndm_strmap_nremove(map, key, strlen(key));
}

void ndm_strmap_remove_by_index(
		struct ndm_strmap_t *map,
		const size_t index);

const char *ndm_strmap_get_key(
		struct ndm_strmap_t *map,
		const size_t index) NDM_ATTR_WUR;

bool ndm_strmap_assign(
		struct ndm_strmap_t *dst,
		const struct ndm_strmap_t *src) NDM_ATTR_WUR;

size_t ndm_strmap_nfind(
		const struct ndm_strmap_t *map,
		const char *const key,
		const size_t key_size) NDM_ATTR_WUR;

static size_t ndm_strmap_find(
		const struct ndm_strmap_t *map,
		const char *const key) NDM_ATTR_WUR;

static inline size_t ndm_strmap_find(
		const struct ndm_strmap_t *map,
		const char *const key)
{
	return ndm_strmap_nfind(map, key, strlen(key));
}

static bool ndm_strmap_nhas(
		struct ndm_strmap_t *map,
		const char *const key,
		const size_t key_size) NDM_ATTR_WUR;

static inline bool ndm_strmap_nhas(
		struct ndm_strmap_t *map,
		const char *const key,
		const size_t key_size)
{
	return
		ndm_strmap_nfind(map, key, key_size) <
		ndm_ptrvec_size(&map->vec_);
}

static bool ndm_strmap_has(
		struct ndm_strmap_t *map,
		const char *const key) NDM_ATTR_WUR;

static inline bool ndm_strmap_has(
		struct ndm_strmap_t *map,
		const char *const key)
{
	return ndm_strmap_nhas(map, key, strlen(key));
}

void ndm_strmap_swap(
		struct ndm_strmap_t *lmap,
		struct ndm_strmap_t *rmap);

#endif /* __NDM_STRMAP_H__ */

