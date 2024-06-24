#ifndef _THRUST_H_
#define _THRUST_H_
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

#define  FORWARD_LEFT_PORT   31002
#define  FORWARD_RIGHT_PORT  31003

typedef struct
{
    unsigned short state;
    unsigned short power;    //单位W
    unsigned short voltage;  //0.1V
    short          speed;    //rpm
    unsigned short current_m;  //0.1A
    unsigned short temp_motor;     //motor℃
    unsigned short temp_mos;
    unsigned short temp_power;    //
    unsigned short current_l;    //0.1A
    unsigned short time_single;  //mim
    unsigned short time_total;   //h
    unsigned char length;
    unsigned char ver_soft;
    unsigned char ver_hard;
    unsigned char date_soft;
    unsigned char date_hard;
} MotorEvo;

int receivedLeftData(char *Buf,int port_id,int len);
int receivedRightData(char *Buf,int port_id,int len);

#endif