#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include<fcntl.h>

int main(int argc, char *argv[]){
    srand(time(NULL));
    pid_t pi = getpid();
    char pid[12]; // should be enough for pid
    sprintf(pid, "%d", pi);
    if(argc != 4){
        printf("4 arguments are required\n");
        return 0;
    }

    int pp = open(argv[1], O_WRONLY|O_APPEND);
    if(pp<0){
        printf("Pipe error!\n");
        return 0;
    }
    int fp = open(argv[2], O_RDONLY);
    if(fp<0){
        printf("File error!\n");
        close(pp);
        return 0;
    }

    int n=atoi(argv[3])+1;
    char* in = calloc(n, sizeof(char));
    char* out = calloc(n+strlen(pid)+2, sizeof(char));
    int len;

    while((len=(int)read(fp, in, n))>0){
        sprintf(out, "#%s#%s", pid, in);
        // fwrite(out, sizeof(char), read+2+strlen(pid), pp);
        write(pp, out, len+2+strlen(pid));
        sleep(rand()%2+1);
    }

    free(in);
    free(out);
    close(fp);
    close(pp);
    return 0;
}

