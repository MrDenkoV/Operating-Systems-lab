#include <stdio.h>
#include <string.h>
#include <dirent.h>


int main(int argc, char *argv[]){
    if(argc != 2){
        printf("1 argument is required");
        return 0;
    }

    char cmd[PATH_MAX];
    strcpy(cmd, "sort ");
    strcat(cmd, argv[1]);

    FILE* srt = popen(cmd, "w");
    pclose(srt);

    return 0;
}

