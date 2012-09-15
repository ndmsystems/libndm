#ifndef __NDM_SYS_H__
#define __NDM_SYS_H__

#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include "attr.h"

#define NDM_SYS_SLEEP_GRANULARITY_MSEC			333

bool ndm_sys_init() NDM_ATTR_WUR;

bool ndm_sys_set_default_signals() NDM_ATTR_WUR;

int ndm_sys_rand() NDM_ATTR_WUR;

const char *ndm_sys_strerror(const int error) NDM_ATTR_WUR;

bool ndm_sys_sleep(const struct timespec *interval);
bool ndm_sys_sleep_msec(const int64_t msec);

const struct timespec *ndm_sys_sleep_granularity() NDM_ATTR_WUR;

bool ndm_sys_is_interrupted() NDM_ATTR_WUR;
void ndm_sys_set_interrupted();

#endif	/* __NDM_SYS_H__ */

