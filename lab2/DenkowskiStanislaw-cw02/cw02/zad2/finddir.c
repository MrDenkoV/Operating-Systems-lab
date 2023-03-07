#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

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

    if(limit[0]>='0'&&limit[0]<='9') {
        int count = atoi(limit);
        if(days>=count&&days<=count+1)
            return 0;
        else
            return 1;
    }
    else{
        int count=atoi(limit+sizeof(char));
        if((limit[0]=='-' && count>=days) || (limit[0]=='+' && count<=days))
            return 0;
        else
            return 1;
    }
}

void finddir(char* path, char* mtime, char* atime, int maxdepth, int depth){
    struct stat buf;// = calloc(1, sizeof(struct stat));
    if(depth==0){
        lstat(path, &buf);
        if (timesok((const time_t *)&(buf.st_atim), atime) == 0 && timesok((const time_t *)&(buf.st_mtim), mtime) == 0)
            printf("%s %lu dir %ld %.24s %s", path, buf.st_nlink, buf.st_size, ctime((const time_t *)&buf.st_atim),
               ctime((const time_t *)&buf.st_mtim));
    }
    if(maxdepth==0) return;
    DIR* dir;
    struct dirent *dirent;
    dir = opendir(path);
    if(dir==NULL){
        // printf("No such directory as %s!!!\n", path);
        return;
    }
    char* type=calloc(10, sizeof(char));
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
                finddir(name, mtime, atime, maxdepth - 1, depth+1);
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
        if (timesok((const time_t *)&(buf.st_atim), atime) == 0 && timesok((const time_t *)&(buf.st_mtim), mtime) == 0)
            printf("%s %lu %s %ld %.24s %s", name, buf.st_nlink, type, buf.st_size, ctime((const time_t *)&buf.st_atim),
               ctime((const time_t *)&buf.st_mtim));
        free(name);
    }
    free(type);
    closedir(dir);

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
    char* atime=NULL;// = calloc(10, sizeof(char));
    char* mtime=NULL;// = calloc(10, sizeof(char));
    int maxdepth=-1;

    for(int i=2; i<argc; i+=2){
        if(argv[i][1]=='a'){
            atime=argv[i+1];
        }
        else if(argv[i][2]=='t'){
            mtime = argv[i+1];
        }
        else{
            maxdepth = atoi(argv[i+1]);
        }
    }

    finddir(dir, mtime, atime, maxdepth, 0);

    return 0;
}

