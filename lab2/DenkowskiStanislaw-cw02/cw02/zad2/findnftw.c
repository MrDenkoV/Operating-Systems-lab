#define _XOPEN_SOURCE 500
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

char* mtime;
char* atime;
int maxdepth;

char* createpath(char* path, char* name){
    char* res = calloc(PATH_MAX, sizeof(char));
    strcpy(res, path);
    char ch = '/';
    strncat(res, &ch, 1);
    strcat(res, name);
    return res;
}

int timesok(time_t* stime, char* limit){
    if(limit==NULL) {
        return 0;
    }
    time_t now;
    time(&now);


    double days = difftime(now, *stime) / 86400.0;

    if(limit[0]>='0'&&limit[0]<='9') {
        int count = atoi(limit);
        if(days>=count&&days<=count+1) {
            return 0;
        }
        else {
            return 1;
        }
    }
    else{
        int count=atoi(limit+sizeof(char));
        if((limit[0]=='-' && count>=days) || (limit[0]=='+' && count<=days))
            return 0;
        else
            return 1;
    }
}

char *types(mode_t mode) {
    if (S_ISREG(mode))
        return "file";
    if (S_ISDIR(mode))
        return "dir";
    if (S_ISCHR(mode))
        return "char dev";
    if (S_ISBLK(mode))
        return "block dev";
    if (S_ISFIFO(mode))
        return "fifo";
    if (S_ISLNK(mode))
        return "slink";
    if (S_ISSOCK(mode))
        return "sock";
    return "undefined";
}

static int display(const char* fpath, const struct stat* stat, int flag, struct FTW* ftw){
    if(ftw->level > maxdepth && maxdepth!=-1)
        return 0;
    if(timesok((time_t *) &(stat->st_atime), atime) == 0 && timesok((time_t *) &(stat->st_mtime), mtime) == 0){
        char* type = types(stat->st_mode);
        printf("%s %lu %s %ld %.24s %s", fpath, stat->st_nlink, type, stat->st_size, ctime(
                (const time_t *) &stat->st_atime),
               ctime((const time_t *) &stat->st_mtime));
    }
    return 0;
}

void findnftw(char* path){
    if(path!=NULL)
        nftw(path, (__nftw_func_t) display, 10, FTW_PHYS);
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
    atime=NULL;
    mtime=NULL;
    maxdepth=-1;

    for(int i=2; i<argc; i+=2){
        if(argv[i][1]=='a'){
            atime = argv[i+1];
        }
        else if(argv[i][2]=='t'){
            mtime = argv[i+1];
        }
        else{
            maxdepth = atoi(argv[i+1]);
        }
    }

    findnftw(dir);


    return 0;
}

