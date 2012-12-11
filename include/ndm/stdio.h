#ifndef __NDM_STDIO_H__
#define __NDM_STDIO_H__

#include <stdarg.h>
#include "attr.h"

int ndm_asprintf(
		char **strp,
		const char *const format,
		...) NDM_ATTR_PRINTF(2, 3);

int ndm_vasprintf(
		char **strp,
		const char *const format,
		va_list ap) NDM_ATTR_PRINTF(2, 0);

int ndm_absprintf(
		char *buffer,
		const size_t buffer_size,
		char **strp,
		const char *const format,
		...) NDM_ATTR_PRINTF(4, 5);

int ndm_vabsprintf(
		char *buffer,
		const size_t buffer_size,
		char **strp,
		const char *const format,
		va_list ap)  NDM_ATTR_PRINTF(4, 0);

#endif	/* __NDM_STDIO_H__ */

