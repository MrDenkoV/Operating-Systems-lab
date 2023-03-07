#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main(int argc, char *argv[]){

    mkfifo("test.fi", 0666);

    if(fork()==0)
        execl("./cons", "./cons", "test.fi", "outed.txt", "8", NULL);

    char name[6], number[3];
    for(int i=0; i<5; i++){
        sprintf(name, "%c.txt", 'a'+i);
        sprintf(number, "%d", 5+i);
        if(fork()==0)
            execl("./prod.o", "./prod.o", "test.fi", name, number, NULL);
    }

    while(wait(NULL)>0);

    return 0;
}

