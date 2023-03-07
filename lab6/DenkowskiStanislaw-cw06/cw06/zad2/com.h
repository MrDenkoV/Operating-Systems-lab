#ifndef com_h
#define com_h

#define MAXCLIENTS 9
#define MAXLEN 1024
#define LEN 8192
#define PRIOR -13
#define PATH "/myserv"

enum type{_, plx, STOP, DISCONNECT, LIST, CONNECT, INIT};

// typedef struct{
//     char txt[MAXLEN];
//     int id;
//     int queue;
// } message;

// const int SIZE = sizeof(message) - sizeof(long);

#endif