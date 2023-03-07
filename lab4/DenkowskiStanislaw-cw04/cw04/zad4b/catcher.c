#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <wait.h>

int caught=0;
int fin=0;
int waiting=0;
pid_t pid;

void handleusr1(int sig, siginfo_t* info, void* more){
    caught++;
    waiting=1;
    pid=info->si_pid;
}

void handleusr2(int sig, siginfo_t* info, void* more){
    fin=1;
    waiting=1;
    pid=info->si_pid;
}

int main(int argc, char *argv[]){
    printf("%d\n", getpid());

    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGUSR1);
    sigdelset(&mask, SIGUSR2);
    sigdelset(&mask, SIGRTMIN);
    sigdelset(&mask, SIGRTMAX);
    sigprocmask(SIG_BLOCK, &mask, NULL);


    struct sigaction usr1;
    usr1.sa_sigaction = handleusr1;
    usr1.sa_flags = SA_SIGINFO;
    // sigemptyset(&usr1.sa_mask);
    sigfillset(&usr1.sa_mask);

    sigaction(SIGUSR1, &usr1, NULL);
    sigaction(SIGRTMIN, &usr1, NULL);

    struct sigaction usr2;
    usr2.sa_sigaction = handleusr2;
    usr2.sa_flags = SA_SIGINFO;
    // sigemptyset(&usr2.sa_mask);
    sigfillset(&usr2.sa_mask);
    sigaction(SIGUSR2, &usr2, NULL);
    sigaction(SIGRTMAX, &usr2, NULL);
    
    while(fin!=1){
        while(waiting==0);
        waiting=0;
        kill(pid, SIGUSR1);
    }

    printf("Catcher caught %d\n", caught);

    for(int i=0; i<caught; i++)
        kill(pid, SIGUSR1);
    kill(pid, SIGUSR2);

    return 0;
}
