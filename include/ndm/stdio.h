#ifndef __NDM_STDIO_H__
#define __NDM_STDIO_H__

#include <stdarg.h>

int ndm_asprintf(
		char **strp,
		const char *const format,
		...);

int ndm_vasprintf(
		char **strp,
		const char *const format,
		va_list ap);

int ndm_absprintf(
		char *buffer,
		const size_t buffer_size,
		char **strp,
		const char *const format,
		...);

int ndm_vabsprintf(
		char *buffer,
		const size_t buffer_size,
		char **strp,
		const char *const format,
		va_list ap);

#endif	/* __NDM_STDIO_H__ */

