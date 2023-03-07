#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include <dlfcn.h>
#include "libcomp.h"

void writetimes(FILE* file, clock_t realend, clock_t realstart, struct tms* end, struct tms* start){
    printf("REAL: %lf\n", (double)difftime(realstart, realend)/((double)sysconf(_SC_CLK_TCK)));
    printf("USER: %lf\n", (double)difftime(start->tms_utime, end->tms_utime)/((double)sysconf(_SC_CLK_TCK)));
    printf("SYS: %lf\n", (double)difftime(start->tms_stime, end->tms_stime)/((double)sysconf(_SC_CLK_TCK)));

    fprintf(file, "REAL: %lf\n", (double)difftime(realstart, realend)/((double)sysconf(_SC_CLK_TCK)));
    fprintf(file, "USER: %lf\n", (double)difftime(start->tms_utime, end->tms_utime)/((double)sysconf(_SC_CLK_TCK)));
    fprintf(file, "SYS: %lf\n", (double)difftime(start->tms_stime, end->tms_stime)/((double)sysconf(_SC_CLK_TCK)));
}

typedef void *(*arbitrary)();


int main(int argc, char *argv[]){
    void *lib = dlopen("./liblibcomp.so", RTLD_LAZY);
    if(!lib)
        printf("No library!\n");
    else if(argc<2){
        printf("At least initialise array\n");
    }
    else{
        struct tms* start=(struct tms*) calloc(1, sizeof(struct tms));
        struct tms* end=(struct tms*) calloc(1, sizeof(struct tms));
        clock_t  realstart=0;
        clock_t  realend=0;
        FILE *file = fopen("raport3a.txt", "a");
        fprintf(file, "\tNEW EXECUTION\n");
        
        void (*clear)() = (void (*)())dlsym(lib, "clear");
        void (*delop)() = (void (*)())dlsym(lib, "delop");
        void (*delblock)() = (void (*)())dlsym(lib, "delblock");
        arbitrary createTable;
        *(void **) (&createTable) = dlsym(lib, "createTable");
        arbitrary createBlocks;
        *(void **) (&createBlocks) = dlsym(lib, "createBlocks");
        // struct Blocks* (*createTable)() = dlsym(lib, "createTabe");
        // int (*createBlocks)() = (int (*)())dlsym(lib, "createBlocks");

        int size = atoi(argv[1]);
        // struct files *files;
        struct Blocks *blocks = createTable(size);
        for(int i=2; i<argc; i++){
            fprintf(file, "Times for arg: %s\n", argv[i]);
            printf("Times for arg: %s\n", argv[i]);
            realstart = times(start);

            if(strcmp(argv[i], "remove_block")==0){
                i++;
                if(argc==i)
                    printf("You need to specify an argument for %s\n", argv[i-1]);
                else
                    delblock(blocks, atoi(argv[i]));
            }
            else if(strcmp(argv[i], "remove_operation")==0){
                i+=2;
                if(argc<=i)
                    printf("You need to specify two arguments for %s\n", argv[i-2]);
                else
                    delop(blocks, atoi(argv[i-1]), atoi(argv[i]));
            }
            else if(strcmp(argv[i], "compare_pairs")==0){
                i++;
                if(argc==i){
                    printf("You need to specify an amount and the right number of arguments for %s\n", argv[i-1]);
                    break;
                }
                else{
                    int j=atoi(argv[i]);
                    if(argc<=i+j)
                        printf("You need to have at least %d arguments for %s\n", j, argv[i-1]);
                    else{
                        for(i=i+1; --j>0; i++){
                            createBlocks(blocks, argv[i]);
                        }
                    }
                }
            }
            else
            {
                printf("Incorect argument or invalid number of arguments %s\n", argv[i]);
            }
            
            realend = times(end);

            writetimes(file, realstart, realend, start, end);
            fprintf(file, "--------------------------------------------\n");
        }
        clear(blocks, NULL);
        fprintf(file, "\n\n");
        fclose(file);
    }
    dlclose(lib);
    return 0;
}
