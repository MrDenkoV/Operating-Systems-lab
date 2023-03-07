#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void sighandler(int sig, siginfo_t* info, void* context){
    printf("Received: %s\n", (char*) info->si_value.sival_ptr);
}

int main(int argc, char* argv[]) {

    if(argc != 3){
        printf("Not a suitable number of program parameters\n");
        return 1;
    }

    struct sigaction action;
    action.sa_sigaction = &sighandler;


    //..........
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO;


    int child = fork();
    if(child == 0) {
        //zablokuj wszystkie sygnaly za wyjatkiem SIGUSR1
        //zdefiniuj obsluge SIGUSR1 w taki sposob zeby proces potomny wydrukowal
        //na konsole przekazana przez rodzica wraz z sygnalem SIGUSR1 wartosc
        sigset_t sigs;
        sigfillset(&sigs);
        sigdelset(&sigs, SIGUSR1);
        sigprocmask(SIG_BLOCK, &sigs, NULL);
        sigaction(SIGUSR1, &action, NULL);
        pause();
    }
    else {
        //wyslij do procesu potomnego sygnal przekazany jako argv[2]
        //wraz z wartoscia przekazana jako argv[1]
        sleep(1);
        union sigval val;
        val.sival_ptr = argv[1];
        sigqueue(child, atoi(argv[2]), val);
    }

    return 0;
}
