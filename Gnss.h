#ifndef _GNSS_H_
#define _GNSS_H_
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


#define  SCALE_L_L_Q        11930464.7111   // 2^31/180 Mr qing lat lon scale value
#define  SCALE_L_l_i        10000000.0   //imu lat lon scale value
#define  PI                 3.141592653589764321
#define  RADI               57.295779513   //180/PI to radion
#define  D256               256

#define  THREAD_ACTIVE             0   //rtk
#define  FORECAST_TIME             1   //加了可能出现异常
#define  SERV_RTCM_PORT            8002
#define  LENGTH_RECV               1024

typedef struct
{
    unsigned char length;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    float second;
    float speed;
    float cog; //gps 方向
    float contrl_angle;
    char  fix_gnss;
    double lat_gnss;
    double lng_gnss;
    int    week;
    double week_second;//周秒
    uint   time_second;//时间戳秒
    unsigned long long  time_Msecond; 
    unsigned short heading; //艏向
    float headingSpeed; //角速度
    short pitch;
    short roll;
    float vel_north;
    float vel_east;
    short vel_gnd;
    int lon;
    int lat;
    short  height;
    unsigned short id;//4字节
    char state;
    char ins_state;
    char fix;
    char num;
    double arrive_times;  //rest times arrive
} Imu_GNSS;


void receivedGnssData(char *NmeaBuf,int length);
void NMEA_MsgDispatch(char* p, int size);
void claculate_Data_q(Imu_GNSS pos);
time_t get_utc_time(void);
long FloatTohex(float HEX);
int setTime();
void CheckArea(double *x);
int ZAngle(double lat1, double lon1, double lat2, double lon2, double *dis, double *ang12);
unsigned short Check_ST(char *p, unsigned int len);
#endif