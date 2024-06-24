#include "Sbus.h"
#include "Thrust.h"

extern int flag_mode,flag_other;
extern int thruster_value[2];
extern void on_valueChanged( int value,int port_id);
//extern void receivedSbusData(char *Buf,int length); 

void receivedSbusData(char *Buf,int length)
{
    char buf[BUFSIZ],ch;
    float chanel=0,remote_val=0;
    memcpy (buf, Buf, length);
    buf[length] = 0;     

    if(length == 25)
    {
      if(buf[0] == 0x0F)
      {//0F 1A 51 9F FA D4 A7 3E F5 A9 4F 7D 2A D6 48 ED 8C A7 3E F5 A9 4F 7D 00 00 
             chanel =  ((uint8_t)buf[1] >> 0 | (uint8_t)buf[2] << 8) & 0x7FF;
             chanel = chanel*Ratio_K+Ratio_B;
             remote_val = chanel * Ratio_K2 + Ratio_B2;
             //str.sprintf("%.0f,%.0f",chanel,remote_val);
             
             chanel = ((uint8_t)buf[2] >> 3 | (uint8_t)buf[3] << 5) & 0x7ff;
             chanel = chanel*Ratio_K+Ratio_B;
             remote_val = -(chanel * Ratio_K2 + Ratio_B2);
             // contrl boat
             if(flag_mode ==1 && flag_other == 0)
             {
                thruster_value[0]=remote_val;
                on_valueChanged(thruster_value[0],FORWARD_LEFT_PORT);   
             }
             else if(flag_mode ==0) //tql 202401316 stop
             {
                 thruster_value[0]=0;
                on_valueChanged(thruster_value[0],FORWARD_LEFT_PORT);                  
             }
             else if(flag_mode ==2) //tql 202400327
             {
                 thruster_value[0]=-30;
                on_valueChanged(thruster_value[0],FORWARD_LEFT_PORT); 
             }
             //str.sprintf("%.0f,%.0f",chanel,remote_val);
            
             chanel = ((uint8_t)buf[3] >> 6 | (uint8_t)buf[4] << 2 | (uint8_t)buf[5] << 10) & 0x7ff;
             chanel = chanel*Ratio_K+Ratio_B;
             remote_val = chanel * Ratio_K2 + Ratio_B2;
             // contrl boat
             if(flag_mode ==1 && flag_other == 0)
             {
                thruster_value[1]=remote_val;
                on_valueChanged(thruster_value[1],FORWARD_RIGHT_PORT);
             }  
             else if(flag_mode ==0) //tql 202401316
             {
                thruster_value[1]=0;
                on_valueChanged(thruster_value[1],FORWARD_RIGHT_PORT);              
             }
              else if(flag_mode ==2) //tql 20240327
             {
                thruster_value[1]=-30;
                on_valueChanged(thruster_value[1],FORWARD_RIGHT_PORT);              
             }            
            // printf("Remote_mode=%d,left=%d,right=%d\n", flag_mode,thruster_value[0], thruster_value[1]);
                       
             chanel = ((uint8_t)buf[5] >> 1 | (uint8_t)buf[6] << 7) & 0x7ff;
             chanel = chanel*Ratio_K+Ratio_B;
             remote_val = chanel * Ratio_K2 + Ratio_B2;
             //str.sprintf("%.0f,%.0f",chanel,remote_val);
             
             chanel = ((uint8_t)buf[6] >> 4 | (uint8_t)buf[7] << 4) & 0x7ff;
             chanel = chanel*Ratio_K+Ratio_B;
             remote_val = chanel * Ratio_K2 + Ratio_B2;
             //str.sprintf("%.0f,%.0f",chanel,remote_val);
             if((int)remote_val ==100 && flag_other ==0)
            {
                flag_mode=3;
                return;
            }

             chanel = ((uint8_t)buf[7] >> 7 | (uint8_t)buf[8] << 1 | (uint8_t)buf[9] << 9) & 0x7ff;
             chanel = chanel*Ratio_K+Ratio_B;
             remote_val = chanel * Ratio_K2 + Ratio_B2;
            // str.sprintf("%.0f,%.0f",chanel,remote_val); 
            if((int)remote_val ==100 && flag_other == 0)
            {
                flag_mode=1;
                return;
            }   
                   
             chanel = ((uint8_t)buf[9] >> 2 | (uint8_t)buf[10] << 6) & 0x7ff;
             chanel = chanel*Ratio_K+Ratio_B;
             remote_val = chanel * Ratio_K2 + Ratio_B2;
            // str.sprintf("%.0f,%.0f",chanel,remote_val);
           if((int)remote_val ==100 && flag_other == 0)
            {
                flag_mode=32;
                return;
            }
            else if((int)remote_val ==-100 && flag_other == 0) //tql 20240327
            {
                flag_mode=2;
                return;
            }         
             chanel = ((uint8_t)buf[10] >> 5 | (uint8_t)buf[11] << 3) & 0x7ff;
             chanel = chanel*Ratio_K+Ratio_B;
             remote_val = chanel * Ratio_K2 + Ratio_B2;
            // remote_val = -100;
            // str.sprintf("%.0f,%.0f",chanel,remote_val);
            if((int)remote_val ==100)
            {
                flag_mode=5;
                return;
            } 
            else if((int)remote_val ==-100)
            {
               flag_other = 38;
               return;
            }
            else if((int)remote_val == 0)
            {
               flag_other=0;
               flag_mode=0;
               return;
            }

             chanel = ((uint8_t)buf[12] >> 0 | (uint8_t)buf[13] << 8) & 0x7ff;
             chanel = chanel*Ratio_K+Ratio_B;
             remote_val = chanel * Ratio_K2 + Ratio_B2;
             //str.sprintf("%.0f,%.1f",chanel,remote_val);
            
             chanel = ((uint8_t)buf[13] >> 3 | (uint8_t)buf[14] << 5) & 0x7ff;
             chanel = chanel*Ratio_K+Ratio_B;
             remote_val = chanel * Ratio_K2 + Ratio_B2;
            //  if((int)remote_val ==100)
            //      flag_Remote_mode=1;
            //  else
            //      flag_Remote_mode=0;
              
             chanel = ((uint8_t)buf[14] >> 6 | (uint8_t)buf[15] << 2 | (uint8_t)buf[16] << 10) & 0x7ff;
             chanel = chanel*Ratio_K+Ratio_B;
             remote_val = chanel * Ratio_K2 + Ratio_B2;
             //str.sprintf("%.0f,%.0f",chanel,remote_val);
             
             chanel = ((uint8_t)buf[16] >> 1 | (uint8_t)buf[17] << 7) & 0x7ff;
             chanel = chanel*Ratio_K+Ratio_B;
             remote_val = chanel * Ratio_K2 + Ratio_B2;
             //str.sprintf("%.0f,%.0f",chanel,remote_val);
             
             chanel = ((uint8_t)buf[17] >> 4 | (uint8_t)buf[18] << 4) & 0x7ff;
             chanel = chanel*Ratio_K+Ratio_B;
             remote_val = chanel * Ratio_K2 + Ratio_B2;
             //str.sprintf("%.0f,%.0f",chanel,remote_val);
             
             chanel = ((uint8_t)buf[18] >> 7 | (uint8_t)buf[19] << 1 | (uint8_t)buf[20] << 9) & 0x7ff;
             chanel = chanel*Ratio_K+Ratio_B;
             remote_val = chanel * Ratio_K2 + Ratio_B2;
             //str.sprintf("%.0f,%.0f",chanel,remote_val);
            
             chanel = ((uint8_t)buf[20] >> 2 | (uint8_t)buf[21] << 6) & 0x7ff;
             chanel = chanel*Ratio_K+Ratio_B;
             remote_val = chanel * Ratio_K2 + Ratio_B2;
            // str.sprintf("%.0f,%.0f",chanel,remote_val);
            
             chanel = ((uint8_t)buf[21] >> 5 | (uint8_t)buf[22] << 3) & 0x7ff;
             chanel = chanel*Ratio_K+Ratio_B;
             remote_val = chanel * Ratio_K2 + Ratio_B2;
             //str.sprintf("%.0f,%.0f",chanel,remote_val);
            }
            else if(buf[0] == 0xFF)
            {
             // contrl boat
             // FF 1A 51 9F FA D4 A7 3E F5 A9 4F 7D 2A D6 48 ED 8C A7 3E F5 A9 4F 7D 00 00
             chanel =  ((uint8_t)buf[1] >> 0 | (uint8_t)buf[2] << 8) & 0x7FF;
             chanel = chanel*Ratio_K+Ratio_B;
             remote_val = chanel * Ratio_K2 + Ratio_B2;
             //str.sprintf("%.0f,%.0f",chanel,remote_val);
             
             chanel = ((uint8_t)buf[2] >> 3 | (uint8_t)buf[3] << 5) & 0x7ff;
             chanel = chanel*Ratio_K+Ratio_B;
             remote_val = -(chanel * Ratio_K2 + Ratio_B2);
             // contrl boat
             if(flag_mode ==1 && flag_other == 38)
             {
                thruster_value[0]=remote_val;
                on_valueChanged(thruster_value[0],FORWARD_LEFT_PORT);   
             }
            
             chanel = ((uint8_t)buf[3] >> 6 | (uint8_t)buf[4] << 2 | (uint8_t)buf[5] << 10) & 0x7ff;
             chanel = chanel*Ratio_K+Ratio_B;
             remote_val = chanel * Ratio_K2 + Ratio_B2;
             // contrl boat
             if(flag_mode ==1 && flag_other == 38)
             {
                thruster_value[1]=remote_val;
                on_valueChanged(thruster_value[1],FORWARD_RIGHT_PORT);
             }  

                
            }
    }

}

