#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> 
#include <sys/signal.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/unistd.h>
#include <sys/errno.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <poll.h>

#define GAMERS 13
#define MSG 20
#define NAME 10

typedef enum msg_type{
    CONNECT,
    DISCONNECT,
    PING,
    WAIT,
    START,
    MOVE,
    END,
    EMPTY,
    FAIL
} msg_type;

typedef struct Game{
    char board[9];
    char turn;
    char winner;
} Game;

typedef struct msg{
    msg_type type;
    Game game;
    char name[NAME];
} msg;

typedef struct gamer{
    char* name;
    Game* game;
    int fd;
    int enemy;
    char sym;
    int active;
} gamer;

void initBoard(Game* game){
    game->turn='O';
    game->winner='.';
    for(int i=0; i<9; i++)
        game->board[i]='.';
}

void sendMsg(int fd, msg_type type, Game* game, char* name){
    char* msg = calloc(MSG, sizeof(char));
    if(type == CONNECT) sprintf(msg, "%d %s", (int) type, name);
    else if(game == NULL) sprintf(msg, "%d", (int) type);
    else sprintf(msg, "%d %c %s", (int) type, game->turn, game->board);
    write(fd, msg, MSG);
    free(msg);
}

msg getmsg(int fd, int nblock){
    msg msg;
    char* buff = calloc(MSG, sizeof(char));
    int size;
    if(nblock){
        size = recv(fd, buff, MSG, MSG_DONTWAIT);
        if(size<0){
            msg.type=EMPTY;
            free(buff);
            return msg;
        }
    }
    else
        size = read(fd, buff, MSG);
    if(size == 0){
        msg.type=DISCONNECT;
        free(buff);
        return msg;
    }

    char* tok;
    char* tmp = buff;
    initBoard(&msg.game);
    strcpy(msg.name, "");
    tok = strtok_r(tmp, " ", &tmp);

    if(nblock){
        char* type;
        msg.type = (msg_type) strtol(tok, &type, 10);
    }
    else msg.type = (msg_type) atoi(tok);
    if(msg.type==CONNECT){
        tok = strtok_r(tmp, " ", &tmp);
        strcpy(msg.name, tok);
    }
    else if(!nblock && (msg.type==PING||msg.type==WAIT||msg.type==DISCONNECT)){
        free(buff);
        return msg;
    }
    else if(msg.type==MOVE||msg.type==START||msg.type==END){
        tok = strtok_r(tmp, " ", &tmp);
        msg.game.turn=tok[0];
        tok = strtok_r(tmp, " ", &tmp);
        strcpy(msg.game.board, tok);
        
    }

    free(buff);
    return msg;
}

#endif