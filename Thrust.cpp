#include "Thrust.h"
#include "Gnss.h"

MotorEvo evo[2]={0};//0-left thrust; 1-right thrust
extern long long milliseconds_left,milliseconds_right;
extern int forw_back[2];
//extern unsigned short Check_ST(char *p, unsigned int len);

int receivedLeftData(char *Buf,int port_id,int len)
{
     char buf[BUFSIZ]={0};
    memcpy (buf, Buf, sizeof(buf));

    buf[len] = 0;     
  //  char val_sym=0;//0退；1前
  //  char val_sym_2=0;      
  //  short evoLeftSpeed=0,evoRightSpeed=0;


    if(len == 18)//
    {
      //==========check sum start tql 20230124=================有问题，需要修改
        //28 04 0D 25 00 00 00 00 00 00 03 E8 00 00 00 00 C3 29 
        unsigned short Checksum = Check_ST((buf+2),len-4);
        Checksum = Checksum & 0x00FF;
        unsigned short Checkvalue = buf[16] & 0x00FF;
      //printf("len=%d,buf=%02X,checksum=%02x ",len,Checkvalue,Checksum);
  
        if(buf[3]==0x25 && Checksum == Checkvalue)
        {
        //28 04 0D 25 00 00 00 12 02 0D 01 1F 00 1E 00 17 22 29
        evo[0].length =(uint8_t) buf[2];
        evo[0].state = (uint8_t) buf[4]*256 + (uint8_t) buf[5];
        evo[0].power = (uint8_t) buf[6]*256 + (uint8_t) buf[7];
        evo[0].voltage =(uint8_t) buf[8]*256 + (uint8_t) buf[9];
        evo[0].speed =(uint8_t)buf[10]*256 + (uint8_t) buf[11];
        if(forw_back[0] == 0)
        evo[0].speed = -evo[0].speed;
        evo[0].current_m = (uint8_t) buf[12]*256 + (uint8_t) buf[13];
        evo[0].temp_motor =(uint8_t) buf[14]*256 + (uint8_t) buf[15];    
        // printf("speed=%d,%d,%d",evo[0].speed,buf[10],buf[11]);       
        }
        else if(buf[3]==0x27  && Checksum == Checkvalue)
        {
        //28 04 0D 27 00 15 00 14 00 01 00 02 00 01 00 00 29 29
        //time= QTime::currentTime();
        struct timeval tv;
        gettimeofday(&tv, NULL);
        if(port_id == FORWARD_LEFT_PORT)
         milliseconds_left = (tv.tv_sec) * 1000LL + (tv.tv_usec) / 1000;
        else if(port_id == FORWARD_RIGHT_PORT)
         milliseconds_right = (tv.tv_sec) * 1000LL + (tv.tv_usec) / 1000;
     // printf("current time of milliseconds：%lld,utc:%ld\n", milliseconds,imu_pos.time_second);
        evo[0].length =(uint8_t) buf[2];
        evo[0].temp_mos = (uint8_t) buf[4]*256 + (uint8_t) buf[5];    
        evo[0].temp_power = (uint8_t) buf[6]*256 + (uint8_t) buf[7];
        evo[0].current_l =(uint8_t) buf[8]*256 + (uint8_t) buf[9];
        evo[0].time_single =(uint8_t) buf[10]*256 + (uint8_t) buf[11];
        evo[0].time_total = (uint8_t) buf[12]*256 + (uint8_t) buf[13];        
        }
    //  printf("left-temp_mos=%dC,temp_power=%dC,time_single=%dmin,time_total=%dh\r\n",evo[0].temp_mos,evo[0].temp_power,evo[0].time_single,evo[0].time_total);
       return 1;
    }
   return 0;
}

int receivedRightData(char *Buf,int port_id,int len)
{
     char buf[BUFSIZ]={0};
    memcpy (buf, Buf, sizeof(buf));
    buf[len] = 0;     
   // char val_sym=0;//0退；1前   

    if(len == 18)//
    {
    //==========check sum start tql 20230124=================需要修改
    //28 04 0D 25 00 00 00 00 00 00 03 E8 00 00 00 00 C3 29 
        unsigned short Checksum = Check_ST((buf+2),len-4);
        Checksum = Checksum & 0x00FF;
        unsigned short Checkvalue = buf[16] & 0x00FF;
      //printf("len=%d,buf=%02X,checksum=%02x ",len,Checkvalue,Checksum); 
        if(buf[3]==0x25 && Checksum == Checkvalue)
        {
        //28 04 0D 25 00 00 00 12 02 0D 01 1F 00 1E 00 17 22 29
        evo[1].length =(uint8_t) buf[2];
        evo[1].state = (uint8_t) buf[4]*256 + (uint8_t) buf[5];
        evo[1].power = (uint8_t) buf[6]*256 + (uint8_t) buf[7];
        evo[1].voltage =(uint8_t) buf[8]*256 + (uint8_t) buf[9];
        evo[1].speed =(uint8_t) buf[10]*256 + (uint8_t) buf[11];
        if(forw_back[1] == 0)
        evo[1].speed = -evo[1].speed;
        evo[1].current_m = (uint8_t) buf[12]*256 + (uint8_t) buf[13];
        evo[1].temp_motor =(uint8_t) buf[14]*256 + (uint8_t) buf[15];  
        // printf("speed=%d ",evo[1].speed);        
        }
        else if(buf[3]==0x27 && Checksum == Checkvalue)
        {
        //28 04 0D 27 00 15 00 14 00 01 00 02 00 01 00 00 29 29
        struct timeval tv;
        gettimeofday(&tv, NULL);
        milliseconds_right = (tv.tv_sec) * 1000LL + (tv.tv_usec) / 1000;
        evo[1].length =(uint8_t) buf[2];
        evo[1].temp_mos = (uint8_t) buf[4]*256 + (uint8_t) buf[5];    
        evo[1].temp_power = (uint8_t) buf[6]*256 + (uint8_t) buf[7];
        evo[1].current_l =(uint8_t) buf[8]*256 + (uint8_t) buf[9];
        evo[1].time_single =(uint8_t) buf[10]*256 + (uint8_t) buf[11];
        evo[1].time_total = (uint8_t) buf[12]*256 + (uint8_t) buf[13];        
        }
      return 1;
    }
    return 0;
}
