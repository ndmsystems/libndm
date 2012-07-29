#ifndef __NDM_STRARG_H__
#define __NDM_STRARG_H__

#include <stdbool.h>
#include "strvec.h"

bool ndm_strarg_parse(
		const char *const command,
		struct ndm_strvec_t *args);

#endif	/* __NDM_STRARG_H__ */

