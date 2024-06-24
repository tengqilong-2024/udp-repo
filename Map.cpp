#include "Map.h"
#include "Contrl.h"
#include "Thrust.h"
#include "Gnss.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern Imu_GNSS imu_pos;
int fd_records=-1;

userData map[30] ={0};
int idPointCurent=0;// current point for route
int idReverse=0; // first point reverse flag
int UserPoints=0;
float UserSog=0;    //set navi speed nk/s
float UserRadius=20.0;
float contrl_angle =0;

//extern void CheckArea(double *x);
//extern int ZAngle(double lat1, double lon1, double lat2, double lon2, double *dis, double *ang12);

void receivedMapData(char *Buf,int length)
{
   char buf[BUFSIZ],ch;
    float chanel=0,remote_val=0;
    char str[256];
    int i=0;
    long value=0;
    double dis = 0, angle;
    memcpy (buf, Buf, length);
    buf[length] = 0;    

    if(length > 13)  
    {  //5A 5A 68 00 16 00 02 A0 03 A0 7E 55 9B 65 0A 64 00 14 00 29 C5 E8 55 AC 89 E8 15 80 E3 E8 55 39 74 E8 15 35 05 E9 55 D2 34 E8 15 B3 E7 E8 55 E9 1E E8 15 7D B8 E8 55 14 02 E8 15 00 AB E8 55 94 F4 E7 15 B4 91 E8 55 2A DD E7 15 5D 83 E8 55 03 C7 E7 15 AA 81 E8 55 5C 97 E7 15 ED 8A E8 55 32 7D E7 15 00 00 00 00 00
        if(buf[0] == '\x5A' && buf[1]== '\x5A' && buf[4]== '\x16')
        {
            UserPoints =(uint8_t)buf[14];
            UserSog =(uint8_t)buf[16]*256 + (uint8_t)buf[15];
            UserSog =UserSog/100.0;
            UserRadius = (uint8_t)buf[18]*256 + (uint8_t)buf[17];
            if(imu_pos.lat_gnss > 30.8104490)
            {
                idReverse = 1;
            }
            else
            {
                idReverse = 0;
            }
             idPointCurent =0;
            for(i=0; i<UserPoints; i++)
            {
            map[i].id = i;
            map[i].speed = UserSog;
            value  =(uint8_t) buf[19+i*8]+(uint8_t)buf[20+i*8]*256 +
                    (uint8_t)buf[21+i*8]*65536+(uint8_t)buf[22+i*8]*16777216;          
            map[i].lng = value/SCALE_L_L_Q;
           //lng= value/SCALE_L_L_Q;
            value  =(uint8_t) buf[23+i*8]+(uint8_t)buf[24+i*8]*256+
                    (uint8_t)buf[25+i*8]*65536+(uint8_t)buf[26+i*8]*16777216;
            map[i].lat = value/SCALE_L_L_Q;
           
                if(i>0)
                {
                    ZAngle( map[i-1].lat,  map[i-1].lng, map[i].lat, map[i].lng, &dis, &angle);
                    map[i].dis = dis;
                }
            printf("id=%d,lng=%f,lat=%f,dis=%f\n",map[i].id,map[i].lng,map[i].lat,map[i].dis);    
            }
            //===创建文件，记录实际轨迹 tql 20240201=======
                // ========create log data=========
            sprintf(str,"%04d%02d%02d%02d%02d_%d_gnss.txt",imu_pos.year,imu_pos.month,imu_pos.day,imu_pos.hour,imu_pos.minute,UserPoints);
            fd_records = open(str,O_RDWR|O_CREAT,0600);//createi文件，若不存在就返回负数
            printf("fd=%d,str=%s\n",fd_records,str);
            if(fd_records>0){
                    printf("creat gnss record success!\n");
                   // sprintf(buf,"time,controlAngle,heading,ThrustValue_left,retRPM,ThrustValue_right,retRPM,origin\r\n");
                   // write(fd,buf,strlen(buf));//写入操作
            }
 
    
            //
        }
    }
    else if(buf[0]== '\x5A' && buf[1]== '\x5A' && buf[4]== '\x19')
    {//5A 5A 15 00 19 00 02 A0 03 A0 5B 70 9B 65 C8 00 00 00 00 00 00
      UserSog =(uint8_t)buf[15]*256 + (uint8_t)buf[14];
      UserSog =UserSog/100.0;
    }
}