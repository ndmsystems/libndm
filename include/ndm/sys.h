#ifndef __NDM_SYS_H__
#define __NDM_SYS_H__

#include <time.h>
#include <stdint.h>
#include <stdbool.h>

const char *ndm_sys_strerror(const int error);

bool ndm_sys_sleep(const struct timespec *interval);
bool ndm_sys_sleep_msec(const int64_t msec);

const struct timespec *ndm_sys_sleep_granularity();

bool ndm_sys_is_interrupted();
void ndm_sys_set_interrupted();

#endif	/* __NDM_SYS_H__ */

