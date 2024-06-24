
#ifndef _char_operate_h_
#define _char_operate_h_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

enum command_type
{
	reset=1,
	feedback=2,
	beamset=3,
	paon=4,
	paoff=5,
	update=6,
	wareset=7
};

enum object_type
{
	control=1,
	radiate103=2,
	switch103=3,
	couple103=4
};
	

int hex2string(unsigned char *hex,unsigned char *ascII,int len,int *newlen)
{
	int i=0;
	char newchar[100]={0};
	*newlen=len*3;
	for(i=0;i<len;i++)
	{
		sprintf(newchar,"%02X",hex[i]);
		strcat((char *)ascII,newchar);
	}
	return 0;
}



bool fromHexChar(char c,unsigned char *outChar)
{	
	//printf("%c\r\n",c);
	
	if('0'<=c&&c<='9')
	{
		*outChar=c-'0';
	}
	else if('a'<=c&&c<='f')
	{
		*outChar=c-'a'+10;
	}
	else if('A'<=c&&c<='F')
	{
		*outChar=c-'A'+10;
	}
	else
	{
		
		return false;
	}
	return true;
}

bool getlen(char *dst,char *src,int srclen,int *len)
{
	if(srclen%2==1)
	{
		return false;
	}
	int i=0,j;

//	printf("%d\r\n",srclen);
	for(j=1;j<srclen;j=+2)
	{
		unsigned char a=0x00;
		unsigned char b=0x00;	
			
		//printf("%c\r\n",src[j-1]);
		if(!fromHexChar(src[j-1],&a)||!fromHexChar(src[j],&b))
		{
		//	printf("!!!!\r\n");
			return false;
		}
		dst[i]=(a<<4)|b;
		i++;	
		
	}
	//printf("%d\r\n",j);
	*len=i;
	
	return true;
}	

int str2Hex(char *outHex,char *inStr,int inStrLen)
{
	int len=0;
	char *inStr2=NULL;
	getlen(inStr,inStr,inStrLen,&len);
	
	if(true)
	{
       //printf("%s\r\n",inStr);
		memcpy(outHex,inStr2,len);
		return len;
	}
	return -1;
}


int str2Hex2(void *outHex,char *inStr,int inStrLen)
{
	
	if(inStrLen%2==1)
	{
		return -1;
	}
	int len=0;
	int i;
   // unsigned char *result=NULL;
	unsigned char a;
	unsigned char b;
	unsigned char c[inStrLen/2];
	for(i=0;i<inStrLen/2;i++)
	{
		fromHexChar(inStr[2*i],&a);
		fromHexChar(inStr[2*i+1],&b);
		//outHex[i]=(a<<4)|b;
		c[i]=(a<<4)|b;
	//	printf("%u\r\n",c[i]);
	}
	len=inStrLen/2;
	memcpy(outHex,c,len);
	return len;

}

void convertStrToUnChar(char *str,unsigned char* Unchar)
{
	int i=strlen(str);
	int j=0,counter=0;
	char c[2];
	unsigned int bytes[2];
	for(j=0;j<i;j+=2)
	{
		if(j%2==0)
		{
			c[0]=str[j];
			c[1]=str[j+1];
			sscanf(c,"%02x",&bytes[0]);
			Unchar[counter]=bytes[0];
			counter++;
		}
	}
	return;
}
		
size_t convert_hex(uint8_t *dest,int count,const char *src)
{
	char buf[3];
	int i;
	int value;
	for(i=0;i<count&&*src;i++)
	{
		buf[0]=*src++;
		buf[1]='\0';
		
		if(*src)
		{
			buf[1]=*src++;
			buf[2]='\0';
		}
		if(sscanf(buf,"%x",&value)!=1)
		{
			break;
		}		
		dest[i]=value;
		
	}
	return i;
}

uint8_t crc_check(uint8_t *prc,int len)
{
	uint8_t crc_out=0;
	int i=0;	
	for(i=0;i<len;i++)
	{
		//printf("%c\r\n",prc[i]);
		crc_out+=prc[i];		
	}
	crc_out=~crc_out;
	crc_out++;	
	return crc_out;
	
}

char crc_check2(char *prc,int len)
{
	char crc_out=0;
	int i=0;	
	for(i=0;i<len;i++)
	{
		//printf("%c\r\n",prc[i]);
		crc_out+=prc[i];		
	}
	crc_out=~crc_out;
	crc_out++;	
	return crc_out;
	
}

size_t convert_hex_inv(char *dest,int count,const uint8_t *src)
{
	int i=0;		
	for(i=0;i<count&&sprintf(dest+i*2,"%02X",src[i])==2;i++)
	{
	    
	};	
	return i;
}

size_t calc_char(char *src,int len,char *out)
{
	// char a2[32]={0};
   // int newlen;
	size_t i=0;
	char crc_out;
	int src_len=len;
	crc_out=crc_check2(src,src_len);	
	for(i=0;i<len;i++)
	{
		out[i+2]=src[i];
	}
	out[0]=0xEB;
	out[1]=0x90;
	out[i+2]=crc_out;
	out[i+3]=0x0D;out[i+4]=0x0A;
	//hex2string(out,a2,18,&newlen);
    /*for(i=0;i<36;i++)
    {
		printf("%c\r\n",a2[i]);
	}*/
	i=src_len+4+1;
	
	return i;
}

size_t gene_command(int type,int object,char *outbuf)
{
   size_t i=0;   
   char inbuf[64]={0}; 
   if(type==reset)
   {
	   if(object==control)
	   {
		   inbuf[0]=0x12;inbuf[1]=0x01;inbuf[2]=0x00;inbuf[3]=0x00;
		   inbuf[4]=0x24;inbuf[5]=0x42;inbuf[6]=0x4B;inbuf[7]=0x5F;
		   inbuf[8]=0x52;inbuf[9]=0x65;inbuf[10]=0x73;inbuf[11]=0x65;inbuf[12]=0x74;
		   calc_char(inbuf,13,outbuf);
		   i=13+5;
	   }
	   else if(object==radiate103)
	   {
		   inbuf[0]=0x12;inbuf[1]=0x10;inbuf[2]=0x01;inbuf[3]=0x01;
		   inbuf[4]=0x24;inbuf[5]=0x42;inbuf[6]=0x4B;inbuf[7]=0x5F;
		   inbuf[8]=0x52;inbuf[9]=0x65;inbuf[10]=0x73;inbuf[11]=0x65;inbuf[12]=0x74;
		   calc_char(inbuf,13,outbuf);
		   i=13+5;
	   }
	   else if(object==switch103)
	   {
		   inbuf[0]=0x12;inbuf[1]=0x20;inbuf[2]=0x01;inbuf[3]=0x03;
		   inbuf[4]=0x24;inbuf[5]=0x42;inbuf[6]=0x4B;inbuf[7]=0x5F;
		   inbuf[8]=0x52;inbuf[9]=0x65;inbuf[10]=0x73;inbuf[11]=0x65;inbuf[12]=0x74;
		   calc_char(inbuf,13,outbuf);
		   i=13+5;
	   }
	   else if(object==couple103)
	   {
		   inbuf[0]=0x12;inbuf[1]=0x30;inbuf[2]=0x01;inbuf[3]=0x02;
		   inbuf[4]=0x24;inbuf[5]=0x42;inbuf[6]=0x4B;inbuf[7]=0x5F;
		   inbuf[8]=0x52;inbuf[9]=0x65;inbuf[10]=0x73;inbuf[11]=0x65;inbuf[12]=0x74;
		   calc_char(inbuf,13,outbuf);
		   i=13+5;
	   }
   }
   else if(type==feedback)
   {
	   if(object==control)
	   {
		   inbuf[0]=0x15;inbuf[1]=0x01;inbuf[2]=0x00;inbuf[3]=0x00;
		   inbuf[4]=0x24;inbuf[5]=0x42;inbuf[6]=0x4B;inbuf[7]=0x5F;
		   inbuf[8]=0x46;inbuf[9]=0x65;inbuf[10]=0x65;inbuf[11]=0x64;
		   inbuf[12]=0x62;inbuf[13]=0x61;inbuf[14]=0x63;inbuf[15]=0x6B;
		   calc_char(inbuf,16,outbuf);
		   i=16+5;
	   }
	   else if(object==radiate103)
	   {
		   inbuf[0]=0x15;inbuf[1]=0x10;inbuf[2]=0x01;inbuf[3]=0x01;
		   inbuf[4]=0x24;inbuf[5]=0x42;inbuf[6]=0x4B;inbuf[7]=0x5F;
		   inbuf[8]=0x46;inbuf[9]=0x65;inbuf[10]=0x65;inbuf[11]=0x64;
		   inbuf[12]=0x62;inbuf[13]=0x61;inbuf[14]=0x63;inbuf[15]=0x6B;
		   calc_char(inbuf,16,outbuf);
		   i=16+5;
	   }
	   else if(object==switch103)
	   {
		   inbuf[0]=0x15;inbuf[1]=0x20;inbuf[2]=0x01;inbuf[3]=0x03;
		   inbuf[4]=0x24;inbuf[5]=0x42;inbuf[6]=0x4B;inbuf[7]=0x5F;
		   inbuf[8]=0x46;inbuf[9]=0x65;inbuf[10]=0x65;inbuf[11]=0x64;
		   inbuf[12]=0x62;inbuf[13]=0x61;inbuf[14]=0x63;inbuf[15]=0x6B;
		   calc_char(inbuf,16,outbuf);
		   i=16+5;
	   }
	   else if(object==couple103)
	   {
		   inbuf[0]=0x15;inbuf[1]=0x30;inbuf[2]=0x01;inbuf[3]=0x02;
		   inbuf[4]=0x24;inbuf[5]=0x42;inbuf[6]=0x4B;inbuf[7]=0x5F;
		   inbuf[8]=0x46;inbuf[9]=0x65;inbuf[10]=0x65;inbuf[11]=0x64;
		   inbuf[12]=0x62;inbuf[13]=0x61;inbuf[14]=0x63;inbuf[15]=0x6B;
		   calc_char(inbuf,16,outbuf);
		   i=16+5;
	   }
   }
   else if(type==beamset)
   {
   }
   else if(type==paon)
   {
   }
   else if(type==paoff)
   {
   }
   else if(type==update)
   {
   }
   else if(type==wareset)
   {
   }
   else 
   {}	   
   return i;
}

int comparearr(unsigned char *data1, unsigned char*data2,int begin_num,int end_num)
{
	int i;
	int count=0;
	for(i=0;i<end_num-begin_num+1;i++)	{
	//	printf("%c,%c\r\n",data1[6+i],data2[i]);
		if(data1[begin_num+i]==data2[i])
		{
			count++;
		}
	}
	if(count==end_num-begin_num+1)
	{
		return 0;
	}
	return -1;	
}

char *left_trim(char *str,int count)
{
	char *beginp=str;
	char *tmp=str;
	while(isspace(*beginp)) 
	{
		beginp++;
		count=count+1;
	}
//	printf("%d\r\n",count);
	while((*tmp++ = *beginp++));
	return str;
}



char *right_trim(char *str,int count)
{
	char *endp=str;
	size_t len =strlen(str);
	if(len==0) return str;
	endp=str+strlen(str)-1;	
	while(isspace(*endp)) 
	{
		endp--;
		count=count+1;
    }
   // printf("%d\r\n",count);
	*(endp+1)='\0';
	return str;
}

char *trim(char *str)
{
	int count=0;
	str=left_trim(str,count);
	str=right_trim(str,count);
	return str;
}



#endif
