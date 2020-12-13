#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ndm/int.h>
#include <ndm/stdio.h>
#include <ndm/spawn.h>
#include <ndm/string.h>
#include <ndm/strmap.h>

#define NDM_SPAWN_SLASH_								((char) '/')
#define NDM_SPAWN_COLON_								((char) ':')
#define NDM_SPAWN_EQL_									((char) '=')
#define NDM_SPAWN_NIL_									((char) '\0')
#define NDM_SPAWN_PTR_SIZE_								sizeof(void *)

extern char **environ;

static bool ndm_spawn_merge_env_(
		struct ndm_strmap_t *map,
		const char *const env[],
		const bool check)
{
	size_t i = 0;

	for (; env[i] != NULL; i++) {
		const char *const n = env[i];
		const char *const v = strchr(n, NDM_SPAWN_EQL_);

		if (v == NULL) {
			if (check) {
				errno = EINVAL;
				return false;
			}

			continue;
		}

		const size_t n_size = (size_t) (v - n);

		if (!ndm_strmap_nset(map, n, n_size, v + 1, strlen(v + 1))) {
			errno = ENOMEM;
			return false;
		}
	}

	return true;
}

static inline size_t ndm_spawn_env_str_size_(
		const char *const name,
		const char *const value)
{
	const size_t size =
		strlen(name) + sizeof(NDM_SPAWN_EQL_) +
		strlen(value) + sizeof(NDM_SPAWN_NIL_);

	return NDM_INT_ALIGN(size, NDM_SPAWN_PTR_SIZE_);
}

static char **ndm_spawn_alloc_env_(const char *const envp[])
{
	int error = errno;
	char **env = NULL;
	struct ndm_strmap_t map = NDM_STRMAP_INITIALIZER_DEFAULT;

	if (!ndm_spawn_merge_env_(&map, (const char *const *)environ, false)) {
		error = errno;
		goto failed;
	}

	if (!ndm_spawn_merge_env_(&map, envp, true)) {
		error = errno;
		goto failed;
	}

	const size_t env_count = ndm_strmap_size(&map);
	size_t env_size = NDM_SPAWN_PTR_SIZE_;
	size_t i = 0;

	for (; i < env_count; i++) {
		const char *const n = ndm_strmap_get_key(&map, i);
		const char *const v = ndm_strmap_get_by_index(&map, i);

		env_size += NDM_SPAWN_PTR_SIZE_ + ndm_spawn_env_str_size_(n, v);
	}

	env = malloc(env_size);

	if (env == NULL) {
		error = errno;
		goto failed;
	}

	char *env_str = ((char *) env) + (env_count + 1) * NDM_SPAWN_PTR_SIZE_;

	for (i = 0; i < env_count; i++) {
		const char *const n = ndm_strmap_get_key(&map, i);
		const char *const v = ndm_strmap_get_by_index(&map, i);
		const size_t env_str_size = ndm_spawn_env_str_size_(n, v);

		env[i] = env_str;
		snprintf(env_str, env_str_size, "%s%c%s", n, NDM_SPAWN_EQL_, v);
		env_str += env_str_size;
	}

	env[i] = NULL;

failed:
	ndm_strmap_clear(&map);
	errno = error;

	return env;
}

static char *ndm_spawn_get_executable__(
		const char *const file_name,
		const char *const path_start,
		const char *const path_end)
{
	int error = errno;
	char *exec = NULL;
	const int ret = ndm_asprintf(&exec, "%.*s%c%s",
		(int) (path_end - path_start), path_start,
		NDM_SPAWN_SLASH_, file_name);

	if (ret < 0) {
		return NULL;
	}

	struct stat statbuf;

	if (stat(exec, &statbuf) != 0) {
		error = errno;
		goto failed;
	}

	if (!S_ISREG(statbuf.st_mode)) {
		/* not a regular file */
		error = ENOENT;
		goto failed;
	}

	if (access(exec, R_OK | X_OK) != 0) {
		error = errno;
		goto failed;
	}

	return exec;

failed:
	free(exec);
	errno = error;

	return NULL;
}

static char *ndm_spawn_get_executable_(const char *const file_name)
{
	if (strchr(file_name, NDM_SPAWN_SLASH_) != NULL) {
		/* do not try to find in PATH directories */
		return ndm_string_dup(file_name);
	}

	const char *const path = getenv("PATH");

	if (path != NULL) {
		const char *const path_end = path + strlen(path);
		const char *start = path;

		while (start < path_end) {
			const char *const colon = strchr(start, NDM_SPAWN_COLON_);
			const char *const end = (colon == NULL) ? path_end : colon;
			char *exec = ndm_spawn_get_executable__(file_name, start, end);

			if (exec != NULL) {
				return exec;
			}

			if (errno != ENOENT) {
				return NULL;
			}

			start = end + 1;
		}
	}

	errno = ENOENT;

	return NULL;
}

bool ndm_spawn_default_at_exec(
		const char *const argv[],
		const char *const envp[],
		const int fd_max,
		const int control_fd,
		void *user_data)
{
	/* make sure all opened descriptors are closed, except STDIO
	 * ones and console_fd which will be closed automatically. */
	int fd = fd_max;
	sigset_t set;

	while (fd > STDERR_FILENO) {
		if (fd != control_fd) {
			close(fd);
		}

		--fd;
	}

	/* unmask all signals and detach */
	if (sigfillset(&set) == -1 ||
		sigprocmask(SIG_UNBLOCK, &set, NULL) == -1)
	{
		/* unable to unmask signals */
	} else
	if (setsid() == -1) {
		/* unable to create a new session */
	} else {
		return true;
	}

	return false;
}

pid_t ndm_spawn_process(
		const char *const argv[],
		const char *const envp[],
		void *user_data,
		bool (*at_exec)(
			const char *const argv[],
			const char *const envp[],
			const int fd_max,
			const int control_fd,
			void *user_data))
{
	int fb_fd[2];
	pid_t pid = NDM_SPAWN_INVALID_PID;

	if (pipe(fb_fd) != 0) {
		/* can't create an execution feedback pipe */
	} else {
		/* call @c sysconf() before a fork(); after the @c fork()
		 * in a child process only async-signal-safe functions
		 * can be used (see "man 7 signal" for a list) */
		const int fd_max = (int) sysconf(_SC_OPEN_MAX) - 1;
		char **env = ndm_spawn_alloc_env_(envp);
		char *executable = ndm_spawn_get_executable_(argv[0]);
		int flags;

		if (env == NULL) {
			/* unable to allocate a new environment variable set */
		} else
		if (executable == NULL) {
			/* unable to find an executable to run */
		} else
		if (((flags = fcntl(fb_fd[1], F_GETFD)) == -1) ||
			(fcntl(fb_fd[1], F_SETFD, flags | FD_CLOEXEC) == -1))
		{
			/* can't initialize a feedback pipe */
		} else
		/* flush all opened I/O streams before @c fork(); after a fork
		 * in a child process no stream writes are allowed */
		if (fflush(NULL) == EOF) {
			/* failed to flush */
		} else
		if ((pid = fork()) < 0) {
			/* can't fork a process */
			pid = NDM_SPAWN_INVALID_PID;
		} else
		if (pid == 0) {
			if (at_exec != NULL &&
				!at_exec(argv, envp, fd_max, fb_fd[1], user_data))
			{
				/* at_exec() returned an error in the "errno" variable */
			} else {
				execve(executable, (char *const *)(&argv[0]), env);
				/* could not execute */
			}

			const int error = errno;	/* errno should be set properly */
			ssize_t n = 0;
			ssize_t left = sizeof(error);

			while (n >= 0 && left > 0) {
				n = write(fb_fd[1],
					((uint8_t *) &error) + sizeof(error) - left,
					(size_t) left);
				left -= n;
			}

			/* terminate a child without a successfull execve() call */
			_exit(EXIT_FAILURE);
		} else {
			int error = 0;
			ssize_t n;
			ssize_t left = sizeof(error);

			close(fb_fd[1]);

			do {
				n = read(fb_fd[0],
					((uint8_t *) &error) + sizeof(error) - left,
					(size_t) left);
				left -= n;
			} while (n > 0 && left > 0);

			close(fb_fd[0]);

			if (n != 0) {
				int local_error = 0;

				if (left != 0) {
					local_error = errno;
				}

				/* can't read execution feedback or start a process;
				 * stop a broken child with a nonexecuted external
				 * process, ignore any error */

				kill(pid, SIGKILL);
				waitpid(pid, NULL, 0);

				errno = (left == 0) ? error : local_error;
				pid = NDM_SPAWN_INVALID_PID;
			}
		}

		free(executable);
		free(env);
	}

	return pid;
}

pid_t ndm_spawn(
		const char *const argv[],
		const char *const envp[])
{
	return ndm_spawn_process(argv, envp, NULL, ndm_spawn_default_at_exec);
}

