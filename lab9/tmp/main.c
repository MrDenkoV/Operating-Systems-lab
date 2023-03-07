#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

int chairsNum, clientsNum;
int freeChairs;
int works;          // 0 - sleeps, 1 - works

pthread_t* chairs;
pthread_t current;

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t  mutex = PTHREAD_MUTEX_INITIALIZER;


void* barber(void* args){
    for(int i=0; i<clientsNum; i++){
        pthread_mutex_lock(&mutex);
        while(freeChairs==chairsNum){
            printf("Golibroda: ide spac\n");
            works=0;
            pthread_cond_wait(&cond, &mutex);
            works = 1;
        }
        current = chairs[freeChairs];
        freeChairs++;
        printf("Golibroda: czeka %d klientow, gole klienta %lu\n", chairsNum-freeChairs, current);

        pthread_mutex_unlock(&mutex);
        sleep(rand()%5 + 3);
    }
    return NULL;
}

void* client(void* args){
    pthread_t id = pthread_self();
    while(1){
        pthread_mutex_lock(&mutex);
        if(works==0){
            printf("Klient %lu: Budze golibrode\n", id);
            current = id;
            freeChairs--;
            chairs[freeChairs] = id;
            works=1;

            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&mutex);
            break;
        }
        else if(freeChairs>0){
            printf("Klient %lu: Poczekalnia, wolne miejsca: %d\n", id, freeChairs-1);
            freeChairs--;
            chairs[freeChairs] = id;

            pthread_mutex_unlock(&mutex);
            break;
        }
        else{
            printf("Klient %lu: Zajete\n", id);
            pthread_mutex_unlock(&mutex);
            sleep(rand()%3+3);
        }
    }
    return NULL;
}

int main(int argc, char* argv[]){
    if(argc!=3){
        printf("Program requires 2 args!\n");
        return 0;
    }
    srand(time(NULL));
    chairsNum = atoi(argv[1]);
    clientsNum = atoi(argv[2]);

    freeChairs = chairsNum;
    works = 0;
    chairs = calloc(chairsNum, sizeof(pthread_t));

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_t barb;
    pthread_create(&barb, NULL, barber, NULL);

    pthread_t *clients = (pthread_t*) calloc(clientsNum, sizeof(pthread_t));
    intptr_t arg;
    for(int i=0; i<clientsNum; i++){
        sleep(rand()%3+1);
        arg = (intptr_t) i+1;
        pthread_create(&clients[i], NULL, client, (void *) arg);
    }
    for(int i=0; i<clientsNum; i++) {
        pthread_join(clients[i], NULL);
    }
    pthread_join(barb, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    free(clients);
    free(chairs);
    return 0;
}