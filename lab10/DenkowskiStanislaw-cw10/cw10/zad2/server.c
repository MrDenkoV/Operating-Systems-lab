#include "common.h"

pthread_mutex_t mutex;
pthread_t net;
pthread_t ping;

gamer gamers[GAMERS];
int freeix;
int waitix;

int port;
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
    if(gamers[ix].addr != NULL) free(gamers[ix].addr);
    gamers[ix].name = NULL;
    gamers[ix].addr = NULL;
    gamers[ix].game = NULL;
    gamers[ix].fd = -1;
    gamers[ix].enemy = -1;
    if(waitix==ix) waitix=-1;
}

void start(){
    loc_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    loc_sockaddr.sa_family = AF_UNIX;
    strcpy(loc_sockaddr.sa_data, path);

    bind(loc_sock, &loc_sockaddr, sizeof(loc_sockaddr));
    printf("Loc sock: %d\n", loc_sock);

    inet_sock = socket(AF_INET, SOCK_DGRAM, 0);
    inet_sockaddr.sin_family = AF_INET;
    inet_sockaddr.sin_port = htons(port);
    inet_sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    bind(inet_sock, (struct sockaddr*) &inet_sockaddr, sizeof(inet_sockaddr));
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
    msgTo(gamers[ix1].fd, gamers[ix1].addr, START, game, gamers[ix1].name);
    game->winner=gamers[ix2].sym;
    msgTo(gamers[ix2].fd, gamers[ix2].addr, START, game, gamers[ix2].name);
    game->winner='.';
}

void cnct(int fd, struct sockaddr* addr, char* nick){
    char* name = calloc(NAME, sizeof(char));
    strcpy(name, nick);
    printf("Cnct\n");
    if(freeix==-1){
        msgTo(fd, addr, FAIL, NULL, "No space");
        free(name);
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
        msgTo(fd, addr, FAIL, NULL, "Name taken");
        free(name);
        return;
    }
    msgTo(fd, addr, CONNECT, NULL, "Cncted");
    gamers[freeix].name=name;
    gamers[freeix].active=1;
    gamers[freeix].fd=fd;
    gamers[freeix].addr=addr;
    for(int i=0; i<GAMERS; i++)
        if(gamers[i].name!=NULL) printf("%d - %s\n", i, gamers[i].name);
    if(waitix!=-1){
        startGame(freeix, waitix);
        waitix=-1;
    }
    else{
        waitix=freeix;
        msgTo(fd, gamers[freeix].addr, WAIT, NULL, name);
        printf("Wait\n");
    }
    for(int i=0; i<GAMERS; i++){
        if(gamers[i].fd==-1) freeix=i;
    }
    printf("Cnctd\n");
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

int getix(char* name){
    for(int i=0; i<GAMERS; i++)
        if(gamers[i].fd!=-1&&strcmp(gamers[i].name, name)==0)
            return i;
    return -1;
}

void neting(void* arg){
    struct pollfd fds[2];
    fds[0].fd = loc_sock;
    fds[0].events = POLLIN;
    fds[1].fd = inet_sock;
    fds[1].events = POLLIN;
    while(1){
        pthread_mutex_lock(&mutex);
        for(int i=0; i<2; i++){
            fds[i].events=POLLIN;
            fds[i].revents=0;
        }
        pthread_mutex_unlock(&mutex);
        poll(fds, 2, -1);
        pthread_mutex_lock(&mutex);
        for(int i=0; i<2; i++){
            if(fds[i].revents && POLLIN){
                struct sockaddr* addr = malloc(sizeof(struct sockaddr));
                socklen_t len = sizeof(&addr);
                printf("Desc: %d\n", fds[i].fd);
                msg msg = msgFrom(fds[i].fd, addr, len);
                int ix;
                if(msg.type==CONNECT){
                    printf("Cnct\n");
                    cnct(fds[i].fd, addr, msg.name);
                }
                else if(msg.type==MOVE){
                    printf("Move\n");
                    status(&msg.game);
                    ix = getix(msg.name);
                    if(msg.game.winner=='.')
                        msgTo(gamers[gamers[ix].enemy].fd, gamers[gamers[ix].enemy].addr, MOVE, &msg.game, gamers[gamers[ix].enemy].name);
                    else{
                        msgTo(gamers[gamers[ix].enemy].fd, gamers[gamers[ix].enemy].addr, END, &msg.game, gamers[gamers[ix].enemy].name);
                        msgTo(gamers[ix].fd, gamers[ix].addr, END, &msg.game, gamers[ix].name);
                        free(gamers[ix].game);
                    }
                    free(addr);
                }
                else if(msg.type==PING){
                    ix =getix(msg.name);
                    gamers[i].active=1;
                    free(addr);
                }
                else if(msg.type==DISCONNECT){
                    printf("Disco from gamer\n");
                    ix=getix(msg.name);
                    if(gamers[ix].enemy>-1 && gamers[gamers[ix].enemy].fd != -1){
                        msgTo(gamers[gamers[ix].enemy].fd, gamers[gamers[ix].enemy].addr, DISCONNECT, NULL, gamers[gamers[ix].enemy].name);
                        cleanGamer(gamers[ix].enemy);
                    }
                    cleanGamer(ix);
                    free(addr);
                }
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
                msgTo(gamers[i].fd, gamers[i].addr, PING, NULL, gamers[i].name);
            }
        }
        pthread_mutex_unlock(&mutex);
        sleep(5);
        pthread_mutex_lock(&mutex);
        for(int i=0; i<GAMERS; i++){
            if(gamers[i].fd!=-1 && gamers[i].active==0){
                printf("%s not responded - disco\n", gamers[i].name);
                msgTo(gamers[i].fd, gamers[i].addr, DISCONNECT, NULL, gamers[i].name);
                if(gamers[i].enemy!=-1 && gamers[gamers[i].enemy].fd!=-1){
                    msgTo(gamers[gamers[i].enemy].fd, gamers[gamers[i].enemy].addr, DISCONNECT, NULL, gamers[gamers[i].enemy].name);
                    cleanGamer(gamers[i].enemy);
                }
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
    port = atoi(argv[1]);
    path = argv[2];
    waitix=-1;
    freeix=0;
    
    for(int i=0; i<GAMERS; i++) cleanGamer(i);

    start();
    pthread_create(&net, NULL, (void*) neting, NULL);
    pthread_create(&ping, NULL, (void*) pinging, NULL);
    pthread_join(net, NULL);
    pthread_join(ping, NULL);

    stop();
    return 0;
}