#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include "com.h"
// msgget, msgctl, msgsnd, msgrcv, ftok

key_t clients[MAXCLIENTS];
int busy[MAXCLIENTS];
int queue;
int avail;

void clean(){
    msgctl(queue, IPC_RMID, NULL);
}

void handleStop(int sig){
    printf("Stopping\n");
    message* msg = (message*) malloc(sizeof(message));
    for(int i=0; i<MAXCLIENTS; i++){
        if(clients[i]!=-1){
            // printf("???? %d %d\n", i, clients[i]);
            msg->type=STOP;
            int cq = msgget(clients[i], 0);
            msgsnd(cq, msg, SIZE, 0);
            msgrcv(queue, msg, SIZE, STOP, 0);
        }
    }
    free(msg);
    exit(0);
}

void start(){
    signal(SIGINT, handleStop);
    for(int i=0; i<MAXCLIENTS; i++)
        clients[i]=-1;
    avail=0;
    
    key_t key = ftok(getenv("HOME"), KEYID);
    printf("Key: %d\n", key);
    queue = msgget(key, IPC_CREAT | 0666);
    printf("Queue: %d\n", queue);
    atexit(clean);
}

void init(message* msg){
    printf("Initialising client\n");
    int ix = -1;
    for(int i=0; i<MAXCLIENTS; i++){
        if(clients[i]==-1){
            ix=i;
            break;
        }
    }
    if(ix==-1){
        printf("No more free client slots\n");
        message *newmsg = malloc(sizeof(message));
        newmsg->type=-1;
        int cq=msgget(msg->queue, 0);
        msgsnd(cq, newmsg, SIZE, 0);
        free(newmsg);
    }
    else{
        message *newmsg = malloc(sizeof(message));
        newmsg->type = ix+1;
        // printf("%d\n", msg->queue);
        int cq = msgget(msg->queue, 0);
        if(cq<0){
            printf("%d dupa\n", cq);
            exit(EXIT_FAILURE);
        }
        clients[ix] = msg->queue;
        busy[ix]=-1;
        msgsnd(cq, newmsg, SIZE, 0);
        free(newmsg);
        avail++;
    }
}

void stop(message* msg){
    printf("Client %d stopped\n", msg->id);
    clients[msg->id-1] = -1;
    busy[msg->id-1] = 1;
}

void connect(message* msg){
    printf("Connecting\n");
    if(busy[msg->other-1]!=-1 || msg->other==msg->id){
        printf("Other is busy\n");
        message* new = malloc(sizeof(message));
        new->type=DISCONNECT;
        msgsnd(msgget(clients[msg->id-1], 0), new, SIZE, 0);
        free(new);
        return;
    }
    avail-=2;
    message* new = malloc(sizeof(message));
    new->type = CONNECT;
    new->queue = clients[msg->id-1];
    new->id = msg->id;
    int cq = msgget(clients[msg->other-1], 0);
    msgsnd(cq, new, SIZE, 0);
    new->type=CONNECT;
    new->queue = clients[msg->other-1];
    new->id = msg->other;
    cq = msgget(clients[msg->id-1], 0);
    msgsnd(cq, new, SIZE, 0);
    busy[msg->id-1]=1;
    busy[msg->other-1]=1;
    free(new);
}

void disconnect(message* msg){
    printf("Disconecting\n");
    avail+=2;
    message* new = malloc(sizeof(message));
    new->type = DISCONNECT;
    int cq = msgget(clients[msg->other-1], 0);
    msgsnd(cq, new, SIZE, 0);
    busy[msg->id-1]=-1;
    busy[msg->other-1]=-1;
    free(new);
}

void list(message* msg){
    printf("Listing\n");
    message* new = calloc(1,sizeof(message));
    sprintf(new->txt, "Number of available %d\n", avail-1);
    for(int i=0; i<MAXCLIENTS; i++)
        if(clients[i]!=-1 && i != msg->id-1)
            sprintf(new->txt, "%sID: %d available: %s\n", new->txt, i+1, busy[i]==-1 ? "yes" : "no");
    
    int cq = msgget(clients[msg->id-1], 0);
    new->type = msg->id;
    msgsnd(cq, new, SIZE, 0);
    free(new);
}

void run(){
    message *msg = (message*) malloc(sizeof(message));
    while(1){
        msgrcv(queue, msg, SIZE, PRIOR, 0);
        switch(msg->type){
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
            default:
                list(msg);
        }
    }
}

int main(){
    start();
    run();
    return 0;
}