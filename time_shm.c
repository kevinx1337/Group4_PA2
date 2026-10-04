#define _DEFAULT_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#if defined(MAP_ANONYMOUS)
#define SHARED_MAP_ANONYMOUS MAP_ANONYMOUS
#else
#define SHARED_MAP_ANONYMOUS MAP_ANON
#endif

/* Measures and prints the elapsed time from the child's start timestamp. */
static int report_elapsed_time(const struct timeval *start)
{
    struct timeval end;

    if (start->tv_sec < 0) {
        fprintf(stderr, "shared memory: missing start timestamp\n");
        return -1;
    }

    if (gettimeofday(&end, NULL) == -1) {
        perror("gettimeofday");
        return -1;
    }

    double elapsed = (double)(end.tv_sec - start->tv_sec) +
                     (double)(end.tv_usec - start->tv_usec) / 1000000.0;
    printf("Elapsed time: %.6f seconds\n", elapsed);
    return 0;
}

/* Shares a start timestamp with a child, executes its command, then cleans up. */
int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* main's argv is NULL-terminated, so argv + 1 is ready for execvp. */
    char **command_argv = argv + 1;

    struct timeval *start = mmap(NULL, sizeof(*start), PROT_READ | PROT_WRITE,
                                 MAP_SHARED | SHARED_MAP_ANONYMOUS, -1, 0);
    if (start == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }
    start->tv_sec = -1;

    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        if (munmap(start, sizeof(*start)) == -1) {
            perror("munmap");
        }
        return EXIT_FAILURE;
    }

    if (child_pid == 0) {
        struct timeval child_start;
        if (gettimeofday(&child_start, NULL) == -1) {
            perror("gettimeofday");
            _exit(EXIT_FAILURE);
        }
        *start = child_start;

        execvp(command_argv[0], command_argv);
        perror("execvp");
        _exit(127);
    }

    int child_status;
    pid_t waited_pid;
    do {
        waited_pid = waitpid(child_pid, &child_status, 0);
    } while (waited_pid == -1 && errno == EINTR);

    if (waited_pid == -1) {
        perror("waitpid");
        if (munmap(start, sizeof(*start)) == -1) {
            perror("munmap");
        }
        return EXIT_FAILURE;
    }

    int timing_result = report_elapsed_time(start);

    if (munmap(start, sizeof(*start)) == -1) {
        perror("munmap");
        return EXIT_FAILURE;
    }

    if (timing_result == -1) {
        return EXIT_FAILURE;
    }

    if (WIFEXITED(child_status)) {
        return WEXITSTATUS(child_status);
    }
    if (WIFSIGNALED(child_status)) {
        return 128 + WTERMSIG(child_status);
    }

    return EXIT_FAILURE;
}
