#include "common.h"

char* name;
char* serv;
int fd;
char sym;
int move;
int type;//0 - INET, 1 - LOCAL
int port;

void Disco(){
    printf("Disco\n");
    sendMsg(fd, DISCONNECT, NULL, name);
    if(type==1)
        unlink(name);
    // exit(0);
}

void handl(int signo){
    exit(0);
}

void cnctLoc(){
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, serv);
    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    struct sockaddr_un c_addr;
    c_addr.sun_family = AF_UNIX;
    strcpy(c_addr.sun_path, name);
    bind(fd, (struct sockaddr*) &c_addr, sizeof(c_addr));
    connect(fd, (struct sockaddr*) &addr, sizeof(addr));
}

void cnctInet(){
    struct sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    addr.sin_addr.s_addr=inet_addr(serv);
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in c_addr;
    c_addr.sin_family = AF_INET;
    c_addr.sin_port = 0;
    c_addr.sin_addr.s_addr = inet_addr(serv);
    bind(fd, (struct sockaddr*) &c_addr, sizeof(c_addr));
    connect(fd, (struct sockaddr*) &addr, sizeof(addr));
}

void printBoard(Game* game){
    printf("\n\n%c:", sym);
    for(int i=0; i<9; i++){
        if(i%3==0) printf("\n");
        printf("%c", game->board[i]);
    }
    printf("\n");
}

void amove(void* arg){
    msg* msggot = (msg*) arg;
    printf("Enter move: ");
    move = getchar() - '0';
    while(move<0 || move>8 || msggot->game.board[move]!='.')
        move=getchar()-'0';
    return;
}

void movd(msg* msggot){
    move = -1;
    pthread_t thread;
    pthread_create(&thread, NULL, (void*) amove, (void*) msggot);
    while(1){
        if(move<0 || move>8 || msggot->game.board[move] != '.'){
            msg rcvd = getmsgnblock(fd);
            if(rcvd.type==PING) sendMsg(fd, PING, NULL, name);
            else if(rcvd.type==DISCONNECT) exit(0);
        }
        else break;
    }
    pthread_join(thread, NULL);
    msggot->game.board[move]=sym;
    printBoard(&msggot->game);
    sendMsg(fd, MOVE, &msggot->game, NULL);
}

void mov(msg msg){
    printf("Your turn\n");
    printBoard(&msg.game);
    movd(&msg);
}

void start(msg msg){
    sym = msg.game.winner;
    printf("You are: %c\n", sym);
    printBoard(&msg.game);
    if(sym=='O') movd(&msg);
    else printf("Enemy's turn\n");
}

void end(Game game){
    printBoard(&game);
    if(game.winner==sym) printf("Gz\n");
    else if(game.winner=='D') printf("Draw\n");
    else printf("nt\n");
    exit(0);
}

void play(){
    while(1){
        msg msg = getmsg(fd);
        switch (msg.type)
        {
        case PING:
            sendMsg(fd, PING, NULL, name);
            break;
        case MOVE:
            mov(msg);
            break;
        case START:
            start(msg);
            break;
        case END:
            end(msg.game);
            break;
        case DISCONNECT:
            exit(0);
            break;
        default:
            printf("W8\n");
            break;
        }
    }
}

void cnct(){
    sendMsg(fd, CONNECT, NULL, name);
    msg msg = getmsg(fd);
    if(msg.type==CONNECT){
        printf("Cnctd\n");
        play();
    }
    else{
        printf("Failed - %s\n", msg.name);
        shutdown(fd, SHUT_RDWR);
        close(fd);
        exit(1);
    }
}

int main(int argc, char** argv){
    if(argc<4){
        printf("Invalid args\n");
        return 0;
    }
    srand(time(NULL));
    signal(SIGINT, handl);
    atexit(Disco);
    name=argv[1];
    if(strcmp(argv[2], "INET")==0) type = 0;
    else type = 1;
    if(type==1){
        serv = argv[3];
        cnctLoc();
    }
    else{
        if(argc!=5){
            printf("Invalid args\n");
            return 0;
        }
        serv=argv[3];
        port=atoi(argv[4]);
        cnctInet();
    }
    printf("Start\n");
    cnct();
    Disco();
}