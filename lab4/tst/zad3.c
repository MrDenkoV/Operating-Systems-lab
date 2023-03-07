#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <wait.h>

void handleSegFPE(int sig, siginfo_t* info, void* more){
    printf("1/0\n");
    printf("Timer overrun count: %d\n", info->si_overrun);
    printf("Signal value: %d\n", info->si_value);
    printf("Exit value or signal: %d\n", info->si_status);
    exit(0);
}

void handleSegABRT(){
    printf("ABRT\n");
    printf("User time consumed: %d\n", (int) info->si_utime);
    printf("System time consumed: %d\n", (int) info->si_stime);
    printf("File descriptor: %d\n", info->si_fd);
    exit(0);
}

int main(int argc, char *argv[]){
    struct sigaction segFPE;
    segFPE.sa_sigaction = handleSegFPE;
    segFPE.sa_flags = SA_SIGINFO;
    sigemptyset(&segFPE.sa_mask);
    sigaction(SIGFPE, &segFPE, NULL);

    struct sigaction segABRT;
    segABRT.sa_sigaction = handleSegABRT;
    segABRT.sa_flags = SA_SIGINFO;
    sigemptyset(&segABRT.sa_mask);
    sigaction(SIGABRT, &segABRT, NULL);

    pid_t child = fork();
    int x;
    if(child == 0)
        x = (1 / 0);

    child = fork();
    if(child==0)
        abort();

    return 0;
}
