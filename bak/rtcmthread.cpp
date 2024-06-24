#include "rtcmthread.h"
#include    <QTime>
#include    <QDebug>
//#include <QCoreApplication>
#include <WinSock2.h>
#include <Windows.h>
#include <iostream>
#include "ntrip_util.h"
using namespace std;
#pragma comment(lib, "WS2_32.lib")
char gpgga[] = "$GNGGA,012313.40,3048.21801555,N,12049.10315790,E,1,27,0.8,13.0419,M,9.6261,M,,*7B\r\n";
char recv_buf[LENGTH_RECV] = {0};
void RtcmThread::run()
{
    //线程任务
    //on_getRtcm();
    SOCKET  m_sock;
    time_t start, stop;

    char request_data[1024] = {0};
    char userinfo_raw[64] = {0};
    char userinfo[64] = {0};
    char server_ip[] = "39.107.207.235";
    int server_port = 8002;
    char mountpoint[] ="AUTO";
    char user[] = "qxxioe007";
    char passwd[] = "6efc85d";
    //服务端地址,客户端地址
    SOCKADDR_IN socketAddr;
    SOCKADDR_IN clientAddr;
    //填充服务端信息
    socketAddr.sin_family = AF_INET;
    socketAddr.sin_addr.S_un.S_addr = inet_addr("39.107.207.235");//127.0.0.1
    socketAddr.sin_port = htons(8002);//10000
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
    //创建套接字
    m_sock = socket(AF_INET, SOCK_STREAM, 0);//
    /* Connect to caster. */
    int ret = ::connect(m_sock, (struct sockaddr *)&socketAddr, sizeof(struct sockaddr_in));		//server
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
        ret = recv(m_sock, recv_buf, sizeof(recv_buf), 0);
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
    while(!m_stop){
        if (!m_Paused)
        {
            start = time(NULL);
            ret = ::recv(m_sock, recv_buf, sizeof(recv_buf), 0);
            stop = time(NULL);
            if(ret > 0){
                printf("recv data:[%d]time:[%d] used time:[%d]\n", ret, (int)stop,(int)(stop - start));
                print_char(recv_buf, ret);
            }else{
                printf("remote socket close!!!\n");
                break;
            }
        //    time_t t=time(NULL);
        //    qDebug()<< t;
        }
        //    RtcmThread::msleep(1000); //线程休眠500ms
    }

    closesocket(m_sock);

//        while(!m_stop)//循环主体
//        {
//            if (!m_Paused)
//           {
//               // m_diceValue=qrand(); //获取随机数
//               // m_diceValue=(m_diceValue % 6)+1;
//               // m_seq++;
//               // emit newValue(m_seq,m_diceValue);  //发射信号

//            }
//            msleep(500); //线程休眠500ms
//        }

    //  在  m_stop==true时结束线程任务
        quit();//相当于  exit(0),退出线程的事件循环
}
RtcmThread::RtcmThread()
{

}
void RtcmThread::diceBegin()
{
    //开始
        m_Paused=false;
}

void RtcmThread::dicePause()
{
    //暂停
        m_Paused=true;
}

void RtcmThread::stopThread()
{
    //停止线程
        m_stop=true;
}

