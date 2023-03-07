#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

void writetimes(FILE* file, struct tms* end, struct tms* start){
    printf("USER: %lf\n", (double)difftime(start->tms_utime, end->tms_utime)/((double)sysconf(_SC_CLK_TCK)));
    printf("SYS: %lf\n", (double)difftime(start->tms_stime, end->tms_stime)/((double)sysconf(_SC_CLK_TCK)));

    fprintf(file, "USER: %lf\n", (double)difftime(start->tms_utime, end->tms_utime)/((double)sysconf(_SC_CLK_TCK)));
    fprintf(file, "SYS: %lf\n", (double)difftime(start->tms_stime, end->tms_stime)/((double)sysconf(_SC_CLK_TCK)));
}

char *randstring(int buffor){
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";
    char *line;
    line=calloc(buffor+1, sizeof(char));
    for(int i=0; i<buffor; i++)
        line[i] = charset[rand() % (int)(sizeof(charset) - 1)];
    line[buffor]='\0';
    return line;
}

void randfile(char* name, int lines, int buffor){
    srand(time(0));
    FILE* file = fopen(name, "w");
    while(lines--){
        char *line = randstring(buffor);
        fprintf(file, "%s\n", line);
        free(line);
    }
    fclose(file);
}

void copy(char* nameA, char* nameB, int lines, int buffor, int f){
    if(f==0) {
        FILE *first = fopen(nameA, "r");
        FILE *second = fopen(nameB, "w");

        char *chunk = calloc(buffor + 2, sizeof(char));

        while (fread(chunk, sizeof(char), buffor + 1, first) > 0)
            fwrite(chunk, sizeof(char), buffor + 1, second);

        fclose(first);
        fclose(second);
    }
    else{
        int first = open(nameA, O_RDONLY);
        int second = open(nameB, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);

        char *chunk = calloc(buffor+1, sizeof(char));

        while(read(first, chunk, buffor+1)>0)
            write(second, chunk, buffor+1);

        close(first);
        close(second);
    }
}

int my_compare (const char *a, const char *b, int buffor){
    for(int i=0; i<buffor; i++) {
        if (a[i] < b[i]) return -1;
        else if(a[i] > b[i]) return 1;
    }
    return 0;
}

void libqsrt(FILE *file, int buffor, char* cmp, char* tmp, int start, int end){
    if(start>=end)
        return;
    int pi=0;
    fseek(file, start*(buffor+1), SEEK_SET);
    fread(cmp, sizeof(char), buffor+1, file);
    int ixb=start+1;

    for(int i=start+1; i<end; i++){
        fread(tmp, sizeof(char), buffor+1, file);
        if(my_compare(tmp, cmp, buffor) < 0) {
            fseek(file, ixb*(buffor+1), SEEK_SET);
            fread(cmp, sizeof(char), buffor+1, file);

            fseek(file, ixb*(buffor+1), SEEK_SET);
            fwrite(tmp, sizeof(char), buffor+1, file);
            fseek(file, i*(buffor+1), SEEK_SET);
            fwrite(cmp, sizeof(char), buffor+1, file);

            fseek(file, start*(buffor+1), SEEK_SET);
            fread(cmp, sizeof(char), buffor+1, file);
            fseek(file, (i+1)*(buffor+1), SEEK_SET);
            ixb++;
        }
    }

    pi=ixb-1;

    fseek(file, start*(buffor+1), SEEK_SET);
    fread(cmp, sizeof(char), buffor+1, file);
    fseek(file, pi*(buffor+1), SEEK_SET);
    fread(tmp, sizeof(char), buffor+1, file);
    fseek(file, start*(buffor+1), SEEK_SET);
    fwrite(tmp, sizeof(char), buffor+1, file);
    fseek(file, pi*(buffor+1), SEEK_SET);
    fwrite(cmp, sizeof(char), buffor+1, file);

    libqsrt(file, buffor, cmp, tmp, start, pi);
    libqsrt(file, buffor, cmp, tmp, pi+1, end);
}

void sysqsrt(int file, int buffor, char* cmp, char* tmp, int start, int end){
    if(start>=end)
        return;
    int pi=0;
    lseek(file, start*(buffor+1), SEEK_SET);
    read(file, cmp, buffor+1);
    int ixb=start+1;

    for(int i=start+1; i<end; i++){
        read(file, tmp, buffor+1);
        if(my_compare(tmp, cmp, buffor) < 0) {
            lseek(file, ixb*(buffor+1), SEEK_SET);
            read(file, cmp, buffor+1);

            lseek(file, ixb*(buffor+1), SEEK_SET);
            write(file, tmp, buffor+1);
            lseek(file, i*(buffor+1), SEEK_SET);
            write(file, cmp, buffor+1);

            lseek(file, start*(buffor+1), SEEK_SET);
            read(file, cmp, buffor+1);
            lseek(file, (i+1)*(buffor+1), SEEK_SET);
            ixb++;
        }
    }

    pi=ixb-1;

    lseek(file, start*(buffor+1), SEEK_SET);
    read(file, cmp, buffor+1);
    lseek(file, pi*(buffor+1), SEEK_SET);
    read(file, tmp, buffor+1);
    lseek(file, start*(buffor+1), SEEK_SET);
    write(file, tmp,  buffor+1);
    lseek(file, pi*(buffor+1), SEEK_SET);
    write(file, cmp, buffor+1);

    sysqsrt(file, buffor, cmp, tmp, start, pi);
    sysqsrt(file, buffor, cmp, tmp, pi+1, end);
}

void srt(char* name, int lines, int buffor, int f){
    char *cmp = calloc(buffor + 2, sizeof(char));
    char *tmp = calloc(buffor + 2, sizeof(char));
    if(f==0) {
        FILE *file = fopen(name, "r+");

        libqsrt(file, buffor, cmp, tmp, 0, lines);

        fclose(file);
    }
    else{
        int file = open(name, O_RDWR);

        sysqsrt(file, buffor, cmp, tmp, 0, lines);

        close(file);
    }
    free(cmp);
    free(tmp);
}

int main(int argc, char *argv[]){
    if(strcmp(argv[1], "generate")==0)
        randfile(argv[2], atoi(argv[3]), atoi(argv[4]));
    else{
        struct tms* start=(struct tms*) calloc(1, sizeof(struct tms));
        struct tms* end=(struct tms*) calloc(1, sizeof(struct tms));
        FILE *file = fopen("wyniki8192.txt", "a");

        fprintf(file, "Times for arg");
        if(strcmp(argv[1], "sort")==0)
            fprintf(file, " using %s calls: %s %s %s %s\n", argv[5], argv[1], argv[2], argv[3], argv[4]);
        else if(strcmp(argv[1], "copy")==0)
            fprintf(file, " using %s calls: %s %s %s %s %s\n", argv[6], argv[1], argv[2], argv[3], argv[4], argv[5]);

        times(start);

        if(strcmp(argv[1], "sort")==0)
            srt(argv[2], atoi(argv[3]), atoi(argv[4]), strcmp("lib", argv[5]));
        else if(strcmp(argv[1], "copy")==0)
            copy(argv[2], argv[3], atoi(argv[4]), atoi(argv[5]), strcmp("lib", argv[6]));

        times(end);

        writetimes(file, start, end);
        fprintf(file, "--------------------------------------------\n");
        fprintf(file, "\n\n");
        printf("\n\n");
        fclose(file);
    }
    return 0;
}

