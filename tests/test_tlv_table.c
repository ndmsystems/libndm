#include <stdio.h>

#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <ndm/crc32.h>
#include <ndm/macro.h>
#include <ndm/endian.h>
#include <ndm/tlv_table.h>
#include "test.h"

#define NDM_TLV_TABLE_MAGIC_					UINT64_MAX
#define NDM_TLV_TABLE_IMG_MAX_					0x80

struct ndm_tlv_test_t
{
	uint32_t tag;
	uint32_t len;
	uint8_t val[];
};

struct ndm_tlv_hdr_test_t
{
	uint64_t magic;
	uint32_t crc32;
	uint32_t size;
};

static inline struct ndm_tlv_test_t *
tlv_get_(
		struct ndm_tlv_table_t *tlv_table,
		const ptrdiff_t val_offs)
{
	return ((struct ndm_tlv_test_t *)
		(((uint8_t *) ndm_tlv_table_img_ptr(tlv_table)) +
		 val_offs -
		 sizeof(struct ndm_tlv_test_t)));
}

static inline void
tlv_set_tag_(
		struct ndm_tlv_table_t *tlv_table,
		const ptrdiff_t val_offs,
		const uint32_t tag)
{
	tlv_get_(tlv_table, val_offs)->tag = ndm_endian_htobe32(tag);
}

static inline uint32_t
tlv_hdr_size_(
		const struct ndm_tlv_hdr_test_t *const hdr)
{
	return (uint32_t) (sizeof(*hdr) + ndm_endian_betoh32(hdr->size));
}

static inline void
tlv_upd_crc_(
		struct ndm_tlv_table_t *tlv_table,
		const int32_t size_diff)
{
	struct ndm_tlv_hdr_test_t *hdr =
		(struct ndm_tlv_hdr_test_t *) ndm_tlv_table_img_ptr(tlv_table);
	struct ndm_crc32_t crc32 = NDM_CRC32_INITIALIZER;

	hdr->size = ndm_endian_htobe32((uint32_t)
		((int32_t) (ndm_endian_betoh32(hdr->size)) + size_diff));

	hdr->crc32 = 0;
	ndm_crc32_update(&crc32, hdr, tlv_hdr_size_(hdr));
	hdr->crc32 = ndm_endian_htobe32(ndm_crc32_digest(&crc32));
}

int main()
{
	struct ndm_tlv_table_t *t = NULL;
	struct ndm_tlv_table_t *u = NULL;
	struct ndm_tlv_table_t *v = NULL;
	uint64_t magic = NDM_TLV_TABLE_MAGIC_;
	uint8_t *p = NULL;
	size_t len = 0;
	const char *val = NULL;
	ptrdiff_t offs = 0;

	NDM_TEST(ndm_tlv_table_new(
		&t, NDM_TLV_TABLE_MAGIC_, NDM_TLV_TABLE_IMG_MAX_ + 1) != 0);
	NDM_TEST(errno == EINVAL);

	NDM_TEST(ndm_tlv_table_new(
		&t, NDM_TLV_TABLE_MAGIC_, NDM_TLV_TABLE_IMG_MAX_ + 2) != 0);
	NDM_TEST(errno == EINVAL);

	NDM_TEST(ndm_tlv_table_new(
		&t, NDM_TLV_TABLE_MAGIC_, NDM_TLV_TABLE_IMG_MAX_ + 3) != 0);
	NDM_TEST(errno == EINVAL);

	NDM_TEST(ndm_tlv_table_new(
		&t, NDM_TLV_TABLE_MAGIC_, NDM_TLV_TABLE_IMG_MAX_) == 0);

	NDM_TEST(
		ndm_tlv_table_img_offs_val(t, 0, &len) == 0 &&
		len == 0);

	NDM_TEST(ndm_tlv_table_set(t, 0, INT32_MAX + 1U, "a") != 0);
	NDM_TEST(errno == EINVAL);

	NDM_TEST(ndm_tlv_table_set(t, 0, 2, "a") == 0);
	NDM_TEST(ndm_tlv_table_set(t, 1, 3, "bb") == 0);
	NDM_TEST(ndm_tlv_table_set(t, 2, 4, "ccc") == 0);
	NDM_TEST(ndm_tlv_table_set(t, 3, 5, "dddd") == 0);
	NDM_TEST(ndm_tlv_table_set(t, 4, 6, "eeeee") == 0);
	NDM_TEST(ndm_tlv_table_set(t, 5, 7, "ffffff") == 0);
	NDM_TEST(ndm_tlv_table_set(t, 6, 8, "ggggggg") == 0);
	NDM_TEST(ndm_tlv_table_set(t, 7, 9, "hhhhhhhh") != 0);
	NDM_TEST(errno == EMSGSIZE);

	NDM_TEST(ndm_tlv_table_del(t, 5) == 0);
	NDM_TEST(ndm_tlv_table_del(t, 6) == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 0, &len)) != NULL &&
		len == 2 &&
		strcmp(val, "a") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 1, &len)) != NULL &&
		len == 3 &&
		strcmp(val, "bb") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 2, &len)) != NULL &&
		len == 4 &&
		strcmp(val, "ccc") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 3, &len)) != NULL &&
		len == 5 &&
		strcmp(val, "dddd") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 4, &len)) != NULL &&
		len == 6 &&
		strcmp(val, "eeeee") == 0);

	NDM_TEST(ndm_tlv_table_get(t, 5, &len) == NULL);


	p = malloc(ndm_tlv_table_img_len(t) + 9);

	NDM_TEST_BREAK_IF(p == NULL);
	memcpy(p, ndm_tlv_table_img_ptr(t), ndm_tlv_table_img_len(t));

	NDM_TEST(ndm_tlv_table_parse_img(
		&u, NDM_TLV_TABLE_MAGIC_, NDM_TLV_TABLE_IMG_MAX_,
		p,
		ndm_tlv_table_img_len(t) + 9) == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 0, &len)) != NULL &&
		len == 2 &&
		strcmp(val, "a") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 1, &len)) != NULL &&
		len == 3 &&
		strcmp(val, "bb") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 2, &len)) != NULL &&
		len == 4 &&
		strcmp(val, "ccc") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 3, &len)) != NULL &&
		len == 5 &&
		strcmp(val, "dddd") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 4, &len)) != NULL &&
		len == 6 &&
		strcmp(val, "eeeee") == 0);

	NDM_TEST(ndm_tlv_table_get(u, 5, &len) == NULL);

	ndm_tlv_table_free(&u);
	free(p);

	NDM_TEST(ndm_tlv_table_parse_img(
		&u, NDM_TLV_TABLE_MAGIC_ + 1, NDM_TLV_TABLE_IMG_MAX_,
		ndm_tlv_table_img_ptr(t),
		ndm_tlv_table_img_len(t)) != 0);
	ndm_tlv_table_free(&u);

	NDM_TEST(ndm_tlv_table_parse_img(
		&u, NDM_TLV_TABLE_MAGIC_, NDM_TLV_TABLE_IMG_MAX_,
		&magic,
		sizeof(magic)) != 0);
	NDM_TEST(errno == EILSEQ);
	ndm_tlv_table_free(&u);

	NDM_TEST(ndm_tlv_table_parse_img(
		&u, NDM_TLV_TABLE_MAGIC_, NDM_TLV_TABLE_IMG_MAX_,
		ndm_tlv_table_img_ptr(t),
		ndm_tlv_table_img_len(t)) == 0);

	NDM_TEST(ndm_tlv_table_img_len(t) > 0);
	NDM_TEST(memcmp(
		ndm_tlv_table_img_ptr(t),
		ndm_tlv_table_img_ptr(u),
		ndm_tlv_table_img_len(t)) == 0);

	NDM_TEST(ndm_tlv_table_img_len(u) > 0);
	NDM_TEST(memcmp(
		ndm_tlv_table_img_ptr(t),
		ndm_tlv_table_img_ptr(u),
		ndm_tlv_table_img_len(u)) == 0);

	NDM_TEST(
		ndm_tlv_table_img_offs_val(t, UINT32_MAX, &len) == 0 &&
		len == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 0, &len)) != NULL &&
		len == 2 &&
		strcmp(val, "a") == 0);

	NDM_TEST((offs = ndm_tlv_table_img_offs_val(t, 0, &len)) > 0);
	NDM_TEST(
		len == 2 &&
		memcmp((uint8_t *) ndm_tlv_table_img_ptr(t) + offs, "a", len) == 0);

	tlv_set_tag_(t, offs, 4);
	tlv_upd_crc_(t, 0);

	NDM_TEST(ndm_tlv_table_parse_img(
		&v, NDM_TLV_TABLE_MAGIC_, NDM_TLV_TABLE_IMG_MAX_,
		ndm_tlv_table_img_ptr(t),
		ndm_tlv_table_img_len(t)) != 0);
	NDM_TEST(errno == EEXIST);
	ndm_tlv_table_free(&v);

	tlv_set_tag_(t, offs, 0);
	tlv_upd_crc_(t, 0);

	NDM_TEST(ndm_tlv_table_parse_img(
		&v, NDM_TLV_TABLE_MAGIC_, NDM_TLV_TABLE_IMG_MAX_,
		ndm_tlv_table_img_ptr(t),
		ndm_tlv_table_img_len(t)) == 0);
	ndm_tlv_table_free(&v);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 1, &len)) != NULL &&
		len == 3 &&
		strcmp(val, "bb") == 0);

	NDM_TEST((offs = ndm_tlv_table_img_offs_val(t, 1, &len)) > 0);
	NDM_TEST(
		len == 3 &&
		memcmp((uint8_t *) ndm_tlv_table_img_ptr(t) + offs, "bb", len) == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 2, &len)) != NULL &&
		len == 4 &&
		strcmp(val, "ccc") == 0);

	NDM_TEST((offs = ndm_tlv_table_img_offs_val(t, 2, &len)) > 0);
	NDM_TEST(
		len == 4 &&
		memcmp((uint8_t *) ndm_tlv_table_img_ptr(t) + offs, "ccc", len) == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 3, &len)) != NULL &&
		len == 5 &&
		strcmp(val, "dddd") == 0);

	NDM_TEST((offs = ndm_tlv_table_img_offs_val(t, 3, &len)) > 0);
	NDM_TEST(
		len == 5 &&
		memcmp(
			(uint8_t *) ndm_tlv_table_img_ptr(t) + offs, "dddd", len) == 0);

	tlv_set_tag_(t, offs, 4);
	tlv_upd_crc_(t, 0);

	NDM_TEST(ndm_tlv_table_parse_img(
		&v, NDM_TLV_TABLE_MAGIC_, NDM_TLV_TABLE_IMG_MAX_,
		ndm_tlv_table_img_ptr(t),
		ndm_tlv_table_img_len(t)) != 0);
	NDM_TEST(errno == EEXIST);
	ndm_tlv_table_free(&v);

	tlv_set_tag_(t, offs, 3);
	tlv_upd_crc_(t, 0);

	NDM_TEST(ndm_tlv_table_parse_img(
		&v, NDM_TLV_TABLE_MAGIC_, NDM_TLV_TABLE_IMG_MAX_,
		ndm_tlv_table_img_ptr(t),
		ndm_tlv_table_img_len(t)) == 0);
	ndm_tlv_table_free(&v);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 4, &len)) != NULL &&
		len == 6 &&
		strcmp(val, "eeeee") == 0);

	NDM_TEST((offs = ndm_tlv_table_img_offs_val(t, 4, &len)) > 0);
	NDM_TEST(
		len == 6 &&
		memcmp(
			(uint8_t *) ndm_tlv_table_img_ptr(t) + offs, "eeeee", len) == 0);

	tlv_upd_crc_(t, -((int32_t) NDM_TLV_TABLE_ALIGNMENT));

	NDM_TEST(ndm_tlv_table_parse_img(
		&v, NDM_TLV_TABLE_MAGIC_, NDM_TLV_TABLE_IMG_MAX_,
		ndm_tlv_table_img_ptr(t),
		ndm_tlv_table_img_len(t)) != 0);
	NDM_TEST(errno == E2BIG);
	ndm_tlv_table_free(&v);

	tlv_upd_crc_(t, NDM_TLV_TABLE_ALIGNMENT);

	NDM_TEST(ndm_tlv_table_parse_img(
		&v, NDM_TLV_TABLE_MAGIC_, NDM_TLV_TABLE_IMG_MAX_,
		ndm_tlv_table_img_ptr(t),
		ndm_tlv_table_img_len(t)) == 0);
	ndm_tlv_table_free(&v);

	NDM_TEST(ndm_tlv_table_parse_img(
		&v, NDM_TLV_TABLE_MAGIC_, NDM_TLV_TABLE_IMG_MAX_,
		ndm_tlv_table_img_ptr(t),
		ndm_tlv_table_img_len(t)) == 0);
	ndm_tlv_table_free(&v);

	NDM_TEST(ndm_tlv_table_set(u, 3, 2, "d") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 0, &len)) != NULL &&
		len == 2 &&
		strcmp(val, "a") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 1, &len)) != NULL &&
		len == 3 &&
		strcmp(val, "bb") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 2, &len)) != NULL &&
		len == 4 &&
		strcmp(val, "ccc") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 3, &len)) != NULL &&
		len == 2 &&
		strcmp(val, "d") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 4, &len)) != NULL &&
		len == 6 &&
		strcmp(val, "eeeee") == 0);

	NDM_TEST(ndm_tlv_table_parse_img(
		&v, NDM_TLV_TABLE_MAGIC_, NDM_TLV_TABLE_IMG_MAX_ / 2,
		ndm_tlv_table_img_ptr(t),
		ndm_tlv_table_img_len(t)) != 0);
	NDM_TEST(errno == EMSGSIZE);

	NDM_TEST(ndm_tlv_table_set(u, 3, 5, "dddd") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 0, &len)) != NULL &&
		len == 2 &&
		strcmp(val, "a") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 1, &len)) != NULL &&
		len == 3 &&
		strcmp(val, "bb") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 2, &len)) != NULL &&
		len == 4 &&
		strcmp(val, "ccc") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 3, &len)) != NULL &&
		len == 5 &&
		strcmp(val, "dddd") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 4, &len)) != NULL &&
		len == 6 &&
		strcmp(val, "eeeee") == 0);

	NDM_TEST(ndm_tlv_table_get(u, 5, &len) == NULL);

	p = malloc(ndm_tlv_table_img_len(t));

	NDM_TEST_BREAK_IF(p == NULL);
	NDM_TEST(ndm_tlv_table_img_len(t) > 0);
	memcpy(p, ndm_tlv_table_img_ptr(t), ndm_tlv_table_img_len(t));

	NDM_TEST((offs = ndm_tlv_table_img_offs_val(t, 4, &len)) > 0);
	NDM_TEST_BREAK_IF(len == 0);

	NDM_TEST(ndm_tlv_table_parse_img(
		&v, NDM_TLV_TABLE_MAGIC_, NDM_TLV_TABLE_IMG_MAX_,
		ndm_tlv_table_img_ptr(t),
		ndm_tlv_table_img_len(t)) == 0);
	ndm_tlv_table_free(&v);

	(*(p + offs + len - 1))++;

	NDM_TEST(ndm_tlv_table_parse_img(
		&v, NDM_TLV_TABLE_MAGIC_, NDM_TLV_TABLE_IMG_MAX_,
		p, ndm_tlv_table_img_len(t)) != 0);
	NDM_TEST(errno == EILSEQ);
	ndm_tlv_table_free(&v);

	(*(p + offs + len - 1))--;

	NDM_TEST(ndm_tlv_table_parse_img(
		&v, NDM_TLV_TABLE_MAGIC_, NDM_TLV_TABLE_IMG_MAX_,
		ndm_tlv_table_img_ptr(t),
		ndm_tlv_table_img_len(t)) == 0);
	ndm_tlv_table_free(&v);

	offs = ndm_tlv_table_img_offs_crc(t, &len);
	NDM_TEST_BREAK_IF(len == 0);
	(*(p + offs + len - 1))++;

	NDM_TEST(ndm_tlv_table_parse_img(
		&v, NDM_TLV_TABLE_MAGIC_, NDM_TLV_TABLE_IMG_MAX_,
		p, ndm_tlv_table_img_len(t)) != 0);
	NDM_TEST(errno == EILSEQ);
	ndm_tlv_table_free(&v);

	(*(p + offs + len - 1))--;

	NDM_TEST(ndm_tlv_table_parse_img(
		&v, NDM_TLV_TABLE_MAGIC_, NDM_TLV_TABLE_IMG_MAX_,
		ndm_tlv_table_img_ptr(t),
		ndm_tlv_table_img_len(t)) == 0);
	ndm_tlv_table_free(&v);

	free(p);

	NDM_TEST(ndm_tlv_table_del(u, 3) == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 0, &len)) != NULL &&
		len == 2 &&
		strcmp(val, "a") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 1, &len)) != NULL &&
		len == 3 &&
		strcmp(val, "bb") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 2, &len)) != NULL &&
		len == 4 &&
		strcmp(val, "ccc") == 0);

	NDM_TEST(ndm_tlv_table_get(u, 3, &len) == NULL);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 4, &len)) != NULL &&
		len == 6 &&
		strcmp(val, "eeeee") == 0);

	NDM_TEST(ndm_tlv_table_get(u, 5, &len) == NULL);

	NDM_TEST(ndm_tlv_table_del(u, 4) == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 0, &len)) != NULL &&
		len == 2 &&
		strcmp(val, "a") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 1, &len)) != NULL &&
		len == 3 &&
		strcmp(val, "bb") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 2, &len)) != NULL &&
		len == 4 &&
		strcmp(val, "ccc") == 0);

	NDM_TEST(ndm_tlv_table_get(u, 3, &len) == NULL);

	NDM_TEST(ndm_tlv_table_get(u, 4, &len) == NULL);

	NDM_TEST(ndm_tlv_table_get(u, 5, &len) == NULL);

	NDM_TEST(ndm_tlv_table_del(u, 0) == 0);

	NDM_TEST(ndm_tlv_table_get(u, 0, &len) == NULL);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 1, &len)) != NULL &&
		len == 3 &&
		strcmp(val, "bb") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 2, &len)) != NULL &&
		len == 4 &&
		strcmp(val, "ccc") == 0);

	NDM_TEST(ndm_tlv_table_get(u, 3, &len) == NULL);

	NDM_TEST(ndm_tlv_table_get(u, 4, &len) == NULL);

	NDM_TEST(ndm_tlv_table_get(u, 5, &len) == NULL);

	NDM_TEST(ndm_tlv_table_del(u, 0) != 0);
	NDM_TEST(errno == ENOENT);

	NDM_TEST(ndm_tlv_table_del(u, 1) == 0);

	NDM_TEST(ndm_tlv_table_get(u, 0, &len) == NULL);

	NDM_TEST(ndm_tlv_table_get(u, 1, &len) == NULL);

	NDM_TEST(
		(val = ndm_tlv_table_get(u, 2, &len)) != NULL &&
		len == 4 &&
		strcmp(val, "ccc") == 0);

	NDM_TEST(ndm_tlv_table_get(u, 3, &len) == NULL);

	NDM_TEST(ndm_tlv_table_get(u, 4, &len) == NULL);

	NDM_TEST(ndm_tlv_table_get(u, 5, &len) == NULL);

	NDM_TEST(ndm_tlv_table_del(u, 1) != 0);
	NDM_TEST(errno == ENOENT);

	NDM_TEST(ndm_tlv_table_del(u, 2) == 0);

	NDM_TEST(ndm_tlv_table_get(u, 0, &len) == NULL);

	NDM_TEST(ndm_tlv_table_get(u, 1, &len) == NULL);

	NDM_TEST(ndm_tlv_table_get(u, 2, &len) == NULL);

	NDM_TEST(ndm_tlv_table_get(u, 3, &len) == NULL);

	NDM_TEST(ndm_tlv_table_get(u, 4, &len) == NULL);

	NDM_TEST(ndm_tlv_table_get(u, 5, &len) == NULL);

	ndm_tlv_table_free(&t);

	NDM_TEST(ndm_tlv_table_new(
		&t, NDM_TLV_TABLE_MAGIC_, NDM_TLV_TABLE_IMG_MAX_) == 0);

	NDM_TEST(ndm_tlv_table_img_len(t) > 0);
	NDM_TEST(memcmp(
		ndm_tlv_table_img_ptr(t),
		ndm_tlv_table_img_ptr(u),
		ndm_tlv_table_img_len(t)) == 0);

	NDM_TEST(ndm_tlv_table_img_len(u) > 0);
	NDM_TEST(memcmp(
		ndm_tlv_table_img_ptr(t),
		ndm_tlv_table_img_ptr(u),
		ndm_tlv_table_img_len(u)) == 0);

	ndm_tlv_table_free(&u);

	ndm_tlv_table_free(&t);
	NDM_TEST(t == NULL);

	NDM_TEST(ndm_tlv_table_new(
		&t, NDM_TLV_TABLE_MAGIC_, 4 * NDM_TLV_TABLE_IMG_MAX_) == 0);

	NDM_TEST(ndm_tlv_table_set(t, 0, 9,  "99999999") == 0);
	NDM_TEST(ndm_tlv_table_set(t, 1, 10, "aaaaaaaaa") == 0);
	NDM_TEST(ndm_tlv_table_set(t, 2, 11, "bbbbbbbbbb") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 0, &len)) != NULL &&
		len == 9 &&
		strcmp(val, "99999999") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 1, &len)) != NULL &&
		len == 10 &&
		strcmp(val, "aaaaaaaaa") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 2, &len)) != NULL &&
		len == 11 &&
		strcmp(val, "bbbbbbbbbb") == 0);

	NDM_TEST(ndm_tlv_table_set(t, 1, 17, "aaaaaaaaaaaaaaaa") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 0, &len)) != NULL &&
		len == 9 &&
		strcmp(val, "99999999") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 1, &len)) != NULL &&
		len == 17 &&
		strcmp(val, "aaaaaaaaaaaaaaaa") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 2, &len)) != NULL &&
		len == 11 &&
		strcmp(val, "bbbbbbbbbb") == 0);

	NDM_TEST(ndm_tlv_table_set(t, 1, 2, "a") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 0, &len)) != NULL &&
		len == 9 &&
		strcmp(val, "99999999") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 1, &len)) != NULL &&
		len == 2 &&
		strcmp(val, "a") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 2, &len)) != NULL &&
		len == 11 &&
		strcmp(val, "bbbbbbbbbb") == 0);

	NDM_TEST(ndm_tlv_table_set(t, 2, 2, "b") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 0, &len)) != NULL &&
		len == 9 &&
		strcmp(val, "99999999") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 1, &len)) != NULL &&
		len == 2 &&
		strcmp(val, "a") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 2, &len)) != NULL &&
		len == 2 &&
		strcmp(val, "b") == 0);

	NDM_TEST(ndm_tlv_table_set(t, 2, 11, "bbbbbbbbbb") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 0, &len)) != NULL &&
		len == 9 &&
		strcmp(val, "99999999") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 1, &len)) != NULL &&
		len == 2 &&
		strcmp(val, "a") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 2, &len)) != NULL &&
		len == 11 &&
		strcmp(val, "bbbbbbbbbb") == 0);

	NDM_TEST(ndm_tlv_table_set(t, 2, 11, "eeeeeeeeee") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 0, &len)) != NULL &&
		len == 9 &&
		strcmp(val, "99999999") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 1, &len)) != NULL &&
		len == 2 &&
		strcmp(val, "a") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 2, &len)) != NULL &&
		len == 11 &&
		strcmp(val, "eeeeeeeeee") == 0);

	NDM_TEST(ndm_tlv_table_set(t, 1, 2, "0") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 0, &len)) != NULL &&
		len == 9 &&
		strcmp(val, "99999999") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 1, &len)) != NULL &&
		len == 2 &&
		strcmp(val, "0") == 0);

	NDM_TEST(
		(val = ndm_tlv_table_get(t, 2, &len)) != NULL &&
		len == 11 &&
		strcmp(val, "eeeeeeeeee") == 0);

	ndm_tlv_table_free(&t);

	return NDM_TEST_RESULT;
}
