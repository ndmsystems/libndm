#ifndef __NDM_CONV_H__
#define __NDM_CONV_H__

#include <stddef.h>
#include <stdint.h>
#include "attr.h"

typedef int32_t ndm_conv_t;

enum ndm_conv_flags_t
{
	NDM_CONV_FLAGS_ENCODE_STRICTLY			= 0x0000,

	/* Encode with a replacement character. */
	NDM_CONV_FLAGS_ENCODE_TRUNCATED			= 0x0001,
	NDM_CONV_FLAGS_ENCODE_ILLEGAL			= 0x0002,
	NDM_CONV_FLAGS_ENCODE_NON_MAPPED		= 0x0004,

	NDM_CONV_FLAGS_ENCODE_NON_STRICTLY		=
		NDM_CONV_FLAGS_ENCODE_TRUNCATED		|
		NDM_CONV_FLAGS_ENCODE_ILLEGAL		|
		NDM_CONV_FLAGS_ENCODE_NON_MAPPED
};

enum ndm_conv_error_t
{
	NDM_CONV_ERROR_OK,
	NDM_CONV_ERROR_INPUT_TRUNCATED,			//!< unterminated sequence
	NDM_CONV_ERROR_INPUT_ILLEGAL,			//!< illegal byte sequence
	NDM_CONV_ERROR_INPUT_NON_MAPPED			//!< no destination code
};

ndm_conv_t ndm_conv_open(
		const char *const from,
		const char *const to,
		const enum ndm_conv_flags_t flags) NDM_ATTR_WUR;

void ndm_conv_close(
		ndm_conv_t *conv);

enum ndm_conv_error_t ndm_conv(
		const ndm_conv_t *conv,
		const char **in,
		const size_t in_bytes,
		char **out,
		const size_t out_bytes,
		size_t *bytes_converted) NDM_ATTR_WUR;

const char *ndm_conv_strerror(
		const enum ndm_conv_error_t error);

#endif	/* __NDM_CONV_H__ */

