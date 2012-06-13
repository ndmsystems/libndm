#include  "ndm_common.h"

static pid_t __spawn(
        const char *const argv[],
        c Эonst char *const pty_device)
{
    int fb_fd[2];
    pid_t pid = INVALID_PID;
    char feedback[SPAWN_FEEDBACK_SIZE] = "";
    int flags;
    int fd = -1;
    int slave_pty = -1;
    sigset_t set;
    ssize_t n;
    ssize_t left;
    struct termios term;

    /* flush all opened streams */
    fflush(NULL);

    if (pipe(fb_fd) != 0) {
        __log(LERROR,
            "can't create an execution feedback pipe: %s.",
            __strerror(errno));
    } else
    if (((flags = fcntl(fb_fd[1], F_GETFD)) == -1) ||
        (fcntl(fb_fd[1], F_SETFD, flags | FD_CLOEXEC) == -1))
    {
        __log(LERROR,
            "can't initialize a feedback pipe: %s.", __strerror(errno));
    } else
    if ((pid = fork()) < 0) {
        __log(LERROR, "can't fork a process: %s.", __strerror(errno));
        pid = INVALID_PID;
    } else
    if (pid == 0) {
        fd = sysconf(_SC_OPEN_MAX) - 1;

        while (fd > STDERR_FILENO) {
            if (fd != fb_fd[1]) {
                close(fd);
            }

            --fd;
        }

        slave_pty = open(pty_device, O_RDWR);

        if (slave_pty < 0) {
            snprintf(feedback, sizeof(feedback),
                "failed to open a slave terminal device: %s",
                    __strerror(errno));
        } else
        if (setsid() == -1) {
            snprintf(feedback, sizeof(feedback),
                "unable to create a new session: %s", __strerror(errno));
        } else
        if (dup2(slave_pty, STDIN_FILENO) < 0 ||
            dup2(slave_pty, STDOUT_FILENO) < 0 ||
            dup2(slave_pty, STDERR_FILENO) < 0)
        {
            snprintf(feedback, sizeof(feedback),
                "unable to clone an I/O descriptor: %s", __strerror(errno));
        } else
        if ((sigfillset(&set) == -1) ||
            (sigprocmask(SIG_UNBLOCK, &set, NULL) == -1))
        {
            snprintf(feedback, sizeof(feedback),
                "unable to unmask signals: %s", __strerror(errno));
        } else
        if (tcgetattr(STDIN_FILENO, &term) != 0) {
            snprintf(feedback, sizeof(feedback),
                "failed to get terminal attributes: %s", __strerror(errno));
        } else {
            close(slave_pty);
            slave_pty = -1;

            term.c_lflag |= ECHO;
            term.c_oflag |= ONLCR | TAB3;
            term.c_iflag |= ICRNL;
            term.c_iflag &= ~IXOFF;

            tcsetattr(STDIN_FILENO, TCSANOW, &term);

            execvp(argv[0], (char *const *) &argv[0]);
            snprintf(feedback, sizeof(feedback),
                "could not execute: %s", __strerror(errno));
        }

        close(slave_pty);

        n = 0;
        left = sizeof(feedback);

        while (n >= 0 && left > 0) {
            n = write(fb_fd[1], feedback + sizeof(feedback) - left, left);
            left -= n;
        }

        /* terminate a child without a successfull execvp call */

        _exit(EXIT_FAILURE);
    } else {
        close(fb_fd[1]);

        left = sizeof(feedback);

        do {
            n = read(fb_fd[0], feedback + sizeof(feedback) - left, left);
            left -= n;
        } while (n > 0 && left > 0);

        close(fb_fd[0]);

        if (n != 0) {
            if (n < 0) {
                __log(LERROR, "can't read execution feedback.");
            } else {
                __log(LERROR, "can't start a \"%s\" process (%s).",
                    argv[0], feedback);
            }

            /* stop a broken child with a failed external process,
             * ignore any error */

            kill(pid, SIGKILL);
            waitpid(pid, NULL, 0);

            pid = INVALID_PID;
        }
    }

    return pid;
}
