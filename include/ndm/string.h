#ifndef __NDM_STRING_H__
#define __NDM_STRING_H__

#include "attr.h"

char *ndm_string_dup(
		const char *const s) NDM_ATTR_WUR;

char *ndm_string_ndup(
		const char *const s,
		const size_t size) NDM_ATTR_WUR;

#endif	/* __NDM_STRING_H__ */

