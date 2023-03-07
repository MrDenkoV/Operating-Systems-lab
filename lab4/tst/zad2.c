#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <wait.h>

int exec=0, masked=0;

void print(int signum){
    printf("PID: %d got SIGUSR1 signal\n", getpid());
}

void msk(){
    masked=1;
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask, NULL);
}

int main(int argc, char *argv[]){
    if(argc != 2){
        printf("1 argument is required");
        return 0;
    }
    if(strcmp(argv[1], "handler")==0)
        signal(SIGUSR1, print);
    else if(strcmp(argv[1], "ignore")==0)
        signal(SIGUSR1, SIG_IGN);
    else
        msk();

    printf("Parent: %d\n", getpid());
    raise(SIGUSR1);
    sigset_t syg;
    if(masked==1) {
        sigpending(&syg);
        printf("Is %d seeing SIGUSR1? %c\n", getpid(), sigismember(&syg, SIGUSR1) == 0 ? 'N' : 'Y');
    }
    pid_t child = fork();
    if(child==0){
        if(strcmp(argv[1], "pending") != 0)
            raise(SIGUSR1);
        if(masked==1) {
            sigpending(&syg);
            printf("Is %d seeing SIGUSR1? %c\n", getpid(), sigismember(&syg, SIGUSR1) == 0 ? 'N' : 'Y');
        }
    } else
        waitpid(child, NULL, 0);
    return 0;
}

