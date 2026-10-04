#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

/* Measures and prints the elapsed time from the child's start timestamp. */
static void report_elapsed_time(const struct timeval *start)
{
  struct timeval end;

  if (start->tv_sec < 0) {
    return;
  }

  if (gettimeofday(&end, NULL) == -1) {
    perror("gettimeofday");
    return;
  }

  double elapsed = (double)(end.tv_sec - start->tv_sec) +
                  (double)(end.tv_usec - start->tv_usec) / 1000000.0;
  printf("Elapsed time: %.6f seconds\n", elapsed);
}

int main(int argc, char *argv[]){

/*person 3 validate command line arg

*/

  if (pipe(pipe_fd) == -1) {//create pipe before firk
    perror("pipe failed");
    return 1;}

  pid_t pid = fork(); //fork process

  if (pid < 0) {
// person 3 error handling
  }
  else if(pid == 0) {
    //CHILD PROCESS

    close(pipe_fd[0]); //close unused end of pipe

    if(gettimeofday(&start_time, NULL) == -1){
      perror("gettimeofday failed in child");
      _exit(1);}

    if (write(pipe_fd[1], &start_time, sizeof(struct timeval)) == -1) { //write struct timeval to pipe
          perror("write to pipe failed");
          _exit(1);}

    close(pipe_fd[1]);//close the end

    //person 3 call execvp() with argv
    }
    else{
      // PARENT PPROCESS

      close(pipe_fd[1]); //close unused end of pipe

      int status; //waiting for child
      if (waitpid(pid,&status,0) == -1){
        perror("Read from pipe failed");
        return 1;}

      close(pipe_fd[0]);//closing end

//person 4 (get start timestamp, close pipe end, get end timestamp, timing calc, output)

      if (read(pipe_fd[0], &start_time, sizeof(struct timeval)) != sizeof(struct timeval)) {
        perror("Read from pipe failed");
        close(pipe_fd[0]);
        return 1;}

      close(pipe_fd[0]); //done reading, close the read end

      report_elapsed_time(&start_time);

  return 0;
}
  

  
