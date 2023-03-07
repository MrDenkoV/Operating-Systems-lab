#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include "com.h"
// msgget, msgctl, msgsnd, msgrcv, ftok

int queue;
int serv;
int id;

void clean(){
    msgctl(queue, IPC_RMID, NULL);
}

void stop(int sig){
    printf("Stopping\n");
    message* msg = malloc(sizeof(message));
    msg->id=id;
    msg->type=STOP;
    msgsnd(serv, msg, SIZE, 0);
    free(msg);
    exit(0);
}

void start(){
    fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
    signal(SIGINT, stop);
    srand(time(NULL));

    key_t key = ftok(getenv("HOME"), KEYID+rand()%100+1);
    printf("Key: %d\n", key);
    queue = msgget(key, IPC_CREAT | 0666);
    printf("Queue: %d\n", queue);
    
    key_t sekey = ftok(getenv("HOME"), KEYID);
    serv = msgget(sekey, 0);
    printf("Server key: %d\nServer Queue ID:%d\n", sekey, serv);

    message* msg = malloc(sizeof(message));
    msg->type=INIT;
    msg->queue=key;
    msgsnd(serv, msg, SIZE, 0);

    message* resp = malloc(sizeof(message));
    msgrcv(queue, resp, SIZE, 0, 0);
    id=resp->type;
    printf("ID:%d\n", id);

    free(msg);
    free(resp);

    atexit(clean);
    if(id==-1)
        exit(0);
}

void gg(int otherid, int otherq){
    printf("Gadu Gadu:\nSay sth or DISCONNECT\n");
    message* msg = malloc(sizeof(message));
    msg->id=id;
    msg->type=CONNECT;
    char txt[1024];
    ssize_t rea=0;

    while(1){
        rea = read(0, &txt, (size_t)1000);
        if(msgrcv(queue, msg, SIZE, STOP, IPC_NOWAIT)>=0){
            free(msg);
            stop(0);
        }
        if(msgrcv(queue, msg, SIZE, DISCONNECT, IPC_NOWAIT)>=0){
            printf("Disconnecting\nChatend\n");
            free(msg);
            return;
        }
        while(msgrcv(queue, msg, SIZE, 0, IPC_NOWAIT)>=0){
            if(msg->type==STOP){
                free(msg);
                stop(0);
            }
            if(msg->type==DISCONNECT){
                printf("Disconnecting\nChatend\n");
                free(msg);
                return;
            }
            printf("%d: %s\n", otherid, msg->txt);
        }
        if(rea>0){
            txt[rea-1]=0;
            if(strcmp(txt, "DISCONNECT")==0){
                msg->id=id;
                msg->other=otherid;
                msg->type=DISCONNECT;
                msgsnd(serv, msg, SIZE, 0);
                // msgsnd(otherq, msg, SIZE, 0);
                break;
            }
            else if(strcmp(txt, "")!=0){
                strcpy(msg->txt, txt);
                msg->type=CONNECT;
                msgsnd(otherq, msg, SIZE, 0);
            }
        }
    }
    printf("Chatend\n");
    free(msg);
}

void status(){
    message* msg = malloc(sizeof(message));

    if(msgrcv(queue, msg, SIZE, 0, IPC_NOWAIT)<0){
        free(msg);
        return;
    }
    if(msg->type==STOP){
        free(msg);
        stop(0);
    }
    else if(msg->type==CONNECT){
        printf("Connecting to: %d\n", msg->id);
        gg(msg->id, msgget(msg->queue, 0));
    }

    free(msg);
}

void list(){
    printf("List\n");
    message* msg = malloc(sizeof(message));
    msg->id=id;
    msg->type=LIST;
    msgsnd(serv, msg, SIZE, 0);

    message* resp = malloc(sizeof(message));
    msgrcv(queue, resp, SIZE, 0, 0);
    printf("%s\n", resp->txt);

    free(resp);
    free(msg);
}

void connect(int other){
    printf("Connecting\n");
    message* msg = malloc(sizeof(message));
    msg->id=id;
    msg->other=other;
    msg->type=CONNECT;
    msgsnd(serv, msg, SIZE, 0);

    message* resp = malloc(sizeof(message));
    msgrcv(queue, resp, SIZE, 0, 0);
    if(resp->type==DISCONNECT){
        free(resp);
        free(msg);
        printf("Other is busy\n");
        return;
    }
    
    gg(other, msgget(resp->queue, 0));

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
                stop(0);
            else if(strcmp(type, "LIST")==0)
                list();
            else if(strcmp(type, "CONNECT")==0)
                connect(atoi(strtok(NULL, " ")));
        }
    }
}

int main(){
    start();
    run();
    return 0;
}