#ifndef _MAP_H_
#define _MAP_H_
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>

typedef struct
{
    int id;
    double lng;
    double lat;
    float speed;
    float dis;
}userData;

void receivedMapData(char *Buf,int length); 

#endif