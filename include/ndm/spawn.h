#ifndef __NDM_SPAWN_H__
#define __NDM_SPAWN_H__

#include <stdbool.h>
#include <sys/types.h>
#include "attr.h"

#define NDM_SPAWN_INVALID_PID		((pid_t) -1)

bool ndm_spawn_default_at_exec(
		const char *const argv[],
		const char *const envp[],
		const int fd_max,
		const int control_fd,
		void *user_data) NDM_ATTR_WUR;

pid_t ndm_spawn_process(
		const char *const argv[],
		const char *const envp[],
		void *user_data,
		bool (*at_exec)(
			const char *const argv[],
			const char *const envp[],
			const int fd_max,
			const int control_fd,
			void *user_data)) NDM_ATTR_WUR;

pid_t ndm_spawn(
		const char *const argv[],
		const char *const envp[]) NDM_ATTR_WUR;

#endif	/* __NDM_SPAWN_H__ */

