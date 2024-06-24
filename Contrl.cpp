#include "Contrl.h"
#include "Thrust.h"
#include "Gnss.h"

extern int flag_mode,flag_other;
extern int thruster_value[2];
extern void on_valueChanged( int value,int port_id);
extern int fd;
extern Imu_GNSS imu_pos;
extern MotorEvo evo[2];
extern unsigned char buf_left[BUFSIZ];
extern int UserPoints;
extern int idPointCurent,idReverse;// current point for route

void receivedContrlData(char *Buf,int length)
{
    char buf[BUFSIZ],ch;
    float chanel=0,remote_val=0;
    char str[256];
    int value=0;
    int port=0;
    memcpy (buf, Buf, length);
    buf[length] = 0;    
  
    if(length == 27)  // && imu_pos.year>2024
    {  // 5A 5A 1B 00 12 00 04 A0 03 A0 49 8D EF 63 00 00 00 00 00 00 00 00 00 00 00 00 00 conctrl to main 
        if(buf[0] == '\x5A' && buf[1]== '\x5A' && buf[4]== '\x12')
        {
        //    sprintf(str,"C源目地址%02X%02X %02X%02X", (uint8_t)array_all.at(7),(uint8_t)array_all.at(6),(uint8_t)array_all.at(9),(uint8_t)array_all.at(8));
        //    timestamp  =(uint8_t) array_all.at(10)+(uint8_t)array_all.at(11)*256+
        //            (uint8_t)array_all.at(12)*65536+(uint8_t)array_all.at(13)*16777216;
        //    QDateTime dateTime = QDateTime::fromSecsSinceEpoch(timestamp);
        //    ui->label_Ctime->setText(dateTime.toString("yy-MM-dd hh:mm:ss.s"));
        
            value  =(uint8_t) buf[14]+(uint8_t)buf[15]*256+(uint8_t)buf[16]*65536+(uint8_t)buf[17]*16777216;
            value/=10;
        //printf("C左油门: %d%",value);
            //contrl
            if(flag_mode == 32)
            {
            //导航到一个点，首向取反，推进取反，左右推取反;判断是否在码头
             if(idPointCurent == 0  && idReverse)//imu_pos.lat_gnss > 30.8102118
             {
                value =  value * -1;
                port = FORWARD_RIGHT_PORT;
             }
             else
                port = FORWARD_LEFT_PORT;

             thruster_value[0] = value;
             on_valueChanged(thruster_value[0],port);//FORWARD_LEFT_PORT
            }
            value  =(uint8_t) buf[18]+(uint8_t)buf[19]*256+(uint8_t)buf[20]*65536+(uint8_t)buf[21]*16777216;
            value/=10;
        //    str.sprintf("C右油门: %d%",value);
            if(flag_mode == 32 )//runing not remote
            {
             if(idPointCurent == 0  && idReverse) //imu_pos.lat_gnss > 30.8102118
             {
                value =  value * -1;
                port = FORWARD_LEFT_PORT;
             }
             else
                port = FORWARD_RIGHT_PORT;
    
              thruster_value[1] = value;
              on_valueChanged(thruster_value[1],port);
            }
            value  =(uint8_t) buf[22]+(uint8_t)buf[23]*256+(uint8_t)buf[24]*65536+(uint8_t)buf[25]*16777216;
            printf("thrustLeft=%d,thrustRight=%d\n",thruster_value[0],thruster_value[1]);

            //   char *buf = "you are very pretty~";
            //   2024010119 tql
            if(fd >-1)
            {
                sprintf(buf,"%02d:%02d:%.2f,%.2f,%.2f,%d,%d,%d,%d,%s,%f,%f\r\n",imu_pos.hour,imu_pos.minute,imu_pos.second,imu_pos.contrl_angle,imu_pos.heading/100.0,thruster_value[0],evo[0].speed,thruster_value[1],evo[1].speed,buf_left,imu_pos.lng_gnss,imu_pos.lat_gnss);
              //  sprintf(buf,"%02d:%02d:%.2f,%.2f,%.2f,%d,%d,%d,%d\r\n",imu_pos.hour,imu_pos.minute,imu_pos.second,imu_pos.contrl_angle,imu_pos.heading/100.0,thruster_value[0],evo[0].speed,thruster_value[1],evo[1].speed);
                write(fd,buf,strlen(buf));//写入操作
            }
                
          //  printf("fd=%d %s",fd,buf_left);
           
        }
    }
    // 5A 5A 13 00 11 00 02 A0 04 A0 BF 04 64 65 04 00 00 00 00  心跳包-暂停 下发
    else if(length==19)//
    {
        value  =(uint8_t) buf[14]+(uint8_t)buf[15]*256+(uint8_t)buf[16]*65536+(uint8_t)buf[17]*16777216;
        if(flag_other ==38)
        {
            flag_mode =value;
        }
        
        if(flag_mode ==1 )
        {
            //    ui->label_48->setText("状态：遥控");
            printf("status:romote.\n");
        }
        else if(flag_mode ==2)
        {
            //    ui->label_48->setText("状态：船长");
            printf("status:Master boat.\n");
        }
        else if(flag_mode ==4)
         {
         //   ui->label_48->setText("状态：暂停");
            printf("status:pause.\n");
        }
        else if(flag_mode == 32)
        {
            //    ui->label_48->setText("状态：跑线");
            printf("status:running.\n");
        }
        else if(flag_mode ==38)
        {
            printf("status:point around.\n");
        }
        else
           // ui->label_48->setText("状态：未知");
           printf("Status:unknow.\n");
    }

}