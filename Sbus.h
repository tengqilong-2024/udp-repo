#ifndef _SBUS_H_
#define _SBUS_H_
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

#define  Ratio_K       0.625    //(1500-1050)/(1002-282)
#define  Ratio_B       873.75  //1050 - Ratio_Y_X*282
#define  Ratio_K2      2/9.0    //(0--100)/(1500-1050)
#define  Ratio_B2     -1000/3.0  // - Ratio_K2*1500
#define  TRUST_RITE   1.27       // Thrust rate

void receivedSbusData(char *Buf,int length); 

#endif