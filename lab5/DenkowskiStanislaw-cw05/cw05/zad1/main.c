#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>

void call(char* cmd, int type, int* fd, int* pfd){
    pipe(fd);
    if(fork()==0){
        if(type!=0) {
            close(pfd[1]);
            dup2(pfd[0], STDIN_FILENO);
        }
        if(type!=2){
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
        }
        execlp("bash", "bash", "-c", cmd, NULL);
    }

    close(fd[1]);
    pfd[0]=fd[0];
    pfd[1]=fd[1];
}

void slv(char* line){
    int fd[2];
    int pfd[2];
    char* cmd;
    int f=0;
    while((cmd = strsep(&line, "|")) !=NULL){
        if(line==NULL) f++;
        call(cmd, f, fd, pfd);
        if(f==0) f++;
    }
    while(wait(NULL)>0);
}

int main(int argc, char *argv[]){
    if(argc != 2){
        printf("1 argument is required\n");
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

    while (getline(&line, &len, fp) != -1)
        slv(line);
    fclose(fp);
    if (line)
        free(line);
    return 0;
}

