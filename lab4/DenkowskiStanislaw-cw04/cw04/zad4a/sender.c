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

void handleusr1(int sig){
    caught++;
    waiting=1;
}

void handleusr2(int sig){
    printf("END\n");
    fin=1;
    waiting=1;
}

void send(pid_t pid, int n, char* mode){
    for(int i=0; i<n; i++){
        if(strcmp(mode, "kill")==0){
            kill(pid, SIGUSR1);
        }
        else if(strcmp(mode, "sigqueue")==0){
            union sigval val;
            val.sival_int=i;
            sigqueue(pid, SIGUSR1, val);
        }
        else{
            kill(pid, SIGRTMIN);
        }
    }
    if(strcmp(mode, "kill")==0){
        kill(pid, SIGUSR2);
    }
    else if(strcmp(mode, "sigqueue")==0){
        union sigval val;
        val.sival_int=n;
        sigqueue(pid, SIGUSR2, val);
    }
    else{
        kill(pid, SIGRTMAX);
    }
}

int main(int argc, char *argv[]){
    if(argc<4){
        printf("3 arguments required\n");
    }
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGUSR1);
    sigdelset(&mask, SIGUSR2);
    sigdelset(&mask, SIGRTMIN);
    sigdelset(&mask, SIGRTMAX);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    signal(SIGUSR1, handleusr1);
    signal(SIGUSR2, handleusr2);

    send((pid_t) atoi(argv[1]), atoi(argv[2]), argv[3]);

    while(fin==0){
        while(waiting==0);
        printf("%d\n", caught);
        waiting=0;
    }

    printf("Sent %d\nCaught %d\n", atoi(argv[2]), caught);

    return 0;
}
