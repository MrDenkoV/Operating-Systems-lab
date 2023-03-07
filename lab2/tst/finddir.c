//#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ftw.h>

#define PATH_MAX 4096

char* mtime, atime;
int maxdepth;

char* createpath(char* path, char* name){
    char* res = calloc(PATH_MAX, sizeof(char));
    strcpy(res, path);
    char ch = '/';
    strncat(res, &ch, 1);
    strcat(res, name);
    return res;
}

int timesok(const time_t* stime, char* limit){
    if(limit==NULL) {
        return 0;
    }
    time_t now;
    time(&now);

    double days = difftime(now, *stime)/86400.0;

    printf("\t\t\t%f ", days);
    if(limit[0]>='0'&&limit[0]<='9') {
        int count = atoi(limit);
        printf("%d\n", count);
        if(days>=count&&days<=count+1)
            return 0;
        else
            return 1;
    }
    else{
        int count=atoi(limit+sizeof(char));
        printf("%d\n", count);
        if((limit[0]=='-' && count>=days) || (limit[0]=='+' && count<=days))
            return 0;
        else
            return 1;
    }
}

void finddir(char* path, int depth){
    printf("\t%s\n", path);
    if(depth==-1) return;
    DIR* dir;
    struct dirent *dirent;
    dir = opendir(path);
    if(dir==NULL){
        printf("No such directory as %s!!!\n", path);
        return;
    }
    char* type=calloc(10, sizeof(char));
    struct stat buf;// = calloc(1, sizeof(struct stat));
    while((dirent = readdir(dir)) != NULL) {
        if (strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0) {
            continue;
        }
//        struct stat* buf = calloc(1, sizeof(struct stat));
        char *name = createpath(path, dirent->d_name);
        switch (dirent->d_type) {
            case DT_REG:
                strcpy(type, "file");
                lstat(name, &buf);
                break;
            case DT_DIR:
                strcpy(type, "dir");
                lstat(name, &buf);
                finddir(name, depth - 1);
                break;
            case DT_CHR:
                strcpy(type, "char dev");
                lstat(name, &buf);
                break;
            case DT_BLK:
                strcpy(type, "block dev");
                lstat(name, &buf);
                break;
            case DT_FIFO:
                strcpy(type, "fifo");
                lstat(name, &buf);
                break;
            case DT_LNK:
                strcpy(type, "slink");
                lstat(name, &buf);
                break;
            case DT_SOCK:
                strcpy(type, "sock");
                lstat(name, &buf);
                break;
            default:
                strcpy(type, "unknown");
                lstat(name, &buf);
        }
        if (timesok((const time_t *) &(buf.st_atim), atime) == 0 && timesok((const time_t *) &(buf.st_mtim), mtime) == 0)
            printf("%s %lu %s %ld %.24s %s\n", name, buf.st_nlink, type, buf.st_size, ctime(
                    (const time_t *) &buf.st_atim),
               ctime((const time_t *) &buf.st_mtim));
        free(name);
    }
    free(type);
    closedir(dir);

}

void findnftw(char* path){
//    nftw(path, );
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
    printf("%d\n", PATH_MAX);
    DIR *dir;
    struct dirent *dirent;
    dir = opendir("/home/DenkoV/Prog/C/SysOp/lab2/tst/../tst/tst");
    char *first = path("tst");
    char *second = path("/home/DenkoV/Prog/C/SysOp/lab2/tst");
    printf("%s\n", first);
    printf("%s\n", second);
    finddir(second, 0);
    free(second);
    if(dir==NULL){
        printf("NULL??\n");
        return 1;
    }
    while((dirent = readdir(dir)) != NULL) {
        printf("%s %d %d %d\n", dirent->d_name, dirent->d_type==DT_DIR, dirent->d_type==DT_REG, dirent->d_type==DT_LNK);
    }
    closedir(dir);
    return 0;
}

