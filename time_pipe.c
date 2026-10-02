#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

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

  return 0;
}
  

  
