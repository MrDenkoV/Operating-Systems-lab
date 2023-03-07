#ifndef SHARED_H
#define SHARED_H

#include <sys/sem.h>

#define RECE 3
#define SEND 3
#define PACK 3
#define ALL (RECE+SEND+PACK)
#define PROD 5

#define FREEAR 0
#define FREEIX 1
#define PACKIX 2
#define PACKNR 3
#define SENDIX 4
#define SENDNR 5

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
} arg;

#endif