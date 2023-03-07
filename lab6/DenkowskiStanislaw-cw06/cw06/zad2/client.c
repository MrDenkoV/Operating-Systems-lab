#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <mqueue.h>
#include "com.h"

mqd_t queue;
mqd_t serv;
unsigned int id;
char* name;

void clean(){
    mq_close(queue);
    mq_unlink(name);
}

void stop(){
    printf("Stopping\n");
    char* msg = calloc(LEN, sizeof(char));
    msg[0]=id+'0';
    mq_send(serv, msg, LEN, STOP);
    free(msg);
    exit(0);
}

void start(char* arg){
    atexit(clean);
    name=arg;
    fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
    signal(SIGINT, stop);
    // srand(time(NULL));

    queue = mq_open(name, O_RDWR | O_CREAT, 0666, NULL);
    printf("Queue: %d\n", queue);
    serv = mq_open(PATH, O_RDWR, 0666, NULL);
    printf("Server Queue:%d\n", serv);
    if(serv<0){
        printf("No serv\n");
        exit(1);
    }


    char* msg = calloc(LEN, sizeof(char));
    strcpy(msg, name);
    printf("%s\n", msg);
    if(mq_send(serv, msg, LEN, INIT)<0)
        printf("DUPA\n");

    mq_receive(queue, msg, LEN, &id);
    printf("ID:%d\n", id);

    struct mq_attr attr;
    mq_getattr(queue, &attr);
    attr.mq_flags=O_NONBLOCK;
    mq_setattr(queue, &attr, NULL);

    free(msg);

    if(id==-1)
        exit(0);
}

void gg(int otherid, mqd_t otherq){
    printf("Gadu Gadu:\nSay sth or DISCONNECT\n");
    char* msg = calloc(LEN, sizeof(char));
    char txt[1024];
    ssize_t rea=0;
    unsigned int type=0;

    while(1){
        rea = read(0, &txt, (size_t)1000);
        while(mq_receive(queue, msg, LEN, &type)>=0){
            switch (type)
            {
            case STOP:
                stop();
                break;
            case DISCONNECT:
                printf("Disconnecting\nChatend\n");
                free(msg);
                return;
            default:
                printf("%d: %s\n", otherid, msg);
                break;
            }
        }
        if(rea>0){
            txt[rea-1]=0;
            if(strcmp(txt, "DISCONNECT")==0){
                msg[0]='0'+id;
                msg[1]='0'+otherid;
                mq_send(serv, msg, LEN, DISCONNECT);
                break;
            }
            else if(strcmp(txt, "")!=0){
                strcpy(msg, txt);
                mq_send(otherq, msg, LEN, CONNECT);
            }
        }
    }
    printf("Chatend\n");
    free(msg);
}

void status(){
    char* msg = calloc(LEN, sizeof(char));
    unsigned int type=0;
    if(mq_receive(queue, msg, LEN, &type)<0){
        free(msg);
        return;
    }
    if(type==STOP){
        free(msg);
        stop();
    }
    else if(type==CONNECT){
        char* othern = calloc(LEN, sizeof(char));
        int ox = msg[0] - '0';
        printf("Connecting to: %d\n", ox);
        strncpy(othern, msg+1, LEN-1);
        gg(ox, mq_open(othern, O_RDWR, 0666, NULL));
        free(othern);
    }

    free(msg);
}

void list(){
    printf("List\n");
    char* msg = calloc(LEN, sizeof(char));
    msg[0]=id+'0';
    mq_send(serv, msg, LEN, LIST);

    char* resp = calloc(LEN, sizeof(char));

    struct mq_attr attr;
    mq_getattr(queue, &attr);
    attr.mq_flags=0;
    mq_setattr(queue, &attr, NULL);

    mq_receive(queue, resp, LEN, NULL);
    printf("%s\n", resp);

    attr.mq_flags=O_NONBLOCK;
    mq_setattr(queue, &attr, NULL);

    free(resp);
    free(msg);
}

void connect(int other){
    printf("Connecting\n");
    char* msg = calloc(LEN, sizeof(char));
    msg[0]=id+'0';
    msg[1]=other+'0';
    mq_send(serv, msg, LEN, CONNECT);

    char* resp = calloc(LEN, sizeof(char));

    struct mq_attr attr;
    mq_getattr(queue, &attr);
    attr.mq_flags=0;
    mq_setattr(queue, &attr, NULL);

    mq_receive(queue, resp, LEN, NULL);

    attr.mq_flags=O_NONBLOCK;
    mq_setattr(queue, &attr, NULL);

    if(resp[0]=='0'-1){
        free(resp);
        free(msg);
        printf("Other is busy\n");
        return;
    }
    char* othern = calloc(LEN, sizeof(char));
    strncpy(othern, msg+1, LEN-1);
    gg(other, mq_open(othern, O_RDWR, 0666, NULL));

    free(othern);
    free(resp);
    free(msg);
}

void run(){
    char cmd[100];
    // size_t len=0;
    ssize_t rea=0;

    printf("Enter commands: STOP LIST CONNECT - DISCONNECT when connected\n");
    while(1){
        // sleep(1);
        rea = read(0, &cmd, (size_t) 95);
        // printf("%ld\n", read);
        // printf("%s\n", cmd);
        status();
        if(rea>0){
            cmd[rea-1]=0;
            if(strcmp(cmd, "")==0){
                continue;
            }
            
            char* type = strtok(cmd, " ");
            if(strcmp(type, "STOP")==0)
                stop();
            else if(strcmp(type, "LIST")==0)
                list();
            else if(strcmp(type, "CONNECT")==0){
                connect(atoi(strtok(NULL, " ")));
            }
        }
    }
}

int main(int argc, char *argv[]){
    if(argc<2){
        printf("Name required\n");
        return 0;
    }
    start(argv[1]);
    run();
    return 0;
}