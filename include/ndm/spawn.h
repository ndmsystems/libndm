#ifndef __NDM_SPAWN_H__
#define __NDM_SPAWN_H__

#include <stdbool.h>
#include <sys/types.h>

#define NDM_SPAWN_INVALID_PID		((pid_t) -1)

bool ndm_spawn_default_at_exec(
		const char *const argv[],
		const char *const envp[],
		const void *const sys_data,
		void *user_data);

pid_t ndm_spawn_process(
		const char *const argv[],
		const char *const envp[],
		void *user_data,
		bool (*at_exec)(
			const char *const argv[],
			const char *const envp[],
			const void *const sys_data,
			void *user_data));

pid_t ndm_spawn(
		const char *const argv[],
		const char *const envp[]);

#endif	/* __NDM_SPAWN_H__ */

