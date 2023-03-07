#include "common.h"

pthread_mutex_t mutex;
pthread_t net;
pthread_t ping;

gamer gamers[GAMERS];
int freeix;
int waitix;

char* path;
struct sockaddr loc_sockaddr;
int loc_sock;
struct sockaddr_in inet_sockaddr;
int inet_sock;

void handl(int signo){
    exit(0);
}

void stop(){
    pthread_cancel(net);
    pthread_cancel(ping);
    close(loc_sock);
    unlink(path);
    close(inet_sock);
}

void cleanGamer(int ix){
    if(gamers[ix].name != NULL) free(gamers[ix].name);
    gamers[ix].name = NULL;
    gamers[ix].game = NULL;
    gamers[ix].fd = -1;
    gamers[ix].enemy = -1;
    if(waitix==ix) waitix=-1;
}

void start(int port){
    loc_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    loc_sockaddr.sa_family = AF_UNIX;
    strcpy(loc_sockaddr.sa_data, path);

    bind(loc_sock, &loc_sockaddr, sizeof(loc_sockaddr));
    listen(loc_sock, GAMERS);
    printf("Loc sock: %d\n", loc_sock);

    inet_sock = socket(AF_INET, SOCK_STREAM, 0);
    inet_sockaddr.sin_family = AF_INET;
    inet_sockaddr.sin_port = htons(port);
    inet_sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    bind(inet_sock, (struct sockaddr*) &inet_sockaddr, sizeof(inet_sockaddr));
    listen(inet_sock, GAMERS);
}

void startGame(int ix1, int ix2){
    gamers[ix1].enemy=ix2;
    gamers[ix2].enemy=ix1;
    if(rand()%2==0){
        gamers[ix1].sym='X';
        gamers[ix2].sym='O';
    }
    else{
        gamers[ix2].sym='X';
        gamers[ix1].sym='O';
    }
    Game* game = malloc(sizeof(game));
    initBoard(game);
    gamers[ix1].game=gamers[ix2].game=game;
    game->winner=gamers[ix1].sym;
    sendMsg(gamers[ix1].fd, START, game, NULL);
    game->winner=gamers[ix2].sym;
    sendMsg(gamers[ix2].fd, START, game, NULL);
    game->winner='.';
}

void cnct(int fd){
    printf("Cnct\n");
    int gfd = accept(fd, NULL, NULL);
    msg msg = getmsg(gfd, 0);
    char* name = calloc(NAME, sizeof(char));
    strcpy(name, msg.name);
    if(freeix==-1){
        sendMsg(gfd, FAIL, NULL, "Server full");
        return;
    }
    int ok=0;
    for(int i=0; i<GAMERS; i++){
        if(gamers[i].fd!=-1 && strcmp(name, gamers[i].name) == 0){
            ok=1;
            break;
        }
    }
    if(ok==1){
        sendMsg(gfd, FAIL, NULL, "Name already taken");
        return;
    }
    sendMsg(gfd, CONNECT, NULL, "Cnctd");
    gamers[freeix].name=name;
    gamers[freeix].active=1;
    gamers[freeix].fd=gfd;
    for(int i=0; i<GAMERS; i++)
        if(gamers[i].name!=NULL) printf("%d - %s\n", i, gamers[i].name);
    if(waitix!=-1){
        startGame(freeix, waitix);
        waitix=-1;
    }
    else{
        waitix=freeix;
        sendMsg(gfd, WAIT, NULL, NULL);
        printf("Wait\n");
    }
    for(int i=0; i<GAMERS; i++){
        if(gamers[i].fd==-1) freeix=i;
    }
    printf("Cnctd\n");
}

void disco(int ix){
    if(ix<0 || ix>=GAMERS) return;
    printf("Disco %s\n", gamers[ix].name);
    if(gamers[ix].fd!=-1){
        shutdown(gamers[ix].fd, SHUT_RDWR);
        close(gamers[ix].fd);
    }
    if(freeix==-1){
        freeix=ix;
    }
}

void status(Game* game){
	int wins[8][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6}};
	for(int i = 0; i < 8; i++){
		if(game->board[wins[i][1]]!='.'&&game->board[wins[i][0]]==game->board[wins[i][1]]&&game->board[wins[i][1]]==game->board[wins[i][2]])
		{
            game->winner = game->board[wins[i][1]];
			return;
		}
	}
	int draw = 1;
	for(int i = 0; i < 9; i++){
		if(game->board[i]=='.'){
			draw = 0;
			break;
		}
	}
    game->winner='.';
	if(draw==1) game->winner='D';
	if(game->turn == 'X') game->turn = 'O';
	else if(game->turn == 'O') game->turn = 'X';
}

void neting(void* arg){
    struct pollfd fds[GAMERS+2];
    fds[GAMERS].fd = loc_sock;
    fds[GAMERS+1].fd = inet_sock;
    while(1){
        pthread_mutex_lock(&mutex);
        for(int i=0; i<GAMERS+2; i++){
            fds[i].events=POLLIN;
            fds[i].revents=0;
            if(i<GAMERS)
                fds[i].fd=gamers[i].fd;
        }
        pthread_mutex_unlock(&mutex);
        poll(fds, GAMERS+2, -1);
        pthread_mutex_lock(&mutex);
        for(int i=0; i<GAMERS+2; i++){
            if(i<GAMERS && gamers[i].fd==-1) continue;

            if(fds[i].revents && POLLIN){
                if(fds[i].fd==loc_sock || fds[i].fd == inet_sock)
                    cnct(fds[i].fd);
                else
                {
                    msg msg = getmsg(fds[i].fd, 0);
                    if(msg.type==MOVE){
                        printf("Move\n");
                        status(&msg.game);
                        if(msg.game.winner=='.')
                            sendMsg(gamers[gamers[i].enemy].fd, MOVE, &msg.game, NULL);
                        else{
                            sendMsg(fds[i].fd, END, &msg.game, NULL);
                            sendMsg(gamers[gamers[i].enemy].fd, END, &msg.game, NULL);
                            free(gamers[i].game);
                        }
                    }
                    else if(msg.type==PING)
                        gamers[i].active=1;
                    else if(msg.type==DISCONNECT){
                        printf("Disco from gamer\n");
                        if(gamers[i].enemy>-1 && gamers[gamers[i].enemy].fd != -1){
                            disco(gamers[i].enemy);
                            cleanGamer(gamers[i].enemy);
                        }
                        disco(i);
                        cleanGamer(i);
                    }
                }
            }
            else if(i<GAMERS && gamers[i].fd!=-1 && fds[i].revents && POLLHUP){
                printf("Disco\n");
                disco(i);
                cleanGamer(i);
            }
        }
        pthread_mutex_unlock(&mutex);
    }
}

void pinging(void* arg){
    while(1){
        sleep(10);
        printf("Ping\n");
        pthread_mutex_lock(&mutex);
        for(int i=0; i<GAMERS; i++){
            if(gamers[i].fd!=-1){
                gamers[i].active=0;
                sendMsg(gamers[i].fd, PING, NULL, NULL);
            }
        }
        pthread_mutex_unlock(&mutex);
        sleep(5);
        pthread_mutex_lock(&mutex);
        for(int i=0; i<GAMERS; i++){
            if(gamers[i].fd!=-1 && gamers[i].active==0){
                printf("%s not responded - disco\n", gamers[i].name);
                sendMsg(gamers[i].fd, DISCONNECT, NULL, NULL);
                if(gamers[i].enemy!=-1 && gamers[gamers[i].enemy].fd!=-1){
                    disco(gamers[i].enemy);
                    cleanGamer(gamers[i].enemy);
                }
                disco(i);
                cleanGamer(i);
            }
        }
        pthread_mutex_unlock(&mutex);
        printf("Pinged\n");
    }
}

int main(int argc, char** argv){
    if(argc!=3){
        printf("Invalid arguments\n");
        return 0;
    }
    srand(time(NULL));
    signal(SIGINT, handl);
    atexit(stop);
    int port = atoi(argv[1]);
    path = argv[2];
    waitix=-1;
    freeix=0;
    
    for(int i=0; i<GAMERS; i++) cleanGamer(i);

    start(port);
    pthread_create(&net, NULL, (void*) neting, NULL);
    pthread_create(&ping, NULL, (void*) pinging, NULL);
    pthread_join(net, NULL);
    pthread_join(ping, NULL);

    stop();
    return 0;
}