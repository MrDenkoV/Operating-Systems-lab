#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]){
    if(argc != 4){
        printf("4 arguments are required\n");
        return 0;
    }

    FILE* pp = fopen(argv[1], "r");
    if(pp==NULL){
        printf("Pipe error!\n");
        return 0;
    }
    FILE* fp = fopen(argv[2], "w");
    if(fp==NULL){
        printf("File error!\n");
        fclose(pp);
        return 0;
    }

    int n=atoi(argv[3])+1;
    char* in = calloc(n, sizeof(char));
    int read;

    while((read=fread(in, sizeof(char), n, pp))>0){
        fwrite(in, sizeof(char), read, fp);
    }
    free(in);
    fclose(fp);
    fclose(pp);
    return 0;
}

