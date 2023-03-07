#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>

int psd = 0;

void hanINT(int signum){
    printf("Odebrano sygnał SIGINT\n");
    exit(0);
}

void hanTSTP(int signum){
    if(psd == 0) {
        psd = 1;
        printf("Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu\n");
        pause();
    } else
        psd = 0;
}

void ls(DIR* dir){
    printf("??\n");
    struct sigaction act;
    act.sa_flags=SA_NODEFER;
    act.sa_handler = hanTSTP;
    sigemptyset(&act.sa_mask);
    sigaction(SIGTSTP, &act, NULL);
    signal(SIGINT, hanINT);
    struct dirent *dirent;
    while(1) {
        while ((dirent = readdir(dir)) != NULL) {
            if (strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0) {
                continue;
            }
            printf("%s\n", dirent->d_name);
        }
        rewinddir(dir);
//        return;
    }
}

char* path(){
    char* res = calloc(PATH_MAX, sizeof(char));
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    strcpy(res, cwd);
    char ch = '/';
    strncat(res, &ch, 1);
    return res;
}

int main(){
    char* dir = path();
    printf("%s\n", dir);
    DIR* dirptr;
    dirptr = opendir(dir);
    ls(dirptr);
    closedir(dirptr);
    free(dir);
    return 0;
}

