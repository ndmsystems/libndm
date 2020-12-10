#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ndm/sys.h>
#include <ndm/time.h>
#include <ndm/stdio.h>
#include <ndm/spawn.h>
#include <ndm/feedback.h>

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
	int ret = -1;
	pid_t pid;
	struct timespec end;

	ndm_time_get_monotonic(&end);
	ndm_time_add_msec(&end, timeout_msec);

	if ((pid = ndm_spawn(argv, env_argv)) != NDM_SPAWN_INVALID_PID) {
		bool timedout = false;
		int error = 0;

		do {
			int status;
			const pid_t p = waitpid(pid, &status, WNOHANG);

			if (p == 0) {
				/* wait for a pid */
				struct timespec now;

				ndm_time_get_monotonic(&now);
				timedout = ndm_time_greater_or_equal(&now, &end);

				if (!timedout) {
					struct timespec t = end;
					int64_t msec;

					ndm_time_sub(&t, &now);
					msec = ndm_time_to_msec(&t);

					ndm_sys_sleep_msec(
						msec <= NDM_FEEDBACK_WAIT_SLEEP_MSEC_ ?
						msec :  NDM_FEEDBACK_WAIT_SLEEP_MSEC_);
				}
			} else if (p == pid) {
				/* the process terminated */
				if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
					/* the process stopped abnormally */
					error = EIO;
					pid = 0;
				} else {
					ret = 0;
				}
			} else {
				error = errno;
			}
		} while (
			ret != 0 &&
			!ndm_sys_is_interrupted() &&
			!timedout &&
			error == 0);

		if (ret != 0) {
			if (pid != 0) {
				kill(pid, SIGKILL);
				waitpid(pid, NULL, 0);
			}

			errno =
				error != 0 ? error :
				timedout ? ETIMEDOUT : EINTR;
		}
	}

	return (ret == 0) ? true : false;
}
