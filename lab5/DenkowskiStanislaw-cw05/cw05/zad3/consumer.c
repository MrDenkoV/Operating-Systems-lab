#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include<fcntl.h>

int main(int argc, char *argv[]){
    if(argc != 4){
        printf("4 arguments are required\n");
        return 0;
    }

    int pp = open(argv[1], O_RDONLY);
    if(pp<0){
        printf("Pipe error!\n");
        return 0;
    }
    int fp = open(argv[2], O_WRONLY|O_CREAT, 0666);
    if(fp<0){
        printf("File error!\n");
        close(pp);
        return 0;
    }

    int n=atoi(argv[3])+1;
    char* in = calloc(n, sizeof(char));
    int len;

    while((len=(int)read(pp, in, n))>0){
        write(fp, in, len);
    }
    free(in);
    close(fp);
    close(pp);
    return 0;
}

