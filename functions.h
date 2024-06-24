#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_
void NMEA_MsgDispatch(NMEA_ReceiveBuf, NMEA_ReceiveBufSize);
void receivedGnssData(char *NmeaBuf,int length);
#endif
