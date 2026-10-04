#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* Print the time between the child's start and its completion. */
static void report_elapsed_time(const struct timeval *start,
                                const struct timeval *end)
{
    double elapsed = (double)(end->tv_sec - start->tv_sec) +
                     (double)(end->tv_usec - start->tv_usec) / 1000000.0;
    printf("Elapsed time: %.6f seconds\n", elapsed);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* main's argv is NULL-terminated, so argv + 1 is ready for execvp. */
    char **command_argv = argv + 1;
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return EXIT_FAILURE;
    }

    if (child_pid == 0) {
        close(pipe_fd[0]);

        struct timeval start;
        if (gettimeofday(&start, NULL) == -1) {
            perror("gettimeofday");
            _exit(EXIT_FAILURE);
        }

        ssize_t written = write(pipe_fd[1], &start, sizeof(start));
        if (written == -1) {
            perror("write");
            _exit(EXIT_FAILURE);
        }
        if (written != sizeof(start)) {
            fprintf(stderr, "write: incomplete start timestamp\n");
            _exit(EXIT_FAILURE);
        }
        close(pipe_fd[1]);

        execvp(command_argv[0], command_argv);
        perror("execvp");
        _exit(127);
    }

    close(pipe_fd[1]);
    int child_status;
    pid_t waited_pid;
    do {
        waited_pid = waitpid(child_pid, &child_status, 0);
    } while (waited_pid == -1 && errno == EINTR);
    if (waited_pid == -1) {
        perror("waitpid");
        close(pipe_fd[0]);
        return EXIT_FAILURE;
    }

    /* Capture the end immediately after the child finishes. */
    struct timeval end;
    if (gettimeofday(&end, NULL) == -1) {
        perror("gettimeofday");
        close(pipe_fd[0]);
        return EXIT_FAILURE;
    }

    struct timeval start;
    size_t received = 0;
    while (received < sizeof(start)) {
        ssize_t count = read(pipe_fd[0], (char *)&start + received,
                             sizeof(start) - received);
        if (count == -1 && errno == EINTR) {
            continue;
        }
        if (count == -1) {
            perror("read");
            close(pipe_fd[0]);
            return EXIT_FAILURE;
        }
        if (count == 0) {
            fprintf(stderr, "read: missing start timestamp\n");
            close(pipe_fd[0]);
            return EXIT_FAILURE;
        }
        received += (size_t)count;
    }
    close(pipe_fd[0]);

    report_elapsed_time(&start, &end);
    if (WIFEXITED(child_status)) {
        return WEXITSTATUS(child_status);
    }
    if (WIFSIGNALED(child_status)) {
        return 128 + WTERMSIG(child_status);
    }
    return EXIT_FAILURE;
}
