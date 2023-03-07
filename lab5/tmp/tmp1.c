#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>

void call(char* cmd, int mode, int* fd, int* pfd){
    pipe(fd);
    if(fork()==0){
        if(mode!=0){
            close(pfd[1]);
            dup2(pfd[0], STDIN_FILENO);
        }
        if(mode!=2){
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
        }
        else

    }
}

void slv(char* line, int len){
    int fd[2];
    int pfd[2];
    char* cmd;
    cmd = strtok(line, "|");
    for(int cnt=0; cnt<=len; cnt++){
        printf("%s\n", cmd);
        if(cnt==0){
            call(cmd, 0, fd, pfd);
        }
        else if(cnt==len){
            call(cmd, 2, fd, pfd);
        }
        else{
            call(cmd, 1, fd, pfd);
        }
        cmd = strtok(NULL, "|");
    }
    while(wait(NULL)>0);
}

int main(int argc, char *argv[]){
    if(argc != 2){
        printf("1 argument is required");
        return 0;
    }

    FILE * fp;
    char * line = NULL;
    size_t len = 0;

    fp = fopen(argv[1], "r");
    if (fp == NULL){
        printf("No such file as %s\n", argv[1]);
        return 1;
    }
    int* T;
    int n=0, cnt=0;
    while (getline(&line, &len, fp) != -1) {
        n++;
    }
    T=calloc(n, sizeof(int));
    fseek(fp, 0, SEEK_SET);
    char* cmd;
    n=0;
    while (getline(&line, &len, fp) != -1) {
        cmd = strtok(line, "|");
        cnt=0;
        while(cmd != NULL) {
            cmd = strtok(NULL, "|");
            cnt++;
        }
        T[n]=cnt;
        n++;
    }

    fseek(fp, 0, SEEK_SET);
    n=0;
    while (getline(&line, &len, fp) != -1) {
        printf("%s", line);
        slv(line, T[n]);
        n++;
    }
    fclose(fp);
    if (line)
        free(line);
    free(T);
    return 0;
}

