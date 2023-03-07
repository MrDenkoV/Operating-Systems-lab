#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]){
    srand(time(NULL));
    pid_t pi = getpid();
    char pid[12]; // should be enough for pid
    sprintf(pid, "%d", pi);
    if(argc != 4){
        printf("4 arguments are required\n");
        return 0;
    }

    FILE* pp = fopen(argv[1], "w");
    if(pp==NULL){
        printf("Pipe error!\n");
        return 0;
    }
    FILE* fp = fopen(argv[2], "r");
    if(fp==NULL){
        printf("File error!\n");
        fclose(pp);
        return 0;
    }

    int n=atoi(argv[3])+1;
    char* in = calloc(n, sizeof(char));
    char* out = calloc(n+strlen(pid), sizeof(char));

    while(fgets(in, n, fp)!=NULL){
        sprintf(out, "#%s#%s", pid, in);
        fwrite(out, sizeof(char), strlen(out), pp);
        sleep(rand()%2+1);
    }

    free(in);
    free(out);
    fclose(fp);
    fclose(pp);
    return 0;
}

