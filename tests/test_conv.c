#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <ndm/sys.h>
#include <ndm/conv.h>
#include <ndm/macro.h>
#include <ndm/dlist.h>
#include <ndm/stdio.h>
#include <ndm/endian.h>
#include <ndm/string.h>
#include "test.h"

#define NDM_CONV_TEST_(													\
		to, from,														\
		in, in_bytes,													\
		in_stop, out_stop,												\
		out,															\
		err, ret)														\
do {																	\
	ndm_conv_t cd_ = ndm_conv_open(to, from);							\
	size_t out_bytes_ = 2 * out_stop;									\
	char *input_ = malloc(in_bytes);									\
	char *output_ = malloc(out_bytes_);									\
																		\
	NDM_TEST_BREAK_IF(cd_ == (ndm_conv_t) -1);							\
	NDM_TEST_BREAK_IF(input_ == NULL);									\
	NDM_TEST_BREAK_IF(output_ == NULL);									\
																		\
	memcpy(input_, in, in_bytes);										\
																		\
	const char *inp_ = input_;											\
	char *outp_ = output_;												\
	size_t inb_ = in_bytes;												\
	size_t outb_ = out_bytes_;											\
	const size_t ret_ = ndm_conv(cd_, &inp_, &inb_, &outp_, &outb_);	\
																		\
	NDM_TEST(ret_ == ret);												\
	NDM_TEST(ret != (ndm_conv_t) -1 || errno == err);					\
																		\
	NDM_TEST(inp_ == input_ + in_stop);									\
	NDM_TEST(outp_ == output_ + out_stop);								\
	NDM_TEST(in_bytes - inb_ == in_stop);								\
	NDM_TEST(out_bytes_ - outb_ == out_stop);							\
	NDM_TEST(memcmp(output_, out, out_bytes_ - outb_) == 0);			\
																		\
	free(input_);														\
	free(output_);														\
																		\
	NDM_TEST(ndm_conv_close(cd_) == 0);									\
} while (0)

static void test_conv_utf16_(
		uint16_t *in,
		const size_t in_bytes,
		const char *const from,
		uint16_t (*from_host)(const uint16_t x))
{
	for (unsigned long i = 0; i < in_bytes/2; i++) {
		in[i] = from_host(in[i]);
	}

	NDM_CONV_TEST_(
		"ISO-8859-1", from,
		in, in_bytes,
		in_bytes, in_bytes / 2,
		"test UTF-16",
		0, 0);

	ndm_conv_t cd = ndm_conv_open("ISO-8859-1", from);
	const char *ip = (const char *) in;
	size_t ib = in_bytes;
	char out[4];
	char *op = out;
	size_t ob = 2;

	NDM_TEST(ndm_conv(cd, &ip, &ib, &op, &ob) == (ndm_conv_t) -1);
	NDM_TEST(errno == E2BIG);
	NDM_TEST(ip - (const char *) in == 4);
	NDM_TEST(op - out == 2);

	ip = (const char *) in;
	ib = 4;
	op = out;
	ob = 2;

	NDM_TEST(ndm_conv(cd, &ip, &ib, &op, &ob) == 0);

	ip = (const char *) in;
	ib = 4;
	op = out;
	ob = 1;

	NDM_TEST(ndm_conv(cd, &ip, &ib, &op, &ob) == (ndm_conv_t) -1);
	NDM_TEST(errno == E2BIG);
	NDM_TEST(ip - (const char *) in == 2);
	NDM_TEST(op - out == 1);

	ip = (const char *) in;
	ib = 3;
	op = out;
	ob = 1;

	NDM_TEST(ndm_conv(cd, &ip, &ib, &op, &ob) == (ndm_conv_t) -1);
	NDM_TEST(errno == EINVAL);
	NDM_TEST(ip - (const char *) in == 2);
	NDM_TEST(op - out == 1);

	ip = (const char *) in;
	ib = 2;
	op = out;
	ob = 1;

	NDM_TEST(ndm_conv(cd, &ip, &ib, &op, &ob) == 0);

	ndm_conv_close(cd);

	/**
	 * Surrogate pairs:
	 * high word [0xd800--0xdbff],
	 * low word  [0xdc00--0xdfff].
	 **/

	/* Hi word only. */

	in[5] = from_host(0xd800);

	NDM_CONV_TEST_(
		"ISO-8859-1", from,
		in, in_bytes,
		5 * 2, 5,
		"test ",
		EILSEQ, -1);

	/* Low word only. */

	in[5] = from_host(0xdcff);

	NDM_CONV_TEST_(
		"ISO-8859-1", from,
		in, in_bytes,
		5 * 2, 5,
		"test ",
		EILSEQ, -1);

	/* Valid surrogate pair. */

	in[5] = from_host(0xd800);
	in[6] = from_host(0xdcff);

	NDM_CONV_TEST_(
		"ISO-8859-1", from,
		in, 5 * 2 + 1,
		5 * 2, 5,
		"test ",
		EINVAL, -1);

	NDM_CONV_TEST_(
		"UTF-8", from,
		in, in_bytes,
		in_bytes, in_bytes / 2 + 2,
		"test \xf0\x90\x83\xbf""F-16",
		0, 0);

	in[5] = from_host('U');
	in[6] = from_host('T');

	/* Truncated code. */

	NDM_CONV_TEST_(
		"ISO-8859-1", from,
		in, in_bytes - 1,
		in_bytes - 2, in_bytes / 2 - 1,
		"test UTF-16",
		EINVAL, -1);

	NDM_CONV_TEST_(
		"ISO-8859-1", from,
		in, 1,
		0, 0,
		"",
		EINVAL, -1);

	/* Last high surrogate. */

	in[10] = from_host(0xd8ff);

	NDM_CONV_TEST_(
		"ISO-8859-1", from,
		in, in_bytes,
		in_bytes - 2, in_bytes / 2 - 1,
		"test UTF-1",
		EINVAL, -1);

	/* Last high and truncated low surrogates. */

	in[9] = from_host(0xd8ff);
	in[10]= from_host(0xdcff);

	NDM_CONV_TEST_(
		"ISO-8859-1", from,
		in, in_bytes - 1,
		in_bytes - 4, in_bytes / 2 - 2,
		"test UTF-",
		EINVAL, -1);

	/* Several surrogate starts. */

	in[7] = from_host(0xd880);
	in[8] = from_host(0xd880);
	in[9] = from_host(0xd880);
	in[10]= from_host(0xd880);

	NDM_CONV_TEST_(
		"ISO-8859-1", from,
		in, in_bytes,
		7 * 2, 7,
		"test UT",
		EILSEQ, -1);

	/* Several surrogate ends. */

	in[7] = from_host(0xdc80);
	in[8] = from_host(0xdc80);
	in[9] = from_host(0xdc80);
	in[10]= from_host(0xdc80);

	NDM_CONV_TEST_(
		"ISO-8859-1", from,
		in, in_bytes,
		7 * 2, 7,
		"test UT",
		EILSEQ, -1);
}

static void test_conv_utf32_(
		uint32_t *in,
		const size_t in_bytes,
		const char *const from,
		uint32_t (*from_host)(const uint32_t x))
{
	for (unsigned long i = 0; i < in_bytes/4; i++) {
		in[i] = from_host(in[i]);
	}

	NDM_CONV_TEST_(
		"ISO-8859-1", from,
		in, in_bytes,
		in_bytes, in_bytes / 4,
		"test UTF-32",
		0, 0);

	ndm_conv_t cd = ndm_conv_open("ISO-8859-1", from);
	const char *ip = (const char *) in;
	size_t ib = in_bytes;
	char out[8];
	char *op = out;
	size_t ob = 2;

	NDM_TEST(ndm_conv(cd, &ip, &ib, &op, &ob) == (ndm_conv_t) -1);
	NDM_TEST(errno == E2BIG);
	NDM_TEST(ip - (const char *) in == 8);
	NDM_TEST(op - out == 2);

	ip = (const char *) in;
	ib = 8;
	op = out;
	ob = 2;

	NDM_TEST(ndm_conv(cd, &ip, &ib, &op, &ob) == 0);

	ip = (const char *) in;
	ib = 8;
	op = out;
	ob = 1;

	NDM_TEST(ndm_conv(cd, &ip, &ib, &op, &ob) == (ndm_conv_t) -1);
	NDM_TEST(errno == E2BIG);
	NDM_TEST(ip - (const char *) in == 4);
	NDM_TEST(op - out == 1);


	ip = (const char *) in;
	ib = 7;
	op = out;
	ob = 1;

	NDM_TEST(ndm_conv(cd, &ip, &ib, &op, &ob) == (ndm_conv_t) -1);
	NDM_TEST(errno == EINVAL);
	NDM_TEST(ip - (const char *) in == 4);
	NDM_TEST(op - out == 1);

	ip = (const char *) in;
	ib = 6;
	op = out;
	ob = 1;

	NDM_TEST(ndm_conv(cd, &ip, &ib, &op, &ob) == (ndm_conv_t) -1);
	NDM_TEST(errno == EINVAL);
	NDM_TEST(ip - (const char *) in == 4);
	NDM_TEST(op - out == 1);

	ip = (const char *) in;
	ib = 5;
	op = out;
	ob = 1;

	NDM_TEST(ndm_conv(cd, &ip, &ib, &op, &ob) == (ndm_conv_t) -1);
	NDM_TEST(errno == EINVAL);
	NDM_TEST(ip - (const char *) in == 4);
	NDM_TEST(op - out == 1);

	ip = (const char *) in;
	ib = 4;
	op = out;
	ob = 1;

	NDM_TEST(ndm_conv(cd, &ip, &ib, &op, &ob) == 0);

	ndm_conv_close(cd);

	/* Illegal code point. */

	in[5] = from_host(0x200000);

	NDM_CONV_TEST_(
		"ISO-8859-1", from,
		in, in_bytes,
		5 * 4, 5,
		"test ",
		EILSEQ, -1);

	/* Non-mapped code point. */

	in[5] = from_host(0x20000);

	NDM_CONV_TEST_(
		"ISO-8859-1", from,
		in, in_bytes,
		11 * 4, 11,
		"test ?TF-32",
		0, 1);

	in[5] = from_host('U');

	/* Truncated, only 1 byte presented. */

	NDM_CONV_TEST_(
		"ISO-8859-1", from,
		in, in_bytes - 3,
		10 * 4, 10,
		"test UTF-3",
		EINVAL, -1);

	/* Truncated, only 2 bytes presented. */

	NDM_CONV_TEST_(
		"ISO-8859-1", from,
		in, in_bytes - 2,
		10 * 4, 10,
		"test UTF-3",
		EINVAL, -1);

	/* Truncated, only 3 bytes presented. */

	NDM_CONV_TEST_(
		"ISO-8859-1", from,
		in, in_bytes - 3,
		10 * 4, 10,
		"test UTF-3",
		EINVAL, -1);

	/* Several invalid code points (from a surrogate range). */

	in[7] = from_host(0xd800);
	in[8] = from_host(0xdbff);
	in[9] = from_host(0xdc00);
	in[10]= from_host(0xdfff);

	NDM_CONV_TEST_(
		"ISO-8859-1", from,
		in, in_bytes,
		7 * 4, 7,
		"test UT",
		EILSEQ, -1);
}

static void test_conv_utf_range_(
		const char *const to,
		const bool is_le,
		const uint32_t start,
		const uint32_t end)
{
	const char *const from = is_le ? "UTF-32LE" : "UTF-32BE";
	uint32_t (*from_host)(const uint32_t x) =
		is_le ? ndm_endian_htole32 : ndm_endian_htobe32;

	ndm_conv_t dcd = ndm_conv_open(to, from);
	ndm_conv_t icd = ndm_conv_open(from, to);

	NDM_TEST(dcd >= 0);
	NDM_TEST(icd >= 0);

	if (dcd != (ndm_conv_t) -1 && icd != (ndm_conv_t) -1) {
		for (uint32_t i = start; i < end; i++) {
			uint32_t enc = 0;
			uint32_t dst = 0;
			uint32_t src = from_host(i);
			const char *in = (const char *) &src;
			char *out = (char *) &enc;
			size_t in_bytes = sizeof(src);
			size_t out_bytes = sizeof(enc);

			NDM_TEST(ndm_conv(dcd, &in, &in_bytes, &out, &out_bytes) == 0);

			size_t size = sizeof(enc) - out_bytes;

			in = (const char *) &enc;
			out = (char *) &dst;
			in_bytes = sizeof(enc);
			out_bytes = sizeof(dst);

			NDM_TEST(ndm_conv(icd, &in, &size, &out, &out_bytes) == 0);
			NDM_TEST(size == 0);
			NDM_TEST(src == dst);
		}
	}

	NDM_TEST(ndm_conv_close(dcd) == 0);
	NDM_TEST(ndm_conv_close(icd) == 0);
}

#define CHARSET_EXT_						".TXT"
#define CHARSET_MAX_LINE_SIZE_				1024
#define CHARSET_UNICODE_REPLACEMENT_		0xfffd
#define CHARSET_UNICODE_MAX_				0x10ffff

struct charset_map_t_
{
	uint32_t from;
	uint32_t to;
};

struct charset_t_
{
	struct ndm_dlist_entry_t list;
	struct charset_map_t_ maps[256];
	size_t map_count;
	char *file_name;
};

static int charset_map_sort_by_source_(
		const void *l,
		const void *r)
{
	const uint32_t lf = ((struct charset_map_t_ *) l)->from;
	const uint32_t rf = ((struct charset_map_t_ *) r)->from;

	return
		lf == rf ? 0  :
		lf <  rf ? -1 : 1;
}

static uint32_t charset_find_to_(
		const struct charset_map_t_ *maps,
		const uint32_t from,
		const size_t min,
		const size_t max)
{
	ssize_t imin = (ssize_t) min;
	ssize_t imax = (ssize_t) max;

	while (imin <= imax) {
		const ssize_t imid = imin + ((imax - imin) / 2);

		if (maps[imid].from == from) {
			return maps[imid].to;
		}

		if (maps[imid].from < from) {
			imin = imid + 1;
		} else {
			imax = imid - 1;
		}
	}

	return UINT32_MAX;
}

static void charset_do_map_test_(
		struct charset_t_ *cs,
		const char *const charset_name)
{
	fprintf(stdout, "Checking %s...\n", charset_name);

	ndm_conv_t to_uni = ndm_conv_open("utf32", charset_name);
	ndm_conv_t uni_to = ndm_conv_open(charset_name, "utf32");

	if (to_uni == (size_t) -1 ||
		uni_to == (size_t) -1)
	{
		fprintf(stdout,
			"No \"%s\" charset found in the library, skipped.\n",
			charset_name);
	} else {
		uint32_t i = 0;

		/* check all 256 input codes */
		while (i < NDM_ARRAY_SIZE(cs->maps)) {
			uint8_t in_ch = (uint8_t) i;
			const char *in = (const char *) &in_ch;
			size_t in_size = sizeof(in_ch);
			uint32_t out_cp = 0;
			char *out = (char *) &out_cp;
			size_t out_size = sizeof(out_cp);
			const size_t res =
				ndm_conv(to_uni, &in, &in_size, &out, &out_size);
			const uint32_t to = charset_find_to_(
				cs->maps, in_ch, 0, cs->map_count - 1);

			if (res == (size_t) -1) {
				if (errno == EILSEQ) {
					/* input byte does not belong to a charset */
					NDM_TEST(to == UINT32_MAX);
				} else {
					NDM_TEST(false);
				}
			} else {
				NDM_TEST(to != UINT32_MAX);
				NDM_TEST(to == out_cp);
			}

			++i;
		}

		for (i = 0; i < cs->map_count; i++) {
			struct charset_map_t_ *m = &cs->maps[i];
			const uint32_t cp = m->to;

			m->to = m->from;
			m->from = cp;
		}

		qsort(
			cs->maps,
			cs->map_count,
			sizeof(*cs->maps),
			charset_map_sort_by_source_);

		uint32_t cp = 0;

		/* check all Unicode input codes */
		while (cp < CHARSET_UNICODE_MAX_) {
			const char *in = (const char *) &cp;
			size_t in_size = sizeof(cp);
			uint8_t out_ch = 0;
			char *out = (char *) &out_ch;
			size_t out_size = sizeof(out_ch);
			const size_t res =
				ndm_conv(uni_to, &in, &in_size, &out, &out_size);
			const uint32_t to = charset_find_to_(
				cs->maps, cp, 0, cs->map_count - 1);

			if (res == 1) {
				/* an invalid codepoint replaced
				 * by a replacement character */
				NDM_TEST(to == UINT32_MAX);
			} else
			if (res == (size_t) -1) {
				if (errno == EILSEQ) {
					/* input byte does not belong to a charset */
					NDM_TEST(to == UINT32_MAX);
				} else {
					NDM_TEST(false);
				}
			} else {
				NDM_TEST(to != UINT32_MAX);
				NDM_TEST(to == (uint32_t) out_ch);
			}

			++cp;
		}
	}

	ndm_conv_close(to_uni);
	ndm_conv_close(uni_to);
}

static inline void charset_scan_spaces_(
		const char **p)
{
	while (isspace(**p)) {
		(*p)++;
	}
}

static bool charset_scan_hex32_(
		const char **line,
		uint32_t *h32)
{
	char *p = (char *) *line;
	unsigned long long ull;

	errno = 0;
	ull = strtoull(p, &p, 16);

	if (   errno == 0
#if ULLONG_MAX > UINT32_MAX
		&& ull <= UINT32_MAX
#endif
		)
	{
		*h32 = (uint32_t) ull;
		*line = p;

		return true;
	}

	return false;
}

static inline bool charset_scan_is_eol_(
		const char *const p)
{
	return p[0] == '\0';
}

static size_t charset_scan_map_(
		const char *const line,
		uint32_t *from,
		uint32_t *to)
{
	const char *p = line;

	charset_scan_spaces_(&p);

	if (!charset_scan_hex32_(&p, from)) {
		return 0;
	}

	charset_scan_spaces_(&p);

	if (charset_scan_is_eol_(p)) {
		/* only a source code is available */
		return 1;
	}

	if (!charset_scan_hex32_(&p, to)) {
		return 0;
	}

	charset_scan_spaces_(&p);

	if (!charset_scan_is_eol_(p)) {
		/* an end of line expected */
		return 0;
	}

	return 2;
}

static bool charset_do_test_(
		struct charset_t_ *cs)
{
	assert (cs->map_count == 0);

	bool done = false;
	const char *name_start = strrchr(cs->file_name, '/');

	if (name_start == NULL) {
		name_start = cs->file_name;
	} else {
		name_start++;
	}

	char *ext_start = strstr(name_start, CHARSET_EXT_);

	if (ext_start == NULL) {
		fprintf(stderr,
			"Input filename \"%s\" should have \"%s\" suffix.\n",
			cs->file_name, CHARSET_EXT_);
	} else {
		const char MAC_PREFIX_[] = "MAC-";
		const bool mac = (strstr(cs->file_name, "APPLE") != NULL);
		char *charset_name = (char *) malloc(
			(mac ? sizeof(MAC_PREFIX_) - 1 : 0) +
			(size_t) (ext_start - name_start) + 1);

		if (charset_name == NULL) {
			fprintf(stderr, "Out of memory.\n");
		} else {
			*charset_name = '\0';

			if (mac) {
				strcat(charset_name, MAC_PREFIX_);
			}

			strncat(
				charset_name,
				name_start,
				(size_t) (ext_start - name_start));

			FILE *fp = fopen(cs->file_name, "r");

			if (fp == NULL) {
				fprintf(stderr,
					"Failed to open \"%s\": %s.\n",
					cs->file_name, ndm_sys_strerror(errno));
			} else {
				size_t line_no = 0;
				char line[CHARSET_MAX_LINE_SIZE_];
				uint32_t prev_from = 0;

				done = true;

				while (fgets(line, sizeof(line) - 1, fp) != NULL && done) {
					line_no++;
					line[sizeof(line) - 1] = '\0';

					char *const comment = strchr(line, '#');

					if (comment != NULL) {
						*comment = '\0';
					}

					const char *p = line;

					while (isspace(*p)) {
						p++;
					}

					if (*p != '\x00' &&
						*p != '\x1a')	/* Control+Z for broken files */
					{
						struct charset_map_t_ *map =
							&cs->maps[cs->map_count];
						const size_t code_count =
							charset_scan_map_(p, &map->from, &map->to);

						/* ignore undefined codes */
						if (code_count == 2 &&
							map->to != CHARSET_UNICODE_REPLACEMENT_)
						{
							if (cs->map_count == NDM_ARRAY_SIZE(cs->maps)) {
								fprintf(stderr, "Too many code maps.\n");
								done = false;

								break;
							}

							cs->map_count++;
						}

						if (code_count != 1 && code_count != 2) {
							fprintf(stderr,
								"\"%s\": unrecognized format at line %zu.\n",
								cs->file_name, line_no);
							done = false;

							break;
						}

						if (map->from > UINT8_MAX) {
							fprintf(stderr,
								"\"%s\": the maximum code (%08" PRIx32
								") is too big,\n"
								"only single byte charsets are supported.\n",
								cs->file_name, map->from);
							done = false;

							break;
						}

						if (map->to > CHARSET_UNICODE_MAX_) {
							fprintf(stderr,
								"\"%s\": a result codepoint value "
								"is out of range "
								"(%08" PRIx32 ") at line %zu.\n",
								cs->file_name,
								map->to, line_no);
							done = false;

							break;
						}

						if (cs->map_count > 1 &&
							prev_from >= map->from)
						{
							fprintf(stderr,
								"\"%s\": invalid format "
								"(codes not unique or not sorted) "
								"at line %zu.\n",
								cs->file_name,
								line_no);
							done = false;

							break;
						}

						prev_from = map->from;
					}
				}

				if (!feof(fp) || ferror(fp)) {
					fprintf(stderr,
						"Failed to read \"%s\".\n",
						cs->file_name);
				} else {
					charset_do_map_test_(cs, charset_name);
					done = true;
				}

				fclose(fp);
			}

			free(charset_name);
		}
	}

	return done;
}

static void charset_clear_(
		struct ndm_dlist_entry_t *charsets)
{
	while (!ndm_dlist_is_empty(charsets)) {
		struct charset_t_ *cs = ndm_dlist_entry(
			charsets->next, struct charset_t_, list);

		ndm_dlist_remove(&cs->list);
		free(cs->file_name);
		free(cs);
	}
}

static int charset_find_(
		const char *const dir_name,
		struct ndm_dlist_entry_t *charsets)
{
	struct dirent **namelist = NULL;
	int n = scandir(dir_name, &namelist, NULL, alphasort);
	int ret = n;
	int i = 0;

	while (i < n && ret >= 0) {
		const char *name = namelist[i]->d_name;
		char *full_name = NULL;

		if (strcmp(name, ".") == 0 ||
			strcmp(name, "..") == 0)
		{
			++i;

			continue;
		}

		ret = ndm_asprintf(
			&full_name,
			"%s%s%s",
			dir_name,
			dir_name[strlen(dir_name) - 1] == '/' ? "" : "/",
			name);

		if (ret < 0) {
			fprintf(stderr, "Out of memory.\n");
		} else {
			struct stat st;
			struct charset_t_ *cs = NULL;

			if ((ret = stat(full_name, &st)) < 0) {
				fprintf(stderr,
					"Failed to get attributes for \"%s\".\n",
					full_name);
			} else
			if (S_ISDIR(st.st_mode) ||
				S_ISREG(st.st_mode))
			{
				if (S_ISDIR(st.st_mode)) {
					ret = charset_find_(full_name, charsets);
				} else {
					const size_t name_len = strlen(name);

					if (name_len > sizeof(CHARSET_EXT_) - 1 &&
						strcmp(
							name + name_len - (sizeof(CHARSET_EXT_) - 1),
							CHARSET_EXT_) == 0)
					{
						cs = malloc(sizeof(*cs));

						if (cs == NULL) {
							fprintf(stderr, "Out of memory.\n");
						} else {
							ndm_dlist_init(&cs->list);
							cs->map_count = 0;
							cs->file_name = full_name;

							ndm_dlist_insert_after(charsets, &cs->list);
						}
					}
				}
			} else {
				fprintf(stdout,
					"\"%s\" is not a directory nor "
					"a regular file, skipped.\n",
					full_name);
			}

			if (cs == NULL) {
				free(full_name);
			}
		}

		i++;
	}

	if (n >= 0) {
		while (n-- > 0) {
			free(namelist[n]);
		}

		free(namelist);
	}

	if (ret < 0) {
		charset_clear_(charsets);
	}

	return ret;
}

static void charset_test_()
{
	struct charset_t_ *cs = NULL;
	NDM_DLIST_HEAD(charsets);

	if (charset_find_("../contrib/MAPPINGS", &charsets) < 0) {
		NDM_TEST_BREAK_IF(
			charset_find_("./contrib/MAPPINGS", &charsets) < 0);
	}

	ndm_dlist_foreach_entry(cs, struct charset_t_, list, &charsets) {
		NDM_TEST(charset_do_test_(cs));
	}

	charset_clear_(&charsets);
}

int main()
{
	fprintf(stdout, "Checking UTF...\n");

	{
		/* ISO 8859-1 */
		/*			 012345678901234 */
		char in[] = "test ISO-8859-1";

		NDM_CONV_TEST_(
			"UTF-8", "ISO-8859-1",
			in, strlen(in) + 1,
			strlen(in) + 1, strlen(in) + 1,
			"test ISO-8859-1",
			0, 0);

		in[5] = '\x80';

		NDM_CONV_TEST_(
			"utf8", "88591",
			in, strlen(in) + 1,
			strlen(in) + 1, strlen(in) + 2,
			"test \xc2\x80SO-8859-1",
			0, 0);

		char iin[] = "тест";

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			iin, strlen(iin) + 1,
			strlen(iin) + 1, 5,
			"????",
			0, 4);
	}

	{
		/* UTF-8 */
		/*           0123456789 */
		char in[] = "test UTF-8";

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			in, strlen(in) + 1,
			strlen(in) + 1, strlen(in) + 1,
			"test UTF-8",
			0, 0);

		in[4] = '\x80';

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			in, strlen(in) + 1,
			4, 4,
			"test",
			EILSEQ, -1);

		/* Illegal character. */

		in[4] = ' ';
		in[5] = '\xd0';

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			in, strlen(in) + 1,
			5, 5,
			"test ",
			EILSEQ, -1);

		/* 1-byte illegal sequence. */

		in[5] = '\xc1';

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			in, strlen(in) + 1,
			5, 5,
			"test ",
			EILSEQ, -1);

		/* 2-byte non mapped character. */

		in[5] = '\xd0';
		in[6] = '\x90'; /* 2-byte Cyrillic 'A' */

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			in, strlen(in) + 1,
			11, 10,
			"test ?F-8",
			0, 1);

		/* 2-byte illegal sequence. */

		in[5] = '\xd0';
		in[6] = '\xf3';

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			in, strlen(in) + 1,
			5, 5,
			"test ",
			EILSEQ, -1);

		/* 2-byte truncated sequence. */

		in[5] = 'U';
		in[6] = 'T';
		in[7] = 'F';
		in[10] = '\xc2';

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			in, 11,
			10, 10,
			"test UTF-8",
			EINVAL, -1);

		in[10] = '\0';

		/* 3-byte illegal sequence, 2nd error byte. */

		in[5] = '\xe0';
		in[6] = '\x9f';
		in[7] = '\x81';

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			in, strlen(in) + 1,
			5, 5,
			"test ",
			EILSEQ, -1);

		/* 3-byte illegal sequence, 3rd error byte. */

		in[5] = '\xe0';
		in[6] = '\xa0';
		in[7] = '\xe2';

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			in, strlen(in) + 1,
			5, 5,
			"test ",
			EILSEQ, -1);

		/* 3-byte non mapped sequence. */

		in[5] = '\xe0';
		in[6] = '\xa0';
		in[7] = '\xbf';

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			in, strlen(in) + 1,
			strlen(in) + 1, 9,
			"test ?-8",
			0, 1);

		/* 3-byte truncated sequence, only 1 byte presented. */

		in[5] = 'U';
		in[6] = 'T';
		in[7] = 'F';
		in[10]= '\xe1';

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			in, 11,
			10, 10,
			"test UTF-8",
			EINVAL, -1);

		in[10] = '\0';

		/* 3-byte truncated sequence, only 2 bytes presented. */

		in[5] = 'U';
		in[6] = 'T';
		in[7] = 'F';
		in[9] = '\xe1';
		in[10]= '\xbf';

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			in, 11,
			9, 9,
			"test UTF-",
			EINVAL, -1);

		in[9] = '8';
		in[10]= '\0';

		/* 4-byte illegal sequence, 2nd error byte. */

		in[5] = '\xf0';
		in[6] = '\x8f';
		in[7] = 'F';

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			in, strlen(in) + 1,
			5, 5,
			"test ",
			EILSEQ, -1);

		/* 4-byte illegal sequence, 3rd error byte. */

		in[5] = '\xf0';
		in[6] = '\x90';
		in[7] = '\xc0';

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			in, strlen(in) + 1,
			5, 5,
			"test ",
			EILSEQ, -1);

		/* 4-byte illegal sequence, 4th error byte. */

		in[5] = '\xf0';
		in[6] = '\x90';
		in[7] = '\xbf';
		in[8] = '\xc0';

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			in, strlen(in) + 1,
			5, 5,
			"test ",
			EILSEQ, -1);

		/* 4-byte non mapped sequence. */

		in[5] = '\xf0';
		in[6] = '\x90';
		in[7] = '\xbf';
		in[8] = '\x80';

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			in, strlen(in) + 1,
			11, 8,
			"test ?8",
			0, 1);

		/* 4-byte truncated sequence, only 1 byte presented. */

		in[5] = 'U';
		in[6] = 'T';
		in[7] = 'F';
		in[8] = '-';
		in[9] = '8';
		in[10]= '\xf4';

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			in, 11,
			10, 10,
			"test UTF-8",
			EINVAL, -1);

		/* 4-byte truncated sequence, only 2 bytes presented. */

		in[9] = '\xf4';
		in[10]= '\x8f';

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			in, 11,
			9, 9,
			"test UTF-",
			EINVAL, -1);

		/* 4-byte truncated sequence, only 3 bytes presented. */

		in[8] = '\xf4';
		in[9] = '\x8f';
		in[10]= '\xbf';

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			in, 11,
			8, 8,
			"test UTF",
			EINVAL, -1);

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			"\xc1\xc1\xc1\xc1\xc1\xc1", 6,
			0, 0,
			"",
			EILSEQ, -1);

		NDM_CONV_TEST_(
			"ISO-8859-1", "UTF-8",
			"\xc2\xc2\xc2\xc2\xc2\xc2", 6,
			0, 0,
			"",
			EILSEQ, -1);
	}

	{
		/* UTF-16 */
		uint16_t in_le[] =
			/*0    1    2    3    4    5    6    7    8    9   10*/
			{'t', 'e', 's', 't', ' ', 'U', 'T', 'F', '-', '1', '6'};

		test_conv_utf16_(in_le, sizeof(in_le),
			"UTF-16LE", ndm_endian_htole16);

		uint16_t in_be[] =
			//0    1    2    3    4    5    6    7    8    9   10
			{'t', 'e', 's', 't', ' ', 'U', 'T', 'F', '-', '1', '6'};

		test_conv_utf16_(in_be, sizeof(in_be),
			"UTF-16BE", ndm_endian_htobe16);
	}

	{
		/* UTF-32 */
		uint32_t in_le[] =
			/*0    1    2    3    4    5    6    7    8    9   10*/
			{'t', 'e', 's', 't', ' ', 'U', 'T', 'F', '-', '3', '2'};

		test_conv_utf32_(in_le, sizeof(in_le),
			"UTF-32LE", ndm_endian_htole32);

		uint32_t in_be[] =
			/*0    1    2    3    4    5    6    7    8    9   10*/
			{'t', 'e', 's', 't', ' ', 'U', 'T', 'F', '-', '3', '2'};

		test_conv_utf32_(in_be, sizeof(in_be),
			"UTF-32BE", ndm_endian_htobe32);
	}

	test_conv_utf_range_("ISO-8859-1", true, 0x0000, 0x80);
	test_conv_utf_range_("ISO-8859-1", false, 0x0000, 0x80);

	test_conv_utf_range_("UTF-8", true, 0x0000, 0xd800);
	test_conv_utf_range_("UTF-8", true, 0xe000, 0x110000);
	test_conv_utf_range_("UTF-8", false, 0x0000, 0xd800);
	test_conv_utf_range_("UTF-8", false, 0xe000, 0x110000);

	test_conv_utf_range_("UTF-16LE", true, 0x0000, 0xd800);
	test_conv_utf_range_("UTF-16LE", true, 0xe000, 0x110000);
	test_conv_utf_range_("UTF-16LE", false, 0x0000, 0xd800);
	test_conv_utf_range_("UTF-16LE", false, 0xe000, 0x110000);

	test_conv_utf_range_("UTF-16BE", true, 0x0000, 0xd800);
	test_conv_utf_range_("UTF-16BE", true, 0xe000, 0x110000);
	test_conv_utf_range_("UTF-16BE", false, 0x0000, 0xd800);
	test_conv_utf_range_("UTF-16BE", false, 0xe000, 0x110000);

	charset_test_();

	return NDM_TEST_RESULT;
}

