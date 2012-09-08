#ifndef NDEBUG	/* only for debug mode */

#define __MIN__(a, b)			(((a) < (b)) ? (a) : (b))

#if defined _MEMORY_OVERFLOW_DEBUG || defined _MEMORY_UNDERFLOW_DEBUG

/* The local @c malloc, @c calloc, @c free, and @c realloc
 * automatically replace standard library functions. */

#include <fcntl.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <memory.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#endif	/* _MEMORY_OVERFLOW_DEBUG || _MEMORY_UNDERFLOW_DEBUG */

#ifdef _MEMORY_LEAK_DEBUG
#ifdef _MEMORY_UNDERFLOW_DEBUG
#error \
	Undeflow memory error debug mode is not available \
	when a memory leak debug mode is on.
#endif	/* _MEMORY_UNDERFLOW_DEBUG */

#include <time.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

#endif	/* _MEMORY_LEAK_DEBUG */

#if defined _MEMORY_OVERFLOW_DEBUG || \
	defined _MEMORY_UNDERFLOW_DEBUG || \
	defined _MEMORY_LEAK_DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define MAX_MESSAGE_SIZE_		1024

static void __abort(const char* const format, ...)
	__attribute__((format(printf, 1, 2)));

static void __abort(const char* const format, ...)
{
	char message[MAX_MESSAGE_SIZE_];
	va_list ap;

	va_start(ap, format);
	vsnprintf(message, sizeof(message), format, ap);
	va_end(ap);

	fprintf(stderr, "\nFATAL: %s\n\n", message);

	abort();
}

#endif	/* _MEMORY_OVERFLOW_DEBUG ||
		 * _MEMORY_UNDERFLOW_DEBUG ||
		 * _MEMORY_LEAK_DEBUG */

#if defined _MEMORY_OVERFLOW_DEBUG || defined _MEMORY_UNDERFLOW_DEBUG

/* @c ps should be a power of 2. */

#define __PAGE_ALIGN__(s, ps)         \
	((s + ps - 1) & ~(ps - 1))

// extern "C" {

static const size_t PAGE_SIZE_ = 4096; // sysconf(_SC_PAGESIZE);
static const size_t SIZE_SIZE_ = sizeof(size_t);

static void __protect_page(void *p, int prot)
{
	if (mprotect(p, PAGE_SIZE_, prot) != 0) {
		__abort("failed to protect page at %p (protection type = %i).",
			p, prot);
	}
}

static void __free_block(void *p, const size_t aligned)
{
	if (munmap(p, aligned) != 0) {
		__abort("failed to free pages at %p (%lu bytes).", p, aligned);
	}
}

void *calloc(size_t nmemb, size_t size)
{
	const size_t l = nmemb*size;
	void *p = malloc(l);

	if (p != NULL) {
		memset(p, 0, l);
	}

	return p;
}

#ifdef _MEMORY_OVERFLOW_DEBUG

#ifdef _MEMORY_UNDERFLOW_DEBUG
#error \
	Memory overflow and underflow debug modes \
	can not be used simultaneously.
#endif	// _MEMORY_UNDERFLOW_DEBUG

void *malloc(size_t size)
{
	const size_t s = size + SIZE_SIZE_;
	const size_t aligned = __PAGE_ALIGN__(s, PAGE_SIZE_) + PAGE_SIZE_;
	int fd = open("/dev/zero", O_RDWR);
	char *p = NULL;

	if (fd >= 0) {
		char *ptr = NULL;

		p = (char *) mmap(
			NULL, aligned,
			PROT_READ | PROT_WRITE,
			MAP_PRIVATE, fd, 0);
		close(fd);

		ptr = p + aligned - PAGE_SIZE_;

		if (p != MAP_FAILED) {
			__protect_page(ptr, PROT_NONE);
			p = ptr - size;
			*((size_t *) (p - SIZE_SIZE_)) = size;
		}
	}

	return p;
}

void free(void *p)
{
	if (p != NULL) {
		size_t s = *(size_t *)(((char *) p) - SIZE_SIZE_);
		void *ptr = (void *) ((ptrdiff_t) p & ~((ptrdiff_t) PAGE_SIZE_ - 1));
		size_t aligned = __PAGE_ALIGN__(s, PAGE_SIZE_) + PAGE_SIZE_;

		__free_block(ptr, aligned);
	}
}

void *realloc(void *p, size_t size)
{
	void *ptr = NULL;

	if (p == NULL) {
		ptr = malloc(size);
	} else
	if (size == 0) {
		free(p);
	} else {
		const size_t old_size = *((size_t *) (((char *) p) - SIZE_SIZE_));

		if (__PAGE_ALIGN__(old_size + SIZE_SIZE_, PAGE_SIZE_) ==
			__PAGE_ALIGN__(size + SIZE_SIZE_, PAGE_SIZE_))
		{
			ptr = ((char *) p) + old_size - size;
			memmove(ptr, p, __MIN__(size, old_size));
			*((size_t *) (((char *) ptr) - SIZE_SIZE_)) = size;
		} else {
			ptr = malloc(size);

			if (ptr != NULL) {
				memcpy(ptr, p, __MIN__(size, old_size));
				free(p);
			}
		}
	}

	return ptr;
}

#else	// _MEMORY_OVERFLOW_DEBUG

static void *__get_block_info(
		void *p,
		size_t *size,
		size_t *aligned)
{
	if ((((ptrdiff_t) p) & (PAGE_SIZE_ - 1)) != 0) {
		__abort("address %p must be aligned by a page size (%lu bytes).",
			p, PAGE_SIZE_);
	}

	char *ptr = ((char *) p) - PAGE_SIZE_;

	__protect_page(ptr, PROT_READ);
	*size = *((size_t *) (ptr + PAGE_SIZE_ - SIZE_SIZE_));
	__protect_page(ptr, PROT_NONE);

	*aligned = __PAGE_ALIGN__(*size, PAGE_SIZE_) + PAGE_SIZE_;

	return ptr;
}

void *malloc(size_t size)
{
	const size_t s = size + SIZE_SIZE_;
	const size_t aligned = __PAGE_ALIGN__(s, PAGE_SIZE_) + PAGE_SIZE_;
	char *p = (char *) mmap(
		NULL, aligned, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_SHARED, -1, 0);

	if (p != MAP_FAILED) {
		*((size_t *) (p + PAGE_SIZE_ - SIZE_SIZE_)) = size;
		__protect_page(p, PROT_NONE);
		p += PAGE_SIZE_;
	}

	return p;
}

void free(void *p)
{
	if (p != NULL) {
		size_t size = 0;
		size_t aligned = 0;
		void *ptr = __get_block_info(p, &size, &aligned);

		__free_block(ptr, aligned);
	}
}

void *realloc(void *p, size_t size)
{
	void *ptr = NULL;
	size_t old_size;
	size_t old_aligned;
	void *old_ptr;

	if (p == NULL) {
		ptr = malloc(size);
	} else
	if (size == 0) {
		free(p);
	} else {
		ptr = malloc(size);

		if (ptr != NULL) {
			old_ptr = __get_block_info(p, &old_size, &old_aligned);
			memcpy(ptr, p, __MIN__(size, old_size));
			__free_block(old_ptr, old_aligned);
		}
	}

	return ptr;
}

#endif 	/* _MEMORY_OVERFLOW_DEBUG */

// }	/* extern "C" */

#endif /* _MEMORY_OVERFLOW_DEBUG || _MEMORY_UNDERFLOW_DEBUG */

#ifdef _MEMORY_LEAK_DEBUG

/* Random generated constants. */

#define BLOCK_GSTART			0xf61abe2d
#define BLOCK_GEND				0xe482abf2

#define BLOCK_GSTART_ARRAY		0x4efab494
#define BLOCK_GEND_ARRAY		0x62bbae2e

#define BLOCK_GSTART_DELETED	0x39a4c41f
#define BLOCK_GEND_DELETED		0x9a577b23

struct chunk_t
{
	uint32_t gstart;
	unsigned long id;
	char* file;
	unsigned long line;
	size_t size;
	time_t time;
	int array;
	struct chunk_t *next;
	struct chunk_t *prev;
	uint32_t gend;
};

static pthread_mutex_t __lock = PTHREAD_MUTEX_INITIALIZER;
static int __installed = 0;
static size_t __max_chunk_size = 0;
static size_t __max_usage = 0;
static size_t __allocated = 0;
static unsigned long __id = 0;
static struct chunk_t __head = {.next = &__head, .prev = &__head};

static void __exit()
{
	/* Does not need a lock. */
	struct chunk_t *c = __head.next;
	size_t leaked = 0;
	const char *s = NULL;
	const char *p = NULL;

	if (c != &__head) {
		leaked = 0;
		fprintf(stderr, "\n");

		do {
			s = ((const char *) c) + sizeof(*c);

			if (s[c->size - 1] == '\0') {
				p = s;

				while (isprint(*p)) {
					++p;
				}

				if (*p != '\0' || p == s) {
					/* It is not a printable string. */
					s = NULL;
				}
			}

			const struct tm *t = localtime(&c->time);

			fprintf(stderr,
				"MEMORY LEAK: block %lu %p " \
				"allocated at %02i:%02i:%02i (%s:%lu), %zu byte(s)%s%s%s.\n",
				c->id, ((char *) c) + sizeof(*c),
				t->tm_hour, t->tm_min, t->tm_sec,
				c->file, c->line, c->size,
				s == NULL ? "" : " [\"",
				s == NULL ? "" : s,
				s == NULL ? "" : "\"]");
			leaked += c->size;
			c = c->next;
		} while (c != &__head);

		fprintf(stderr,
			"\nTotal leaked            : %lu byte(s).\n", leaked);
	}

	fprintf(stdout,
		"\n" \
		"Blocks totally allocated: %lu.\n" \
		"Maximum usage           : %lu bytes.\n" \
		"Maximum chunk size      : %lu bytes.\n\n",
		__id, __max_usage, __max_chunk_size);
}

static void __check_chunk(const struct chunk_t *c, int array)
{
	char *p = ((char *) c) + sizeof(*c);

	if (c->gstart == BLOCK_GSTART_DELETED && c->gend == BLOCK_GEND_DELETED) {
		__abort(
			"block %lu (allocated at %s:%lu) freed.",
			c->id, c->file, c->line);
	}

	if (array) {
		if (c->gstart != BLOCK_GSTART_ARRAY || c->gend != BLOCK_GEND_ARRAY) {
			__abort("corrupted array block at %p.", p);
		}
	} else {
		if (c->gstart != BLOCK_GSTART || c->gend != BLOCK_GEND) {
			__abort("corrupted plain block at %p.", p);
		}
	}

	if (c->array != array) {
		__abort("block %lu (%s:%lu) allocated " \
			"as %s but freed as %s.",
			c->id, c->file, c->line,
			c->array ? "array" : "plain",
			array ? "array" : "plain");
	}
}

static struct chunk_t *__get_chunk(void *p, const int array)
{
	struct chunk_t *c = (struct chunk_t *) (((char *) p) - sizeof(*c));

	__check_chunk(c, array);

	return c;
}

static void __free(struct chunk_t *c)
{
	c->gstart = BLOCK_GSTART_DELETED;
	c->gend = BLOCK_GEND_DELETED;

	c->prev->next = c->next;
	c->next->prev = c->prev;
	c->next = c->prev = c;

	__allocated -= c->size;

	free(c->file);
}

void *__debug_malloc__(
		const size_t size,
		const char *const file,
		const unsigned long line,
		const int array)
{
	/* @c file name should be copied since a library
	 * with the name constant can be removed. */

	const size_t name_size = strlen(file) + 1;
	char *p = malloc(size + name_size + sizeof(struct chunk_t));

	if (p != NULL) {
		struct chunk_t *c = (struct chunk_t *) (p + name_size);

		c->gstart = array ? BLOCK_GSTART_ARRAY : BLOCK_GSTART;
		c->gend = array ? BLOCK_GEND_ARRAY : BLOCK_GEND;
		c->line = line;
		c->size = size;
		c->time = time(NULL);
		c->array = array;
		c->file = p;
		strcpy(c->file, file);

		pthread_mutex_lock(&__lock);

		if (!__installed) {
			if (atexit(__exit) != 0) {
				free(p);
				p = NULL;
			} else {
				__installed = 1;
			}
		}

		if (p != NULL) {
			p = ((char * ) p) + name_size + sizeof(struct chunk_t);

			c->id = ++__id;
			c->next = &__head;
			c->prev = __head.prev;
			__head.prev->next = c;
			__head.prev = c;
			__allocated += size;

			if (__max_usage < __allocated) {
				__max_usage = __allocated;
			}

			if (__max_chunk_size < size) {
				__max_chunk_size = size;
			}
		}

		pthread_mutex_unlock(&__lock);
	}

	return p;
}

void *__debug_calloc__(
		const size_t nmemb,
		const size_t size,
		const char *const file,
		const unsigned long line)
{
	void *p = __debug_malloc__(nmemb*size, file, line, 0);

	if (p != NULL) {
		memset(p, 0, nmemb*size);
	}

	return p;
}

void __debug_free__(void *p, const int array)
{
	if (p != NULL) {
		pthread_mutex_lock(&__lock);
		__free(__get_chunk(p, array));
		pthread_mutex_unlock(&__lock);
	}
}

void *__debug_realloc__(
		void *p,
		const size_t size,
		const char *const file,
		const unsigned long line)
{
	void *ptr = NULL;
	struct chunk_t *c;

	if (p == NULL) {
		ptr = __debug_malloc__(size, file, line, 0);
	} else
	if (size == 0) {
		__debug_free__(p, 0);
	} else {
		ptr = __debug_malloc__(size, file, line, 0);

		if (ptr != NULL) {
			pthread_mutex_lock(&__lock);

			c = __get_chunk(p, 0);
			memcpy(ptr, p, __MIN__(size, c->size));
			__free(c);

			pthread_mutex_unlock(&__lock);
		}
	}

	return ptr;
}

char *__debug_strdup__(
		const char *const s,
		const char *const file,
		const unsigned long line)
{
	const size_t size = strlen(s) + 1;
	char *p = __debug_malloc__(size, file, line, 0);

	if (p != NULL) {
		memcpy(p, s, size);
	}

	return p;
}

void check_memory()
{
	struct chunk_t *c;

	pthread_mutex_lock(&__lock);

	for (c = __head.next; c != &__head; c = c->next) {
		__check_chunk(c, c->array);
	}

	pthread_mutex_unlock(&__lock);
}

#endif	// _MEMORY_LEAK_DEBUG

#endif	// !NDEBUG

