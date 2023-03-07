#ifndef com_h
#define com_h

#define MAXCLIENTS 10
#define MAXLEN 1024
#define PRIOR -13
#define KEYID 7

enum type{_, plx, STOP, DISCONNECT, LIST, CONNECT, INIT};

typedef struct{
    long type;
    char txt[MAXLEN];
    int id;
    int other;
    key_t queue;
} message;

const int SIZE = sizeof(message) - sizeof(long);

#endif