#ifndef __NDM_TLV_TABLE__
#define __NDM_TLV_TABLE__

#include <stdint.h>
#include <stddef.h>

#define NDM_TLV_TABLE_ALIGNMENT						8U

struct ndm_tlv_table_t;

int
ndm_tlv_table_new(
		struct ndm_tlv_table_t **tlv_table,
		const uint64_t magic,
		const size_t img_len_max);

int
ndm_tlv_table_parse_img(
		struct ndm_tlv_table_t **tlv_table,
		const uint64_t magic,
		const size_t img_len_max,
		const void *const img,
		const size_t img_len);

void
ndm_tlv_table_free(
		struct ndm_tlv_table_t **tlv_table);

const void *
ndm_tlv_table_img_ptr(
		const struct ndm_tlv_table_t *const tlv_table);

size_t
ndm_tlv_table_img_len(
		const struct ndm_tlv_table_t *const tlv_table);

int
ndm_tlv_table_set(
		struct ndm_tlv_table_t *tlv_table,
		const uint32_t tag,
		const size_t len,
		const void *const val);

int
ndm_tlv_table_del(
		struct ndm_tlv_table_t *tlv_table,
		const uint32_t tag);

const void *
ndm_tlv_table_get(
		const struct ndm_tlv_table_t *const tlv_table,
		const uint32_t tag,
		size_t *const len);

ptrdiff_t
ndm_tlv_table_img_offs_val(
		const struct ndm_tlv_table_t *const tlv_table,
		const uint32_t tag,
		size_t *const len);

ptrdiff_t
ndm_tlv_table_img_offs_crc(
		const struct ndm_tlv_table_t *const tlv_table,
		size_t *const crc_len);

#endif /* __NDM_TLV_TABLE__ */
