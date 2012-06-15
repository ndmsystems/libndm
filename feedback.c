#include "ndm_common.h"
#include <stddef.h>
#include <stdarg.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <termios.h>
#define FEEDBACK_MAX_ENV_COUNT  10
#define INVALID_PID                 ((pid_t) -1)
#define SPAWN_FEEDBACK_SIZE         128 


typedef enum {
    STDIO_ENABLED,
    STDIO_DISABLED
} SPAWN_STDIO_MODE;

static pid_t ndmFeedback_spawn(
 	const char *const argv[],
        const char *const envp[],
        const SPAWN_STDIO_MODE mode)
{
    int fb_fd[2];
    pid_t pid = (pid_t) -1;
    char feedback[SPAWN_FEEDBACK_SIZE] = "";

    if (pipe(fb_fd) != 0) {
       ndmLog_error("can't create an execution feedback pipe.");
    } else {
        int flags;

        if (((flags = fcntl(fb_fd[1], F_GETFD)) == -1) ||
            (fcntl(fb_fd[1], F_SETFD, flags | FD_CLOEXEC) == -1))
        {
            ndmLog_error("can't initialize a feedback pipe.");
        } else
        if ((pid = fork()) < 0) {
            ndmLog_error("can't fork a process.");
        } else 
        if (pid == 0) {
            // Make sure all opened descriptors are closed,
            // except STDIO ones and fb_fd[1] which 
            // will be closed automatically.

            int fd = sysconf(_SC_OPEN_MAX) - 1;
            const int first_fd = (mode == STDIO_ENABLED) ? STDERR_FILENO : 0;

            while (fd > first_fd) {
                if (fd != fb_fd[1]) {
                    close(fd);
                }

                --fd;
            }

            sigset_t set;

            // Unmask all signals, detach and execute.

            if ((sigfillset(&set) == -1) ||
                (pthread_sigmask(SIG_UNBLOCK, &set, NULL) == -1))
            {
                snprintf(feedback, sizeof(feedback),
                    "%s", "unable to unmask signals");
            } else
            if (setsid() == -1) {
                snprintf(feedback, sizeof(feedback),
                    "%s", "unable to create a new session");
            } else {
                extern char **environ;
                char **self_environ = environ;

                if (envp != NULL) {
                    environ = (char **) envp;
                }

                execvp(argv[0], (char *const *)(&argv[0]));
                environ = self_environ;
                snprintf(feedback, sizeof(feedback),
                    "%s", "could not execute");
            }

            ssize_t n = 0;
            ssize_t left = sizeof(feedback);

            while (n >= 0 && left > 0) {
                n = write(fb_fd[1], feedback + 
                    sizeof(feedback) - left, left);
                left -= n;
            }

            // Terminate a child without a successfull execvp call.

            _exit(EXIT_FAILURE);
        } else {
            close(fb_fd[1]);

            ssize_t n;
            ssize_t left = sizeof(feedback);

            do {
                n = read(fb_fd[0], feedback +
                    sizeof(feedback) - left, left);
                left -= n;
            } while (n > 0 && left > 0);

            close(fb_fd[0]);

            if (n != 0) {
                if (n < 0) {
                    ndmLog_error("can't read execution feedback.");
                } else {
                   ndmLog_error("can't start a process (%s).",
                        feedback);
                }

                // Stop a broken children with
                // a nonexecuted external process,
                // ignore any error.

                kill(pid, SIGKILL);
                waitpid(pid, NULL, 0);
            }
        }
    }

    return pid;
}

void ndmFeedback(
        const char *const executable,
        const char *const env_vars,
        const int env_length)
{
    assert (executable != NULL);
    assert (env_vars != NULL);

    // no any error handling here

    if (env_length >= 0) {
        const char *argv[] = {
            executable,
            NULL
        };
        const char *envp[FEEDBACK_MAX_ENV_COUNT + 1];
        const char *s = env_vars;
        const char *e = env_vars;
        size_t i = 0;

        while (
            e < env_vars + env_length &&
            i < FEEDBACK_MAX_ENV_COUNT)
        {
            if (*e == '\0') {
                envp[i++] = s;
                s = e + 1;
            }

            ++e;
        }

        envp[i] = NULL;

        pid_t pid = ndmFeedback_spawn(argv, envp, STDIO_ENABLED);

        if (pid != INVALID_PID) {
            waitpid(pid, NULL, 0);
        }
    }
}

void ndmFeedback_multienv(
        const char *const executable,
        char *args,
        const int arg_length)
{
    char *p = args;
    char *e = args + arg_length;

    while (p < e) {
        if (*p == '\n') {
            *p = '\0';
        }

        ++p;
    }

   ndmFeedback(executable, args, arg_length);
}





