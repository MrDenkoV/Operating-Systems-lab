#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
//#include "libcomp.h"

void writetimes(FILE* file, clock_t realstart, clockid_t realend, struct tms* start, struct tms* end){
    printf("REAL: %lf\n", (double)difftime(realstart, realend)/((double)sysconf(_SC_CLK_TCK)));
    printf("USER: %lf\n", (double)difftime(start->tms_utime, end->tms_utime)/((double)sysconf(_SC_CLK_TCK)));
    printf("SYS: %lf\n", (double)difftime(start->tms_stime, end->tms_stime)/((double)sysconf(_SC_CLK_TCK)));

    fprintf(file, "REAL: %lf\n", (double)difftime(realstart, realend)/((double)sysconf(_SC_CLK_TCK)));
    fprintf(file, "USER: %lf\n", (double)difftime(start->tms_utime, end->tms_utime)/((double)sysconf(_SC_CLK_TCK)));
    fprintf(file, "SYS: %lf\n", (double)difftime(start->tms_stime, end->tms_stime)/((double)sysconf(_SC_CLK_TCK)));
}

int main(int argc, char *argv[]){
    struct tms* start=(struct tms*) calloc(1, sizeof(struct tms));
    struct tms* end=(struct tms*) calloc(1, sizeof(struct tms));
    clock_t  realstart=0;
    clock_t  realend=0;
    FILE *file = fopen("../raport2.txt", "a");
    fprintf(file, "\tNEW EXECUTION\n");
    for(int i=0; i<argc; i++) {
        realstart = times(start);
        printf("\t%s\n", argv[i]);
        realend = times(end);
        fprintf(file, "Times for arg: %s\n", argv[i]);
        printf("Times for arg: %s\n", argv[i]);
        writetimes(file, realstart, realend, start, end);
        fprintf(file, "--------------------------------------------\n");
    }
    fprintf(file, "\n\n");
    fclose(file);
    return 0;
}

