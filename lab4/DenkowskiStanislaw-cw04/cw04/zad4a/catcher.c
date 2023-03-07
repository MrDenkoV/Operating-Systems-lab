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
pid_t pid;
int waiting=0;

void handleusr1(int sig){
    caught++;
    waiting=1;
}

void handleusr2(int sig, siginfo_t* info, void* more){
    waiting=1;
    fin=1;
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

    signal(SIGUSR1, handleusr1);
    signal(SIGRTMIN, handleusr1);
    signal(SIGUSR1, handleusr1);

    struct sigaction usr2;
    usr2.sa_sigaction = handleusr2;
    usr2.sa_flags = SA_SIGINFO;
    sigemptyset(&usr2.sa_mask);
    // sigaddset(&usr2.sa_mask, sigusr1);
    sigaction(SIGUSR2, &usr2, NULL);
    sigaction(SIGRTMAX, &usr2, NULL);
    
    while(fin!=1){
        while(waiting==0);
        waiting=0;
    }
    
    printf("Received\n");

    for(int i=0; i<caught; i++)
        kill(pid, SIGUSR1);
    printf("sending end\n");
    kill(pid, SIGUSR2);

    return 0;
}
