#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define chunksize 256
#define maxsize 1000

struct op{
    int len;
    char *op;
};

struct Operations{
    int size;
    int empty;
    char *firstFile;
    char *secondFile;
    struct op **operations;
};

struct Blocks{
    struct Operations **blocks;
    int empty;
    int size;
};

struct files{
    char** files;
    int size;
};

struct files* seq(char* in){
    if(strlen(in)==0){
        printf("No files given\n");
        return NULL;
    }
    char* input = strdup(in);
    struct files* files = (struct files*) calloc(1, sizeof(struct files));
    int count=0;
    char *pch;
    pch = strtok (in," ");
    while (pch != NULL)
    {
        count++;
        pch = strtok (NULL, " ");
    }
    files->files = (char**) calloc(count, sizeof(char*));
    files->size = count;
    for(int i=0; i<files->size; i++){
        files->files[i] = (char*) calloc(chunksize, sizeof(char));
    }
    pch = strtok (input," ");
    count=0;
    while(pch != NULL){
        strcpy(files->files[count], pch);
        count++;
        pch = strtok (NULL, " ");
    }
    free(input);
    return files;
}

int compare(char* fstfile, char* sndfile){
    char tmp[60];
    sprintf(tmp, "diff %s %s >../.c123321123321.txt", fstfile, sndfile);
    system(tmp);

    FILE *fp = fopen("../.c123321123321.txt", "r");
    if(fp == NULL) {
        perror("Unable to create, find or open file!");
        exit(1);
    }

    int count=0;
    char chunk[chunksize];
    char* f;
    while(fgets(chunk, sizeof(chunk), fp) != NULL) {
        count++;
        do{
//            printf("ins\t");
//            fputs(chunk, stdout);
        }while((f = fgets(chunk, sizeof(chunk), fp)) != NULL && !(chunk[0]>='0' && chunk[0]<='9'));

        if(f==NULL)
            break;
//        fputs(chunk, stdout);
    }
    fclose(fp);
    return count;
}

void fill(struct Operations* operation){
    FILE *fp = fopen("../.c123321123321.txt", "r");
    if(fp == NULL) {
        perror("Unable to create, find or open file!");
        exit(1);
    }


    char chunk[chunksize];
    char* f;
    for(int i=0; i<operation->size; i++){
        operation->operations[i] = (struct op*) calloc(1, sizeof(struct op));
        operation->operations[i]->op = (char*) calloc(maxsize, sizeof(char));
        operation->operations[i]->len=0;
    }
    while(fgets(chunk, sizeof(chunk), fp) != NULL) {
        do{
//            printf("%s\n", operation->operations[operation->size - operation->empty]->op);
//            printf("ins\t");
//            fputs(chunk, stdout);
            memcpy(operation->operations[operation->size - operation->empty]->op + operation->operations[operation->size - operation->empty]->len, chunk, strlen(chunk)+1);
            operation->operations[operation->size - operation->empty]->len += (int)strlen(chunk);
        }while((f = fgets(chunk, sizeof(chunk), fp)) != NULL && !(chunk[0]>='0' && chunk[0]<='9'));

//        fputs(operation->operations[operation->size - operation->empty]->op, stdout);
        operation->empty--;
        if(f==NULL)
            break;
        memcpy(operation->operations[operation->size - operation->empty]->op + operation->operations[operation->size - operation->empty]->len, chunk, strlen(chunk));
//        fputs(operation->operations[operation->size - operation->empty]->op, stdout);
        operation->operations[operation->size - operation->empty]->len += (int)strlen(chunk);
//        printf("end\n\n");

        //        fputs(chunk, stdout);
    }
    fclose(fp);
}

int createBlocks(struct Blocks* blocks, char* in){
    if((blocks->empty) == 0)
        printf("Not enough space for the next block\n");
    int count=0;
    int ix=0;
    struct Operations *operation = (struct Operations*) calloc(1, sizeof(struct Operations));
    for(int i=0; i<blocks->size; i++){
        if(blocks->blocks[i]==NULL){
            blocks->blocks[i] = operation;
            blocks->empty--;
            ix=i;
            break;
        }
    }
//    blocks->blocks[blocks->size - blocks->empty] = (struct Operations*) calloc(1, sizeof(struct Operations));
//    struct Operations *operation = blocks->blocks[blocks->size - blocks->empty];
//    blocks->empty--;

    char *pch;
    pch = strtok (in,":");
    operation->firstFile = pch;
    pch = strtok (NULL, ":");
    operation->secondFile = pch;

//    operation->firstFile = "../a.txt";
//    operation->secondFile = "../b.txt";
    count = compare(operation->firstFile, operation->secondFile);
    operation->size=count;
    operation->empty=count;
    operation->operations = (struct op **) calloc(count, sizeof(struct op*));

    fill(operation);
    return ix;
}

int countops(struct Operations* block){
    return block->size-block->empty;
}

void delop(struct Blocks* blocks, int bix, int ix){
    if(blocks->size <= bix || bix < 0){
        printf("Wrong block index\n");
    }
    else if(blocks->blocks[bix] != NULL && blocks->blocks[bix]->operations[ix] != NULL){
        free(blocks->blocks[bix]->operations[ix]->op);
        free(blocks->blocks[bix]->operations[ix]);
        blocks->blocks[bix]->operations[ix]=NULL;
        blocks->blocks[bix]->empty++;
    }
}

void delblock(struct Blocks* blocks, int ix){
    if(blocks->size <= ix || ix < 0){
        printf("Wrong index\n");
    }
    else if(blocks->blocks[ix] != NULL){
        for(int i=0; i<blocks->blocks[ix]->size; i++){
            if(blocks->blocks[ix]->operations[i] != NULL){
                free(blocks->blocks[ix]->operations[i]->op);
                free(blocks->blocks[ix]->operations[i]);
            }
        }
        free(blocks->blocks[ix]->operations);
//        free(blocks->blocks[ix]->firstFile);
//        free(blocks->blocks[ix]->secondFile);
        free(blocks->blocks[ix]);
        blocks->blocks[ix]=NULL;
    }
    blocks->empty++;
}

struct Blocks * createTable(int size){
    struct Blocks *Array = (struct Blocks*) calloc(1, sizeof(struct Blocks));
    Array->empty = size;
    Array->size = size;
    Array->blocks = (struct Operations**) calloc((Array->size),  sizeof(struct Operations*));
    for(int i=0; i<Array->size; i++)
        Array->blocks[i] = NULL;
    return Array;
}

void print(struct Blocks* blocks){
    printf("Blocks: %d\tempty: %d\n", blocks->size, blocks->empty);
    for(int i=0; i<blocks->size; i++){
        printf("\tBlock: %d:\n", i);
        if(blocks->blocks[i]==NULL){
            printf("\t\tBlock is empty\n");
            continue;
        }
        printf("\t\tOperations: %d\tempty: %d\n", blocks->blocks[i]->size, blocks->blocks[i]->empty);
        for(int j=0; j<blocks->blocks[i]->size; j++){
            printf("\t\t\tOperation: %d\n", j);
            if(blocks->blocks[i]->operations[j]==NULL){
                printf("\t\t\tOperation is empty\n");
                continue;
            }
            printf("%s", blocks->blocks[i]->operations[j]->op);
        }
        printf("\n");
    }
    printf("\n");
}

void clear(struct Blocks* blocks, struct files* files){
    if(blocks != NULL) {
        for (int i = 0; i < blocks->size; i++) {
            if (blocks->blocks[i] != NULL) {
                for (int j = 0; j < blocks->blocks[i]->size; j++) {
                    if (blocks->blocks[i]->operations[j] != NULL) {
                        free(blocks->blocks[i]->operations[j]->op);
                        free(blocks->blocks[i]->operations[j]);
                    }
                }
                free(blocks->blocks[i]->operations);
                free(blocks->blocks[i]);
            }
        }
        free(blocks->blocks);
        free(blocks);
    }
    if(files != NULL){
        for(int i=0; i<files->size; i++){
            free(files->files[i]);
        }
        free(files->files);
        free(files);
    }
}

int main() {
    char in[] = "../a.txt:../b.txt ../c.txt:../d.txt ../e.txt:../f.txt";
//    char in[] = "a.txt:b.txt\n";

    struct Blocks *blocks = createTable(3);
    struct files* files = seq(in);
//    for(int i=0; i<files->size; i++){
//        printf("%lu %s\n", strlen(files->files[i]), files->files[i]);
//    }
    for(int i=0; i<files->size; i++){
        createBlocks(blocks, files->files[i]);
        printf("%s %s\n", blocks->blocks[i]->firstFile, blocks->blocks[i]->secondFile);
        printf("%d %d\n", blocks->blocks[i]->size, blocks->blocks[i]->empty);
    }
//    createBlocks(blocks, in);
//    char* a = "../a.txt";
//    char* b = "../b.txt";
//    printf("%d\n", compare(a, b));
    system("rm -f ../.c123321123321.txt");
    print(blocks);
    delop(blocks, 0, 1);
    delblock(blocks, 2);
    print(blocks);
    clear(blocks, files);
    return 0;
}
