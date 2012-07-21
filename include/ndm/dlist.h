#ifndef __NDM_DLIST_H__
#define __NDM_DLIST_H__

#include <stdbool.h>

struct ndm_dlist_entry_t
{
	struct ndm_dlist_entry_t *next;
	struct ndm_dlist_entry_t *prev;
};

#define NDM_DLIST_INITIALIZER(name) 								\
	{&(name), &(name)}

#define NDM_DLIST_HEAD(name) 										\
	struct ndm_dlist_entry_t name = NDM_DLIST_INITIALIZER(name)

static inline void ndm_dlist_init(struct ndm_dlist_entry_t *entry)
{
	entry->next = entry->prev = entry;
}

static inline void __ndm_dlist_add(
		struct ndm_dlist_entry_t *new_entry,
		struct ndm_dlist_entry_t *prev,
		struct ndm_dlist_entry_t *next)
{
	next->prev = new_entry;
	new_entry->next = next;
	new_entry->prev = prev;
	prev->next = new_entry;
}

static inline void ndm_dlist_insert_after(
		struct ndm_dlist_entry_t *head,
		struct ndm_dlist_entry_t *new_entry)
{
	__ndm_dlist_add(new_entry, head, head->next);
}

static inline void ndm_dlist_insert_before(
		struct ndm_dlist_entry_t *head,
		struct ndm_dlist_entry_t *new_entry)
{
	__ndm_dlist_add(new_entry, head->prev, head);
}

static inline void ndm_dlist_remove(
		struct ndm_dlist_entry_t *entry)
{
	entry->prev->next = entry->next;
	entry->next->prev = entry->prev;
	ndm_dlist_init(entry);
}

static inline bool ndm_dlist_is_empty(
		const struct ndm_dlist_entry_t *head)
{
	return (head->next == head) ? true : false;
}

#define ndm_dlist_entry(ptr, type, member)							\
	((type *) (((char *) ptr) - ((char *) &((type *) 0)->member)))

#define ndm_dlist_foreach_entry(e, type, member, head)				\
	for (e = ndm_dlist_entry((head)->next, type, member);			\
		 &e->member != (head); 										\
	     e = ndm_dlist_entry(e->member.next, type, member))

#define ndm_dlist_foreach_entry_safe(e, type, member, head, n)		\
	for (e = ndm_dlist_entry((head)->next, type, member),			\
		 n = ndm_dlist_entry(e->member.next, type, member);			\
		 &e->member != (head); 										\
		 e = n, n = ndm_dlist_entry(n->member.next, type, member))

#endif	/* __NDM_DLIST_H__ */

