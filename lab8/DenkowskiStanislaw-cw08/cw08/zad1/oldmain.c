#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

int **image;
int hist[256];
int W, H, M;
int threads;

pthread_t* ids;

void readMatrix(char* name){
    FILE* fp = fopen(name, "r");
    fscanf(fp, "%*[^\n]\n");    //P2
    fscanf(fp, "%*[^\n]\n");    //# credits
    fscanf(fp, "%d%d%d", &W, &H, &M);
    image = (int**) calloc(W, sizeof(int *));
    for(int i=0; i<W; i++){
        image[i] = (int*) calloc(H, sizeof(int));
        for(int j=0; j<H; j++)
            fscanf(fp, "%d", &image[i][j]);
    }
    fclose(fp);
}

void clean(){
    for(int i=0; i<W; i++){
        free(image[i]);
    }
    free(image);
    free(ids);
}

void* countnumbers(void* arg){
    struct timeval start;
    struct timeval end;
    gettimeofday(&start,NULL);

    intptr_t id = (intptr_t) arg;
    for(int i=0; i<W; i++){
        for(int j=0; j<H; j++){
            if(image[i][j]%threads==(int)id)
                hist[image[i][j]]++;
        }
    }

    gettimeofday(&end,NULL);

    unsigned long res = ((1000000 * end.tv_sec + end.tv_usec) - (1000000 * end.tv_sec + start.tv_usec));

    return (void *) res;
}

void countcol(int row){
    for(int i=0; i<H; i++)
        hist[image[row][i]]++;
}

void* countblock(void* arg){
    struct timeval start;
    struct timeval end;
    gettimeofday(&start,NULL);

    intptr_t id = (intptr_t) arg;
    for(int i=(int)id*((W+threads-1)/threads); i<(id+1)*((W+threads-1)/threads); i++)
        countcol(i);

    gettimeofday(&end,NULL);

    unsigned long res = ((1000000 * end.tv_sec + end.tv_usec) - (1000000 * end.tv_sec + start.tv_usec));

    return (void *) res;
}

void* countinter(void* arg){
    struct timeval start;
    struct timeval end;
    gettimeofday(&start,NULL);

    intptr_t id = (intptr_t) arg;
    for(int i=(int)id; i<W; i+=threads)
        countcol(i);

    gettimeofday(&end,NULL);

    unsigned long res = ((1000000 * end.tv_sec + end.tv_usec) - (1000000 * end.tv_sec + start.tv_usec));

    return (void *) res;
}

void createThreads(void* (*method)(void*)){
    intptr_t tmp;
    for(int i=0; i<threads; i++) {
        tmp = (intptr_t) i;
        pthread_create(&ids[i], NULL, method, (void*) tmp);
    }
}

void writehist(char* name){
    FILE* fp = fopen(name, "w");
    for(int i=0; i<256; i++){
        fprintf(fp, "%d: %d\n", i, hist[i]);
    }
    fclose(fp);
}

void timeThreads(){
    unsigned long sum=0;
    void *status;
    for(int i=0; i<threads; i++){
        pthread_join(ids[i], &status);
        printf("Thread %d %lu: %ld\n", i, ids[i], (long) status);
        sum += (unsigned long) status;
    }
    printf("Sum of times: %ld\n", sum);
}

int main(int argc, char* argv[]){
    if(argc!=5){
        printf("Program requires 5 args!\n");
        return 0;
    }
    for(int i=0; i<256; i++)
        hist[i]=0;
    threads = atoi(argv[1]);
    ids = (pthread_t*) calloc(threads, sizeof(pthread_t));
    readMatrix(argv[3]);
    if(strcmp(argv[2], "sign")==0)
        createThreads(&countnumbers);
    else if(strcmp(argv[2], "block")==0)
        createThreads(&countblock);
    else
        createThreads(&countinter);
    timeThreads();
    clean();
    writehist(argv[4]);
    return 0;
}