#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ndm/time.h>
#include <ndm/stdio.h>
#include <ndm/feedback.h>
#include "core.h"

#define NDM_FEEDBACK_WAIT_SLEEP_MSEC_				100		/* 0.1 sec. */

bool ndm_feedback(
		const int64_t timeout_msec,
		const char *const args[],
		const char *const env_format,
		...)
{
	va_list ap;
	int ret = -1;
	char *env;
	const char *env_fmt = (env_format == NULL) ? "" : env_format;
	bool answ = false;

	va_start(ap, env_format);
	ret = ndm_vasprintf(&env, env_fmt, ap);
	va_end(ap);

	if (ret < 0)
		return false;

	unsigned int i;
	size_t env_count = (ret > 0) ? 1u : 0u;
	const char **env_argv;

	for (i = 0; i < ret; i++) {
		if (env[i] == *NDM_FEEDBACK_ENV_SEPARATOR) {
			++env_count;
		}
	}

	++env_count; /* NULL terminator */

	if (env_count > SIZE_MAX / sizeof(char *)) {
		/* too many environment variables */
		env_argv = NULL;
		errno = ENOMEM;
	} else
	if ((env_argv = malloc(env_count * sizeof(char *))) != NULL) {
		int env_index = 0;
		char *env_start = env;
		bool valid_format = true;

		i = 0;

		while (i <= ret && valid_format) {
			if (env[i] == *NDM_FEEDBACK_ENV_SEPARATOR || i == ret) {
				if (env_start == &env[i] && i == ret) {
					/* skip last empty entry */
				} else {
					env[i] = '\0';
					env_argv[env_index++] = env_start;
					valid_format = (strchr(env_start, '=') != NULL);
					env_start = &env[i + 1];
				}
			}

			++i;
		}

		env_argv[env_index] = NULL;

		if (!valid_format) {
			/* no '=' symbol in an environment entry */
			errno = EINVAL;
		} else	{
			answ = ndm_feedback_ve(timeout_msec, args, env_argv);
		}

		free(env_argv);
	}

	free(env);

	return answ;
}

bool ndm_feedback_ve(
		const int64_t timeout_msec,
		const char *const argv[],
		const char *const env_argv[])
{
	return ndm_core_feedback_ve(timeout_msec, argv, env_argv);
}
