#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>
#include "shared.h"

pid_t* kids;
int sem_id;
int mem_id;
int* shared;

void intHandler(int sig){
    shmdt(shared);
    exit(0);
}

void setup(){
    srand(time(NULL));
    sem_id = semget(ftok(getenv("HOME"), '8'), 0, 0);
    mem_id = shmget(ftok(getenv("HOME"), '9'), 0, 0);
    shared = (int*) shmat(mem_id, NULL, 0);
}

void setbuffers(struct sembuf* buff, int num, int op, int flag){
    buff->sem_num=num;
    buff->sem_op=op;
    buff->sem_flg=flag;
}

int main(){
    setup();
    signal(SIGINT, intHandler);

    struct sembuf *init=(struct sembuf*) calloc(3, sizeof(struct sembuf));
    struct sembuf *end=(struct sembuf*) calloc(2, sizeof(struct sembuf));
    
    setbuffers(&init[0], FREEAR, 0, 0);
    setbuffers(&init[1], FREEAR, 1, 0);
    setbuffers(&init[2], FREEIX, 1, 0);
    setbuffers(&end[0], FREEAR, -1, 0);
    setbuffers(&end[1], PACKNR, 1, 0);

    int ix;
    while(1){
        if(semctl(sem_id, PACKNR, GETVAL, NULL) + semctl(sem_id, SENDNR, GETVAL, NULL) < PROD){
            semop(sem_id, init, 3);

            ix = semctl(sem_id, FREEIX, GETVAL, NULL);
            ix = (ix - 1 + PROD) % PROD;
            
            shared[ix] = rand()%8+3;

            int prep = semctl(sem_id, PACKNR, GETVAL, NULL)+1;
            int send = semctl(sem_id, SENDNR, GETVAL, NULL);

            printf("%d %ld Dodałem liczbę: %d. Liczba zamównień do przygotowania: %d. Liczba zamównień do wysłania: %d.\n",
                    getpid(), time(NULL), shared[ix], prep, send);

            semop(sem_id, end, 2);
        }
        sleep(rand()%4);
    }
    return 0;
}