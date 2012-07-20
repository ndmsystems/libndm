#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ndm/stdio.h>

int ndm_vasprintf(
		char **strp,
		const char *const format,
		va_list ap)
{
	int ret = -1;
	char dummy_buf;
	va_list ap_copy;
	int size;

	va_copy(ap_copy, ap);
	size = vsnprintf(&dummy_buf, 0, format, ap_copy) + 1;
	va_end(ap_copy);

	if (size <= 0) {
		errno = EINVAL;
	} else {
		char *str = malloc(size);

		if (str != NULL) {
			va_copy(ap_copy, ap);
			ret = vsnprintf(str, size, format, ap_copy);
			va_end(ap_copy);

			if (ret < 0) {
				free(str);
			} else {
				*strp = str;
			}
		}
	}

	return ret;
}

int ndm_asprintf(
		char **strp,
		const char *const format,
		...)
{
	va_list ap;
	int ret = -1;

	va_start(ap, format);
	ret = ndm_vasprintf(strp, format, ap);
	va_end(ap);

	return ret;
}

