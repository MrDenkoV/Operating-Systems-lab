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
#include <sys/ioctl.h>
#include <linux/sockios.h>

#define GAMERS 13
#define MSG 33
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
    struct sockaddr* addr;
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
    if(game==NULL) sprintf(msg, "%d %s", (int) type, name);
    else sprintf(msg, "%d %s %c %c %s", (int) type, name, game->turn, game->winner, game->board);
    write(fd, msg, MSG);
    free(msg);
}

void msgTo(int fd, struct sockaddr* addr, msg_type type, Game* game, char* name){
    char* msg = calloc(MSG, sizeof(char));
    if(game==NULL) sprintf(msg, "%d %s", (int) type, name);
    else sprintf(msg, "%d %s %c %c %s", (int) type, name, game->turn, game->winner, game->board);
    sendto(fd, msg, MSG, 0, addr, sizeof(struct sockaddr));
    free(msg);
}

msg getmsg(int fd){
    msg msg;
    char* buff = calloc(MSG, sizeof(char));
    int size;
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
    msg.type = (msg_type) atoi(tok);

    if(msg.type==CONNECT){
        tok = strtok_r(tmp, " ", &tmp);
        strcpy(msg.name, tok);
    }
    else if(msg.type==PING||msg.type==WAIT||msg.type==DISCONNECT){
        tok = strtok_r(tmp, " ", &tmp);
        strcpy(msg.name, tok);
    }
    else if(msg.type==MOVE||msg.type==START||msg.type==END){
        tok = strtok_r(tmp, " ", &tmp);
        strcpy(msg.name, tok);
        tok = strtok_r(tmp, " ", &tmp);
        msg.game.turn = tok[0];
        tok = strtok_r(tmp, " ", &tmp);
        msg.game.winner = tok[0];
        tok = strtok_r(tmp, " ", &tmp);
        strcpy(msg.game.board, tok);
    }
    free(buff);
    return msg;
}

msg msgFrom(int fd, struct sockaddr* addr, socklen_t len){
    msg msg;
    int size;
    char* buf = calloc(MSG, sizeof(char));
    size=recvfrom(fd, buf, MSG, 0, addr, &len);
    if(size==0){
        msg.type=DISCONNECT;
        free(buf);
        return msg;
    }

    char* tok;
    char* tmp = buf;
    strcpy(msg.name, "");
    initBoard(&msg.game);
    tok=strtok_r(tmp, " ", &tmp);
    msg.type = (msg_type) atoi(tok);

    if(msg.type==CONNECT){
        tok = strtok_r(tmp, " ", &tmp);
        strcpy(msg.name, tok);
    }
    else if(msg.type==PING||msg.type==WAIT||msg.type==DISCONNECT){
        tok = strtok_r(tmp, " ", &tmp);
        strcpy(msg.name, tok);
    }
    else if(msg.type==MOVE||msg.type==START||msg.type==END){
        tok = strtok_r(tmp, " ", &tmp);
        strcpy(msg.name, tok);
        tok = strtok_r(tmp, " ", &tmp);
        msg.game.turn = tok[0];
        tok = strtok_r(tmp, " ", &tmp);
        msg.game.winner = tok[0];
        tok = strtok_r(tmp, " ", &tmp);
        strcpy(msg.game.board, tok);
    }
    free(buf);
    return msg;
}

msg getmsgnblock(int fd){
    msg msg;
    int size;
    char* buf = calloc(MSG, sizeof(char));
    size=recv(fd, buf, MSG, MSG_DONTWAIT);
    if(size<0){
        msg.type=EMPTY;
        free(buf);
        return msg;
    }
    if(size==0){
        msg.type=DISCONNECT;
        free(buf);
        return msg;
    }

    char* tok;
    char* tmp = buf;
    char* p;
    strcpy(msg.name, "");
    initBoard(&msg.game);
    tok=strtok_r(tmp, " ", &tmp);
    msg.type = (msg_type) strtol(tok, &p, 10);

    if(msg.type==CONNECT){
        tok = strtok_r(tmp, " ", &tmp);
        strcpy(msg.name, tok);
    }
    else if(msg.type==PING||msg.type==WAIT||msg.type==DISCONNECT){
        tok = strtok_r(tmp, " ", &tmp);
        strcpy(msg.name, tok);
    }
    else if(msg.type==MOVE||msg.type==START||msg.type==END){
        tok = strtok_r(tmp, " ", &tmp);
        strcpy(msg.name, tok);
        tok = strtok_r(tmp, " ", &tmp);
        msg.game.turn = tok[0];
        tok = strtok_r(tmp, " ", &tmp);
        msg.game.winner = tok[0];
        tok = strtok_r(tmp, " ", &tmp);
        strcpy(msg.game.board, tok);
    }
    free(buf);
    return msg;
}

#endif