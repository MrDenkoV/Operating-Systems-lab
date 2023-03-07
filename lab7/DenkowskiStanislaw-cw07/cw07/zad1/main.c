#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "shared.h"

pid_t* kids;
int sem_id;
int mem_id;

void clear(){
    semctl(sem_id, 0, IPC_RMID, NULL);
    shmctl(mem_id, IPC_RMID, NULL);
    free(kids);
}

void intHandler(int sig){
    for(int i=0; i<ALL; i++)
        kill(kids[i], SIGINT);
    clear();
    exit(0);
}

void setup(){
    sem_id = semget(ftok(getenv("HOME"), '8'), 6, IPC_CREAT | 0666);
    arg.val=0;
    semctl(sem_id, FREEAR, SETVAL, arg);
    semctl(sem_id, FREEIX, SETVAL, arg);
    semctl(sem_id, PACKIX, SETVAL, arg);
    semctl(sem_id, PACKNR, SETVAL, arg);
    semctl(sem_id, SENDIX, SETVAL, arg);
    semctl(sem_id, SENDNR, SETVAL, arg);

    kids = calloc(ALL, sizeof(pid_t));

    mem_id = shmget(ftok(getenv("HOME"), '9'), PROD*sizeof(int), IPC_CREAT|0666);
}

void create(char* name, int num, int alrd){
    for(int i=0; i<num; i++){
        pid_t child = fork();
        if(child==0)
            execlp(name, name, NULL);
        kids[alrd+i]=child;
    }
}

int main(){
    setup();
    signal(SIGINT, intHandler);

    create("./receiver.o", RECE, 0);
    create("./packer.o", PACK, RECE);
    create("./sender.o", SEND, RECE+PACK);

    for(int i=0; i<ALL; i++)
        wait(NULL);

    clear();
    return 0;
}