#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "shared.h"

pid_t* kids;

void clear(){
    for(int i=0; i<SEMSNR; i++)
        sem_unlink(NAMES[i]);
    shm_unlink(MEM);
    free(kids);
}

void intHandler(int sig){
    for(int i=0; i<ALL; i++)
        kill(kids[i], SIGINT);
    clear();
    exit(0);
}

void setup(){
    sem_t *sem;
    for(int i=0; i<SEMSNR; i++){
        sem = sem_open(NAMES[i], O_CREAT | O_RDWR, S_IRWXG | S_IRWXO | S_IRWXU, (i==0 ? 1 : 0));
        sem_close(sem);
    }

    kids = calloc(ALL, sizeof(pid_t));

    int shared = shm_open(MEM, O_CREAT | O_RDWR, S_IRWXG | S_IRWXO | S_IRWXU);
    ftruncate(shared, PROD*sizeof(int));
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

    while(1);

    clear();
    return 0;
}