#ifndef __NDM_INT_H__
#define __NDM_INT_H__

#include <stdbool.h>

bool ndm_int_parse_long(char *str, long *value);
bool ndm_int_parse_ulong(char *str, unsigned long *value);

#endif	/* __NDM_INT_H__ */

