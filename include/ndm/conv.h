#ifndef __NDM_CONV_H__
#define __NDM_CONV_H__

#include <stddef.h>
#include <stdint.h>
#include "attr.h"

typedef int32_t ndm_conv_t;

/* iconv compatibility functions. */

#ifdef NDM_CONV_ICONV_COMPAT

#define iconv_t		ndm_conv_t
#define iconv_open	ndm_conv_open
#define iconv		ndm_conv
#define iconv_close	ndm_conv_close

#endif /* NDM_CONV_ICONV_COMPAT */

ndm_conv_t ndm_conv_open(
		const char *const to,
		const char *const from) NDM_ATTR_WUR;

size_t ndm_conv(
		const ndm_conv_t cd,
		const char **inp,
		size_t *in_bytes_left,
		char **outp,
		size_t *out_bytes_left) NDM_ATTR_WUR;

int ndm_conv_close(
		ndm_conv_t cd);

#endif	/* __NDM_CONV_H__ */

