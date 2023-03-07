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

sem_t* sems[SEMSNR];
int mem_desc;
int* shared;

void intHandler(int sig){
    for(int i=0; i<SEMSNR; i++)
        sem_close(sems[i]);
    munmap(shared, PROD*sizeof(int));
    exit(0);
}

void setup(){
    srand(time(NULL));
    for(int i=0; i<SEMSNR; i++)
        sems[i] = sem_open(NAMES[i], O_RDWR);

    mem_desc = shm_open(MEM, O_RDWR, S_IRWXG | S_IRWXO | S_IRWXU);
    shared = mmap(NULL, PROD*sizeof(int), PROT_WRITE | PROT_READ, MAP_SHARED, mem_desc, 0);
}

int main(){
    setup();
    signal(SIGINT, intHandler);

    int ix, val, val1;
    while(1){
        sem_getvalue(sems[PACKNR], &val);
        sem_getvalue(sems[SENDNR], &val1);
        if(val+val1<PROD){
            sem_wait(sems[FREEAR]);
            sem_post(sems[FREEIX]);

            sem_getvalue(sems[FREEIX], &val);
            ix = (val - 1 + PROD) % PROD;
            
            shared[ix] = rand()%8+3;

            sem_getvalue(sems[PACKNR], &val);
            int prep = val+1;
            sem_getvalue(sems[SENDNR], &val);
            int send = val;

            printf("%d %ld Dodałem liczbę: %d. Liczba zamównień do przygotowania: %d. Liczba zamównień do wysłania: %d.\n",
                    getpid(), time(NULL), shared[ix], prep, send);

            sem_post(sems[FREEAR]);
            sem_post(sems[PACKNR]);
        }
        sleep(rand()%2+1);
    }
    return 0;
}