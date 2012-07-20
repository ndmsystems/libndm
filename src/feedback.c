#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ndm/stdio.h>
#include <ndm/spawn.h>
#include <ndm/feedback.h>

bool ndm_feedback(
		const char *const executable,
		const char *const env_format,
		...)
{
	va_list ap;
	int ret = -1;
	char *env;

	va_start(ap, env_format);
	ret = ndm_vasprintf(&env, env_format, ap);
	va_end(ap);

	if (ret >= 0) {
		int i;
		int env_count = 0;
		const char **env_argv;

		for (i = 0; i < ret; i++) {
			if (env[i] == NDM_FEEDBACK_ENV_SEPARATOR) {
				++env_count;
			}
		}

		if ((env_argv = malloc((env_count + 1)*sizeof(char *))) != NULL) {
			int env_index = 0;
			char *env_start = env;
			pid_t pid;
			const char *const argv[] = {
				executable,
				NULL
			};

			for (i = 0; i < ret; i++) {
				if (env[i] == NDM_FEEDBACK_ENV_SEPARATOR) {
					env[i] = '\0';
					env_argv[env_index++] = env_start;
					env_start = &env[i + 1];
				}
			}

			env_argv[env_index] = NULL;

			if ((pid = ndm_spawn(argv, env_argv)) != NDM_SPAWN_INVALID_PID) {
				waitpid(pid, NULL, 0);
				ret = 0;
			}

			free(env_argv);
		}

		free(env);
	}

	return (ret == 0) ? true : false;
}

