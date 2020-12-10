#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ndm/spawn.h>
#include <ndm/string.h>

bool ndm_spawn_default_at_exec(
		const char *const argv[],
		const char *const envp[],
		const int control_fd,
		void *user_data)
{
	/* make sure all opened descriptors are closed, except STDIO
	 * ones and console_fd which will be closed automatically. */
	int fd = (int) sysconf(_SC_OPEN_MAX) - 1;
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
			const int control_fd,
			void *user_data))
{
	int fb_fd[2];
	pid_t pid = NDM_SPAWN_INVALID_PID;

	if (pipe(fb_fd) != 0) {
		/* can't create an execution feedback pipe */
	} else {
		int flags;

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
			bool env_error = false;

			if (envp != NULL) {
				/* append new (or replace old) environment variables */
				unsigned long k = 0;

				while (envp[k] != NULL && !env_error) {
					char *name = ndm_string_dup(envp[k]);

					if (name == NULL) {
						env_error = true;
					} else {
						char *value = strchr(name, '=');

						if (value == NULL) {
							/* no '=' symbol in a environment entry */
							errno = EINVAL;
							env_error = true;
						} else {
							*value = '\0';
							++value;
							env_error = (setenv(name, value, 1) != 0);
						}

						free(name);
					}

					++k;
				}
			}

			if (env_error) {
				/* failed to setup environment variables */
			} else
			if (at_exec != NULL &&
				!at_exec(argv, envp, fb_fd[1], user_data))
			{
				/* at_exec() returned an error in the "errno" variable */
			} else {
				execvp(argv[0], (char *const *)(&argv[0]));
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

			/* terminate a child without a successfull execvp() call */
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
	}

	return pid;
}

pid_t ndm_spawn(
		const char *const argv[],
		const char *const envp[])
{
	return ndm_spawn_process(argv, envp, NULL, ndm_spawn_default_at_exec);
}

