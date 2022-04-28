#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <stdio.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#define errExit(msg) do { perror(msg); exit(EXIT_FAILURE);} while (0)
#define STACK_SIZE (1024 * 1024)
static char child_stack[STACK_SIZE];

int child_main(void* arg)
{
  char **argv = arg;
  execvp(argv[0], &argv[0]);
  errExit("execvp");	    
  return 1;
}

int main(int argc, char *argv[])
{
  int flags = 0;
  flags |= CLONE_NEWNS;
  flags |= CLONE_NEWNET;

  int child_pid = clone(child_main, child_stack+STACK_SIZE, flags | SIGCHLD, &argv[optind]);

  printf("Run : %s\n", argv[optind]);

  if (waitpid(child_pid, NULL, 0) == -1) {
    errExit("waitpid");
  }

  exit(EXIT_SUCCESS);
  return 0;
}
