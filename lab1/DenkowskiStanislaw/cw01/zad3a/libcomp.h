#ifndef _LIBCOMP_H
#define _LIBCOMP_H

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

struct files* seq(char* in);
int compare(char* fstfile, char* sndfile);
void fill(struct Operations* operation);
int createBlocks(struct Blocks* blocks, char* in);
int countops(struct Operations* block);
void delop(struct Blocks* blocks, int bix, int ix);
void delblock(struct Blocks* blocks, int ix);
struct Blocks * createTable(int size);
void print(struct Blocks* blocks);
void clear(struct Blocks* blocks, struct files* files);

#endif //LIBCOMP_H