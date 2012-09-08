#ifndef __NDM_MEMCHK__
#define __NDM_MEMCHK__

#include <stdlib.h>
#include <string.h>

#if !defined NDEBUG && defined _MEMORY_LEAK_DEBUG

#ifdef __cplusplus
extern "C" {
#endif

void *__debug_malloc__(
		const size_t size,
		const char *const file,
		const unsigned long line,
		const int array);

void *__debug_calloc__(
		const size_t nmemb,
		const size_t size,
		const char *const file,
		const unsigned long line);

void __debug_free__(
		void *p,
		const int array);

void *__debug_realloc__(
		void *p,
		const size_t size,
		const char *const file,
		const unsigned long line);

char *__debug_strdup__(
		const char *const s,
		const char *const file,
		const unsigned long line);

#define malloc(s)		__debug_malloc__(s, __FILE__, __LINE__, 0)
#define calloc(n, s)	__debug_calloc__(n, s, __FILE__, __LINE__)
#define realloc(p, s)	__debug_realloc__(p, s, __FILE__, __LINE__)
#define free(p)			__debug_free__(p, 0)
#define strdup(p)		__debug_strdup__(p, __FILE__, __LINE__)

void check_memory();

#ifdef __cplusplus
}	/* extern "C" */
#endif

#else

#define check_memory()

#endif	/* !NDEBUG && _MEMORY_LEAK_DEBUG */

#endif	/* __NDM_MEMCHK__ */

