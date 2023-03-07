#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

struct matrix{
    int col, row;
    char* name;
};

struct operation{
    struct matrix A, B, C;
};

int createfiles(FILE* list, struct operation **ptr){

    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    int lines = 0;
    while ((read = getline(&line, &len, list)) != -1){
        lines++;
    }

    fseek(list, 0, SEEK_SET);

    (*ptr) = calloc(lines, sizeof(struct operation));

    struct operation *ops= (*ptr);


    int ob=0;
    while ((read = getline(&line, &len, list)) != -1) {

        char* fileAName = strtok(line, " ");
        char* fileBName = strtok(NULL, " ");
        char* fileCName = strtok(NULL, " ");
        fileCName[strlen(fileCName)-1]=0;

        ops[ob].A.name = calloc(20, sizeof(char));
        ops[ob].B.name = calloc(20, sizeof(char));
        ops[ob].C.name = calloc(20, sizeof(char));
        strcpy(ops[ob].A.name, fileAName);
        strcpy(ops[ob].B.name, fileBName);
        strcpy(ops[ob].C.name, fileCName);

        char c;

        int width = 1;
        int height = 1;

        FILE* fileA = fopen(fileAName, "r");

        fseek(fileA, 0, SEEK_SET);

        while ((c = (char) fgetc(fileA)) != EOF) {
            if (c == ' ') {
                width++;
            } else if (c == '\n')
                break;
        }
        fseek(fileA, 0, SEEK_SET);
        height=0;
        while ((read = getline(&line, &len, fileA)) != -1){
            height++;
        }

        ops[ob].A.col = width;
        ops[ob].A.row = height;



        FILE* fileB = fopen(ops[ob].B.name, "r");
                
        fseek(fileB, 0, SEEK_SET);

        width = 1;

        while ((c = (char) fgetc(fileB)) != EOF) {
            if (c == ' ') {
                width++;
            } else if (c == '\n')
                break;
        }
        fseek(fileB, 0, SEEK_SET);
        height=0;
        while ((read = getline(&line, &len, fileB)) != -1){
            height++;
        }

        ops[ob].B.col = width;
        ops[ob].B.row = height;


        FILE* fileC = fopen(ops[ob].C.name, "w");

        ops[ob].C.row = ops[ob].A.row;
        ops[ob].C.col = ops[ob].B.col;

        for(int i=0; i<ops[ob].C.row; i++){
            for(int j=0; j<ops[ob].C.col-1; j++){
                fprintf(fileC, "00000000 ");
            }
            fprintf(fileC, "00000000\n");
        }


        fclose(fileA);
        fclose(fileB);
        fclose(fileC);
        ob++;
    }
    return lines;
}

void clear(struct operation* ops, int n){
    for(int i=0; i<n; i++){
        free(ops[i].A.name);
        free(ops[i].B.name);
        free(ops[i].C.name);
    }
    free(ops);
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
        // printf("%d * %d\n", A, B);
    }
    // printf("\n\n\n");
    return wyn;
}

void multiply(struct operation op, int ix, int n, int type){
    int wyn;
    FILE* fileA = fopen(op.A.name, "r");
    FILE* fileB = fopen(op.B.name, "r");
    if(type==0) {
        FILE* fileC = fopen(op.C.name, "r+");
        for (int i = ix; i < op.B.col; i += n) {
            for (int j = 0; j < op.A.row; j++) {
                wyn = mult(op, fileA, fileB, i, j);
                fseek(fileC, op.B.col*j*9+i*9, SEEK_SET);
                if(wyn<0){
                    fputc('-', fileC);
                    wyn=-wyn;
                }
                else{
                    fputc('+', fileC);
                }
                for(int x=1000000; x>0; x/=10){
                    fputc(('0' + wyn / x), fileC);
                    wyn -= (wyn/x)*x;
                }
            }
        }
        fclose(fileC);
    }
    else{
        for (int i = ix; i < op.B.col; i += n) {
            char* name = calloc(27, sizeof(char));
            strcpy(name, ".");
            char nr[8];
            sprintf(nr, "%d", i);
            strcat(name, nr);
            strcat(name, op.C.name);
            FILE* fileC = fopen(name, "w");
            for (int j = 0; j < op.A.row; j++) {
                wyn = mult(op, fileA, fileB, i, j);
                if(wyn<0){
                    fputc('-', fileC);
                    wyn=-wyn;
                }
                else{
                    fputc('+', fileC);
                }
                for(int x=1000000; x>0; x/=10){
                    fputc(('0' + wyn / x), fileC);
                    wyn -= (wyn/x)*x;
                }
                if(j<op.A.row-1)
                    putc('\n', fileC);
            }
            free(name);
            fclose(fileC);
        }

    }
    fclose(fileA);
    fclose(fileB);
}

void conn(struct operation op){
    char cmd[30000]="paste -d \" \"";
    for(int i=0; i<op.B.col; i++){
        char name[27] = " .";
        char nr[8];
        sprintf(nr, "%d", i);
        strcat(name, nr);
        strcat(name, op.C.name);
        strcat(cmd, name);
    }
    char out[40];
    sprintf(out, " >%s 2>/dev/null", op.C.name);
    strcat(cmd, out);
    execlp("bash", "bash", "-c", cmd, NULL);
}

void del(struct operation op){
    char rm[30000] = "rm";
    for(int i=0; i<op.B.col; i++){
        char name[27] = " .";
        char nr[8];
        sprintf(nr, "%d", i);
        strcat(name, nr);
        strcat(name, op.C.name);
        strcat(rm, name);
//        printf("%s\n", rm);
    }
    strcat(rm, " 2>/dev/null");
    system(rm);
}

void manager(char* name, int n, int s, int type){
    FILE *list = fopen(name, "r");
    if (list == NULL) {
        printf("No such file as %s!!!\n", name);
        return;
    }

    struct operation* ops;
    int count = createfiles(list, &ops);
    fclose(list);

    int ix=-1;
    pid_t* child_pid = calloc(n, sizeof(pid_t));
    for(int i=0; i<n; i++){
        child_pid[i] = fork();
        if(child_pid[i]==0) {
            ix = i;
            break;
        }
    }


    if(ix!=-1) {
        clock_t begin = clock();
        clock_t end;
        for (int i = 0; i < count; i++) {
            end = clock();
            if((double)s < (double)(end - begin) / CLOCKS_PER_SEC){
                // printf("%d %d %d %lf\n", i, count, s, (double)(end - begin) / CLOCKS_PER_SEC);
                exit(i);
            }
            multiply(ops[i], ix, n, type);
        }
        exit(count);
    } else {
        int stat=0;
        for(int i=0; i<n; i++){
            waitpid(child_pid[i], &stat, 0);
            printf("Proces %d wykonał %d mnożeń macierzy\n", (int)child_pid[i], WEXITSTATUS(stat));
        }
    }

    if(type!=0){
        pid_t tmp;
        for(int i=0; i<count; i++){
            tmp=fork();
            if(tmp==0){
                conn(ops[i]);
                exit(0);
            } else {
                waitpid(tmp, NULL, 0);
                del(ops[i]);
            }
        }
    }
    clear(ops, count);
    exit(0);
}

int main(int argc, char *argv[]){
    if(argc<5) {
        printf("Prog needs 4 args\n");
        return 1;
    }
    manager(argv[1], atoi(argv[2]), atoi(argv[3]), strcmp(argv[4], "one"));
    return 0;
}
