#include <pthread.h>
#include <fcntl.h> 
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <sys/epoll.h> 
#include <netinet/in.h> 
#include "ntrip_util.h"
#include "Gnss.h"

#include <unistd.h> 
#include <arpa/inet.h> 
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include "Map.h"

#define  PA18        0.01745329251994278 /*3.1415926535897/180.0*/
#define  PA          3.1415926535897
#define  MINDOUBLE   1.0E-15
#define  MAXDOUBLE   1.0E15

char* SERVER_RTCM_IP   =      "39.107.207.235";//rtk.ntrip.qxwz.com

extern int updata ;
extern int updata_rtcm;
extern char str_all[256];
extern userData map[30];
extern int idPointCurent,idReverse;// current point for route
extern float UserRadius;
extern float UserSog;
extern int UserPoints;
extern int fd_records;

int fd; //file
int updata_print=0;
char NMEA_ReceiveBuf[BUFSIZ];//接受串口送过来的GPS信号
int NMEA_ReceiveBufSize = 0;
int NMEA_FlagSize = 0;
int NMEA_BufStatus = 0;
Imu_GNSS imu_pos={0};
int flag_rtcm =0;
char recv_buf[LENGTH_RECV] = {0};
char gpgga[] = "$GNGGA,012313.40,3048.21801555,N,12049.10315790,E,1,27,0.8,13.0419,M,9.6261,M,,*7B\r\n";

void *thread_func(void *arg);

void *thread_func(void *arg) {
    printf("Thread stared!\n");
    int m_sock;
	time_t start, stop;

	char request_data[1024] = {0};
	char userinfo_raw[64] = {0};
  	char userinfo[64] = {0};
 
	//char server_ip[] = "rtk.ntrip.qxwz.com";
    char server_ip[] = "39.107.207.235";
	int server_port = 8002;
	char mountpoint[] ="AUTO";
	char user[] = "qxxioe007";
	char passwd[] = "6efc85d";

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = inet_addr(server_ip);
	//printf("test");
	/* Generate base64 encoding of user and passwd. */
	sprintf(userinfo_raw , "%s:%s", user, passwd);
	base64_encode(userinfo_raw, userinfo);
 
 	/* Generate request data format of ntrip. */
	//sprintf(request_data, (const char *)1023,
		sprintf(request_data, 
		"GET /%s HTTP/1.1\r\n"
		"User-Agent: %s\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"Authorization: Basic %s\r\n"
		"\r\n"
		, mountpoint, client_agent, userinfo);
    printf("%s",request_data);

	m_sock = socket(AF_INET, SOCK_STREAM, 0);		//	int socket(int domain, int type, int protocol)
	if(m_sock == -1) {
		printf("create socket fail\n");
		exit(1);
	}

	/* Connect to caster. */
	int ret = connect(m_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));		//server
	if(ret < 0){
		printf("connect caster fail\n");
		exit(1);
	}

	/* Send request data. */
	ret = send(m_sock, request_data, strlen(request_data), 0);		
	if(ret < 0){
		printf("send request fail\n");
		exit(1);
	}

	/* Wait for request to connect caster success. */
	while(1){
		ret = recv(m_sock, (void *)recv_buf, sizeof(recv_buf), 0);
		if(ret > 0 && !strncmp(recv_buf, "ICY 200 OK\r\n", 12)){
			ret = send(m_sock, gpgga, strlen(gpgga), 0);
			if(ret < 0){
				printf("send gpgga data fail\n");
				exit(1);
			}
			printf(gpgga);
			printf("send gpgga data ok. length =%d\n",strlen(gpgga));
			break;
		}
	}

	/* Receive data returned by caster. */
	while(1){
		start = time(NULL);
		ret = recv(m_sock, (void *)recv_buf, sizeof(recv_buf), 0);
		stop = time(NULL);
		if(ret > 0){
			printf("recv data:[%d] used time:[%d]\n", ret, (int)(stop - start));
            updata_rtcm =ret;
		//	print_char(recv_buf, ret);
        //  print_char_hex(recv_buf,ret);
		}else{
			printf("remote socket close!!!\n");
			break;
		}
	}

	close(m_sock);
	//return  0;
  pthread_exit(NULL);
}

void receivedGnssData(char *NmeaBuf,int length)
{
     char buf[BUFSIZ],ch;
    memcpy (buf, NmeaBuf, strlen(NmeaBuf));
    buf[strlen(NmeaBuf)] = 0;     
        
            for (int i = 0;i < length;i++)
        {
            ch = buf[i];
            char bt = ch;//字符型
           //======NMEA解析========
            if (NMEA_BufStatus == 0 && bt == '$')
            {
                NMEA_BufStatus = 1;
                NMEA_ReceiveBufSize = 1;
                NMEA_ReceiveBuf[0] = bt;
            }
            else if (NMEA_BufStatus && bt == 0x0A)
            {
                NMEA_ReceiveBuf[NMEA_ReceiveBufSize] = bt;
                NMEA_ReceiveBufSize++;
                NMEA_ReceiveBuf[NMEA_ReceiveBufSize] = 0;
                NMEA_MsgDispatch(NMEA_ReceiveBuf, NMEA_ReceiveBufSize);
                NMEA_BufStatus = 0;
                NMEA_ReceiveBufSize = 0;
            }
            else if (NMEA_BufStatus)
            {
                NMEA_ReceiveBuf[NMEA_ReceiveBufSize] = bt;
                NMEA_ReceiveBufSize++;
            }
            else
            {
                NMEA_BufStatus = 0;
                NMEA_ReceiveBufSize = 0;
            }
            //======NMEA解析END========
          }
        //   if(updata_print ==1)
        //   {
        //      printf("utc= %02d:%02d:%.2f,lng=%.7f,lat=%.7f,heading=%.2f,fix=%d\r\n", imu_pos.hour,imu_pos.minute,imu_pos.second,imu_pos.lng_gnss,imu_pos.lat_gnss,imu_pos.heading/100.0,imu_pos.fix);  
        //      updata_print=0;         
        //   }

        //  if(imu_pos.lng_gnss ==0 || imu_pos.lat_gnss ==0)
        //  {
        //      printf("\n strlen2=%d, length=%d,%s",strlen(NmeaBuf),length,buf);
        //  }
        
}


 void NMEA_MsgDispatch(char* p, int size)
 {
    int i = 0, j = 0, m = 0, n = 0;
    char m_buf[21][16] = { 0 };
    char ch[256] = { 0 };
    char sum[3] ={0};
    static float headingPro=0;
    static int timestamp =0;
    static int flag_use =0;
   //===check sum start tql 20231211=================
    unsigned short Checksum = Check_ST((p+1),size-6);
     memcpy(ch, (p+size-4), 2); ch[2] = 0;
    sprintf(sum,"%02X",Checksum);
    //i=strcmp(ch,sum);
    // printf("Checksum=%02x,ret_sum=%s,i=%d",Checksum,ch,i);
    if(strcmp(ch,sum) !=0)
    return ;
    //===check sum end ===============================
    //$GPRMC,032100.000,A,3047.3978,N,12045.8720,E,0.000,0.00,030718,,A*75
    //$GNRMC,104802.60,A,3048.21838824,N,12049.10308131,E,0.059,345.6,241123,6.1,W,D*3B company
    if (*(p + 3) == 'R' && *(p + 4) == 'M' && *(p + 5) == 'C')
    {
        for (i = 0; i < size; i++)
        {
            if (j == 1)     // time
                m_buf[j][m++] = *(p + i);
            else if (j == 2)//fix or not
                m_buf[j][m++] = *(p + i);
            else if (j == 3)//lat
                m_buf[j][m++] = *(p + i);
            else if (j == 4)//lat staus
                m_buf[j][m++] = *(p + i);
            else if (j == 5)//long
                m_buf[j][m++] = *(p + i);
            else if (j == 6)//long staus
                m_buf[j][m++] = *(p + i);
            else if (j == 7)//speed
                m_buf[j][m++] = *(p + i);
            else if (j == 8)//direction
                m_buf[j][m++] = *(p + i);
            else if (j == 9)//date
                m_buf[j][m++] = *(p + i);

            if (*(p + i) == ',') { j++; m = 0; }
        }
        memcpy(ch, m_buf[9], 2); ch[2] = 0;
        imu_pos.day = atoi(ch);
        memcpy(ch, m_buf[9] + 2, 2); ch[2] = 0;
        imu_pos.month = atoi(ch);
        memcpy(ch, m_buf[9] + 4, 2); ch[2] = 0;
        imu_pos.year = atoi(ch) + 2000;
        memcpy(ch, m_buf[1], 2); ch[2] = 0;
        imu_pos.hour = atoi(ch) +8;
        imu_pos.hour = imu_pos.hour % 24;
        memcpy(ch, m_buf[1] + 2, 2); ch[2] = 0;
        imu_pos.minute = atoi(ch);
        memcpy(ch, m_buf[1] + 4, 8); ch[6] = 0;
        imu_pos.second = atof(ch);

        //3047.3978
        if(m_buf[2][0] =='A')
        {
           imu_pos.fix_gnss = 1; 
           int ret;
           if(flag_use ==0)
           {
             ret= setTime();//ret == EXIT_SUCCESS
             flag_use=1;
           }
          
            if(flag_use ==1)
            {
            time_t utc_time = get_utc_time();
        //   printf("utc_time = %ld s\n", utc_time);
            struct tm *local_tm = localtime(&utc_time); 
            int myWeek = local_tm->tm_wday;
            //  char str[16];
            // sprintf(str,"%02d",myWeek);
            //  printf("week = %d\n", myWeek);
                imu_pos.time_second =utc_time;
                imu_pos.week_second =myWeek*24*3600 + (imu_pos.hour-8)*3600+imu_pos.minute * 60 + imu_pos.second;
                imu_pos.time_Msecond = imu_pos.week_second *1000;
                // 1980,1,6,0,0,0 =315936000
                imu_pos.week = (utc_time -315936000)/604800;       
            }
        }
        
        else
        imu_pos.fix_gnss = 0;
        //12049.1030813
        imu_pos.lat_gnss= (int)(atof(m_buf[3]) / 100) + (atof(m_buf[3]) - (int)(atof(m_buf[3]) / 100) * 100) / 60.0;
        imu_pos.lng_gnss= (int)(atof(m_buf[5]) / 100) + (atof(m_buf[5]) - (int)(atof(m_buf[5]) / 100) * 100) / 60.0;
        imu_pos.lat=imu_pos.lat_gnss*10000000;
        imu_pos.lon=imu_pos.lng_gnss*10000000;
        imu_pos.speed = atof(m_buf[7]);
        imu_pos.cog = atof(m_buf[8]);

       // get contrl angle
        double dis = 0, angle,escapeTime=0.0;
       if(idPointCurent <UserPoints)
       ZAngle(imu_pos.lat_gnss, imu_pos.lng_gnss, map[idPointCurent].lat, map[idPointCurent].lng, &dis, &angle);
       else
       close(fd_records);  //tql 20240201

       imu_pos.contrl_angle = angle;
       imu_pos.arrive_times = 0.0;
       if(FORECAST_TIME && UserSog>0 && idPointCurent <UserPoints)  //tql tql 20240201
         {
          escapeTime = dis/(UserSog * 0.5144444);//stUserInfo[idPointCurent].sog.toDouble()
          for(i=idPointCurent;i<UserPoints;i++)
          {
              escapeTime = escapeTime + map[i].dis/(UserSog *0.5144444);
          }
          imu_pos.arrive_times = escapeTime;
//          printf("dis =%f,arriveTimes =%f ",dis,imu_pos.arrive_times);
          }
       if(dis < UserRadius && escapeTime >0 )//tql 20210412 
        idPointCurent++; // next point
       // printf("idPoint=%d,%d\n",idPointCurent,UserPoints);
       // imu_pos.speed =1.5;
       // imu_pos.cog =90;
        imu_pos.vel_north = imu_pos.speed  * cos(imu_pos.cog/RADI); // 
        imu_pos.vel_east = imu_pos.speed * sin(imu_pos.cog/RADI);   //* 0.5144444 *100
        if(imu_pos.lng_gnss !=0 && imu_pos.lat_gnss !=0)
        claculate_Data_q(imu_pos); //嵌入式给主控
        updata_print=1;
    }
    //$GPGGA,074310.00,3048.21835436,N,12049.10310831,E,4,19,1.0,18.1663,M,9.7991,M,01,3203*62
    else if(*(p + 3) == 'G' && *(p + 4) == 'G' && *(p + 5) == 'A')
    {
        for (i = 0; i < size; i++)
        {
            if (j == 1)     // time
                m_buf[j][m++] = *(p + i);
            else if (j == 2)// lat
                m_buf[j][m++] = *(p + i);
            else if (j == 3)//lat state
                m_buf[j][m++] = *(p + i);
            else if (j == 4)//lng
                m_buf[j][m++] = *(p + i);
            else if (j == 5)//long state
                m_buf[j][m++] = *(p + i);
            else if (j == 6)// fix
                m_buf[j][m++] = *(p + i);
            else if (j == 7)// fix num
                m_buf[j][m++] = *(p + i);
            else if (j == 8)// hdop
                m_buf[j][m++] = *(p + i);
            else if (j == 9)// height
                m_buf[j][m++] = *(p + i);

            if (*(p + i) == ',') { j++; m = 0; }
        }
         imu_pos.fix = atoi(m_buf[6]);
         imu_pos.height = atof(m_buf[9])*100;
        if(THREAD_ACTIVE && imu_pos.fix > 0 && flag_rtcm ==0)
        {
         //   printf("\nfix =%d",imu_pos.fix);
            strcpy(gpgga,(char *)p);
            pthread_t tid;
            pthread_create(&tid, NULL, thread_func, NULL);
            //=====可以等待特定线程完成执行======
            // pthread_join(tid, NULL);  
            flag_rtcm = 1;
        }

    }
    //$GPHDT,155.4,T*1B
    else if (*(p + 3) == 'H' && *(p + 4) == 'D' && *(p + 5) == 'T')
    {
        for (i = 0; i < size; i++)
        {
            if (j == 1)     // heading
                m_buf[j][m++] = *(p + i);
            else if (j == 2)// T
                m_buf[j][m++] = *(p + i);


            if (*(p + i) == ',') { j++; m = 0; }
        }

         imu_pos.heading = atof(m_buf[1])*100;
        //导航到一个点，首向取反，推进取反，左右推取反;判断是否在码头
        if(idPointCurent == 0 && idReverse)  //imu_pos.lat_gnss > 30.8102118
        {
            imu_pos.heading = imu_pos.heading + 18000 ;
            if(imu_pos.heading >36000)
            imu_pos.heading = imu_pos.heading - 36000;
        }
       //   printf("\nPointCurrent =%d,userPoints=%d,heading=%d",idPointCurent,UserPoints,imu_pos.heading);

         uint t = imu_pos.week_second;
         t =t-timestamp;
         imu_pos.headingSpeed =(imu_pos.heading-headingPro)/(t/10.0);
    //     printf("%02d:%02d:%.1f %.1f,%f,%f\n",imu_pos.hour,imu_pos.minute,imu_pos.second,imu_pos.heading/100.0,imu_pos.lng_gnss,imu_pos.lat_gnss);
    //     str.sprintf("角速度%.2f度/秒",imu_pos.headingSpeed);
    //     ui->label_pitch->setText(str);
    //     headingPro =imu_pos.heading;
    //     timestamp = t;
    }
}
void claculate_Data_q(Imu_GNSS pos)
{
     int usMessageID=0x0010;
     int ip_soure=0xA004;
     int ip_dist=0xA003;
     unsigned char ch[3][4];
     long long val_l;
     char str[256];

    // QString calcu_Data,str;
    // QDateTime time= QDateTime::currentDateTime();//获取系统当前的时间
     uint nTime = (int)(imu_pos.time_second);
     ch[0][0]=nTime%D256;
     nTime=nTime/D256;
      ch[0][1]=nTime%D256;
     nTime=nTime/D256;
      ch[0][2]=nTime%D256;
      ch[0][3]=nTime/D256;
     double m_scale=SCALE_L_l_i / SCALE_L_L_Q;
     int val = pos.lon/m_scale;
     ch[1][0]=val%D256;
     val=val/D256;
      ch[1][1]=val%D256;
     val=val/256;
      ch[1][2]=val%D256;
      ch[1][3]=val/D256;
      val = pos.lat/m_scale;
      ch[2][0]=val%D256;
      val=val/D256;
       ch[2][1]=val%D256;
      val=val/D256;
       ch[2][2]=val%D256;
       ch[2][3]=val/D256;
      sprintf(str_all,"5A 5A 43 00 %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",\
              usMessageID%D256,usMessageID/D256,ip_soure%D256,ip_soure/D256,
                        ip_dist%D256,ip_dist/D256,ch[0][0],ch[0][1],ch[0][2],ch[0][3],
                        ch[1][0],ch[1][1],ch[1][2],ch[1][3],ch[2][0],ch[2][1],ch[2][2],ch[2][3]);

     val = pos.roll*10/RADI;
     ch[1][0]=val%D256;
     val=val/D256;
      ch[1][1]=val%D256;
     val=val/D256;
      ch[1][2]=val%D256;
      ch[1][3]=val/D256;
      sprintf(str," %02X %02X %02X %02X",ch[1][0],ch[1][1],ch[1][2],ch[1][3]);
      strcat(str_all, str);

     val = pos.pitch*10/RADI;
     ch[1][0]=val%D256;
     val=val/D256;
      ch[1][1]=val%D256;
     val=val/D256;
      ch[1][2]=val%D256;
      ch[1][3]=val/D256;
      sprintf(str," %02X %02X %02X %02X",ch[1][0],ch[1][1],ch[1][2],ch[1][3]);
      strcat(str_all, str);
      //calcu_Data = calcu_Data +str;
     val = pos.heading*10/RADI;
     ch[1][0]=val%D256;
     val=val/D256;
      ch[1][1]=val%D256;
     val=val/D256;
      ch[1][2]=val%D256;
      ch[1][3]=val/D256;
      sprintf(str," %02X %02X %02X %02X",ch[1][0],ch[1][1],ch[1][2],ch[1][3]);
      strcat(str_all, str);
      //calcu_Data = calcu_Data +str;

      val = pos.vel_north*1.852/3.6*100;
      if(val<0)
        val_l =4294967296 + val;
          else
        val_l =val;

      ch[1][0]=val_l%D256;
      val_l=val_l/D256;
       ch[1][1]=val_l%D256;
      val_l=val_l/D256;
       ch[1][2]=val_l%D256;
       ch[1][3]=val_l/D256;
       sprintf(str," %02X %02X %02X %02X",ch[1][0],ch[1][1],ch[1][2],ch[1][3]);
       strcat(str_all, str);
    //   calcu_Data = calcu_Data +str;

      val = pos.vel_east*1.852/3.6*100;
      if(val<0)
       val_l =4294967296 + val;
      else
        val_l =val;

      ch[1][0]=val_l%D256;
      val_l=val_l/D256;
       ch[1][1]=val_l%D256;
      val_l=val_l/D256;
       ch[1][2]=val_l%D256;
       ch[1][3]=val_l/D256;
       sprintf(str," %02X %02X %02X %02X",ch[1][0],ch[1][1],ch[1][2],ch[1][3]);
       strcat(str_all, str);
    //   calcu_Data = calcu_Data +str;

      sprintf(str," 00 00 00 00 00 00 00 00 00 00 00 00");//橫摇,纵摇,艏摇(后补)
      strcat(str_all, str);
    //  calcu_Data = calcu_Data +str;
      val = pos.height;
      ch[1][0]=val%D256;
      val=val/D256;
       ch[1][1]=val%D256;
      val=val/D256;
       ch[1][2]=val%D256;
       ch[1][3]=val/D256;
       sprintf(str," %02X %02X %02X %02X",ch[1][0],ch[1][1],ch[1][2],ch[1][3]);
       strcat(str_all, str);

 //      long long timestamp = imu_pos.time_Msecond;
 //      val_l =timestamp;
        struct timeval tv;
        gettimeofday(&tv, NULL);
        long long milliseconds = (tv.tv_sec) * 1000LL + (tv.tv_usec) / 1000;
        val_l =milliseconds;
       ch[1][0]=val_l%D256;
       val_l=val_l/D256;
        ch[1][1]=val_l%D256;
       val_l=val_l/D256;
        ch[1][2]=val_l%D256;
       val_l=val_l/D256;
        ch[1][3]=val_l%D256;
        val_l=val_l/D256;
         ch[2][0]=val_l%D256;
         val_l=val_l/D256;
         ch[2][1]=val_l%D256;
         val_l=val_l/D256;
         ch[2][2]=val_l%D256;
         val_l=val_l/D256;
         ch[2][3]=val_l;
        sprintf(str," %02X %02X %02X %02X %02X %02X %02X %02X 00 ",ch[1][0],ch[1][1],ch[1][2],ch[1][3],
                     ch[2][0],ch[2][1],ch[2][2],ch[2][3]);
        strcat(str_all, str);   
        updata=1;  
    //   printf("second=%ld,Msecond=%llu\r\n", imu_pos.time_second,imu_pos.time_Msecond);        
    //    calcu_Data = calcu_Data +str;

   //  ui->editMsg_q->setText(calcu_Data);
   //  on_btnSend_udp_clicked();//send to udp For hex
}
time_t get_utc_time(void)
{
    return time(NULL);
}
unsigned short Check_ST(char *p, unsigned int len)
{
    unsigned short sum=0;
    int i=0;
        for(i=0;i<len;i++)
         sum = sum^(*(p+i));
        return sum;
}
//float to hex
long FloatTohex(float HEX)
{
    return *( long *)&HEX;
}
//#include <sys/time.h> 
int setTime()
{
    struct tm newTime; // 存放新的时间结构体变量
    char str[64];
    char buf[BUFSIZ];
    // 初始化newTime结构体
    memset(&newTime, 0, sizeof(struct tm));
    newTime.tm_year = imu_pos.year - 1900; // 年份减去1900得到正确的值
    newTime.tm_mon = imu_pos.month - 1;         // 月份范围是[0, 11]，所以需要减去1
    newTime.tm_mday = imu_pos.day;            // 第几号
    newTime.tm_hour = imu_pos.hour-8;            // 小时
    newTime.tm_min = imu_pos.minute;             // 分钟
    newTime.tm_sec = (int)imu_pos.second;             // 秒数
 
    // 转换为time_t格式并设置系统时间 *** s实际系统中需要打开此功能****
    time_t t = mktime(&newTime);
    if (stime(&t) == -1) {
        perror("Failed to set system time");
        exit(EXIT_FAILURE);
    } else {
        printf("System time has been successfully set.\n");
    }

    // ========create log data=========
    sprintf(str,"%04d%02d%02d_log.csv",imu_pos.year,imu_pos.month,imu_pos.day);
    fd = open(str,O_RDWR);//打开lainxi文件，若不存在就返回负数
    if(fd == -1){
            printf("open file faild \n");//不存在就创建文件
            fd=open(str,O_RDWR|O_CREAT,0600);
            if(fd>0){
                    printf("creat success!\n");
                    sprintf(buf,"time,controlAngle,heading,ThrustValue_left,retRPM,ThrustValue_right,retRPM,origin\r\n");
                    write(fd,buf,strlen(buf));//写入操作
            }
    }
    else
    {
        lseek(fd,0,SEEK_END);
    }
    

    return EXIT_SUCCESS;
}

void CheckArea(double *x)
{
    if ((*x < MINDOUBLE) && (*x >= 0)) *x = MINDOUBLE;
    else if (((-(*x)) < MINDOUBLE) && (*x <= 0.0)) *x = -MINDOUBLE;
    else if (*x > MAXDOUBLE) *x = MAXDOUBLE;   /* maybe we need not  two line of following*/
    else if (*x <= -MAXDOUBLE) *x = -MAXDOUBLE;
}

int ZAngle(double lat1, double lon1, double lat2, double lon2, double *dis, double *ang12)
{

    long int i;
    double rp;
    double ep2;
    double La, Ff;
    double u1, u2, lamd, snu1, snu2, csu1, csu2, csl, ssgm;
    double l, snl, sccl, csgm, sgm, snm, csm2, c, cs2sgm, e, k1, b, a, mit, dsgm, s, fz;
    double fm, a12, alfa12;
    double b1, l1, b2, l2;
    double temk;
    *dis = 0;
    *ang12 = 0;
    La = 6378137.0;
    rp = 6378137.0;
    Ff = (La - rp) / La;
    ep2 = (La*La - rp*rp) / (rp*rp);
    if ((lat1 > 90) || (lat2 > 90) || (lon1 > 180) || (lon2 > 180))
        return(0);
    if ((lat1 < -90) || (lat2 < -90) || (lon1 < -180) || (lon2 < -180))
        return(0);
    b1 = lat1*PA18;
    l1 = lon1*PA18;
    b2 = lat2*PA18;
    l2 = lon2*PA18;
    u1 = atan((1 - Ff)*tan(b1));
    u2 = atan((1 - Ff)*tan(b2));

    lamd = l2 - l1;

    snu1 = sin(u1);
    snu2 = sin(u2);
    csu1 = cos(u1);
    csu2 = cos(u2);
    i = 0;
l1: l = lamd;
    i = i + 1;
    snl = sin(l);
    csl = cos(l);
    sccl = snu1*csu2*csl;
    ssgm = sqrt((csu2*snl*csu2*snl) + (csu1*snu2 - sccl)*(csu1*snu2 - sccl));
    csgm = snu1*snu2 + csu1*csu2*csl;

    CheckArea(&csgm);
    CheckArea(&ssgm);

    sgm = atan(ssgm / csgm);
    if (sgm > 0)
    {
        if (ssgm < 0) sgm = sgm - PA;
    }
    else
    {
        if (ssgm > 0) sgm = sgm + PA;
    }
    CheckArea(&ssgm);
    snm = csu1*csu2*snl / ssgm;
    csm2 = 1 - snm*snm;
    CheckArea(&csm2);
    c = Ff / 16 * csm2*(4 + Ff*(4 - 3 * csm2));

    CheckArea(&csm2);
    cs2sgm = csgm - 2 * snu1*snu2 / csm2;
    e = 2 * cs2sgm*cs2sgm - 1;
    CheckArea(&e);
    CheckArea(&cs2sgm);
    lamd = l2 - l1 + (1 - c)*Ff*snm*(sgm + c*ssgm*(cs2sgm + e*c*csgm));
    temk = l - lamd;
    if (temk<0) temk = -temk;
    if ((temk>0.3*(10E-11)) && (i < 30)) goto l1;
    if (i >= 30)
    {

        *dis = 90 * PA18*6378137.0;
        if (lon2 >= lon1) *ang12 = 90; else *ang12 = 270;
        return(1);
    }
    if (csm2 < 0) csm2 = 0;
    k1 = (sqrt(1 + ep2*csm2) - 1) / (sqrt(1 + ep2*csm2) + 1);
    CheckArea(&k1);
    b = k1*(1 - 3 / 8 * k1*k1);
    temk = 1 - k1;
    CheckArea(&temk);
    a = (1 + k1*k1 / 4) / temk;
    mit = b / 6 * cs2sgm*(4 * ssgm*ssgm - 3)*(2 * e - 1);
    dsgm = b*ssgm*(cs2sgm + b / 4 * (e*csgm - mit));
    s = (sgm - dsgm)*rp*a;
    if (s < 0) s = -s;
    fz = csu2*snl;
    fm = csu1*snu2 - sccl;
    CheckArea(&fm);
    a12 = atan(fz / fm);
    alfa12 = a12;
    if (fm < 0)
        alfa12 = a12 + PA;
    else
        if (fz < 0) alfa12 = 2 * PA + a12;
    *dis = s;
    *ang12 = alfa12 / PA18;
    return(1);
}