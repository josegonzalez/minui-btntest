#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "btntest.h"

// the pid of the command currently being run, or 0 if none is running
static volatile pid_t child_pid = 0;

// the exit code to use after a signal was forwarded to the running command
static volatile sig_atomic_t pending_exit_code = 0;

static int signal_exit_code(int signal)
{
    if (signal == SIGINT)
    {
        return ExitCodeKeyboardInterrupt;
    }
    else if (signal == SIGTERM)
    {
        return ExitCodeSigterm;
    }

    return ExitCodeError;
}

void signal_handler(int signal)
{
    // forward the signal to a running command and let run_command
    // return once it exits so that the caller can clean up
    if (child_pid > 0)
    {
        pending_exit_code = signal_exit_code(signal);
        kill(child_pid, signal);
        return;
    }

    // ctrl+c exits with code 130, sigterm with code 143
    exit(signal_exit_code(signal));
}

void install_signal_handlers(void)
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = signal_handler;
    sigemptyset(&action.sa_mask);

    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
}

int pending_signal_exit_code(void)
{
    return pending_exit_code;
}

// run_command runs the command and waits for it to exit
// returns the exit code of the command, or 128 + the signal number
// if the command was terminated by a signal
int run_command(char *const argv[])
{
    fflush(stdout);
    fflush(stderr);

    // block signals until the child pid is recorded so that
    // a signal arriving in between is forwarded instead of
    // exiting and leaving the command running
    sigset_t block_mask, original_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGINT);
    sigaddset(&block_mask, SIGTERM);
    sigprocmask(SIG_BLOCK, &block_mask, &original_mask);

    pid_t pid = fork();
    if (pid < 0)
    {
        sigprocmask(SIG_SETMASK, &original_mask, NULL);

        char buff[256];
        snprintf(buff, sizeof(buff), "failed to run %s: %s", argv[0], strerror(errno));
        log_error(buff);
        return ExitCodeError;
    }

    if (pid == 0)
    {
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        sigprocmask(SIG_SETMASK, &original_mask, NULL);

        execvp(argv[0], argv);

        // only reached if the command could not be executed
        int exec_errno = errno;
        char buff[256];
        snprintf(buff, sizeof(buff), "failed to execute %s: %s", argv[0], strerror(exec_errno));
        log_error(buff);
        _exit(exec_errno == ENOENT ? 127 : 126);
    }

    child_pid = pid;
    sigprocmask(SIG_SETMASK, &original_mask, NULL);

    int status = 0;
    pid_t result;
    do
    {
        result = waitpid(pid, &status, 0);
    } while (result < 0 && errno == EINTR);

    child_pid = 0;

    if (result < 0)
    {
        char buff[256];
        snprintf(buff, sizeof(buff), "failed to wait for %s: %s", argv[0], strerror(errno));
        log_error(buff);
        return ExitCodeError;
    }

    if (WIFSIGNALED(status))
    {
        return 128 + WTERMSIG(status);
    }

    return WEXITSTATUS(status);
}
