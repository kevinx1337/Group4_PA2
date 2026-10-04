# Group4_PA2

## Command and error-handling standard (Person 3)

Run either program as `./time_shm <command> [args...]` or
`./time_pipe <command> [args...]`. Both reject a missing command with a
usage message and a nonzero exit status. The command always comes from
`argv`; neither source file hard-codes one.

In `main(int argc, char *argv[])`, `argv + 1` points to the command and all
of its arguments. The C runtime already puts a `NULL` after the final
argument, so this array can go directly to
`execvp(command_argv[0], command_argv)`.

Both programs check `fork()` and `gettimeofday()`. The shared-memory
version checks `mmap()` and `munmap()`; the pipe version checks `pipe()`,
`write()`, and `read()`. A failed `execvp()` reports the error and exits
the child with status 127. The parent waits for the child and returns its
exit status. Brief comments identify the command array and the timing/IPC
steps.

Both files compiled with `cc -std=c11 -Wall -Wextra -Werror` on macOS and
passed local checks for missing commands, arguments, a nonexistent command,
child exit status, and `sleep 1`. The team still needs to run the final
Linux build and capture the two required output files before submission.
