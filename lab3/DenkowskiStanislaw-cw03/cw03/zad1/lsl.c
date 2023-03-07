#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>

char* createpath(char* path, char* name){
    char* res = calloc(PATH_MAX, sizeof(char));
    strcpy(res, path);
    char ch = '/';
    strncat(res, &ch, 1);
    strcat(res, name);
    return res;
}

void callls(char* path, char* curr){
    printf("\t%s\t%d\n", curr, (int)getpid());
    char tmp[300];
    sprintf(tmp, "ls -l %s", path);
    system(tmp);
    printf("\n\n");
    //execlp("ls", path, "-l", NULL);
}

void ls(char* path, char* curr){
    callls(path, curr);
    DIR *dir;
    struct dirent *dirent;
    dir = opendir(path);
    if (dir == NULL) {
        printf("No such directory as %s!!!\n", path);
        return;
    }
    while ((dirent = readdir(dir)) != NULL) {
        if (strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0) {
            continue;
        }
        char *name = createpath(path, dirent->d_name);
        char *newdir = createpath(curr, dirent->d_name);
        if (dirent->d_type == DT_DIR){
            pid_t child_pid;
            child_pid = fork();
            if(child_pid==0)
                ls(name, newdir);
            waitpid(child_pid, NULL, 0);
        }
        free(name);
        free(newdir);
    }
    closedir(dir);
    free(curr);
    exit(0);
}

char* path(char* name){
    char* res = calloc(PATH_MAX, sizeof(char));
    if(name[0]=='/') {
        strcpy(res, name);
    }
    else{
        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));
        strcpy(res, cwd);
        char ch = '/';
        strncat(res, &ch, 1);
        strcat(res, name);
    }
    return res;
}

int main(int argc, char *argv[]){
    char* dir = path(argv[1]);
    puts(dir);
    char* curr = calloc(PATH_MAX, sizeof(char));
    curr[0]='.';
    ls(dir, curr);
    // free(curr);
    return 0;
}
