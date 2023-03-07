#include <stdio.h>
#include <stdlib.h>
#include "libcomp.h"


int main() {
    char in[] = "a.txt:b.txt c.txt:d.txt e.txt:f.txt";
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
    system("rm -f .c123321123321.txt");
    print(blocks);
    delop(blocks, 0, 1);
    delblock(blocks, 2);
    print(blocks);
    clear(blocks, files);
    return 0;
}
