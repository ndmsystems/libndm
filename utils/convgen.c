/**
 * A charset code generator for the libndm.
 **/

#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

#define MAX_MAP_LINE_SIZE_					1024
#define UNICODE_REPLACEMENT_				0xfffd
#define UNICODE_MAX_						0x10ffff

#define ILLEGAL_CODE_16						0xd800
#define ILLEGAL_CODE_32						(UNICODE_MAX_ + 1)
#define ILLEGAL_NOT_SET						UINT32_MAX

#define SBCS_TABLE_NAME_MAX_SIZE_			128

#define RANGE_MERGE_GAP_					128

#define CHARSET_EXT_						".TXT"

#define MAX_(a, b)							((a) > (b) ? (a) : (b))

struct dlist_entry_t
{
	struct dlist_entry_t *next;
	struct dlist_entry_t *prev;
};

#define DLIST_INITIALIZER(name)									\
	{&(name), &(name)}

#define DLIST_HEAD(name)										\
	struct dlist_entry_t name = DLIST_INITIALIZER(name)

static inline void dlist_init(struct dlist_entry_t *entry)
{
	entry->next = entry->prev = entry;
}

static inline void dlist_add__(
		struct dlist_entry_t *new_entry,
		struct dlist_entry_t *prev,
		struct dlist_entry_t *next)
{
	next->prev = new_entry;
	new_entry->next = next;
	new_entry->prev = prev;
	prev->next = new_entry;
}

static inline void dlist_insert_after(
		struct dlist_entry_t *head,
		struct dlist_entry_t *new_entry)
{
	dlist_add__(new_entry, head, head->next);
}

static inline void dlist_insert_before(
		struct dlist_entry_t *head,
		struct dlist_entry_t *new_entry)
{
	dlist_add__(new_entry, head->prev, head);
}

static inline void dlist_remove(
		struct dlist_entry_t *entry)
{
	entry->prev->next = entry->next;
	entry->next->prev = entry->prev;
	dlist_init(entry);
}

static inline int dlist_is_empty(
		const struct dlist_entry_t *head)
{
	return head->next == head;
}

#define dlist_entry(ptr, type, member)							\
	((type *) (((char *) ptr) - ((char *) &((type *) 0)->member)))

#define dlist_foreach_entry(e, type, member, head)				\
	for (e = dlist_entry((head)->next, type, member);			\
		 &e->member != (head);									\
	     e = dlist_entry(e->member.next, type, member))

#define dlist_foreach_entry_safe(e, type, member, head, n)		\
	for (e = dlist_entry((head)->next, type, member),			\
		 n = dlist_entry(e->member.next, type, member);			\
		 &e->member != (head);									\
		 e = n, n = dlist_entry(n->member.next, type, member))

struct map_t_
{
	uint32_t from;
	uint32_t to;
};

enum map_range_type_t_
{
	EQUAL,		/* from == to				*/
	ARRAY		/* needs a map to transform	*/
};

struct map_range_t_
{
	int from_code_width;
	int to_code_width;
	size_t start_map_idx;
	size_t end_map_idx;
	enum map_range_type_t_ type;
	struct dlist_entry_t list;
};

struct map_cluster_t_
{
	uint32_t illegal_code;
	int illegal_code_width;
	struct dlist_entry_t ranges;
	struct dlist_entry_t list;
};

static inline enum map_range_type_t_ map_type_(
		const struct map_t_ *map)
{
	return (map->from == map->to) ? EQUAL : ARRAY;
}

static int map_sort_by_source_(
		const void *l,
		const void *r)
{
	const uint32_t lf = ((struct map_t_ *) l)->from;
	const uint32_t rf = ((struct map_t_ *) r)->from;

	return
		lf == rf ? 0  :
		lf <  rf ? -1 : 1;
}

static inline int map_code_width_(
		const uint32_t value)
{
	if (value <= UINT8_MAX) {
		return 1;
	}

	if (value <= UINT16_MAX) {
		return 2;
	}

	return 4;
}

static int map_range_append_(
		struct dlist_entry_t *ranges,
		struct map_t_ *maps,
		const size_t map_count,
		size_t *i)
{
	size_t idx = *i;

	if (idx >= map_count) {
		return 0;
	}

	uint32_t from_code_max = 0;
	uint32_t to_code_max = 0;
	struct map_range_t_ range = {
		.start_map_idx = idx,
		.end_map_idx = idx,
		.type = map_type_(&maps[idx])
	};

	dlist_init(&range.list);

	do {
		if (from_code_max < maps[idx].from) {
			from_code_max = maps[idx].from;
		}

		if (to_code_max < maps[idx].to) {
			to_code_max = maps[idx].to;
		}

		range.end_map_idx = idx++;
	} while (
		idx < map_count &&
		maps[range.end_map_idx].from + 1 == maps[idx].from &&
		map_type_(&maps[idx]) == range.type);

	range.end_map_idx = idx;

	struct map_range_t_ *r = malloc(sizeof(*r));

	if (r == NULL) {
		return -1;
	}

	*r = range;

	r->from_code_width = map_code_width_(from_code_max);
	r->to_code_width = map_code_width_(to_code_max);

	dlist_insert_before(ranges, &r->list);

	*i = idx;

	return 1;
}

static inline int map_range_is_large_equal_(
		const struct map_range_t_ *range)
{
	return
		range->type == EQUAL &&
		range->end_map_idx - range->start_map_idx >= RANGE_MERGE_GAP_;
}

static uint32_t sbcs_find_illegal_8_(
		const struct map_cluster_t_ *c,
		const struct map_t_ *const maps)
{
	bool in_tables[256];
	size_t i = 0;

	while (i < sizeof(in_tables)/sizeof(in_tables[0])) {
		in_tables[i++] = false;
	}

	const struct map_range_t_ *last_range = dlist_entry(
		c->ranges.prev, struct map_range_t_, list);
	const struct map_range_t_ *first_range = dlist_entry(
		c->ranges.next, struct map_range_t_, list);

	/* check only clusters with at least
	 * two ranges and byte destination code width */

	if (last_range != first_range &&
		first_range->to_code_width == 1)
	{
		const struct map_range_t_ *r = dlist_entry(
			c->ranges.next, struct map_range_t_, list);

		dlist_foreach_entry(r, struct map_range_t_, list, &c->ranges) {
			size_t k = r->start_map_idx;

			while (k < r->end_map_idx) {
				in_tables[maps[k].to] = true;
				++k;
			}
		}
	}

	i = 0;

	while (i < sizeof(in_tables)/sizeof(in_tables[0])) {
		if (!in_tables[i]) {
			return (uint32_t) i;
		}

		++i;
	}

	return ILLEGAL_NOT_SET;
}

static bool sbcs_is_cluster_have_illegal_codes_(
		const struct map_cluster_t_ *c,
		const struct map_t_ *const maps)
{
	const struct map_range_t_ *r = dlist_entry(
		c->ranges.next, struct map_range_t_, list);
	const struct map_range_t_ *last_range = dlist_entry(
		c->ranges.prev, struct map_range_t_, list);

	while (r != last_range) {
		const struct map_range_t_ *n =
			dlist_entry(r->list.next, struct map_range_t_, list);

		if (maps[r->end_map_idx - 1].from + 1 !=
				maps[n->start_map_idx].from)
		{
			/* a range hole found,
			 * this cluster needs an illegal code */
			return true;
		}

		r = n;
	}

	return false;
}

static bool map_range_can_be_merged_(
		const struct map_t_ *const maps,
		const struct map_cluster_t_ *last_cluster,
		const struct map_range_t_ *next_range)
{
	/* no cluster to merge to */
	if (last_cluster == NULL) {
		return false;
	}

	const struct map_range_t_ *last_clustered =
		dlist_entry(last_cluster->ranges.prev, struct map_range_t_, list);

	/* do not merge new clusters with large EQUAL one */
	if (map_range_is_large_equal_(last_clustered)) {
		return false;
	}

	/* can not be merged due to a given code gap */
	if (maps[next_range->start_map_idx].from -
		maps[last_clustered->end_map_idx - 1].from > RANGE_MERGE_GAP_)
	{
		return false;
	}

	/* do not make byte clusters with size > 256 */
	const struct map_range_t_ *r;

	dlist_foreach_entry(
			r,
			struct map_range_t_,
			list,
			&last_cluster->ranges)
	{
		if (r->to_code_width > 2) {
			return true;
		}
	}

	const struct map_range_t_ *first_clustered =
		dlist_entry(last_cluster->ranges.next, struct map_range_t_, list);

	return
		maps[next_range->end_map_idx - 1].from -
		maps[first_clustered->start_map_idx].from <= 0x100;
}

static int map_range_merge_(
		const struct map_t_ *const maps,
		struct dlist_entry_t *clusters,
		struct dlist_entry_t *ranges)
{
	while (!dlist_is_empty(ranges)) {
		struct map_range_t_ *next_range = dlist_entry(
			ranges->next, struct map_range_t_, list);
		struct map_cluster_t_ *last_cluster = dlist_is_empty(clusters) ?
			NULL : dlist_entry(clusters->prev, struct map_cluster_t_, list);
		struct map_range_t_ *last_clustered = (last_cluster == NULL) ?
			NULL : dlist_entry(
				last_cluster->ranges.prev,
				struct map_range_t_, list);

		if ( dlist_is_empty(clusters) ||
			 map_range_is_large_equal_(next_range) ||
			!map_range_can_be_merged_(maps, last_cluster, next_range))
		{
			if ((last_cluster = malloc(sizeof(*last_cluster))) == NULL) {
				return -1;
			}

			last_cluster->illegal_code = ILLEGAL_NOT_SET;
			last_cluster->illegal_code_width = 0;
			dlist_init(&last_cluster->ranges);
			dlist_init(&last_cluster->list);
			dlist_insert_before(clusters, &last_cluster->list);
		}

		dlist_remove(&next_range->list);
		dlist_insert_before(&last_cluster->ranges, &next_range->list);

		if (last_clustered != NULL) {
			/* do not decrease code widths with merged range */

			if (next_range->to_code_width <
					last_clustered->to_code_width)
			{
				next_range->to_code_width =
					last_clustered->to_code_width;
			}

			if (next_range->from_code_width <
					last_clustered->from_code_width)
			{
				next_range->from_code_width =
					last_clustered->from_code_width;
			}
		}
	}

	/* find the maximum code width for a whole cluster */
	struct map_cluster_t_ *c;

	dlist_foreach_entry(c, struct map_cluster_t_, list, clusters) {
		int to_code_width = 0;
		struct map_range_t_ *r;

		dlist_foreach_entry(r, struct map_range_t_, list, &c->ranges) {
			if (to_code_width < r->to_code_width) {
				to_code_width = r->to_code_width;
			}
		}

		dlist_foreach_entry(r, struct map_range_t_, list, &c->ranges) {
			r->to_code_width = to_code_width;
		}

		c->illegal_code_width = to_code_width;
	}

	/* find illegal codes, if exist */
	dlist_foreach_entry(c, struct map_cluster_t_, list, clusters) {
		if (sbcs_is_cluster_have_illegal_codes_(c, maps)) {
			if (c->illegal_code_width == 1) {
				c->illegal_code = sbcs_find_illegal_8_(c, maps);
			} else
			if (c->illegal_code_width == 2) {
				c->illegal_code = ILLEGAL_CODE_16;
			} else
			if (c->illegal_code_width == 4) {
				c->illegal_code = ILLEGAL_CODE_32;
			} else {
				fprintf(stderr,
						"Invalid illegal code width for a cluster.\n");

				return -1;
			}
		} else {
			c->illegal_code = ILLEGAL_NOT_SET;
		}
	}

	return 0;
}

static void map_range_clear_(
		struct dlist_entry_t *ranges)
{
	while (!dlist_is_empty(ranges)) {
		struct map_range_t_ *range = dlist_entry(
			ranges->next, struct map_range_t_, list);

		dlist_remove(&range->list);
		free(range);
	}
}

static void sbcs_code_fprintf_(
		FILE *fp,
		size_t *index,
		const size_t bound_index,
		const int num_width,
		const uint32_t code)
{
	fprintf(fp, "%s0x%0*" PRIx32,
		(*index == 0)				? "\n\t"  :
		(*index % bound_index) == 0	? ",\n\t" : ", ",
		num_width,
		code);
	(*index)++;
}

const char *sbcs_table_name_(
		const char *const charset_name,
		const uint32_t start_code,
		const uint32_t end_code,
		const bool to_uni)
{
	assert (*charset_name != '\0');

	static const char UNI_STR_[] = "UNI";
	static const char ISO_STR_[] = "ISO_";
	static char name_[SBCS_TABLE_NAME_MAX_SIZE_];
	const int ret = snprintf(name_, sizeof(name_),
		"%s%s_%08" PRIX32 "_%08" PRIX32 "_TO_%s%s_",
		to_uni && isdigit(*charset_name) ? ISO_STR_ : "",
		to_uni ? charset_name : UNI_STR_,
		start_code,
		end_code,
		!to_uni && isdigit(*charset_name) ? ISO_STR_ : "",
		to_uni ? UNI_STR_ : charset_name);

	return (ret < 0 || ret >= sizeof(name_)) ? NULL : name_;
}

static int sbcs_generate_tables_(
		FILE *fp,
		const struct dlist_entry_t *clusters,
		const struct map_t_ *const maps,
		const char *const charset_name,
		const bool to_uni)
{
	struct map_cluster_t_ *c;

	/* generate convertion tables */
	dlist_foreach_entry(c, struct map_cluster_t_, list, clusters) {
		const struct map_range_t_ *last_range = dlist_entry(
			c->ranges.prev, struct map_range_t_, list);
		const struct map_range_t_ *first_range = dlist_entry(
			c->ranges.next, struct map_range_t_, list);

		if ( last_range == first_range &&
			(map_range_is_large_equal_(first_range) ||
			 first_range->start_map_idx == first_range->end_map_idx - 1))
		{
			/* cluster should be encoded as a simple @c if statement,
			 * no table needs */

			continue;
		}

		const uint32_t first_from = maps[first_range->start_map_idx].from;
		const uint32_t last_from = maps[last_range->end_map_idx - 1].from;
		const struct map_range_t_ *r = dlist_entry(
			c->ranges.next, struct map_range_t_, list);
		const size_t bound_index = (r->to_code_width < 4) ? 8 : 4;
		const char *table_name = sbcs_table_name_(
			charset_name, first_from, last_from, to_uni);

		if (table_name == NULL) {
			fprintf(stderr, "An input charset name is too long.\n");

			return -1;
		}

		fprintf(fp,
			"\n"
			"static const uint%i_t %s[] = {",
			r->to_code_width * 8,
			table_name);

		size_t code_index = 0;

		dlist_foreach_entry(r, struct map_range_t_, list, &c->ranges) {
			size_t i = r->start_map_idx;

			while (i < r->end_map_idx) {
				sbcs_code_fprintf_(
					fp,
					&code_index,
					bound_index,
					r->to_code_width * 2,
					maps[i].to);
				++i;
			}

			/* fill values between cluster ranges */
			if (r->list.next != &c->ranges) {
				const struct map_range_t_ *next_range = dlist_entry(
					r->list.next, struct map_range_t_, list);
				const size_t high_bound =
					maps[next_range->start_map_idx].from;

				i = maps[r->end_map_idx - 1].from + 1;

				if (i != high_bound) {
					assert (c->illegal_code != ILLEGAL_NOT_SET);
					assert (c->illegal_code_width == r->to_code_width);

					while (i < high_bound) {
						sbcs_code_fprintf_(
							fp,
							&code_index,
							bound_index,
							c->illegal_code_width * 2,
							c->illegal_code);
						++i;
					}
				}
			}
		}

		fprintf(fp,
			"\n};\n");
	}

	return 0;
}

static int sbcs_generate_function_body_(
		FILE *fp,
		const struct dlist_entry_t *clusters,
		const struct map_t_ *const maps,
		const char *const charset_name,
		const char *const user_charset_name,
		const bool to_uni)
{
	struct map_cluster_t_ *c;
	bool first_code = false;
	bool last_code = false;
	const char *in_arg = "b0";
	const char *out_arg = "*cp";
	const int ret_error_code = to_uni ? -1 : 0;

	if (to_uni) {
		fprintf(fp,
			"\n"
			"static inline long conv_%s_to_uni_(\n"
			"		const uint8_t *const in,\n"
			"		const size_t in_bytes,\n"
			"		uint32_t *cp)\n"
			"{\n"
			"	assert (in_bytes > 0);\n",
			user_charset_name);
	} else {
		fprintf(fp,
			"\n"
			"static inline long conv_uni_to_%s_(\n"
			"		uint32_t cp,\n"
			"		uint8_t *out,\n"
			"		const size_t out_bytes)\n"
			"{\n"
			"	if (out_bytes == 0) {\n"
			"		return -1;\n"
			"	}\n",
			user_charset_name);

		in_arg = "cp";
		out_arg = "*out";
	}

	dlist_foreach_entry(c, struct map_cluster_t_, list, clusters) {
		const struct map_range_t_ *last_range = dlist_entry(
			c->ranges.prev, struct map_range_t_, list);
		const struct map_range_t_ *first_range = dlist_entry(
			c->ranges.next, struct map_range_t_, list);
		const uint32_t first_from = maps[first_range->start_map_idx].from;
		const uint32_t last_from = maps[last_range->end_map_idx - 1].from;
		const int cp_hex_width = first_range->from_code_width * 2;
		const char *ident = "";

		first_code = (first_from != 0);
		last_code = (last_from != UCHAR_MAX) || !to_uni;

		if (!first_code && !last_code) {
			/* no range checking needs,
			 * possible for a charset with a single cluster only */

			assert (
				dlist_entry(clusters->next, struct map_cluster_t_, list) ==
				dlist_entry(clusters->prev, struct map_cluster_t_, list));

			if (to_uni) {
				/* use a single cluster without range checking */
				in_arg = "*in";

				fprintf(fp,
					"\n");
			}
		} else {
			if (to_uni && &c->list == clusters->next) {
				/* first cluster */
				fprintf(fp,
					"\n"
					"	const uint8_t b0 = *in;\n");
			}

			ident = "	";

			if (first_from == last_from) {
				fprintf(fp,
					"\n"
					"	if (%s == 0x%0*" PRIx32 ") {\n",
					in_arg,
					cp_hex_width,
					last_from);
			} else
			if (first_code && last_code) {
				fprintf(fp,
					"\n"
					"	if (0x%0*" PRIx32 " <= %s && "
						"%s <= 0x%0*" PRIx32 ") {\n",
					cp_hex_width,
					first_from,
					in_arg,
					in_arg,
					cp_hex_width,
					last_from);
			} else
			if (first_code) {
				fprintf(fp,
					"\n"
					"	if (0x%0*" PRIx32 " <= %s) {\n",
					cp_hex_width,
					first_from,
					in_arg);
			} else {
				assert (last_code);

				fprintf(fp,
					"\n"
					"	if (%s <= 0x%0*" PRIx32 ") {\n",
					in_arg,
					cp_hex_width,
					last_from);
			}
		}

		if ( last_range == first_range &&
			(map_range_is_large_equal_(first_range) ||
			 first_range->start_map_idx == first_range->end_map_idx - 1))
		{
			/* no table required, this is an equal transformation */
			if (map_range_is_large_equal_(first_range)) {
				fprintf(fp,
					"%s	%s = %s%s;\n",
					ident, out_arg, to_uni ? "" : "(uint8_t) ", in_arg);
			} else {
				/* single constant value */
				fprintf(fp,
					"%s	%s = 0x%0*" PRIx32 ";\n",
					ident, out_arg,
					first_range->to_code_width * 2,
					maps[first_range->start_map_idx].to);
			}
		} else {
			/* table transform */
			const char *table_name = sbcs_table_name_(
				charset_name, first_from, last_from, to_uni);

			if (table_name == NULL) {
				fprintf(stderr, "An input charset name is too long.\n");

				return -1;
			}

			if (c->illegal_code != ILLEGAL_NOT_SET) {
				fprintf(fp,
					"%s	const uint%d_t code =\n",
					ident, first_range->to_code_width * 8);
			} else {
				fprintf(fp,
					"%s	%s =\n",
					ident, out_arg);
			}

			fprintf(fp,
				"%s		%s[%s",
				ident, table_name, in_arg);

			if (first_code) {
				fprintf(fp,
					" - 0x%0*" PRIx32,
					cp_hex_width, first_from);
			}

			fprintf(fp,
				"];\n");

			if (c->illegal_code != ILLEGAL_NOT_SET) {
				fprintf(fp,
					"\n"
					"%s	/* illegal code for this range */\n"
					"%s	if (code == 0x%0*" PRIx32 ") {\n"
					"%s		return %d;\n"
					"%s	}\n"
					"\n"
					"%s	%s = code;\n",
					ident,
					ident, c->illegal_code_width * 2, c->illegal_code,
					ident, ret_error_code,
					ident,
					ident, out_arg);
			}
		}

		fprintf(fp,
			"\n"
			"%s	return 1;\n",
			ident);

		if (first_code || last_code) {
			fprintf(fp,
				"	}\n");
		}
	}

	if (first_code || last_code) {
		fprintf(fp,
			"\n"
			"	return %d;\n",
			ret_error_code);
	}

	fprintf(fp,
		"}\n");

	return 0;
}

static int sbcs_generate_function_(
		FILE *fp,
		struct map_t_ *maps,
		const size_t map_count,
		const char *const program,
		const char *const charset_name,
		const char *const user_charset_name,
		const bool to_uni)
{
	DLIST_HEAD(ranges);
	DLIST_HEAD(clusters);
	size_t range_idx = 0;
	int exit_code = EXIT_FAILURE;
	size_t i = 0;
	int res = 0;

	fprintf(stdout,
			"\nBuilding range set (%zu codes); input ranges:\n",
			map_count);

	while ((res = map_range_append_(&ranges, maps, map_count, &i)) > 0) {
		const struct map_range_t_ *r = dlist_entry(
			ranges.prev, struct map_range_t_, list);

		fprintf(stderr,
			"%03zu: "
			"%05zu [%08" PRIx32 "] -- "
			"%05zu [%08" PRIx32 "] %s\n",
			range_idx++,
			r->start_map_idx,
			maps[r->start_map_idx].from,
			r->end_map_idx,
			maps[r->end_map_idx - 1].from,
			r->type == EQUAL ? "EQUAL" : "ARRAY");
	}

	if (res < 0 ||
		map_range_merge_(maps, &clusters, &ranges) < 0)
	{
		fprintf(stderr, "Out of memory.\n");
	} else {
		struct map_cluster_t_ *c;

		fprintf(stdout, "\nOutput ranges:\n");

		range_idx = 0;

		dlist_foreach_entry(c, struct map_cluster_t_, list, &clusters) {
			const struct map_range_t_ *last_range = dlist_entry(
				c->ranges.prev, struct map_range_t_, list);
			const struct map_range_t_ *first_range = dlist_entry(
				c->ranges.next, struct map_range_t_, list);
			const size_t start_idx = first_range->start_map_idx;
			const size_t end_idx =last_range->end_map_idx;
			const enum map_range_type_t_ type =
				( last_range == first_range &&
				 (map_range_is_large_equal_(first_range) ||
				  start_idx == end_idx - 1)) ? EQUAL : ARRAY;

			fprintf(stderr,
				"%03zu: "
				"%05zu [%08" PRIx32 "] -- "
				"%05zu [%08" PRIx32 "] %s\n",
				range_idx++,
				start_idx, maps[start_idx].from,
				end_idx, maps[end_idx - 1].from,
				(type == EQUAL) ? "EQUAL" : "ARRAY");
		}

		if (sbcs_generate_tables_(fp, &clusters,
				maps, charset_name, to_uni) < 0)
		{
			exit_code = EXIT_FAILURE;
		} else {
			exit_code = sbcs_generate_function_body_(
				fp, &clusters, maps, charset_name,
				user_charset_name, to_uni);
		}
	}

	while (!dlist_is_empty(&clusters)) {
		struct map_cluster_t_ *cluster = dlist_entry(
			clusters.next, struct map_cluster_t_, list);

		map_range_clear_(&cluster->ranges);
		dlist_remove(&cluster->list);

		free(cluster);
	}

	map_range_clear_(&ranges);

	return exit_code;
}

static int sbcs_generate_charset_source_(
		FILE *fp,
		struct map_t_ *maps,
		const size_t map_count,
		const char *const file_name,
		const char *const program,
		const char *const charset_name,
		const char *const user_charset_name)
{
	int exit_code = EXIT_FAILURE;

	fprintf(fp,
		"#ifndef __NDM_SRC_CHARSET_%s_H__\n"
		"#define __NDM_SRC_CHARSET_%s_H__\n"
		"\n"
		"/**\n"
		" * Automatically generated by @c %s\n"
		" * from @c %s\n"
		" * Range merge code gap is %i.\n"
		" * See http://www.unicode.org/Public/MAPPINGS/ for\n"
		" * other charsets.\n"
		" **/\n"
		"\n"
		"#include <assert.h>\n"
		"#include <stddef.h>\n"
		"#include <stdint.h>\n",
		charset_name,
		charset_name,
		program, file_name,
		RANGE_MERGE_GAP_);

	/* decode to UTF-32 */
	exit_code = sbcs_generate_function_(
		fp,
		maps,
		map_count,
		program,
		charset_name,
		user_charset_name,
		true);

	if (exit_code == EXIT_SUCCESS) {
		size_t i = 0;

		/* swap @c from and @c to codes */
		for (i = 0; i < map_count; i++) {
			const uint32_t c = maps[i].from;

			maps[i].from = maps[i].to;
			maps[i].to = c;
		}

		qsort(maps, map_count, sizeof(*maps), map_sort_by_source_);

		/* encode from UTF-32 */
		exit_code = sbcs_generate_function_(
			fp,
			maps,
			map_count,
			program,
			charset_name,
			user_charset_name,
			false);

		if (exit_code == EXIT_SUCCESS) {
			fprintf(fp,
				"\n"
				"#endif /* __NDM_SRC_CHARSET_%s_H__ */\n",
				charset_name);

			if (ferror(fp)) {
				fprintf(stderr, "Failed to write to \"%s\".\n", file_name);
				exit_code = EXIT_FAILURE;
			}
		}
	}

	return exit_code;
}

static inline void sbcs_scan_spaces_(
		const char **p)
{
	while (isspace(**p)) {
		(*p)++;
	}
}

static bool sbcs_scan_hex32_(
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

static inline bool sbcs_scan_is_eol_(
		const char *const p)
{
	return p[0] == '\0';
}

static size_t sbcs_scan_map_(
		const char *const line,
		uint32_t *from,
		uint32_t *to)
{
	const char *p = line;

	sbcs_scan_spaces_(&p);

	if (!sbcs_scan_hex32_(&p, from)) {
		return 0;
	}

	sbcs_scan_spaces_(&p);

	if (sbcs_scan_is_eol_(p)) {
		/* only a source code is available */
		return 1;
	}

	if (!sbcs_scan_hex32_(&p, to)) {
		return 0;
	}

	sbcs_scan_spaces_(&p);

	if (!sbcs_scan_is_eol_(p)) {
		/* an end of line expected */
		return 0;
	}

	return 2;
}

static int sbcs_load_charset_(
		FILE *fp,
		const char *const file_name,
		const char *const program,
		const char *const charset_name,
		const char *const user_charset_name)
{
	size_t map_count = 0;
	size_t line_no = 0;
	char line[MAX_MAP_LINE_SIZE_];
	uint32_t from_max = 0;
	uint32_t prev_from = 0;
	int exit_code = EXIT_SUCCESS;

	while (fgets(line, sizeof(line) - 1, fp) != NULL) {
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
			struct map_t_ map;
			const size_t code_count = sbcs_scan_map_(p, &map.from, &map.to);

			if (code_count == 0) {
				fprintf(stderr,
					"\"%s\": unrecognized format at line %zu.\n",
					file_name, line_no);
				exit_code = EXIT_FAILURE;

				break;
			}

			if (code_count == 2 &&
				map.to > UNICODE_MAX_)
			{
				fprintf(stderr,
					"\"%s\": a result codepoint value is out of range "
					"(%08" PRIx32 "), line %zu.\n",
					file_name, map.to, line_no);
				exit_code = EXIT_FAILURE;

				break;
			}

			/* ignore undefined codes */
			if (code_count == 2 &&
				map.to != UNICODE_REPLACEMENT_)
			{
				map_count++;
			}

			if (map.from > UINT8_MAX) {
				fprintf(stderr,
					"\"%s\": the maximum code (%08" PRIx32 ") is too big,\n"
					"only single byte charsets are supported.\n",
					file_name, map.from);
				exit_code = EXIT_FAILURE;

				break;
			}

			if (from_max < map.from) {
				from_max = map.from;
			}

			if (map_count > 1 &&
				prev_from >= map.from)
			{
				fprintf(stderr,
					"\"%s\": invalid format "
					"(codes not unique or not sorted), line %zu.\n",
					file_name, line_no);
				exit_code = EXIT_FAILURE;

				break;
			}

			prev_from = map.from;
		}
	}

	if (exit_code != EXIT_SUCCESS) {
		/* an error already printed */
	} else
	if (!feof(fp)) {
		fprintf(stderr, "Failed to read \"%s\".\n", file_name);
	} else
	if (map_count == 0) {
		fprintf(stderr, "\"%s\": no mapping lines found.\n", file_name);
	} else {
		clearerr(fp);
		rewind(fp);

		const size_t map_size = sizeof(struct map_t_) * map_count;
		struct map_t_ *maps = malloc(map_size);

		if (maps == NULL) {
			fprintf(stderr,
				"Out of memory: unable to allocate: %zu bytes.\n",
				map_size);
		} else {
			fprintf(stderr,
				"\"%s\": the maximum code is 0x%08" PRIx32 ".\n",
				file_name, from_max);

			map_count = 0;

			while (fgets(line, sizeof(line) - 1, fp) != NULL) {
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
					uint32_t from;
					uint32_t to;
					const size_t code_count = sbcs_scan_map_(p, &from, &to);

					assert (code_count != 0);

					if (code_count == 2 &&
						to != UNICODE_REPLACEMENT_)
					{
						assert (to < UNICODE_MAX_);

						maps[map_count].from = from;
						maps[map_count].to = to;
						map_count++;
					}
				}
			}

			if (ferror(fp)) {
				fprintf(stderr, "Failed to read \"%s\".\n", file_name);
			} else {
				const int hname_size =
					snprintf(NULL, 0, "%s.h", user_charset_name) + 1;
				char *hname = NULL;

				if (hname_size <= 1) {
					fprintf(
						stderr,
						"Failed to generate an output file name.\n");
				} else
				if ((hname = malloc((size_t) hname_size)) == NULL) {
					fprintf(stderr, "Out of memory.\n");
				} else {
					if (snprintf(
							hname, (size_t) hname_size,
							"%s.h", user_charset_name) <= 0)
					{
						fprintf(
							stderr,
							"Failed to generate a header file name.\n");
					} else {
						size_t i = 0;

						for (i = 0; i < hname_size - 1; i++) {
							hname[i] = (char) tolower(hname[i]);
						}

						FILE *hfp = fopen(hname, "w");

						if (hfp == NULL) {
							fprintf(stderr,
								"Failed to open \"%s\" for writing.\n",
								hname);
						} else {
							exit_code = sbcs_generate_charset_source_(
								hfp,
								maps,
								map_count,
								file_name,
								program,
								charset_name,
								user_charset_name);
							fclose(hfp);

							if (exit_code != EXIT_SUCCESS) {
								remove(hname);
							}
						}
					}

					free(hname);
				}
			}

			free(maps);
		}
	}

	return exit_code;
}

int main(int argc, char *argv[])
{
	int exit_code = EXIT_FAILURE;
	const char *const name = strrchr(argv[0], '/');
	const char *const program = (name == NULL) ? argv[0] : name + 1;

	if (argc != 2) {
		fprintf(stdout,
			"A charset filename expected:\n"
			"	%s <charset-file>\n",
			program);
	} else {
		const char *file_name = argv[1];
		const char *name_start = strrchr(file_name, '/');

		if (name_start == NULL) {
			name_start = file_name;
		} else {
			name_start++;
		}

		char *ext_start = strstr(name_start, CHARSET_EXT_);

		if (ext_start == NULL) {
			fprintf(stderr,
				"Input filename should have \"%s\" suffix.\n",
				CHARSET_EXT_);
		} else {
			char *charset_name = strndup(
				name_start,
				(size_t) (ext_start - name_start));

			if (charset_name == NULL) {
				fprintf(stderr, "Out of memory.\n");
			} else {
				char *user_charset_name = strdup(charset_name);

				if (user_charset_name == NULL) {
					fprintf(stderr, "Out of memory.\n");
				} else {
					FILE *fp = fopen(file_name, "r");
					size_t i = 0;

					while (
						isalnum(user_charset_name[i]) ||
						user_charset_name[i] == '_' ||
						user_charset_name[i] == '-')
					{
						if (user_charset_name[i] == '-') {
							user_charset_name[i] = '_';
							charset_name[i] = '_';
						} else {
							user_charset_name[i] =
								(char) tolower(user_charset_name[i]);
							charset_name[i] =
								(char) toupper(charset_name[i]);
						}

						++i;
					}

					if (user_charset_name[i] != '\0') {
						fprintf(stderr,
							"A file name has invalid character: '%c'.\n",
							user_charset_name[i]);
					} else
					if (fp == NULL) {
						fprintf(stderr,
							"Failed to open \"%s\": %s.\n",
							file_name, strerror(errno));
					} else {
						exit_code = sbcs_load_charset_(
							fp,
							file_name,
							program,
							charset_name,
							user_charset_name);
						fclose(fp);
					}

					free(user_charset_name);
				}

				free(charset_name);
			}
		}
	}

	fprintf(stdout, "\n");

	return exit_code;
}

