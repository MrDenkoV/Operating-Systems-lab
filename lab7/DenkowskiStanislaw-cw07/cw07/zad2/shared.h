#ifndef SHARED_H
#define SHARED_H

#include <sys/sem.h>

#define RECE 3
#define SEND 3
#define PACK 3
#define ALL (RECE+SEND+PACK)
#define PROD 5
#define SEMSNR 6

#define FREEAR 0
#define FREEIX 1
#define PACKIX 2
#define PACKNR 3
#define SENDIX 4
#define SENDNR 5

const char* MEM = "/shared";
const char* NAMES[6] = {"/FREEAR", "/FREEIX", "/PACKIX", "/PACKNR", "/SENDIX", "/SENDNR"};

#endif