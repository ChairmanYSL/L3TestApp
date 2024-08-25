//#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int GetCompileTime(char *pasCompileTime, char *pasDate)
{
	char temp_date[64] = {0},str_year[5] = {0}, str_month[4] = {0}, str_day[3] = {0};
	char en_month[12][4]={ "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	int i = 0;

	if(pasCompileTime==NULL) return 0;

	sprintf(temp_date,"%s",pasDate);    //"Sep 7 2012"
	sscanf(temp_date,"%s %s %s",str_month, str_day, str_year);

	for(i=0; i < 12; i++)
	{
		if(strncmp(str_month,en_month[i],3)==0)
		{
		    memset(str_month, 0, sizeof(str_month));
	  		sprintf(str_month, "%02d", i + 1);
	        break;
	    }
	}

	if(strlen(str_day)==1)//若日期为1位需要前补0x30
	{
		str_day[1]=str_day[0];
		str_day[0]=0x30;
	}
	return sprintf(pasCompileTime, "%s%s%s", str_year, str_month, str_day);
}

void EMVBaseMath_GetLibVersion(unsigned char *version)
{
	GetCompileTime(version, __DATE__);//如2020年01月21日编译，输出version为:200121
	strcat(version,"001");
}

void EMVBaseBcdToAsc(unsigned char *Dest,unsigned char *Src,unsigned short Len)
{
    unsigned short i;
	for(i=0;i<Len;i++)
	{
	    if(((*(Src + i) & 0xF0) >> 4) <= 9)
	    {
            *(Dest + 2*i) = ((*(Src + i) & 0xF0) >> 4) + 0x30;
        }
        else
        {
            *(Dest + 2*i)  = ((*(Src + i) & 0xF0) >> 4) + 0x37;
        }

        if((*(Src + i) & 0x0F) <= 9)
        {
            *(Dest + 2*i + 1) = (*(Src + i) & 0x0F) + 0x30;
        }
        else
        {
            *(Dest + 2*i + 1) = (*(Src + i) & 0x0F) + 0x37;
        }
    }
}

void EMVBaseAscToBcd(unsigned char *Dest,unsigned char *Src,unsigned short Len)
{
    unsigned short i;
    unsigned char high = 0,low = 0;
    for(i = 0; i < Len; i++)
    {
        //待转bcd码高Nibble
	    if((*(Src + i) >= 0x61) && (*(Src + i) <= 0x66))      //range a~f
	    {
	        high = *(Src + i) - 0x57;
	    }
	    else if((*(Src + i) >= 0x41) && (*(Src + i) <= 0x46))  //range A~F
	    {
	        high = *(Src + i) - 0x37;
	    }
	    else if((*(Src + i) >= 0x30) && (*(Src + i) <= 0x39))  //range 0~9
	    {
	        high = *(Src + i) - 0x30;
	    }
        else
        {
            high = 0x00 ;                                       //其他
        }

        //待转bcd码低Nibble
        i++;
        if(i < Len)
        {
	        if((*(Src + i) >= 0x61) && (*(Src + i) <= 0x66))    //range a~f
	        {
	            low = *(Src + i) - 0x57;
            }
            else if((*(Src + i) >= 0x41) && (*(Src + i) <= 0x46)) //range A~F
            {
                low = *(Src + i) - 0x37;
	    	}
	    	else if((*(Src + i) >= 0x30) && (*(Src + i) <= 0x39))  //range 0~9
		    {
		        low = *(Src + i) - 0x30;
	        }
	        else
	        {
	            low = 0x00 ;                                       //其他
		    }
	    }
	    else
	    {
	        i--;                                                //预防255个时溢出出错
	        low = 0x00 ;                                       //如果是奇数个末尾补0x00
	    }
        *(Dest + i/2) = (high << 4) | low;                      //合并BCD码
    }
}


void EMVBaseBcdToU32(unsigned int  *puiDest, const unsigned char *pbcSrc, unsigned short Len)
{
    unsigned int i = 0;

    *puiDest = 0;

    for(i = 0; i < Len; i++)
    {
        *puiDest *= 100;
        *puiDest += 10 * ((*(pbcSrc + i) & 0xF0) >> 4) + (*(pbcSrc + i) & 0x0F);
    }
}

void EMVBaseU32ToBcd(unsigned char *Bcd, unsigned int const Src,unsigned char Len)
{
	unsigned char  i;
    unsigned int num = Src;
    for(i = Len; i > 0; i--)
    {
        *(Bcd+i-1) = (((num%100) / 10) << 4) | ((num%100) % 10);
        num /= 100;
    }
}

void EMVBaseU16ToBcd(unsigned char *Bcd, unsigned short const Src,unsigned char Len)
{
	unsigned char  i;
    unsigned short num = Src;
    for(i = Len; i > 0; i--)
    {
        *(Bcd+i-1) = (((num%100) / 10) << 4) | ((num%100) % 10);
        num /= 100;
    }
}
void EMVBaseBcdToU16(unsigned short *Dest,unsigned char *Bcd,unsigned char Len)
{
  	 unsigned char i;
	 *Dest = 0;
     for(i = 0; i < Len; i++)
     {
        *Dest *= 100;
        *Dest += 10*((*(Bcd+i) & 0xF0) >> 4) + (*(Bcd+i) & 0x0F);
     }
}

unsigned short EMVBaseWGet(unsigned char *aSrc)
{
    return (unsigned short)((unsigned short)*(aSrc + 1) + (unsigned short)(*aSrc << 8));
}

unsigned int EMVBaseDWGet(unsigned char *aSrc)
{
    return (unsigned int)((unsigned int)*(aSrc + 2) + (unsigned int)(*(aSrc + 1) << 8) + (unsigned int)(*aSrc << 16));
}

unsigned short EMVBaseTlvTSize(unsigned char *aTLV)
{
    if (aTLV == NULL) { return 0; }

    if((*aTLV & 0x1F) == 0x1F)
    {
        if((*(aTLV + 1) & 0x80) == 0x80)
        {
            return 3;
        }
        else
        {
            return 2;
        }
    }
    else
    {
        return 1;
    }
}

unsigned short EMVBaseTlvLSize(unsigned char *aTLV)
{
    unsigned char *pb = NULL;

    if (aTLV == NULL) { return 0; }
    pb = aTLV + EMVBaseTlvTSize(aTLV);

    if (*pb & 0x80)
    {
        return (unsigned short)((*pb & 0x7F) + 1);
    }
    return 1;
}

unsigned short EMVBaseTlvLen(unsigned char *aTLV)
{
    unsigned char *pb = NULL;

    if (aTLV == NULL) { return 0; }
    pb = aTLV + EMVBaseTlvTSize(aTLV);

    if ((*pb & 0x80) == 0)
    {
        return *pb;
    }
    else if (*pb == 0x81)
    {
        return *(pb + 1);
    }
    else if (*pb == 0x82)
    {
        return EMVBaseWGet(pb + 1);
    }
    return 0xFFFF;
}

unsigned short EMVBaseTlvSizeOf(unsigned char *aTLV)
{
    if (aTLV == NULL) { return 0; }
    return (unsigned short)(EMVBaseTlvTSize(aTLV) + EMVBaseTlvLSize(aTLV) + EMVBaseTlvLen(aTLV));
}

unsigned int EMVBaseTlvTag(unsigned char *aTLV)
{
    if (aTLV == NULL) { return 0; }

    if ((*aTLV & 0x1F) == 0x1F)
    {
        if((*(aTLV + 1) & 0x80) == 0x80)
        {
            return EMVBaseDWGet(aTLV);
        }
        else
        {
            return EMVBaseWGet(aTLV);
        }
    }
    else
    {
        return *aTLV;
    }
}

unsigned char *EMVBaseTlvVPtr(unsigned char *aTLV)
{
    if (aTLV == NULL) { return NULL; }
    return aTLV + EMVBaseTlvTSize(aTLV) + EMVBaseTlvLSize(aTLV);
}

unsigned char *EMVBaseTlvSeek(unsigned char *aTlvList, unsigned short aLen, unsigned int aTag)
{
    unsigned char *pb = NULL;

    if (aTlvList == NULL) { return NULL; }
    pb = aTlvList;

    while (pb < aTlvList + aLen)
    {
        if (EMVBaseTlvTag(pb) == aTag)
        {
            return pb;
        }
        pb += EMVBaseTlvTSize(pb) + EMVBaseTlvLSize(pb) + EMVBaseTlvLen(pb);
    }

    return NULL;
}

unsigned short EMVBaseTlvSeekSame(unsigned char *aTlvList, unsigned short aLen, unsigned long aTag, unsigned char *pout)
{
    unsigned char *pb = NULL;
    unsigned short offset = 0, taglen = 0, buflen = 0;

    if (aTlvList == NULL) { return 0; }
    pb = aTlvList;

    while(pb < aTlvList + aLen)
    {
        if(EMVBaseTlvTag(pb) == aTag)
        {
            taglen = EMVBaseTlvSizeOf(pb);
            memcpy(&pout[offset], pb, taglen);
            offset += taglen;
        }
        buflen += EMVBaseTlvSizeOf(pb);

        if(buflen == aLen)
        {
            return offset;
        }
        pb += EMVBaseTlvTSize(pb) + EMVBaseTlvLSize(pb) + EMVBaseTlvLen(pb);
    }

    return 0;
}

unsigned char EMVBaseStrPos(unsigned char *Src, unsigned char chr)
{
    unsigned char Len;
    unsigned char i;

    Len = strlen((char *)Src);

    for(i = 0; i < Len; i++)
    {
        if(Src[i] == chr)
        {
            return i;
        }
    }

    return 0xFF;
}

unsigned long EMVBaseByteArrayToInt(unsigned char* buf,unsigned char bufLen)
{
    unsigned char i;
    unsigned long temp;
    temp=0;
    for(i=0; i<bufLen; i++)
    {
        temp=(temp<<8)+buf[i];
    }
    return temp;
}

void EMVBaseIntToByteArray(unsigned long var,unsigned char* buf,unsigned char bufLen)
{
	int i;
	unsigned long temp;
	temp=var;
	for(i=0;i<bufLen;i++)
	{
		buf[bufLen-1-i]=temp%256;
		temp=temp/256;
	}
}

void EMVBaseBcdAdd(unsigned char *Dest,unsigned char *Src,unsigned char Len)
{
    unsigned char i;
    unsigned char high,low;
    unsigned char carry=0;                     //运算进位标志

    for(i=Len;i>0;i--)
    {
         //低nibble的运算
         low = (*(Dest+i-1) & 0x0F) + (*(Src+i-1) & 0x0F) + carry;
         carry=0;
         if(low > 0x09)
         {
            low = low-0x0A;
            carry = 1;
         }
         //高nibble的运算
         high  = ( (*(Dest+i-1) & 0xF0) >> 4) + ((*(Src+i-1) & 0xF0) >> 4) + carry;
         carry=0;
         if(high > 0x09)
         {
            high= high-0x0A;
            carry = 1;
         }
         *(Dest+i-1) = (high << 4) | low;
    }
}

void EMVBaseBcdSub(unsigned char *Dest,unsigned char *Src,unsigned char Len)
{
    unsigned char i;
    unsigned char high,low;
    unsigned char carry=0;                     //运算借位标志

    for(i=Len;i>0;i--)
    {
         //低nibble的运算
         if( (*(Dest+i-1) & 0x0F) < ( (*(Src+i-1) & 0x0F) + carry) )
         {
            low = ((*(Dest+i-1) & 0x0F)+0x0A) - (*(Src+i-1) & 0x0F) - carry;
            carry=1;
         }
         else
         {
            low = (*(Dest+i-1) & 0x0F) - (*(Src+i-1) & 0x0F) - carry;
            carry=0;
         }

         //高nibble的运算
         if( ((*(Dest+i-1) & 0xF0) >> 4) < (((*(Src+i-1) & 0xF0) >> 4) + carry) )
         {
            high = (((*(Dest+i-1) & 0xF0) >> 4)+0x0A) - ((*(Src+i-1) & 0xF0) >> 4)  - carry;
            carry=1;
         }
         else
         {
            high = ((*(Dest+i-1) & 0xF0) >> 4) - ((*(Src+i-1) & 0xF0) >> 4)  - carry;
            carry=0;
         }
         *(Dest+i-1) = (high << 4) | low;
    }
}


unsigned char EMVBaseCalcBcc(unsigned char *src,unsigned int num )
{
	unsigned char bcc = 0;										//累加和
	unsigned int  i;											//临时个数

	for(i=0; i<num; i++)
	{
		bcc += src[i];								//计算校验和
	}
	return bcc;										//返回校验和
}



