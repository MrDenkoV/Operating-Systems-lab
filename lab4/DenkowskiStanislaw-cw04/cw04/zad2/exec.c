#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <wait.h>

int exec=0;

int main(int argc, char *argv[]){
    if(strcmp("other", argv[1]) == 0){
        raise(SIGUSR1);
    }
    else if(strcmp("mask", argv[1]) == 0){
        raise(SIGUSR1);
        sigset_t syg;
        sigpending(&syg);
        printf("Is %d seeing SIGUSR1? %c\n", getpid(), sigismember(&syg, SIGUSR1) == 0 ? 'N' : 'Y');
    }
    else{
        sigset_t syg;
        sigpending(&syg);
        printf("Is %d seeing SIGUSR1? %c\n", getpid(), sigismember(&syg, SIGUSR1) == 0 ? 'N' : 'Y');
    }
    printf("\n");
    return 0;
}
