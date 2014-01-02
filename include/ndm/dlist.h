#ifndef __NDM_DLIST_H__
#define __NDM_DLIST_H__

#include <stdbool.h>

/**
 * Describes the minimal entry of a doubly-linked list.
 *
 * @param next Pointer to the next entry.
 * @param prev Pointer to the previous entry.
 */

struct ndm_dlist_entry_t
{
	struct ndm_dlist_entry_t *next;
	struct ndm_dlist_entry_t *prev;
};

/**
 * Macro for static initialization of head entry.
 * @code
 * struct ndm_dlist_entry_t name = NDM_DLIST_INITIALIZER(name)
 * @endcode
 *
 * @param name The list name.
 */

#define NDM_DLIST_INITIALIZER(name)									\
	{&(name), &(name)}

/**
 * Macro to declare a variable which stores a head entry of the list with
 * initialization.
 *
 * @param name The list name.
 */

#define NDM_DLIST_HEAD(name)										\
	struct ndm_dlist_entry_t name = NDM_DLIST_INITIALIZER(name)

/**
 * Initialize a list entry to the the initial (unlinked) value.
 *
 * @param entry The list entry.
 */

static inline void ndm_dlist_init(struct ndm_dlist_entry_t *entry)
{
	entry->next = entry->prev = entry;
}

/**
 *
 */

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

/**
 * Insert a new list entry @a new_entry into the list after the head entry.
 *
 * @param head Pointer to the head entry of the list.
 * @param new_entry Pointer to the new entry to insert.
 */

static inline void ndm_dlist_insert_after(
		struct ndm_dlist_entry_t *head,
		struct ndm_dlist_entry_t *new_entry)
{
	__ndm_dlist_add(new_entry, head, head->next);
}

/**
 * Insert a new list entry @a new_entry into the list before the head entry.
 *
 * @param head Pointer to the head entry of the list.
 * @param new_entry Pointer to the new entry to insert.
 */

static inline void ndm_dlist_insert_before(
		struct ndm_dlist_entry_t *head,
		struct ndm_dlist_entry_t *new_entry)
{
	__ndm_dlist_add(new_entry, head->prev, head);
}

/**
 * Remove an entry from the list.
 *
 * @param entry Pointer to the entry to remove.
 */

static inline void ndm_dlist_remove(
		struct ndm_dlist_entry_t *entry)
{
	entry->prev->next = entry->next;
	entry->next->prev = entry->prev;
	ndm_dlist_init(entry);
}

/**
 * Check if the list is empty.
 *
 * @param head Pointer to the head entry.
 *
 * @returns @c true if given entry is not linked to the list, @c false —
 * otherwise.
 */

static inline bool ndm_dlist_is_empty(
		const struct ndm_dlist_entry_t *head)
{
	return (head->next == head) ? true : false;
}

/**
 * Count list elements.
 *
 * @param head Pointer to the head entry.
 *
 * @returns Size of a list.
 */

static inline size_t ndm_dlist_size(
		const struct ndm_dlist_entry_t *head)
{
	size_t size = 0;
	const struct ndm_dlist_entry_t *entry = head->next;

	while (entry != head) {
		size++;
		entry = entry->next;
	}

	return size;
}

/**
 * Convert a pointer to the base entry of the list to a pointer to
 * the structure that contains this entry.
 *
 * @param ptr Pointer to the parent structure.
 * @param type Type of the parent structure.
 * @param member Name of the structure member which corresponds to the list
 * entry.
 */

#define ndm_dlist_entry(ptr, type, member)							\
	((type *) (((char *) ptr) - ((char *) &((type *) 0)->member)))

/**
 * Macro for declaring a cycle that goes through all the entries of the list
 * in the forward order. The current list entry can not be removed.
 *
 * @param e Pointer to the list entry.
 * @param type Type of the list entry.
 * @param member Name of the structure member which corresponds to the list
 * entry.
 * @param head Pointer to the head entry of the list.
 */

#define ndm_dlist_foreach_entry(e, type, member, head)				\
	for (e = ndm_dlist_entry((head)->next, type, member);			\
		 &e->member != (head);										\
	     e = ndm_dlist_entry(e->member.next, type, member))

/**
 * Macro for declaring a cycle that goes through all the entries of the list
 * in the forward order. The current list entry can be removed.
 *
 * @param e Pointer to the list entry.
 * @param type Type of the list entry.
 * @param member Name of the structure member which corresponds to the list
 * entry.
 * @param head Pointer to the head entry of the list.
 * @param n Additional variable to store the pointers during the removal
 * of the current list entry.
 */

#define ndm_dlist_foreach_entry_safe(e, type, member, head, n)		\
	for (e = ndm_dlist_entry((head)->next, type, member),			\
		 n = ndm_dlist_entry(e->member.next, type, member);			\
		 &e->member != (head);										\
		 e = n, n = ndm_dlist_entry(n->member.next, type, member))

#endif	/* __NDM_DLIST_H__ */

