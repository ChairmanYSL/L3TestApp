#include "appglobal.h"
#include "host.h"

unsigned short WGet(u8 *aSrc)
{
    return (unsigned short)((unsigned short)*(aSrc + 1) + (unsigned short)(*aSrc << 8));
}

unsigned int DWGet(u8 *aSrc)
{
    return (unsigned int)((unsigned int)*(aSrc + 2) + (unsigned int)(*(aSrc + 1) << 8) + (unsigned int)(*aSrc << 16));
}

unsigned short TlvTSize(u8 *aTLV) //3byte
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

unsigned short TlvLSize(u8 *aTLV)
{
    u8 *pb;

    if (aTLV == NULL) { return 0; }
    pb = aTLV + TlvTSize(aTLV);

    if (*pb & 0x80)
    {
        return (unsigned short)((*pb & 0x7F) + 1);
    }
    return 1;
}

unsigned short TlvLen(u8 *aTLV)
{
    u8 *pb;

    if (aTLV == NULL) { return 0; }
    pb = aTLV + TlvTSize(aTLV);

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
        return WGet(pb + 1);
    }
    return 0xFFFF;
}

unsigned short TlvSizeOf(u8 *aTLV)
{
    if (aTLV == NULL) { return 0; }
    return (unsigned short)(TlvTSize(aTLV) + TlvLSize(aTLV) + TlvLen(aTLV));
}

unsigned int TlvTag(u8 *aTLV)
{
    if (aTLV == NULL) { return 0; }

    if ((*aTLV & 0x1F) == 0x1F)
    {
        if((*(aTLV + 1) & 0x80) == 0x80)
        {
            return DWGet(aTLV);
        }
        else
        {
            return WGet(aTLV);
        }
    }
    else
    {
        return *aTLV;
    }
}

u8 *TlvVPtr(u8 *aTLV)
{
    if (aTLV == NULL) { return NULL; }
    return aTLV + TlvTSize(aTLV) + TlvLSize(aTLV);
}

u8 *TlvSeek(u8 *aTlvList, unsigned short aLen, unsigned int aTag)
{
    u8 *pb;

    if (aTlvList == NULL) { return NULL; }
    pb = aTlvList;

    while (pb < aTlvList + aLen)
    {
        if (TlvTag(pb) == aTag)
        {
            return pb;
        }
        pb += TlvTSize(pb) + TlvLSize(pb) + TlvLen(pb);
    }

    return NULL;
}
//#endif

u16 TlvSeekSame(u8 *aTlvList, u16 aLen, u32 aTag, u8 *pout)
{
    u8 *pb;
    u16 offset = 0, taglen = 0, buflen = 0;

    if (aTlvList == NULL) { return 0; }
    pb = aTlvList;

    while(pb < aTlvList + aLen)
    {
        if(TlvTag(pb) == aTag)
        {
            taglen = TlvSizeOf(pb);
            memcpy(&pout[offset], pb, taglen);
            offset += taglen;
        }
        buflen += TlvSizeOf(pb);

        if(buflen == aLen)
        {
            return offset;
        }
        pb += TlvTSize(pb) + TlvLSize(pb) + TlvLen(pb);
    }

    return 0;
}


s32 TlvToCAPKStruct(u8 *buf, u16 ilen)
{
    u8 *pb, *pbTlv, *p9F06, *pDF03;
    u16 checkLen = 0;
    u8 checksum[20];
    u8 DISP[32];
    SDK_EMVBASE_CAPK_STRUCT *msgCAPK;
    u8 checkSum[20];
    u32 uiIndex;
    u8 *checkData = NULL;
    u16 checkData_size;
	s32 ret;

    pbTlv = TlvSeek(buf, ilen, 0x9F06);    //RID

    if(pbTlv == NULL)
    {
		Trace("Download CAPK", "Error Code:-13\r\n");
		Trace("Download CAPK", "Input CAPK RID is NULL\r\n");
        return SDK_ERR;
    }
    else
    {
        msgCAPK = (SDK_EMVBASE_CAPK_STRUCT *)sdkGetMem(sizeof(SDK_EMVBASE_CAPK_STRUCT));
        memset(msgCAPK, 0, sizeof(SDK_EMVBASE_CAPK_STRUCT));

        p9F06 = pbTlv;
        pb = TlvVPtr(pbTlv);
        memcpy(msgCAPK->RID, pb, 5);
        TraceHex("Download CAPK", "msgCAPK.RID", msgCAPK->RID, 5);
    }

    pbTlv = TlvSeek(buf, ilen, 0x9F22);
    if(pbTlv != NULL)
    {
        pb = TlvVPtr(pbTlv);
        msgCAPK->CAPKI = *pb;
        Trace("Download CAPK", "msgCAPK.CAPKI=%2X\r\n", msgCAPK->CAPKI);
    }
	else
	{
		Trace("Download CAPK", "Error Code:-13\r\n");
		Trace("Download CAPK", "Input TLV Data Miss CAPK Index\r\n");
	}

    pbTlv = TlvSeek(buf, ilen, 0xDF07);
    if(pbTlv != NULL)
    {
        pb = TlvVPtr(pbTlv);
        msgCAPK->ArithInd = *pb;
        Trace("Download CAPK", "msgCAPK.ArithInd=%2X\r\n", msgCAPK->ArithInd);
    }
	else
	{
		Trace("Download CAPK", "Error Code:-13\r\n");
		Trace("Download CAPK", "Input TLV Data Miss CAPK Algorithm identification\r\n");
	}


    pbTlv = TlvSeek(buf, ilen, 0xDF06);
    if(pbTlv != NULL)
    {
        pb = TlvVPtr(pbTlv);
        msgCAPK->HashInd = *pb;
        Trace("Download CAPK", "msgCAPK.HashInd=%2X\r\n", msgCAPK->HashInd);
    }
	else
	{
		Trace("Download CAPK", "Error Code:-13\r\n");
		Trace("Download CAPK", "Input TLV Data Miss CAPK Hash identification\r\n");
	}

    pbTlv = TlvSeek(buf, ilen, 0xDF02);
    if(pbTlv != NULL)
    {
        pb = TlvVPtr(pbTlv);
        msgCAPK->ModulLen = TlvLen(pbTlv);
        memcpy(msgCAPK->Modul, pb, msgCAPK->ModulLen);
        TraceHex("Download CAPK", "msgCAPK.Modul", msgCAPK->Modul, msgCAPK->ModulLen);
    }
	else
	{
		Trace("Download CAPK", "Error Code:-13\r\n");
		Trace("Download CAPK", "Input TLV Data Miss CAPK Modul\r\n");
	}


    pbTlv = TlvSeek(buf, ilen, 0xDF04);
    if(pbTlv != NULL)
    {
        pb = TlvVPtr(pbTlv);
        msgCAPK->ExponentLen = TlvLen(pbTlv);
        memcpy(msgCAPK->Exponent, pb, msgCAPK->ExponentLen);
        TraceHex("Download CAPK", "msgCAPK.Exponent", msgCAPK->Exponent, msgCAPK->ExponentLen);
    }
	else
	{
		Trace("Download CAPK", "Error Code:-13\r\n");
		Trace("Download CAPK", "Input TLV Data Miss CAPK Exponent\r\n");
	}

    pbTlv = TlvSeek(buf, ilen, 0xDF03);
    if(pbTlv != NULL)
    {
        pDF03 = pbTlv;
        pb = TlvVPtr(pbTlv);
        msgCAPK->CheckSumLen = TlvLen(pbTlv);
        memcpy(msgCAPK->CheckSum, pb, msgCAPK->CheckSumLen);
        TraceHex("Download CAPK", "msgCAPK.CheckSum", msgCAPK->CheckSum, msgCAPK->CheckSumLen);
    }
    else
    {
		msgCAPK->CheckSumLen = 0;	//2021.9.9 lishiyao DF03不存在时不用校验公钥
    }

    pbTlv = TlvSeek(buf, ilen, 0xDF05);
    if(pbTlv != NULL)
    {
        pb = TlvVPtr(pbTlv);
        memcpy(msgCAPK->ExpireDate, pb, TlvLen(pbTlv));
        TraceHex("Download CAPK", "msgCAPK.ExpireDate", msgCAPK->ExpireDate, 4);
    }
	else
	{
		Trace("Download CAPK", "Error Code:-13\r\n");
		Trace("Download CAPK", "Input TLV Data Miss CAPK Expire Date\r\n");
	}

    TraceHex("Download CAPK", "msgcapk", msgCAPK->RID, sizeof(SDK_EMVBASE_CAPK_STRUCT));

	if(pDF03 < p9F06)
	{
		if(msgCAPK->CheckSumLen == 0)	//2021.9.9 lishiyao DF03不存在时不用校验公钥
		{

		}
		else
		{
			return SDK_ERR;
		}
    }
    checkLen = pDF03 - p9F06;
    Trace("Download CAPK", "checklen=%d\r\n", checkLen);
    TraceHex("Download CAPK", "checkbuf", buf, checkLen);
    sdkSHA1(buf, checkLen, checksum);
    TraceHex("Download CAPK", "checksum_hash", checksum, 20);

    if(msgCAPK->CheckSumLen == 0)	//2021.9.9 lishiyao DF03不存在时不用校验公钥//The hash value in the background is wrong. The hash value is not verified here
    {
		memcpy(msgCAPK->CheckSum, checksum, 20);
    }

    if(memcmp(msgCAPK->CheckSum, checksum, 20) == 0)
    {
        if(msgCAPK->ArithInd == 0x04)
        {
            ret = sdkEMVBaseAddAnyGMCAPKList(msgCAPK, 1);
        }
        else
        {
            checkData_size = 5 + 1 + msgCAPK->ModulLen + msgCAPK->ExponentLen;
            checkData = (u8*)sdkGetMem(checkData_size);

            if(NULL == checkData)
            {
                return SDK_ERR;
            }
            memset(checkData, 0, checkData_size);

            memcpy(checkData, msgCAPK->RID, 5);
            uiIndex = 5;
            checkData[5] = msgCAPK->CAPKI;
            uiIndex += 1;

            memcpy((u8*)&checkData[uiIndex], msgCAPK->Modul, msgCAPK->ModulLen);
            uiIndex += msgCAPK->ModulLen;

            memcpy((u8*)&checkData[uiIndex], msgCAPK->Exponent, msgCAPK->ExponentLen);
            uiIndex += msgCAPK->ExponentLen;


            sdkSHA1(checkData, uiIndex, checkSum);
            memcpy(msgCAPK->CheckSum, checkSum, msgCAPK->CheckSumLen);
            sdkFreeMem(checkData);
            ret = sdkEMVBaseAddAnyCAPKList(msgCAPK, 1);
			Trace("Download CAPK", "sdkEMVBaseAddAnyCAPKList ret = %d\r\n", ret);
        }
		if(ret == SDK_OK)
		{
		}
		else
		{

			return SDK_ERR;
	    }
    }
    else
    {
		Trace("Download CAPK", "checksum invalid\r\n");
    }
    sdkFreeMem(msgCAPK);
    return SDK_OK;
}

s32 TlvToSzztAIDStruct(u8 *buf, u16 ilen)	//兼容证通旧架构
{
	u8 *pb, *pbTlv;
	SDK_EMVBASE_AID_STRUCT *tempAid;
	u32 num;
	s32 ret;
	u8 DISP[33];
	u32 index = 0;


	pbTlv = TlvSeek(buf, ilen, 0x9F06);

	if(pbTlv == NULL)
	{
		Trace("Download AID", "Error Code:-10\r\n");
		Trace("Download AID", "Input AID Name is NULL\r\n");
		return SDK_ERR;
	}
	else
	{
		tempAid = (SDK_EMVBASE_AID_STRUCT *)sdkGetMem(sizeof(SDK_EMVBASE_AID_STRUCT));
		memset(tempAid, 0, sizeof(SDK_EMVBASE_AID_STRUCT));

//		tempAid->transvaule = 0xFF;
		tempAid->contactorcontactless = 0;

		pb = TlvVPtr(pbTlv);
		tempAid->AidLen = TlvLen(pbTlv);
		if(tempAid->AidLen > 16)
		{
			Trace("Download AID", "Error Code:-12\r\n");
			Trace("Download AID", "Input AID Name length greater than 16,which is invalid\r\n");
			sdkFreeMem(tempAid);
			return SDK_ERR;
		}
		memcpy(tempAid->Aid, pb, tempAid->AidLen);
		TraceHex("Download AID", "Aid Name", tempAid->Aid, tempAid->AidLen);
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF01); 	//Asi

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			tempAid->Asi = 0;	//不支持部分匹配
		}
		else
		{
			tempAid->Asi = 1;	//支持部分匹配
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF11);    //TAC DEFAULT

	if(pbTlv != NULL)
	{
		if( TlvLen(pbTlv) == 0)
		{
			Trace("Download AID", "Input TAC-default length is 0\r\n");
			memset(tempAid->TacDefault, 0xFF, 5);
		}
		else
		{
			pb = TlvVPtr(pbTlv);
			memcpy(tempAid->TacDefault, pb, TlvLen(pbTlv));
		}
	}


	pbTlv = TlvSeek(buf, ilen, 0xDF12);    //TacOnline

	if(pbTlv != NULL)
	{
		if( TlvLen(pbTlv) == 0)
		{
			Trace("Download AID", "Input TAC-online length is 0\r\n");
			memset(tempAid->TacOnline, 0xFF, 5);
		}
		else
		{
			pb = TlvVPtr(pbTlv);
			memcpy(tempAid->TacOnline, pb, TlvLen(pbTlv));
		}

	}

	pbTlv = TlvSeek(buf, ilen, 0xDF13);    //TacDecline

	if(pbTlv != NULL)
	{
		if( TlvLen(pbTlv) == 0)
		{
			Trace("Download AID", "Input Tac-denial length is 0\r\n");
			memset(tempAid->TacDecline, 0xFF, 5);
		}
		else
		{
			 pb = TlvVPtr(pbTlv);
			 memcpy(tempAid->TacDecline, pb, TlvLen(pbTlv));
		}

	}
	else
	{
			Trace("Download AID", "Input Tac-denial is NULL\r\n");
			memset(tempAid->TacDecline, 0xFF, 5);
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF15);
	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		sdkBcdToU32(&num, pb, TlvLen(pbTlv));
		sdkU32ToHex(tempAid->Threshold, num, 4);
	}
	pbTlv = TlvSeek(buf, ilen, 0xDF16);

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		sdkBcdToU8(&(tempAid->MaxTargetPercent), pb, 1);
	}
	pbTlv = TlvSeek(buf, ilen, 0xDF17);

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		sdkBcdToU8(&(tempAid->TargetPercent), pb, 1);
	}
	pbTlv = TlvSeek(buf, ilen, 0x9F09);

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		memcpy(tempAid->AppVerNum, pb, 2);
	}

	pbTlv = TlvSeek(buf, ilen, 0x9F7B);

	if(pbTlv != NULL)
	{
		if(0 == TlvLen(pbTlv))
		{
			memset(tempAid->ectranslimit, 0xFF, 6);
		}
		else
		{
			pb = TlvVPtr(pbTlv);
			memcpy(tempAid->ectranslimit, pb, TlvLen(pbTlv));
		}

	}

	tempAid->TermPinCap = 1;

	pbTlv = TlvSeek(buf, ilen, 0xDF19);

	if(pbTlv != NULL)
	{
		if(0 == TlvLen(pbTlv))
		{
			memset(tempAid->cl_offlinelimit, 0xFF, 6);
		}
		else
		{
			 pb = TlvVPtr(pbTlv);
			memcpy(tempAid->cl_offlinelimit, pb, TlvLen(pbTlv));
		}
	}
//	TraceHex("Download AID", "Contactless Floor Limit", tempAid->cl_offlinelimit, 6);

	pbTlv = TlvSeek(buf, ilen, 0xDF20);
	if(pbTlv != NULL)
	{
		if(0 == TlvLen(pbTlv))
		{
			memset(tempAid->cl_translimit, 0xFF, 6);
		}
		else
		{
			pb = TlvVPtr(pbTlv);
			memcpy(tempAid->cl_translimit, pb, TlvLen(pbTlv));
		}
	}
//	TraceHex("Download AID", "cl_translimit", tempAid->cl_translimit, 6);

	 pbTlv = TlvSeek(buf, ilen, 0xDF21);

	if(pbTlv != NULL)
	{
		if(0 == TlvLen(pbTlv))
		{
			memset(tempAid->cl_cvmlimit, 0xFF, 6);
		}
		else
		{
			pb = TlvVPtr(pbTlv);
			memcpy(tempAid->cl_cvmlimit, pb, TlvLen(pbTlv));
		}
	}

	ret = sdkEMVBaseAddAnyAIDList(tempAid, 1);
//	Trace("BCTC", "sdkEMVBaseAddAnyAIDList ret = %d\r\n", ret);

	sdkFreeMem(tempAid);
	return ret;
}

s32 TlvToAIDStruct(u8 *buf, u16 ilen)	//兼容加减
{
    u8 *pb, *pbTlv;
    SDK_EMVBASE_AID_STRUCT *tempAid;
    u32 num;
	s32 ret;
	u8 DISP[33];
	u32 index = 0;


    pbTlv = TlvSeek(buf, ilen, 0x9F06);

    if(pbTlv == NULL)
    {
		Trace("Download AID", "Error Code:-10\r\n");
		Trace("Download AID", "Input AID Name is NULL\r\n");
        return SDK_ERR;
    }
    else
    {
        tempAid = (SDK_EMVBASE_AID_STRUCT *)sdkGetMem(sizeof(SDK_EMVBASE_AID_STRUCT));
        memset(tempAid, 0, sizeof(SDK_EMVBASE_AID_STRUCT));

//		tempAid->transvaule = 0xFF;
		tempAid->contactorcontactless = 0;

        pb = TlvVPtr(pbTlv);
        tempAid->AidLen = TlvLen(pbTlv);
		if(tempAid->AidLen > 16)
		{
			Trace("Download AID", "Error Code:-12\r\n");
			Trace("Download AID", "Input AID Name length greater than 16,which is invalid\r\n");
			sdkFreeMem(tempAid);
			return SDK_ERR;
		}
        memcpy(tempAid->Aid, pb, tempAid->AidLen);
		TraceHex("Download AID", "Aid Name", tempAid->Aid, tempAid->AidLen);
    }

    pbTlv = TlvSeek(buf, ilen, 0xDF01); 	//Asi

    if(pbTlv != NULL)
    {
        pb = TlvVPtr(pbTlv);
        if(*pb)
        {
            tempAid->Asi = 1;	//不支持部分匹配
        }
        else
        {
            tempAid->Asi = 0;	//支持部分匹配
        }
		Trace("Download AID", "ASI: %d\r\n", tempAid->Asi);
    }

    pbTlv = TlvSeek(buf, ilen, 0x9F09);    //AppVerNum

    if(pbTlv != NULL)
    {
		pb = TlvVPtr(pbTlv);
    	memcpy(tempAid->AppVerNum, pb, TlvLen(pbTlv));
		TraceHex("Download AID", "App Version Num", tempAid->AppVerNum, 2);
    }

    pbTlv = TlvSeek(buf, ilen, 0xDF8120);    //TAC DEFAULT

    if(pbTlv != NULL)
    {
    	if( TlvLen(pbTlv) == 0)
    	{
    		Trace("Download AID", "Input TAC-default length is 0\r\n");
			memset(tempAid->TacDefault, 0xFF, 5);
    	}
		else
		{
			pb = TlvVPtr(pbTlv);
        	memcpy(tempAid->TacDefault, pb, TlvLen(pbTlv));
		}
		TraceHex("Download AID", "TAC-default", tempAid->TacDefault, 5);
    }

    pbTlv = TlvSeek(buf, ilen, 0xDF8121);    //TAC Denial

    if(pbTlv != NULL)
    {
    	if( TlvLen(pbTlv) == 0)
    	{
    		Trace("Download AID", "Input TAC-Denial length is 0\r\n");
			memset(tempAid->TacDecline, 0xFF, 5);
    	}
		else
		{
			pb = TlvVPtr(pbTlv);
        	memcpy(tempAid->TacDecline, pb, TlvLen(pbTlv));
		}
		TraceHex("Download AID", "TAC-Denial", tempAid->TacDecline, 5);
    }

    pbTlv = TlvSeek(buf, ilen, 0xDF8122);    //TacOnline

    if(pbTlv != NULL)
    {
    	if( TlvLen(pbTlv) == 0)
    	{
    		Trace("Download AID", "Input TAC-online length is 0\r\n");
			memset(tempAid->TacOnline, 0xFF, 5);
    	}
		else
		{
			pb = TlvVPtr(pbTlv);
        	memcpy(tempAid->TacOnline, pb, TlvLen(pbTlv));
		}
		TraceHex("Download AID", "TAC-Online", tempAid->TacOnline, 5);
    }

    pbTlv = TlvSeek(buf, ilen, 0xDF14);    //DEFAULT DDOL
    if(pbTlv != NULL)
    {
    	if(0 == TlvLen(pbTlv))
    	{

    	}
		else
		{
			pb = TlvVPtr(pbTlv);
       	 	tempAid->TermDDOLLen = TlvLen(pbTlv);
        	memcpy(tempAid->TermDDOL, pb, tempAid->TermDDOLLen);
		}
    }

    pbTlv = TlvSeek(buf, ilen, 0x9F49);    //DEFAULT DDOL
    if(pbTlv != NULL)
    {
    	if(0 == TlvLen(pbTlv))
    	{

    	}
		else
		{
			pb = TlvVPtr(pbTlv);
       	 	tempAid->TermDDOLLen = TlvLen(pbTlv);
        	memcpy(tempAid->TermDDOL, pb, tempAid->TermDDOLLen);
		}
		TraceHex("Download AID", "TAC-Online", tempAid->TacOnline, 5);
    }

    tempAid->TermPinCap = 1;

    pbTlv = TlvSeek(buf, ilen, 0xDF8123);

    if(pbTlv != NULL)
    {
    	if(0 == TlvLen(pbTlv))
    	{
    		memset(tempAid->cl_offlinelimit, 0xFF, 6);
    	}
		else
		{
			 pb = TlvVPtr(pbTlv);
        	memcpy(tempAid->cl_offlinelimit, pb, TlvLen(pbTlv));
		}
    }
	TraceHex("Download AID", "Contactless Floor Limit", tempAid->cl_offlinelimit, 6);

    pbTlv = TlvSeek(buf, ilen, 0xDF8124);
    if(pbTlv != NULL)
    {
    	if(0 == TlvLen(pbTlv))
    	{
    		memset(tempAid->cl_translimit, 0xFF, 6);
    	}
		else
		{
			pb = TlvVPtr(pbTlv);
        	memcpy(tempAid->cl_translimit, pb, TlvLen(pbTlv));
		}
    }
	TraceHex("Download AID", "Contactless Trans Limit", tempAid->cl_translimit, 6);

    pbTlv = TlvSeek(buf, ilen, 0xDF8126);

    if(pbTlv != NULL)
    {
    	if(0 == TlvLen(pbTlv))
    	{
    		memset(tempAid->cl_cvmlimit, 0xFF, 6);
    	}
		else
		{
			pb = TlvVPtr(pbTlv);
       		memcpy(tempAid->cl_cvmlimit, pb, TlvLen(pbTlv));
		}
    }
	TraceHex("Download AID", "Contactless CVM limit", tempAid->cl_cvmlimit, 6);

    ret = sdkEMVBaseAddAnyAIDList(tempAid, 1);
	Trace("BCTC", "sdkEMVBaseAddAnyAIDList ret = %d\r\n", ret);

    sdkFreeMem(tempAid);
    return ret;
}


s32 TlvToTERMINFO(unsigned char *buf, int ilen)
{
	u8 *pb, *pbTlv;
	s32 ret = SDK_OK;

	pbTlv = TlvSeek(buf, ilen, 0x9F06);

	if(pbTlv == NULL)
	{
		Trace("Download Term Info", "Error Code:-10\r\n");
		Trace("Download Term Info", "Input AID Name is NULL\r\n");
		return SDK_ERR;
	}
	else
	{
		pb = TlvVPtr(pbTlv);
		TraceHex("Download Term Info", "Aid Name", pb, TlvLen(pbTlv));
		if(memcmp(pb, "\xA0\x00\x00\x00\x04", 5) != 0)
		{
			return SDK_OK;
		}
	}

    pbTlv = TlvSeek(buf, ilen, 0x9F09);    //AppVerNum

    if(pbTlv != NULL)
    {
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			ret = sdkEMVBaseConfigTLV("\x9F\x09", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Application Version Number(9F09)", pb, TlvLen(pbTlv));
		}
    }

	pbTlv = TlvSeek(buf, ilen, 0x9F01); 	//Acquirer Identifier

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			ret = sdkEMVBaseConfigTLV("\x9F\x01", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Acquirer Identifier(9F01)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF8132);	 //Minimum Relay Resistance Grace Period

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\xDF\x81\x32", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Minimum Relay Resistance Grace Period(DF8132)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF8125);	 // Contactless Transaction Limit (On-device CVM)

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\xDF\x81\x25", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Contactless Transaction Limit (On-device CVM)(DF8125)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF8118);	 //CVM Capability – CVM Required

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\xDF\x81\x18", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "CVM Capability – CVM Required(DF8118)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0x5F36);	 //Transaction Currency Exponent

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\x5F\x36", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Transaction Currency Exponent(5F36)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0x9F35);	 //Terminal Type

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\x9F\x35", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Terminal Type(9F35)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0x9F66);	 //TTQ

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\x9F\x66", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "TTQ(9F66)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF811C);	 //Max Lifetime of Torn Transaction Log Record

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\xDF\x81\x1C", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Max Lifetime of Torn Transaction Log Record(DF811C)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF811C);	 //Max Lifetime of Torn Transaction Log Record

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\xDF\x81\x1C", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Max Lifetime of Torn Transaction Log Record(DF811C)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF811F);	 //Security Capability

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\xDF\x81\x1F", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Security Capability(DF811F)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF8135);	 //Terminal Expected Transmission Time For Relay Resistance R-APDU

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\xDF\x81\x35", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Terminal Expected Transmission Time For Relay Resistance R-APDU(DF8135)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF8133);	 //Maximum Relay Resistance Grace Period

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\xDF\x81\x33", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Maximum Relay Resistance Grace Period(DF8133)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF8119);	 //CVM Capability – No CVM Required

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\xDF\x81\x19", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "CVM Capability – No CVM Required(DF8119)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF811A);	 //Default UDOL

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\xDF\x81\x1A", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Default UDOL(DF811A)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF8134);	 //Terminal Expected Transmission Time For Relay Resistance C-APDU

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\xDF\x81\x34", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Terminal Expected Transmission Time For Relay Resistance C-APDU(DF8134)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF8117);	 //Card Data Input Capability

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\xDF\x81\x17", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Card Data Input Capability(DF8117)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF812D);	 //Message Hold Time

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\xDF\x81\x2D", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Message Hold Time(DF812D)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF811E);	 //Mag-stripe CVM Capability – CVM Required

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\xDF\x81\x1E", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Mag-stripe CVM Capability – CVM Required(DF811E)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0x5F2A);	 //Transaction Currency Code

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\x5F\x2A", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Transaction Currency Code(5F2A)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF8127);	 //Time Out Value

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\xDF\x81\x27", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Time Out Value(DF8127)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF811B);	 //Kernel Configuration

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\xDF\x81\x1B", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Kernel Configuration(DF811B)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF8136);	 //Relay Resistance Accuracy Threshold

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\xDF\x81\x36", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Relay Resistance Accuracy Threshold(DF8136)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF811D);	 //Max Number of Torn Transaction Log Records

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\xDF\x81\x1D", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Max Number of Torn Transaction Log Records(DF811D)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF8137);	 //Relay Resistance Transmission Time Mismatch Threshold

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\xDF\x81\x37", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Relay Resistance Transmission Time Mismatch Threshold(DF8137)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0xDF812C);	 //Mag-stripe CVM Capability – No CVM Required

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\xDF\x81\x2C", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Mag-stripe CVM Capability – No CVM Required(DF812C)", pb, TlvLen(pbTlv));
		}
	}

	pbTlv = TlvSeek(buf, ilen, 0x9F1A);	 //Terminal Country Code

	if(pbTlv != NULL)
	{
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			sdkEMVBaseConfigTLV("\x9F\x1A", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Terminal Country Code(9F1A)", pb, TlvLen(pbTlv));
		}
	}

    pbTlv = TlvSeek(buf, ilen, 0x1F30);

    if(pbTlv != NULL)
    {
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			ret = sdkEMVBaseConfigTLV("\x1F\x30", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "Extended Selection Support Flag(1F30)", pb, TlvLen(pbTlv));
		}
    }

    pbTlv = TlvSeek(buf, ilen, 0x9F1D);

    if(pbTlv != NULL)
    {
		pb = TlvVPtr(pbTlv);
		if(*pb)
		{
			ret = sdkEMVBaseConfigTLV("\x9F\x1D", pb, TlvLen(pbTlv));
			if(SDK_OK != ret)
			{
				return ret;
			}
			TraceHex("Download Term Info", "TRM DATA(9F1D)", pb, TlvLen(pbTlv));
		}
    }


	return ret;
}

s32 SzztCAPK2EmvbaseCAPK(u8 *src, s32 srcLen)
{
    SDK_EMVBASE_CAPK_STRUCT msgCAPK = {0};
	int i,ret,index=0;

	if(src == NULL)
	{
		return SDK_PARA_ERR;
	}
	memcpy(msgCAPK.RID, src, 5);
	TraceHex("Download CAPK", "RID", msgCAPK.RID, 5);
	index += 5;

	msgCAPK.CAPKI = src[index++];
	Trace("Download CAPK", "Index:%02X\r\n", msgCAPK.CAPKI);
	msgCAPK.ArithInd = src[index++];
	msgCAPK.HashInd = src[index++];

	msgCAPK.ModulLen = src[index+248];
	Trace("Download CAPK", "ModulLen:%d\r\n", msgCAPK.ModulLen);
	memcpy(msgCAPK.Modul, src+index, msgCAPK.ModulLen);
	TraceHex("Download CAPK", "Modul", msgCAPK.Modul, msgCAPK.ModulLen);
	index += 249;
	msgCAPK.ExponentLen = 3;

	memcpy(msgCAPK.Exponent, src+index, 3);
	TraceHex("Download CAPK", "Exponent", msgCAPK.Exponent, msgCAPK.ExponentLen);
	index += 3;
	memcpy(msgCAPK.ExpireDate, src+index, 4);
	TraceHex("Download CAPK", "ExpireDate", msgCAPK.ExpireDate, 4);
	index += 4;
	msgCAPK.CheckSumLen = 20;
	memcpy(msgCAPK.CheckSum, src+index, 20);
	TraceHex("Download CAPK", "CheckSum", msgCAPK.CheckSum, msgCAPK.CheckSumLen);

    ret = sdkEMVBaseAddAnyCAPKList(&msgCAPK, 1);
	Trace("Download CAPK", "sdkEMVBaseAddAnyCAPKList ret = %d\r\n", ret);

    return SDK_OK;
}
