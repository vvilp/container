#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char *argv[]){
    printf("%s\n",argv[1]);
    printf("%s\n",argv[2]);
    pivot_root(argv[1],argv[2]);
    perror(NULL);
    return 1;
}


