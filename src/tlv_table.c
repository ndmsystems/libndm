#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ndm/int.h>
#include <ndm/attr.h>
#include <ndm/crc32.h>
#include <ndm/endian.h>
#include <ndm/tlv_table.h>

struct ndm_tlv_t
{
	uint32_t tag;				/* in BE format */
	uint32_t len;				/* in BE format */
	uint8_t val[];				/* size of TLV entry data, may be zero,
								 * aligned by NDM_TLV_TABLE_ALIGNMENT */
} NDM_ATTR_PACKED;

struct ndm_tlv_hdr_t
{
	uint64_t magic;				/* in BE format */
	uint32_t crc32;				/* in BE format */
	uint32_t size;				/* in BE format;
								 * total size of all TLVs, may be zero,
								 * aligned by NDM_TLV_TABLE_ALIGNMENT */

	struct ndm_tlv_t tlv[];
} NDM_ATTR_PACKED;

struct ndm_tlv_table_t
{
	size_t img_len_max;
	struct ndm_tlv_hdr_t *hdr;
};

static inline uint32_t
ndm_tlv_size__(
		const uint32_t len)
{
	return (uint32_t)
		(sizeof(struct ndm_tlv_t) +
		 NDM_INT_ALIGN(len, NDM_TLV_TABLE_ALIGNMENT));
}

static inline uint32_t
ndm_tlv_size_(
		const struct ndm_tlv_t *const tlv)
{
	return ndm_tlv_size__(ndm_endian_betoh32(tlv->len));
}

static inline struct ndm_tlv_t *
ndm_tlv_next_(
		const struct ndm_tlv_t *const tlv)
{
	return (struct ndm_tlv_t *) (((uint8_t *) tlv) + ndm_tlv_size_(tlv));
}

static inline ptrdiff_t
ndm_tlv_offs_(
		const struct ndm_tlv_t *const end,
		const struct ndm_tlv_t *const beg)
{
	return ((const uint8_t *) end) - ((const uint8_t *) beg);
}

static inline struct ndm_tlv_t *
ndm_tlv_dup_(
		const struct ndm_tlv_t *const tlv)
{
	const uint32_t size = ndm_tlv_size_(tlv);
	struct ndm_tlv_t *dup = (struct ndm_tlv_t *) malloc(size);

	if (dup == NULL) {
		return NULL;
	}

	memcpy(dup, tlv, size);

	return dup;
}

static inline void
ndm_tlv_free_(struct ndm_tlv_t *tlv)
{
	free(tlv);
}

static inline uint32_t
ndm_tlv_hdr_size_(
		const struct ndm_tlv_hdr_t *const hdr)
{
	return (uint32_t) (sizeof(*hdr) + ndm_endian_betoh32(hdr->size));
}

static inline struct ndm_tlv_t *
ndm_tlv_hdr_beg_(
		const struct ndm_tlv_hdr_t *const hdr)
{
	return (struct ndm_tlv_t *) hdr->tlv;
}

static inline struct ndm_tlv_t *
ndm_tlv_hdr_end_(
		const struct ndm_tlv_hdr_t *const hdr)
{
	return (struct ndm_tlv_t *) (((uint8_t *) hdr) + ndm_tlv_hdr_size_(hdr));
}

static struct ndm_tlv_t *
ndm_tlv_hdr_find_(
		const struct ndm_tlv_hdr_t *const hdr,
		const uint32_t tag)
{
	const struct ndm_tlv_t *tlv = ndm_tlv_hdr_beg_(hdr);
	const struct ndm_tlv_t *const tlv_end = ndm_tlv_hdr_end_(hdr);
	const uint32_t be_tag = ndm_endian_htobe32(tag);

	while (tlv != tlv_end) {
		if (tlv->tag == be_tag) {
			return (struct ndm_tlv_t *) tlv;
		}

		tlv = ndm_tlv_next_(tlv);
	}

	return (struct ndm_tlv_t *) tlv;
}

static inline void
ndm_tlv_hdr_update_(
		struct ndm_tlv_hdr_t *hdr,
		const int32_t size_diff)
{
	struct ndm_crc32_t crc32 = NDM_CRC32_INITIALIZER;

	hdr->size = ndm_endian_htobe32((uint32_t)
		((int32_t) (ndm_endian_betoh32(hdr->size)) + size_diff));

	hdr->crc32 = 0;
	ndm_crc32_update(&crc32, hdr, ndm_tlv_hdr_size_(hdr));
	hdr->crc32 = ndm_endian_htobe32(ndm_crc32_digest(&crc32));
}

static int
ndm_tlv_table_create_(
		struct ndm_tlv_table_t **tlv_table,
		const size_t img_len_max,
		const void *const img,
		const size_t img_len)
{
	struct ndm_tlv_table_t *table = NULL;
	struct ndm_tlv_hdr_t *hdr = NULL;

	if (tlv_table == NULL ||
		img_len < sizeof(*hdr) ||
		img_len > UINT32_MAX ||
		img_len % NDM_TLV_TABLE_ALIGNMENT != 0 ||
		img_len_max < sizeof(*hdr) ||
		img_len_max > UINT32_MAX ||
		img_len_max % NDM_TLV_TABLE_ALIGNMENT != 0)
	{
		errno = EINVAL;
		return -1;
	}

	table = malloc(sizeof(*table));
	hdr = malloc(img_len);

	if (table == NULL ||
		hdr == NULL)
	{
		free(table);
		free(hdr);

		errno = ENOMEM;
		return -1;
	}

	table->img_len_max	= img_len_max;
	table->hdr			= hdr;

	memcpy(hdr, img, img_len);
	ndm_tlv_hdr_update_(hdr, 0);

	*tlv_table = table;

	return 0;
}

int
ndm_tlv_table_new(
		struct ndm_tlv_table_t **tlv_table,
		const uint64_t magic,
		const size_t img_len_max)
{
	struct ndm_tlv_hdr_t hdr = {
		.magic = ndm_endian_htobe64(magic),
		.crc32 = ndm_endian_htobe32(0),
		.size  = ndm_endian_htobe32(0)
	};

	return ndm_tlv_table_create_(tlv_table, img_len_max, &hdr, sizeof(hdr));
}

int
ndm_tlv_table_parse_img(
		struct ndm_tlv_table_t **tlv_table,
		const uint64_t magic,
		const size_t img_len_max,
		const void *const img,
		const size_t img_len)
{
	const struct ndm_tlv_hdr_t *hdr = NULL;
	struct ndm_tlv_hdr_t hdr_copy;
	struct ndm_crc32_t crc32 = NDM_CRC32_INITIALIZER;
	const struct ndm_tlv_t *tlv = NULL;
	const struct ndm_tlv_t *tlv_end = NULL;
	uint32_t size = 0;

	if (img == NULL ||
		((uintptr_t) img) % sizeof(void*) != 0)
	{
		errno = EINVAL;
		return -1;
	}

	if (img_len < sizeof(*hdr)) {
		errno = EILSEQ;
		return -1;
	}

	hdr = (const struct ndm_tlv_hdr_t *) img;
	size = ndm_endian_betoh32(hdr->size);

	if (ndm_tlv_hdr_size_(hdr) > img_len_max) {
		errno = EMSGSIZE;
		return -1;
	}

	if (ndm_endian_betoh64(hdr->magic) != magic ||
		ndm_tlv_hdr_size_(hdr) > img_len ||
		size % NDM_TLV_TABLE_ALIGNMENT != 0)
	{
		errno = EILSEQ;
		return -1;
	}

	memcpy(&hdr_copy, hdr, sizeof(*hdr));

	hdr_copy.crc32 = 0;
	ndm_crc32_update(&crc32, &hdr_copy, sizeof(hdr_copy));
	ndm_crc32_update(&crc32, hdr->tlv, size);

	if (ndm_endian_betoh32(hdr->crc32) != ndm_crc32_digest(&crc32)) {
		errno = EILSEQ;
		return -1;
	}

	tlv = ndm_tlv_hdr_beg_(hdr);
	tlv_end = ndm_tlv_hdr_end_(hdr);

	while (tlv != tlv_end) {
		const struct ndm_tlv_t *next = NULL;

		if (ndm_tlv_offs_(tlv_end, tlv) < sizeof(*tlv)) {
			/* the last entry is too short */
			errno = EBADMSG;
			return -1;
		}

		next = ndm_tlv_next_(tlv);

		if (next > tlv_end) {
			/* the last entry is too large */
			errno = E2BIG;
			return -1;
		}

		tlv = next;
	}

	/* duplicate tag check */
	tlv = ndm_tlv_hdr_beg_(hdr);

	while (tlv != tlv_end) {
		const struct ndm_tlv_t *next = ndm_tlv_next_(tlv);

		while (next != tlv_end) {
			if (tlv->tag == next->tag) {
				errno = EEXIST;
				return -1;
			}

			next = ndm_tlv_next_(next);
		}

		tlv = ndm_tlv_next_(tlv);
	}

	return ndm_tlv_table_create_(
		tlv_table, img_len_max, hdr, ndm_tlv_hdr_size_(hdr));
}

void
ndm_tlv_table_free(
		struct ndm_tlv_table_t **tlv_table)
{
	if ( tlv_table == NULL ||
		*tlv_table == NULL)
	{
		return;
	}

	free((*tlv_table)->hdr);
	free(*tlv_table);

	*tlv_table = NULL;
}

const void *
ndm_tlv_table_img_ptr(
		const struct ndm_tlv_table_t *const tlv_table)
{
	return tlv_table->hdr;
}

size_t
ndm_tlv_table_img_len(
		const struct ndm_tlv_table_t *const tlv_table)
{
	return ndm_tlv_hdr_size_(tlv_table->hdr);
}

int
ndm_tlv_table_set(
		struct ndm_tlv_table_t *tlv_table,
		const uint32_t tag,
		const size_t len,
		const void *const val)
{
	struct ndm_tlv_hdr_t *hdr = tlv_table->hdr;
	const struct ndm_tlv_t *const tlv_end = ndm_tlv_hdr_end_(hdr);
	struct ndm_tlv_t *tlv = ndm_tlv_hdr_find_(hdr, tag);
	int32_t size_diff = (int32_t) ndm_tlv_size__((uint32_t) len);
	size_t img_len = 0;
	size_t tail = len % NDM_TLV_TABLE_ALIGNMENT;

	if (len > INT32_MAX) {
		errno = EINVAL;
		return -1;
	}

	if (tlv != tlv_end) {
		size_diff -= (int32_t) ndm_tlv_size_(tlv);
	}

	img_len = ndm_tlv_hdr_size_(hdr) + (size_t) size_diff;

	if (img_len > tlv_table->img_len_max) {
		errno = EMSGSIZE;
		return -1;
	}

	if (size_diff < 0) {
		/* shrink required; preserve an old value */
		struct ndm_tlv_t *beg = ndm_tlv_hdr_beg_(hdr);
		struct ndm_tlv_hdr_t *new_hdr = NULL;
		struct ndm_tlv_t *tlv_dup = ndm_tlv_dup_(tlv);
		struct ndm_tlv_t *next = ndm_tlv_next_(tlv);
		ptrdiff_t tail_size = ndm_tlv_offs_(tlv_end, next);
		const ptrdiff_t tlv_offs = ndm_tlv_offs_(tlv, beg);

		if (tlv_dup == NULL) {
			errno = ENOMEM;
			return -1;
		}

		memmove(((uint8_t *) next) + size_diff, next, (size_t) tail_size);
		new_hdr = realloc(hdr, img_len);

		if (new_hdr == NULL) {
			/* restore shrinked TLV */
			memmove(next, ((uint8_t *) next) + size_diff, (size_t) tail_size);
			memcpy(tlv, tlv_dup, ndm_tlv_size_(tlv_dup));
			ndm_tlv_free_(tlv_dup);
			errno = ENOMEM;
			return -1;
		}

		ndm_tlv_free_(tlv_dup);
		tlv_table->hdr = new_hdr;
		beg = ndm_tlv_hdr_beg_(tlv_table->hdr);
		tlv = (struct ndm_tlv_t *) (((uint8_t *) beg) + tlv_offs);
	} else
	if (size_diff > 0) {
		struct ndm_tlv_t *beg = ndm_tlv_hdr_beg_(hdr);
		struct ndm_tlv_t *next = (tlv == tlv_end) ? tlv : ndm_tlv_next_(tlv);
		const ptrdiff_t tail_size = ndm_tlv_offs_(tlv_end, next);
		const ptrdiff_t tlv_offs = ndm_tlv_offs_(tlv, beg);
		const ptrdiff_t next_offs = ndm_tlv_offs_(next, beg);
		struct ndm_tlv_hdr_t *new_hdr = realloc(hdr, img_len);

		if (new_hdr == NULL) {
			errno = ENOMEM;
			return -1;
		}

		tlv_table->hdr = new_hdr;
		beg = ndm_tlv_hdr_beg_(tlv_table->hdr);
		tlv = (struct ndm_tlv_t *) (((uint8_t *) beg) + tlv_offs);
		next = (struct ndm_tlv_t *) (((uint8_t *) beg) + next_offs);

		if (tail_size > 0) {
			memmove(((uint8_t *) next) + size_diff, next, (size_t) tail_size);
		} else {
			tlv->tag = ndm_endian_htobe32(tag); /* for new TLVs */
		}
	}

	tlv->len = ndm_endian_htobe32((uint32_t) len); /* update a real length */
	memcpy(tlv->val, val, len);

	if (tail != 0) {
		memset(tlv->val + len, 0, NDM_TLV_TABLE_ALIGNMENT - tail);
	}

	ndm_tlv_hdr_update_(tlv_table->hdr, size_diff);

	return 0;
}

int
ndm_tlv_table_del(
		struct ndm_tlv_table_t *tlv_table,
		const uint32_t tag)
{
	struct ndm_tlv_hdr_t *hdr = tlv_table->hdr;
	const struct ndm_tlv_t *const tlv_end = ndm_tlv_hdr_end_(hdr);
	struct ndm_tlv_t *tlv = ndm_tlv_hdr_find_(hdr, tag);
	struct ndm_tlv_t *tlv_dup = NULL;
	struct ndm_tlv_t *next = NULL;
	struct ndm_tlv_hdr_t *new_hdr = NULL;
	ptrdiff_t offs = 0;
	uint32_t size = 0;

	if (tlv == tlv_end) {
		errno = ENOENT;
		return -1;
	}

	tlv_dup = ndm_tlv_dup_(tlv);

	if (tlv_dup == NULL) {
		errno = ENOMEM;
		return -1;
	}

	size = ndm_tlv_size_(tlv);
	next = ndm_tlv_next_(tlv);
	offs = ndm_tlv_offs_(tlv_end, next);
	memmove(tlv, next, (size_t) offs);

	new_hdr = realloc(hdr, ndm_tlv_hdr_size_(hdr) - size);

	if (new_hdr == NULL) {
		/* restore removed TLV */
		memmove(next, tlv, (size_t) offs);
		memcpy(tlv, tlv_dup, size);
		ndm_tlv_free_(tlv_dup);
		errno = ENOMEM;
		return -1;
	}

	ndm_tlv_free_(tlv_dup);

	tlv_table->hdr = new_hdr;
	ndm_tlv_hdr_update_(tlv_table->hdr, -((int32_t) size));

	return 0;
}

const void *
ndm_tlv_table_get(
		const struct ndm_tlv_table_t *const tlv_table,
		const uint32_t tag,
		size_t *const len)
{
	const struct ndm_tlv_hdr_t *const hdr = tlv_table->hdr;
	const struct ndm_tlv_t *const tlv = ndm_tlv_hdr_find_(hdr, tag);

	if (tlv == ndm_tlv_hdr_end_(hdr)) {
		*len = 0;
		return NULL;
	}

	*len = (size_t) ndm_endian_betoh32(tlv->len);

	return tlv->val;
}

ptrdiff_t
ndm_tlv_table_img_offs_val(
		const struct ndm_tlv_table_t *const tlv_table,
		const uint32_t tag,
		size_t *const len)
{
	const struct ndm_tlv_hdr_t *const hdr = tlv_table->hdr;
	const struct ndm_tlv_t *const tlv = ndm_tlv_hdr_find_(hdr, tag);

	if (tlv == ndm_tlv_hdr_end_(hdr)) {
		*len = 0;
		return 0; /* wrong offset */
	}

	*len = (size_t) ndm_endian_betoh32(tlv->len);

	return ((const uint8_t *) tlv->val) - ((const uint8_t *) hdr);
}

ptrdiff_t
ndm_tlv_table_img_offs_crc(
		const struct ndm_tlv_table_t *const tlv_table,
		size_t *const crc_len)
{
	*crc_len = sizeof(tlv_table->hdr->crc32);

	return offsetof(struct ndm_tlv_hdr_t, crc32);
}
