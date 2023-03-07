#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>

struct matrix{
    int col, row;
    char* name;
};

struct operation{
    struct matrix A, B, C;
};

void createfiles(char* name, int rows, int cols){
    FILE* file = fopen(name, "w");
    for(int i=0; i<cols; i++){
        for(int j=0; j<rows; j++){
            if(rand()%200==0) {
                if(rand()%2==0)
                    fprintf(file, "+100 ");
                else
                    fprintf(file, "-100 ");
            }
            else
                fprintf(file, "%c0%d%d ", rand()%2==0 ? '+' : '-',  rand()%9, rand()%9);
        }
        if(rand()%200==0) {
            if(rand()%2==0)
                fprintf(file, "+100\n");
            else
                fprintf(file, "-100\n");
        }
        else
            fprintf(file, "%c0%d%d\n", rand()%2==0 ? '+' : '-',  rand()%9, rand()%9);
    }
    fclose(file);
}

int mult(struct operation op, FILE* fileA, FILE* fileB, int col, int row){
    int wyn=0;
    int A, B;
    char number[6];
    fseek(fileA, op.A.col*5*row, SEEK_SET);
    for(int i=0; i<op.B.row; i++){

        fgets(number, 6, fileA);
        A=atoi(number);

        fseek(fileB, op.B.col*5*i+col*5, SEEK_SET);
        fgets(number, 6, fileB);
        B=atoi(number);

        wyn+=A*B;
    }
    return wyn;
}

void gen(char* name, int nr, int min, int max){
    srand(time(NULL));
    FILE *list = fopen(name, "w");
    for(int i=0; i<nr; i++){
        int rows, cols, shared;
        rows = rand()%(max-min) + min;
        cols = rand()%(max-min) + min;
        shared = rand()%(max-min) + min;
        char sub[25] = "A";
        char ix[8];
        sprintf(ix, "%d", i);
        strcat(sub, ix);
        strcat(sub, ".txt");
        createfiles(sub, rows, shared);

        fprintf(list, "%s ", sub);

        sub[0]='B';
        createfiles(sub, shared, cols);

        fprintf(list, "%s ", sub);

        sub[0]='C';

        fprintf(list, "%s\n", sub);
    }
    fclose(list);

}

void tst(char* name){
    FILE *list = fopen(name, "r");
    if (list == NULL) {
        printf("No such file as %s!!!\n", name);
        return;
    }


    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int sum=0, f=-1;

    while ((read = getline(&line, &len, list)) != -1 && f==-1) {

        struct operation tmp;

        char* fileAName = strtok(line, " ");
        char* fileBName = strtok(NULL, " ");
        char* fileCName = strtok(NULL, " ");
        fileCName[strlen(fileCName)-1]=0;

        char c;

        int width = 1, f_size, exp;

        FILE* fileA = fopen(fileAName, "r");

        fseek(fileA, 0, SEEK_END);
        f_size = (int)ftell(fileA);
        fseek(fileA, 0, SEEK_SET);

        while ((c = (char) fgetc(fileA)) != EOF) {
            if (c == ' ') {
                width++;
            } else if (c == '\n')
                break;
        }
//        size = width;
//        rows = (f_size / width / 5);

        tmp.A.col = width;
        tmp.A.row = (f_size / width / 5);


        FILE* fileB = fopen(fileBName, "r");

        fseek(fileB, 0, SEEK_END);
        f_size = (int)ftell(fileB);
        fseek(fileB, 0, SEEK_SET);

        width = 1;

        while ((c = (char) fgetc(fileB)) != EOF) {
            if (c == ' ') {
                width++;
            } else if (c == '\n')
                break;
        }
//        cols = width;

        tmp.B.col = width;
        tmp.B.row = (f_size / width / 5);

        FILE* fileC = fopen(fileCName, "r");

        for (int i = 0; i < tmp.B.col; i ++) {
            for (int j = 0; j < tmp.A.row; j++) {
                exp = mult(tmp, fileA, fileB, i, j);
//                fseek(fileC, tmp.B.col*j*9+i*9, SEEK_SET);
                char wyn[9];
                fgets(wyn, 9, fileC);
                if(atoi(wyn)!=exp){
                    printf("źle\n");
                    f=1;
                }
            }
        }

        fclose(fileA);
        fclose(fileB);
        fclose(fileC);
        sum++;
    }

    fclose(list);

    printf("Multiplied correctly %d tim%s!\n", sum, sum==1 ? "e" : "es");
}

int main(int argc, char *argv[]){
    if(argc<2) {
        printf("Prog needs more args\n");
        return 1;
    }
    if(strcmp(argv[1], "tst")==0){
        tst(argv[2]);
    } else{//gen ../genlist.txt 10 3 9
        gen(argv[2], atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
    }
    return 0;
}
