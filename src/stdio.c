#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ndm/stdio.h>

static int __ndm_vabsprintf(
		char *buffer,
		const size_t buffer_size,
		char **strp,
		const char *const format,
		va_list ap)
{
	int size = -1;
	va_list aq;

	*strp = buffer;

	va_copy(aq, ap);
	size = vsnprintf(buffer, buffer_size, format, aq);
	va_end(aq);

	if (size < 0) {
		*strp = NULL;
		errno = EINVAL;
	} else
	if ((size_t) size >= buffer_size) {
		*strp = malloc((size_t) size + 1);

		if (*strp == NULL) {
			size = -1;
			errno = ENOMEM;
		} else {
			va_copy(aq, ap);
			size = vsnprintf(*strp, (size_t) size + 1, format, aq);
			va_end(aq);

			if (size < 0) {
				free(*strp);
				*strp = NULL;
				errno = EINVAL;
			}
		}
	}

	return size;
}

int ndm_asprintf(
		char **strp,
		const char *const format,
		...)
{
	char buffer;
	va_list ap;
	int ret = -1;

	va_start(ap, format);
	ret = __ndm_vabsprintf(&buffer, 0, strp, format, ap);
	va_end(ap);

	return ret;
}

int ndm_vasprintf(
		char **strp,
		const char *const format,
		va_list ap)
{
	char buffer;
	va_list aq;
	int ret = -1;

	va_copy(aq, ap);
	ret = __ndm_vabsprintf(&buffer, 0, strp, format, aq);
	va_end(aq);

	return ret;
}

int ndm_absprintf(
		char *buffer,
		const size_t buffer_size,
		char **strp,
		const char *const format,
		...)
{
	va_list ap;
	int ret = -1;

	va_start(ap, format);
	ret = __ndm_vabsprintf(buffer, buffer_size, strp, format, ap);
	va_end(ap);

	return ret;
}

int ndm_vabsprintf(
		char *buffer,
		const size_t buffer_size,
		char **strp,
		const char *const format,
		va_list ap)
{
	va_list aq;
	int ret = -1;

	va_copy(aq, ap);
	ret = __ndm_vabsprintf(buffer, buffer_size, strp, format, aq);
	va_end(aq);

	return ret;
}

