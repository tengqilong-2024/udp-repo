#ifndef _CONCTRL_H_
#define _CONCTRL_H_
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

void receivedContrlData(char *Buf,int length); 

#endif