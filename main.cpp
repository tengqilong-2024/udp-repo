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
#include "Gnss.h"
#include "Thrust.h"
#include "Sbus.h"
#include "Contrl.h"
#include "char_operate.h"
#include "Map.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define  TIMER_ACTIVE_THRUST       1      
#define  DEBUG_INPUT               0

//#define  SERV_CONTRL_IP        "192.168.2.235"
#define  SERV_MAP_PORT       29000    //  8001
#define  SERV_LOCAL_PORT     29000
#define  SERV_COM_PORT       8001
#define  SERV_CONTRL_PORT    29003   //Mr qing
#define  SERV_MAIN_PORT      29002   //local
#define  SERV_NAVI_PORT      31001
#define  FORWARD_SBUS_PORT   31004

#define  SBUS_INTERVAL_COUNT 5     //sbus get interval

// struct tm
// {
//   int tm_sec;   /* Seconds. [0-60] (1 leap second) */
//   int tm_min;   /* Minutes. [0-59] */
//   int tm_hour;   /* Hours. [0-23] */
//   int tm_mday;   /* Day.  [1-31] */
//   int tm_mon;   /* Month. [0-11] 注意：0代表1月，以此类推*/
//   int tm_year;   /* Year - 1900.  该值为实际年份减去1900*/
//   int tm_wday;   /* Day of week. [0-6] 注意：0代表星期一，以此类推*/
//   int tm_yday;   /* Days in year.[0-365] 从每年的1月1日开始的天数，其中0代表1月1日，以此类推*/
//   int tm_isdst;   /* DST.  [-1/0/1] 夏玲时标识符*/
// };


char* SERVER_CONTR_IP  = "192.168.2.22";   //Mr qing
char* SERVER_MAIN_IP   = "192.168.2.118";  //linux
char* SERVER_LEFT_IP   = "192.168.2.100";  //server data
char* SERVER_RIGHT_IP  = "192.168.2.100";
char* SERVER_MAP_IP    = "192.168.2.22";  // 
char* SERVER_Local_IP  = "192.168.2.119"; // companyip 192.168.1.201
char* SERVER_COM_IP    = "192.168.1.201";//36.26.8.6

int updata = 0;         //GNSS updata
int updata_rtcm =0;     
int flag_mode =1;//0:null 1:remote  2:reverse 3:captain 4:pause 5:stop 32:run route
int flag_other = 38;//38 other auth
char str_all[256];
long long milliseconds_left,milliseconds_right;//thruster updata time
int thruster_value[2]={0}; //contrl real data(sbus)
int forw_back[2]={1,1};   //Forward or backward 0:back

unsigned char buf_left[BUFSIZ];
unsigned char buf_right[BUFSIZ];
int sockfd;
//int fd;  //file
struct sockaddr_in left_addr,right_addr,navi_addr,map_addr,local_addr,com_addr;
extern Imu_GNSS imu_pos;
extern MotorEvo evo[2];
extern int fd_records;
extern char recv_buf[LENGTH_RECV];


char ConvertHexChar(char ch);
int String2Hex(char *str, char *senddata,char tag);
void on_valueChanged( int value,int port_id);
unsigned short Check_ST(unsigned char *p, unsigned int len);

int String2Hex(char *str, char *senddata,char tag)
{//28 04 02 44 22 64 29
 int   hexdata,lowhexdata;   
 int   hexdatalen=0;   
 int   len=strlen(str);   
 //senddata.SetSize(len/2);   
 for(int i=0;i<len;)   
 {   
  char   lstr,hstr = *(str+i);   
  if(hstr == tag)   
  {   
   i++;   
   continue;   
  }   
  i++;   
  if(i>=len)   
   break;   
  lstr= *(str+i);   
  hexdata=ConvertHexChar(hstr);   
  lowhexdata=ConvertHexChar(lstr);   
  if((hexdata==16)||(lowhexdata==16))   
   break;   
  else     
   hexdata=hexdata*16+lowhexdata;   
  i++;   
  *(senddata+hexdatalen)=(char)hexdata;   
  hexdatalen++;   
 }   
 //senddata.SetSize(hexdatalen);  
 *(senddata+hexdatalen)=0; 
 return   hexdatalen;
}

char ConvertHexChar(char ch)
{
  if((ch>='0')&&(ch<='9'))   
  return   ch-0x30;   
  else   if((ch>='A')&&(ch<='F'))   
  return   ch-'A'+10;   
  else   if((ch>='a')&&(ch<='f'))   
  return   ch-'a'+10;   
  else   return   (-1);   

}
void t_handler(int signum)
{
    static int i = 0;
    char buf[BUFSIZ];
    char buf_ret[BUFSIZ];
    int n=0;
    long long value=0;
    unsigned char ch[4][4];
    //每1000微妙进入一次
    i++;

    if(i==1000)
    {//当 1000次时候也就是 1000*1000 = 1000000（1秒）时运行
        struct timeval tv;
        gettimeofday(&tv, NULL);
        long long milliseconds = (tv.tv_sec) * 1000LL + (tv.tv_usec) / 1000;
        int elapsed_left=0,elapsed_right=0;
        if(TIMER_ACTIVE_THRUST)
        {
           elapsed_left = milliseconds - milliseconds_left;
          if(1)//elapsed_left >1000  20240219 by tql
          {
              sprintf(buf,"28 04 02 44 22 64 29 "); 
              n = strlen(buf);
              n= n/3;
              String2Hex(buf,buf_ret,' ');
              n = sendto(sockfd, buf_ret, n, 0, (struct sockaddr *)&left_addr, sizeof(left_addr));  
              usleep(2000);
              sprintf(buf,"28 04 02 44 26 60 29 ");
              n = strlen(buf);
              n= n/3;
              String2Hex(buf,buf_ret,' ');            
              n = sendto(sockfd, buf_ret, n, 0, (struct sockaddr *)&left_addr, sizeof(left_addr));   
              // printf("left=%s ",buf);       
          }
            elapsed_right = milliseconds - milliseconds_right;
          if(1)//elapsed_right >1000   20240219 by tql
          {
              sprintf(buf,"28 04 02 44 22 64 29 "); 
              n = strlen(buf);
              n= n/3;
              String2Hex(buf,buf_ret,' ');
              n = sendto(sockfd, buf_ret, n, 0, (struct sockaddr *)&right_addr, sizeof(right_addr));  
              usleep(2000);
              sprintf(buf,"28 04 02 44 26 60 29 ");
              n = strlen(buf);
              n= n/3;
              String2Hex(buf,buf_ret,' ');            
              n = sendto(sockfd, buf_ret, n, 0, (struct sockaddr *)&right_addr, sizeof(right_addr));        
          }
         // printf("elapsed_left=%d,elapsed_right=%d\n",elapsed_left,elapsed_right); 
        }
        if(navi_addr.sin_port == htons(SERV_CONTRL_PORT))
        {
          //heardbeat 跑线状态的：5A 5A 13 00 11 00 A0 04 A0 03 13 8E EF 63 20 00 00 00 00
            
            long value =(int)(imu_pos.time_second);
            ch[0][0]=value%D256;
            value=value/D256;
            ch[0][1]=value%D256;
            value=value/D256;
            ch[0][2]=value%D256;
            ch[0][3]=value/D256;
            value = flag_mode;//航行状态32位
            ch[1][0]=value%D256;
            value=value/D256;
              ch[1][1]=value%D256;
            value=value/D256;
              ch[1][2]=value%D256;
              ch[1][3]=value/D256;
            sprintf(buf,"5A 5A 13 00 11 00 04 A0 03 A0 %02X %02X %02X %02X",
                        ch[0][0],ch[0][1],ch[0][2],ch[0][3]);
            sprintf(buf_ret," %02X %02X %02X %02X 00 ",
                        ch[1][0],ch[1][1],ch[1][2],ch[1][3]);
            strcat(buf,buf_ret);
             n = strlen(buf);
             n= n/3;
             String2Hex(buf,buf_ret,' ');            
             n = sendto(sockfd, buf_ret, n, 0, (struct sockaddr *)&navi_addr, sizeof(navi_addr));         
        }
         printf("time=%lld,flag_mode=%d,flag_other=%d,left=%d,right=%d,left_speed=%d,right_speed=%d\n",milliseconds,flag_mode,flag_other,thruster_value[0],thruster_value[1],evo[0].speed,evo[1].speed);  
         printf("utc= %02d:%02d:%.2f,lng=%.7f,lat=%.7f,heading=%.2f,fix=%d,Gpsweek=%d,weeksecond=%f\r\n", imu_pos.hour,imu_pos.minute,imu_pos.second,imu_pos.lng_gnss,imu_pos.lat_gnss,imu_pos.heading/100.0,imu_pos.fix,imu_pos.week,imu_pos.week_second); 
       // printf(" time = %lld hello world!\n",milliseconds);  
       // ==record gnss txt by tql 20240201========================================================
        if(fd_records >-1)
        {
            sprintf(buf,"%lld,%.7f,%.7f,%.2f\r\n",milliseconds,imu_pos.lng_gnss,imu_pos.lat_gnss,imu_pos.heading/100.0);
            write(fd_records,buf,strlen(buf));//写入操作
        }
      // ==record gnss txt by tql 20240201========================================================
       i=0; 
    }
    //uint value=0;
    if(i%1000 == 100)
    { 
   
     ////5A 5A 1F 00 20 00 04 A0 03 A0 13 8E EF 63 EC FF FF FF 20 03 00 00 20 03 00 00 00 00 00 00 00 evo feedback
        if(navi_addr.sin_port == htons(SERV_CONTRL_PORT)) 
        {
          //  QDateTime time= QDateTime::currentDateTime();//获取系统当前的时间
          long long left=0;
            value = (int)(imu_pos.time_second);
            ch[0][0]=value%D256;
            value=value/D256;
            ch[0][1]=value%D256;
            value=value/D256;
            ch[0][2]=value%D256;
            ch[0][3]=value/D256;
            value = evo[0].speed;//speed 为返回值 thruster_value控制值
            // value = thruster_value[0];
              if(value <0)
              value = 4294967296 + value; //4294967296
            left = value;
            ch[1][0]=value%D256;
            value=value/D256;
              ch[1][1]=value%D256;
            value=value/D256;
              ch[1][2]=value%D256;
              ch[1][3]=value/D256;
              value = evo[1].speed;
              if(value <0)
              value = 4294967296 + value;
              ch[2][0]=value%D256;
              value=value/D256;
              ch[2][1]=value%D256;
              value=value/D256;
              ch[2][2]=value%D256;
              ch[2][3]=value/D256;

             value = (int)(imu_pos.arrive_times);
              ch[3][0]=value%D256;
              value=value/D256;
              ch[3][1]=value;
              value=flag_mode;
              ch[3][2]=value%D256;
              ch[3][3]=value/D256;
            sprintf(buf,"5A 5A 1F 00 20 00 04 A0 03 A0 %02X %02X %02X %02X 00 00 00 00",
                        ch[0][0],ch[0][1],ch[0][2],ch[0][3]);
            sprintf(buf_ret," %02X %02X %02X %02X %02X %02X %02X %02X 00 00 00 00 00 ",
                        ch[1][0],ch[1][1],ch[1][2],ch[1][3],ch[2][0],ch[2][1],ch[2][2],ch[2][3]);
            strcat(buf,buf_ret);
            //on_btnSend_udp_clicked();//send to udp For hex
             n = strlen(buf);
             n= n/3;
             String2Hex(buf,buf_ret,' ');            
             n = sendto(sockfd, buf_ret, n, 0, (struct sockaddr *)&navi_addr, sizeof(navi_addr));  
            sprintf(buf,"5A 5A 1F 00 20 00 04 A0 02 A0 %02X %02X %02X %02X 00 00 00 00",
                        ch[0][0],ch[0][1],ch[0][2],ch[0][3]);
            sprintf(buf_ret," %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X 00 ",
                        ch[1][0],ch[1][1],ch[1][2],ch[1][3],ch[2][0],ch[2][1],ch[2][2],ch[2][3],ch[3][0],ch[3][1],ch[3][2],ch[3][3]);
          //  printf("speed= %lld,%s",left,buf_ret);
            strcat(buf,buf_ret);
             n = strlen(buf);
             n= n/3;
             String2Hex(buf,buf_ret,' ');  
            // printf("%s ",buf);          
             n = sendto(sockfd, buf_ret, n, 0, (struct sockaddr *)&map_addr, sizeof(map_addr));  //
             n = sendto(sockfd, buf_ret, n, 0, (struct sockaddr *)&local_addr, sizeof(local_addr));
        }
    }
    else if(i%1000 == 200 && imu_pos.week >0 )
    {
    //5A 5A 2E 6C 00 66 08 19 D6 CE 47 2C 6D 03 48 8D 43 5C 12 47 00 BA 8A 7A FF 6A FF 2C 01 90 01 00 00 05 20 20 5A 0A 0A 0A 00 00 00 00 00 00 00 01 00 BA
         value =108;
        ch[0][0]=value%D256;
        value=value/D256;
        ch[0][1]=value%D256;
        value = imu_pos.week;
         ch[0][2]=value%D256;
         ch[0][3]=value/D256;
         sprintf(buf,"5A 5A 2E %02X %02X %02X %02X",ch[0][0],ch[0][1],ch[0][2],ch[0][3]);
         //imu_pos.week_second =371127.187500;
        //  value = FloatTohex(imu_pos.week_second);
        //     ch[1][0]=value%D256;
        //     ch[1][0] = 0x000000FF & ch[1][0];
        //     value=value/D256;
        //     ch[1][1]=value%D256;
        //     ch[1][1] = 0x000000FF & ch[1][1];
        //     value=value/D256;
        //     ch[1][2]=value%D256;
        //     ch[1][2] = 0x000000FF & ch[1][2];
        //     ch[1][3]=value/D256;
        //     ch[1][3] = 0x000000FF & ch[1][3];
        // ==========tql 20240305=====
        float f = imu_pos.week_second;
        unsigned char *hex = (unsigned char *)&f;
          for(int j = 0; j < 4; j++) 
          {
          //   printf("0x%02X ", hex[j]);
             ch[1][j] = hex[j];
          }
          // ==========tql 20240305=====
           sprintf(buf_ret," %02X %02X %02X %02X",ch[1][0],ch[1][1],ch[1][2],ch[1][3]);
         // printf("imu_pos.week_second = %f,%s\n",imu_pos.week_second,buf_ret);
           strcat(buf,buf_ret);
         //  printf("imu_pos.week_second = %f,%s\n",imu_pos.week_second,buf_ret);
         value = imu_pos.lon;
            ch[0][0]=value%D256;
            value=value/D256;
            ch[0][1]=value%D256;
            value=value/D256;
            ch[0][2]=value%D256;
            ch[0][3]=value/D256;
           sprintf(buf_ret," %02X %02X %02X %02X",ch[0][0],ch[0][1],ch[0][2],ch[0][3]);
           strcat(buf,buf_ret);
         value = imu_pos.lat;
            ch[0][0]=value%D256;
            value=value/D256;
            ch[0][1]=value%D256;
            value=value/D256;
            ch[0][2]=value%D256;
            ch[0][3]=value/D256;
           sprintf(buf_ret," %02X %02X %02X %02X",ch[0][0],ch[0][1],ch[0][2],ch[0][3]);
           strcat(buf,buf_ret);

          value = imu_pos.height;
            ch[0][0]=value%D256;
          value=value/D256;
            ch[0][1]=value%D256;
            value = imu_pos.heading;
            ch[0][2]=value%D256;
            ch[0][3]=value/D256;
           sprintf(buf_ret," %02X %02X %02X %02X",ch[0][0],ch[0][1],ch[0][2],ch[0][3]);
           strcat(buf,buf_ret);  
          value = imu_pos.pitch;
            ch[0][0]=value%D256;
          value=value/D256;
            ch[0][1]=value%D256;
            value = imu_pos.roll;
            ch[0][2]=value%D256;
            ch[0][3]=value/D256;
           sprintf(buf_ret," %02X %02X %02X %02X",ch[0][0],ch[0][1],ch[0][2],ch[0][3]);
           strcat(buf,buf_ret);  
          // imu_pos.vel_north=3.0;  //test
          value = imu_pos.vel_north * 0.5144444 * 100;
           if(value<0)
           value = 65536 + value;
            ch[0][0]=value%D256;
          value=value/D256;
            ch[0][1]=value%D256;
          //  imu_pos.vel_east =4.0; //test
            value = imu_pos.vel_east * 0.5144444 * 100;
            if(value<0)
           value = 65536 + value;
            ch[0][2]=value%D256;
            ch[0][3]=value/D256;
           sprintf(buf_ret," %02X %02X %02X %02X",ch[0][0],ch[0][1],ch[0][2],ch[0][3]);
           strcat(buf,buf_ret);  
          value = imu_pos.vel_gnd;
            ch[0][0]=value%D256;
          value=value/D256;
            ch[0][1]=value%D256;
           sprintf(buf_ret," %02X %02X %02X %02X %02X 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ",ch[0][0],ch[0][1],imu_pos.fix_gnss,imu_pos.fix,imu_pos.num);
           strcat(buf,buf_ret);  
           n = strlen(buf);
          n= n/3;
          String2Hex(buf,buf_ret,' ');            
          n = sendto(sockfd, buf_ret, n, 0, (struct sockaddr *)&map_addr, sizeof(map_addr));   //local_addr
          n = sendto(sockfd, buf_ret, n, 0, (struct sockaddr *)&local_addr, sizeof(local_addr));
          n = sendto(sockfd, buf_ret, n, 0, (struct sockaddr *)&com_addr, sizeof(com_addr));
    }
    // if(i%100 ==0)
    // {
    //       ;
    // }
}
void on_valueChanged( int value,int port_id)
{
   int val_pre=0,val_check=0;
   int n=0;
   char buf[BUFSIZ],buf_ret[BUFSIZ];
    unsigned char ch[5]={0};
  //  QString msg,msglab;
    if(value >= 0)
    {
       if(port_id == FORWARD_LEFT_PORT)
        forw_back[0]=1;
       else if(port_id == FORWARD_RIGHT_PORT)
        forw_back[1]=1;
       val_pre = value;
    } 
    else
    {
       if(port_id == FORWARD_LEFT_PORT)
        forw_back[0]=0;
       else if(port_id == FORWARD_RIGHT_PORT)
        forw_back[1]=0;
        val_pre = -value; // 负的转正的
    }
    val_pre = val_pre *TRUST_RITE;
    ch[0]=0x03;
    ch[1]=0x40;
    if(port_id == FORWARD_LEFT_PORT)
    ch[2]= forw_back[0];
    else if(port_id == FORWARD_RIGHT_PORT)
    ch[2]= forw_back[1];
    ch[3]=val_pre ;
    ch[4]=0;
    unsigned short Checksum = Check_ST(ch,4);
    sprintf(buf,"28 04 03 40 %02X %02X %02X 29 ",ch[2],val_pre,Checksum);
   // if(flag_mode ==32 || flag_mode ==1)
    if(FORWARD_LEFT_PORT == port_id)
    {
        //n = strlen(buf);
        //n= n/3;
        n=8;
        String2Hex(buf,buf_ret,' ');
        n = sendto(sockfd, buf_ret, n, 0, (struct sockaddr *)&left_addr, sizeof(left_addr));  
      //  printf("left=%s ",buf);        
    }
    else if(FORWARD_RIGHT_PORT == port_id)
    {
        //n = strlen(buf);
        //n= n/3;
        n=8;
        String2Hex(buf,buf_ret,' ');
        n = sendto(sockfd, buf_ret, n, 0, (struct sockaddr *)&right_addr, sizeof(right_addr)); 
       //  printf("right=%s ",buf);           
    }

}

unsigned short Check_ST(unsigned char *p, unsigned int len)
{
    unsigned short sum=0;
    int i=0;
        for(i=0;i<len;i++)
         sum = sum^(*(p+i));
        return sum;
}

using namespace std;
int main(void)
{
    struct sockaddr_in serv_addr, clie_addr,gnss_addr;
    socklen_t clie_addr_len;
    static int count_sbus = 0;
   unsigned char buf[BUFSIZ],buf_print[BUFSIZ];
    char buf_ret[BUFSIZ];
    char temp[5];
    char str[INET_ADDRSTRLEN];
    int i, n,n_new;
    static char recv_pre[LENGTH_RECV];
    int ret=0;
   // unsigned char sendbuf[BUFSIZ] = {0};    
   // unsigned char recvbuf[BUFSIZ] = {0};
 
  //  memset(&imu_pos,0, sizeof(imu_pos));
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  //  serv_addr.sin_addr.s_addr = inet_addr(SERVER_MAIN_IP);
    serv_addr.sin_port = htons(SERV_MAIN_PORT);

    bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    printf("Accepting connections ...\n");

    // navi
    //bzero(&navi_addr, sizeof(navi_addr));
  //  navi_addr.sin_addr.s_addr = htonl(SERV_NAVI_IP);
    gnss_addr.sin_addr.s_addr=inet_addr(SERVER_LEFT_IP);
    gnss_addr.sin_port = htons(SERV_NAVI_PORT); 
    navi_addr.sin_addr.s_addr = inet_addr(SERVER_CONTR_IP);
    navi_addr.sin_port = htons(SERV_CONTRL_PORT);  
    map_addr.sin_addr.s_addr = inet_addr(SERVER_MAP_IP);
    map_addr.sin_port = htons(SERV_MAP_PORT); 
    local_addr.sin_addr.s_addr = inet_addr(SERVER_Local_IP);
    local_addr.sin_port = htons(SERV_LOCAL_PORT); 
    com_addr.sin_addr.s_addr = inet_addr(SERVER_COM_IP);
    com_addr.sin_port = htons(SERV_COM_PORT); 

    left_addr.sin_addr.s_addr = inet_addr(SERVER_LEFT_IP);
    left_addr.sin_port = htons(FORWARD_LEFT_PORT);    
    right_addr.sin_addr.s_addr = inet_addr(SERVER_RIGHT_IP);
    right_addr.sin_port = htons(FORWARD_RIGHT_PORT);   
    // navi-end
    // ===================settime interal start=====================
    struct itimerval itv;
    //1.设定定时器的初始值
    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = 1000;
    printf("the timer start\n");
    //2.设置定时器什么时候开始
    itv.it_value.tv_sec = 1;
    itv.it_value.tv_usec= 0;
     //3.定时方式
    setitimer(ITIMER_REAL,&itv,NULL);
    //4.信号处理
    signal(SIGALRM,t_handler);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    milliseconds_left = (tv.tv_sec) * 1000LL + (tv.tv_usec) / 1000;
    milliseconds_right = milliseconds_left;
    //time_t start;
    //start = time(NULL);
    //time_t utc_time = get_utc_time();
    //printf("%lld,%lld,%lld",milliseconds_left,start,utc_time);
   //int week = (utc_time -315936000)/604800;
   //  freopen("log.txt", "w", stdout); // 直接写入文件，不进行屏幕打印
       printf("Ver 1.02\n");
    // ===================settime interal start end=====================
    // usleep(10000000);
    // printf("delay 10s\n");
    while (1) {
           // top to send rtcm data     
          if(THREAD_ACTIVE && updata_rtcm >0 && imu_pos.fix>0)//strcmp(recv_buf,recv_pre) !=0
          {
             n = sendto(sockfd, recv_buf, updata_rtcm, 0, (struct sockaddr *)&gnss_addr, sizeof(gnss_addr)); 
           if(DEBUG_INPUT)
             printf("send length=%d ",updata_rtcm);
             updata_rtcm=0;
          }
          // strcpy(recv_pre,recv_buf);
        clie_addr_len = sizeof(clie_addr);
        bzero(&buf, sizeof(buf));  
        n = recvfrom(sockfd, buf, BUFSIZ,0, (struct sockaddr *)&clie_addr, &clie_addr_len);
         
        if (n == -1)
            perror("recvfrom error");
          // 31002- 31002-31003 -29003
          //|| ntohs(clie_addr.sin_port) == SERV_CONTRL_PORT close qing
        if((ntohs(clie_addr.sin_port) >= FORWARD_LEFT_PORT && ntohs(clie_addr.sin_port) <= FORWARD_SBUS_PORT)  || ntohs(clie_addr.sin_port) == SERV_MAP_PORT)
        {
          //bzero(&buf_print, sizeof(buf_print));
           memset(&buf_print, 0, sizeof(buf_print));
           if(n ==19)
          for (i = 0; i < n; i++)//    buf_print[i] = toupper(buf[i]);
          {
            sprintf(temp,"%02X ",buf[i]);
            temp[3]=0;
            strcat((char *)buf_print,temp);
            // memcpy(buf_print+2*i, temp, 2); 
          }    
        //    hex2string(recvbuf,sendbuf,n,&n_new);      
        }

        if( ntohs(clie_addr.sin_port) == SERV_NAVI_PORT)//recive navi,send to SERV_CONTRL_PORT
        {
          
          receivedGnssData((char *)buf,n); 
        // printf("strlen=%d,length=%d",strlen(buf),n); 
          if(updata ==1)
          {
            n = strlen(str_all);
            n= n/3;
            String2Hex(str_all,buf_ret,' ');  
            n = sendto(sockfd, buf_ret, n, 0, (struct sockaddr *)&navi_addr, sizeof(navi_addr)); 
           // printf("test ok\r\n");
          //  printf("send qing=%s\n",str_all);   // tql 20240219
            updata=0;
          }
        }
        else if(ntohs(clie_addr.sin_port) == FORWARD_LEFT_PORT)
        {
          // printf("len=%d",strlen((char *)buf));
          ret =0;
          ret = receivedLeftData((char *)buf,FORWARD_LEFT_PORT,n);  
          if(DEBUG_INPUT)
           printf("%s",buf_print);
          if(buf[3] == 0x25 && ret ==1)
          strcpy((char *)buf_left, (char *)buf_print);
        }
        else if (ntohs(clie_addr.sin_port) == FORWARD_RIGHT_PORT)
        {
          ret =0;
          ret = receivedRightData((char *)buf,FORWARD_RIGHT_PORT,n); 
          if(DEBUG_INPUT)
           printf("%s",buf_print);
          if(buf[3] == 0x25 && ret ==1)
          strcpy((char *)buf_right, (char *)buf_print);
        }
        else if (ntohs(clie_addr.sin_port) == FORWARD_SBUS_PORT)
        {
          if(count_sbus % SBUS_INTERVAL_COUNT ==0)
          {
          receivedSbusData((char *)buf,n);
          if(DEBUG_INPUT)
          printf("%s",buf_print);            
          }
          
        }
        else if(ntohs(clie_addr.sin_port) == SERV_CONTRL_PORT && n ==27) //receive by qing
        {
           receivedContrlData((char *)buf,n);
        //   if(DEBUG_INPUT)
           printf("%s",buf_print);
        }
        else if(ntohs(clie_addr.sin_port) == SERV_MAP_PORT)
        {
          if(n == 19)//mode contrlo
          {
             receivedContrlData((char *)buf,n);
          }
          else if(n == 25)//sbus contrlo
          {
            receivedSbusData((char *)buf,n);
             printf("%s",buf_print);   
          }
          else
          {
            receivedMapData((char *)buf,n);
          }
            
        }
        // feedback data
        // for (i = 0; i < n; i++)
        //     buf[i] = toupper(buf[i]);
        // if(strcmp((char *)buf,"START")==0)
        // {
        //    bzero(&buf, sizeof(buf));
        //    bzero(&buf_ret, sizeof(buf_ret));  
        //   strcat(buf_ret,"connect successed. Ver1.0");
        //   n = sendto(sockfd,buf_ret ,strlen(buf_ret), 0, (struct sockaddr *)&clie_addr, sizeof(clie_addr));			
        // }
        // feedback data end
        if(DEBUG_INPUT)
        if(count_sbus % SBUS_INTERVAL_COUNT ==0)
        {
        printf("received from %s at PORT %d\n",
                inet_ntop(AF_INET, &clie_addr.sin_addr, str, sizeof(str)),
                ntohs(clie_addr.sin_port));          
        }

        if (ntohs(clie_addr.sin_port) == FORWARD_SBUS_PORT)
        {
          count_sbus++;
        }

   
        if (n == -1)
            perror("sendto error");
    }
    close(sockfd);

    return 0;
}

