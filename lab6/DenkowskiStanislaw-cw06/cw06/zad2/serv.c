#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <mqueue.h>
#include <errno.h>
#include "com.h"

char* clients[MAXCLIENTS];
int busy[MAXCLIENTS];
mqd_t queue;
int avail;

void clean(){
    mq_close(queue);
    mq_unlink(PATH);
}

void handleStop(int sig){
    printf("Stopping\n");
    char* msg = calloc(LEN, sizeof(char));
    for(int i=0; i<MAXCLIENTS; i++){
        if(clients[i]!=NULL){
            mqd_t cq = mq_open(clients[i], O_RDWR, 0666, NULL);
            if(mq_send(cq, msg, LEN, STOP)<0){
                printf("WTF\n");
                free(msg);
                exit(1);
            }
            if(mq_receive(queue, msg, LEN, NULL)<0){
                printf("MEH\n");
                free(msg);
                exit(1);
            }
            mq_close(cq);
        }
    }
    free(msg);
    exit(0);
}

void start(){
    signal(SIGINT, handleStop);
    for(int i=0; i<MAXCLIENTS; i++)
        clients[i]=NULL;
    avail=0;
    // struct mq_attr attr;
    queue = mq_open(PATH, O_RDWR | O_CREAT, 0666, NULL);
    // mq_getattr(queue, &attr);
    // printf("%ld %ld\n", attr.mq_maxmsg, attr.mq_msgsize);
    printf("Queue: %d\n", queue);
    atexit(clean);
}

void init(char* msg){
    printf("Initialising client\n");
    int ix = -1;
    for(int i=0; i<MAXCLIENTS; i++){
        if(clients[i]==NULL){
            ix=i;
            break;
        }
    }
    char *newmsg = calloc(LEN, sizeof(char));
    mqd_t cq = mq_open(msg, O_RDWR, 0666, NULL);
    if(ix==-1){
        printf("No more free client slots\n");
    }
    else{
        clients[ix]=calloc(LEN, sizeof(char));
        strcpy(clients[ix], msg);
        busy[ix]=-1;
        avail++;
    }
    mq_send(cq, newmsg, LEN, ix+1);
    mq_close(cq);
    free(newmsg);
}

void stop(char* msg){
    printf("Client %d stopped\n", msg[0]-1-'0');
    free(clients[msg[0]-1-'0']);
    clients[msg[0]-1-'0'] = NULL;
    busy[msg[0]-1-'0'] = 1;
}

void connect(char* msg){
    printf("Connecting\n");
    int ix = msg[0] - '0';
    int ox = msg[1] - '0';
    char* new = calloc(LEN, sizeof(char));
    mqd_t cq = mq_open(clients[ix-1], O_RDWR, 0666, NULL);
    if(busy[ox-1]!=-1 || ox==ix){
        printf("Other is busy\n");
        char* new = calloc(LEN,sizeof(char));
        new[0]='0'-1;
        mq_send(cq, new, LEN, DISCONNECT);
        free(new);
        return;
    }
    busy[ox-1]=1;
    busy[ix-1]=1;
    avail-=2;
    new[0]=ox+'0';
    strcat(new, clients[ox-1]);
    mq_send(cq, new, LEN, CONNECT);
    
    memset(new, 0, LEN);
    new[0]=ix+'0';
    strcat(new, clients[ix-1]);
    cq = mq_open(clients[ox-1], O_RDWR, 0666, NULL);
    mq_send(cq, new, LEN, CONNECT);

    free(new);
}

void disconnect(char* msg){
    printf("Disconecting\n");
    avail+=2;
    char* new = calloc(LEN, sizeof(char));
    int ix = msg[0] - '0';
    int ox = msg[1] - '0';
    mqd_t cq = mq_open(clients[ox-1], O_RDWR, 0666, NULL);
    mq_send(cq, new, LEN, DISCONNECT);
    mq_close(cq);
    busy[ix-1]=-1;
    busy[ox-1]=-1;
    free(new);
}

void list(char* msg){
    printf("Listing\n");
    char* new = calloc(LEN,sizeof(char));
    sprintf(new, "Number of available %d\n", avail-1);
    int ix = msg[0] - '0';
    for(int i=0; i<MAXCLIENTS; i++)
        if(clients[i]!=NULL && i!=ix-1)// && i != msg->id-1)
            sprintf(new, "%sID: %d available: %s\n", new, i+1, busy[i]==-1 ? "yes" : "no");
    
    mqd_t cq = mq_open(clients[ix-1], O_RDWR, 0666, NULL);
    mq_send(cq, new, LEN, LIST);
    mq_close(cq);
    free(new);
}

void run(){
    char *msg = calloc(LEN, sizeof(char));
    unsigned int type;
    while(1){
        if(mq_receive(queue, msg, LEN, &type)<0){
            printf("%d %d %d %d %d %d %d\n", errno, EAGAIN, EBADF, EINTR, EINVAL, EMSGSIZE, ETIMEDOUT);
            printf("%d %s\n", queue, msg);
            free(msg);
            exit(1);
        }
        switch(type){
            case INIT:
                init(msg);
                break;
            case STOP:
                stop(msg);
                break;
            case CONNECT:
                connect(msg);
                break;
            case DISCONNECT:
                disconnect(msg);
                break;
            case LIST:
                list(msg);
                break;
            default:
                printf("???\n");
                free(msg);
                exit(1);
        }
    }
}

int main(){
    start();
    run();
    return 0;
}