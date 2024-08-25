
#include "dllemvbase.h"
#include "dllpaypass.h"
#include "sdkGlobal.h"
#include "sdkemvbaseprivate.h"
#include "sdkpaypassprivate.h"
#include "sdkpaypass.h"
#include "sdkped.h"
#include "emv_type.h"
//#include "dllpaypassprivate.h"



#ifndef PAYPASS_DISP_DEK_AND_CMD
#define PAYPASS_DISP_DEK_AND_CMD	//20160614_LHD
#endif


static u8 gsdkPaypassRefundrequestAAC = 0;
static u8 gsdkPaypassForCertification = 0; //1:for certification test


PAYPASSDLLTRADEPARAMETER *gstPaypassTradeParam = NULL;
PAYPASSTradeUnionStruct *gstPaypassTradeUnionStruct = NULL;
SDK_PAYPASS_TRADE_PARAM *gstsdkPaypassTradeTable = NULL;
SDK_PAYPASS_APPEX_AID_STRUCT *gstsdkpaypass_appex_aid_list = NULL;

PAYPASS_TORNTRANSACTIONMAPTOTALINFO gPaypassTornTotalInfo;
PaypassTornTransactionLogRecord *pPaypasstorntransactionlogrecord = NULL;



#ifdef PAYPASS_OUTCOME_SUPPORT_FLAG
    PAYPASS_USERINTERFACEREQDATA gPaypassUserInterfaceReqData;
#endif
PAYPASS_OUTPARAMETERSET gPaypassOutParameterSet;
PAYPASS_OUTCOME_ERRORID gstPaypassOutComeErrID;
PAYPASS_PHONEMESSAGETABLEINFO gPaypassPhoneMessageTableInfo;
PAYPASS_APDU_ERRORID gPaypassApduErrorID;
u8 gPaypassTransStuatus = 0;

extern u32 sdkTimerGetId();
extern u32 sdkTimerGetIdtemp();


static const  int crc16tab[] =
{
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

static unsigned char flipAByte(unsigned char dat)
{
    unsigned char i;
    unsigned char v;

    v = 0;

    for(i = 0; i < 8; ++i)
    {
        v += ((dat >> (7 - i)) & 0x01) << i;
    }

    return v;
}

/*******************************************************************
Function Nam	     :void Crc16CCITT(const u8 *pbyDataIn, u32 dwDataLen, u8 abyCrcOut[2])
Function Purpose : Get CRC16 check value
Input Paramters  : 1:pbyDataIn - data input
                          2:dwDataLen  - input data len
                          3:abyCrcOut - result
Return Value:crc value
Remark:             20131010
********************************************************************/
static void sdkPaypass_Crc16CCITT(const u8 *pbyDataIn, u32 dwDataLen, u8 *abyCrcOut)
{
    u16 wCrc = 0;
    u8 result[2];
    //u8 byTemp;
    //u16 mg_awhalfCrc16CCITT[16];
    u32 i;		//sxl20110225
    int val;

    val = 0;
    for(i = 0; i < dwDataLen; i++)
    {
        val = (val >> 8) ^ crc16tab[(val ^ flipAByte(pbyDataIn[i])) & 0xFF];
        //		mg_awhalfCrc16CCITT[i] = g_awhalfCrc16CCITT[i];
    }
    result[0] = (u8) (val >> 8);
    result[1] = (u8) val;

    for(i = 0; i < 2; i++)
    {
        result[i] = flipAByte(result[i]);
    }
    wCrc = (u16) (result[1] << 8) + result[0];
    /*
    	while(dwDataLen-- != 0)
    	{
    		byTemp = ((u8)(wCrc>>8))>>4;
    		wCrc <<= 4;
    		wCrc ^= mg_awhalfCrc16CCITT[byTemp^(*pbyDataIn/16)];
    		byTemp = ((u8)(wCrc>>8))>>4;
    		wCrc <<= 4;
    		wCrc ^= mg_awhalfCrc16CCITT[byTemp^(*pbyDataIn&0x0f)];
    		pbyDataIn++;
    	}
    */
    //	abyCrcOut[0] = wCrc/256;
    //	abyCrcOut[1] = wCrc%256;

    abyCrcOut[0] = wCrc >> 8;
    abyCrcOut[1] = (u8)wCrc;
}



u8 sdkPaypassSetRefundrequestAAC(u8 enable)//1:do not request aac,0:request aac
{
    gsdkPaypassRefundrequestAAC = enable;
    return 0;
}

u8 sdkPaypassGetRefundrequestAAC(void)
{
    return gsdkPaypassRefundrequestAAC;
}

u8 sdkPaypassSetCertificationStatus(u8 enable)//1:for certification test
{
    gsdkPaypassForCertification = enable;
    return 0;
}

u8 sdkPaypassGetCertificationStatus(void)
{
    return gsdkPaypassForCertification;
}



void sdkpaypassInitOutcomePar(void)
{
    #ifdef PAYPASS_OUTCOME_SUPPORT_FLAG

    unsigned char MessageHoldTime[3];
    //    unsigned short tagdatalen = 0;
    #endif
    memset(&gPaypassOutParameterSet, 0, sizeof(PAYPASS_OUTPARAMETERSET));
    #ifdef PAYPASS_OUTCOME_SUPPORT_FLAG
    gPaypassOutParameterSet.FieldoffinoutParameter = 0xFF;
    gPaypassOutParameterSet.AIPinoutParameter = 0x0F;
    #endif
    gPaypassOutParameterSet.start = PAYPASS_OPS_START_NA;
    #ifdef PAYPASS_OUTCOME_SUPPORT_FLAG
    gPaypassOutParameterSet.DisDataPresent = 1;
    gPaypassOutParameterSet.OnlineResDatainoutParameter = 0x0F;
    #endif

    gPaypassOutParameterSet.status = PAYPASS_OPS_STATUS_NA;
    gPaypassOutParameterSet.CVM = PAYPASS_OPS_CVM_NA;

    #ifdef PAYPASS_OUTCOME_SUPPORT_FLAG

    memset(&gPaypassUserInterfaceReqData, 0, sizeof(PAYPASS_USERINTERFACEREQDATA));
    gPaypassUserInterfaceReqData.MessageID = 0xFF;
    gPaypassUserInterfaceReqData.Status = 0xFF;

    memcpy(gPaypassUserInterfaceReqData.HoldTime, PAYPASS_USERREQDATA_MSGHOLDTIME_DEFAULT, 3);
    if(emvbase_avl_checkiftagexist(EMVTAG_MessageHoldTime))
    {
        #ifdef EMVB_DEBUG
        Trace("emv", "\r\n sdkpaypassInitOutcomePar emvbase_avl_checkiftagexist != 0 !! \r\n");
        #endif

        emvbase_avl_gettagvalue_spec(EMVTAG_MessageHoldTime, MessageHoldTime, 0, 3);
        memcpy(gPaypassUserInterfaceReqData.HoldTime, MessageHoldTime, 3);
    }
    else
    {
        //Paypass_getspectagdata(EMVTAG_MessageHoldTime, 3, MessageHoldTime, &tagdatalen, &updatecondition);
        //20200421_LHD if(tagdatalen == 3)
        //{
        //    memcpy(gPaypassUserInterfaceReqData.HoldTime, MessageHoldTime, 3);
        //}
    }

    #ifdef EMVB_DEBUG
    Trace("emv", "gPaypassUserInterfaceReqData.HoldTime", gPaypassUserInterfaceReqData.HoldTime, 3);
    #endif
    #endif
    memset(&gstPaypassOutComeErrID, 0, sizeof(PAYPASS_OUTCOME_ERRORID));
    gstPaypassOutComeErrID.MsgOnError = 0xFF;
}

void sdkPaypassKernalOutComeInit(void)
{
    sdkpaypassInitOutcomePar();
}

unsigned char sdkPayPassSupportSaveDekLogGetStatus(void);
unsigned char sdkPaypass_TornRecordread(unsigned int index, PaypassTornTransactionLogRecord *torntransactionlogrecord);
void sdkPaypassGetdatetime(u8 *datetime);


#define SDKPAYPASS_FILE_OPRETION_DEBUG
#undef SDKPAYPASS_FILE_OPRETION_DEBUG

static s32 sdkPaypassDelFile(const s8 *filepath)
{
    u8 fn[SDK_EMVBASE_MAX_PATH];
    memset(fn, 0, sizeof(fn));
    sdkSysGetCurAppDir(fn);
//    strcat(fn, SDKPAYPASS_FILE_PATH);
    strcat(fn, filepath);
    #ifdef SDKPAYPASS_FILE_OPRETION_DEBUG
    Trace("emvcontactless", "sdkPaypassDelFile = %s\n", fn);
    #endif
    if(SDK_OK == sdkDelFile(fn))
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

static s32 sdkPaypassAccessFile(const s8 *filepath)
{
    u8 fn[SDK_EMVBASE_MAX_PATH];
    memset(fn, 0, sizeof(fn));
    sdkSysGetCurAppDir(fn);
//    strcat(fn, SDKPAYPASS_FILE_PATH);
    strcat(fn, filepath);
    #ifdef SDKPAYPASS_FILE_OPRETION_DEBUG
    Trace("emvcontactless", "sdkPaypassAccessFile = %s\n", fn);
    #endif
    if(true == sdkAccessFile(fn))
    {
        return 0;
    }
    else
    {
        return -1;
    }
}
static s32 sdkPaypassCreateFile(const s8 *filepath, s32 len, u8 initialvalue)
{
    u8 fn[SDK_EMVBASE_MAX_PATH];
    memset(fn, 0, sizeof(fn));
    sdkSysGetCurAppDir(fn);
//    strcat(fn, SDKPAYPASS_FILE_PATH);
    strcat(fn, filepath);
    #ifdef SDKPAYPASS_FILE_OPRETION_DEBUG
    Trace("emvcontactless", "sdkPaypassCreateFile = %s\n", fn);
    #endif
    //s32 ret =	sdkWriteFile(fn, NULL, len);
    u8 *inivalue = NULL;
    s32 ret;
    inivalue = (u8 *)emvbase_malloc(len + 1);
    if(inivalue == NULL)
    {
        return 1;
    }
    memset(inivalue, initialvalue, len);

    ret = sdkInsertFile(fn, inivalue, 0, len);
    if(inivalue != NULL)
    {
        emvbase_free(inivalue);
    }
    #ifdef SDKPAYPASS_FILE_OPRETION_DEBUG
    Trace("emv", "\r\n sdkPaypassCreateFile ret=%d\r\n", ret);
    #endif
    if(SDK_OK == ret)
    {
        return 0;
    }
    return 1;
}
static s32 sdkPaypassWriteFile(u8 *filepath, u8 *data, u32 datasize, u32 addr)
{
    u8 fn[SDK_EMVBASE_MAX_PATH];

    memset(fn, 0, sizeof(fn));
    sdkSysGetCurAppDir(fn);

//    strcat(fn, SDKPAYPASS_FILE_PATH);
    strcat(fn, filepath);

    Trace("emvcontactless", "sdkPaypassWriteFile = %s\n", fn);

    s32 ret = sdkWriteFile((u8 *)fn, data, datasize);
    Trace("emv", "\r\n sdkPaypassWriteFile ret=%d\r\n", ret);
    if(SDK_OK == ret)
    {
        return 0;
    }
    return 1;
}
static s32 sdkPaypassInsertFile(u8 *filepath, u8 *data, u32 datasize, u32 addr)
{
    u8 fn[SDK_EMVBASE_MAX_PATH];
    memset(fn, 0, sizeof(fn));
    sdkSysGetCurAppDir(fn);
//    strcat(fn, SDKPAYPASS_FILE_PATH);
    strcat(fn, filepath);
    #ifdef SDKPAYPASS_FILE_OPRETION_DEBUG
    Trace("emvcontactless", "sdkPaypassInsertFile = %s\n", fn);
    #endif

    s32 ret = sdkInsertFile(fn, data, addr, datasize);
    #ifdef SDKPAYPASS_FILE_OPRETION_DEBUG
    Trace("emv", "\r\n sdkPaypassInsertFile ret=%d\r\n", ret);
    #endif
    if(SDK_OK == ret)
    {
        return 0;
    }
    return 1;
}
static s32 sdkPaypassReadFile(u8 *filepath, u8 *data, u32 datasize, u32 addr)
{
    u32 datalen;
    s32 ret;

    u8 fn[SDK_EMVBASE_MAX_PATH];
    memset(fn, 0, sizeof(fn));
    sdkSysGetCurAppDir(fn);
//    strcat(fn, SDKPAYPASS_FILE_PATH);
    strcat(fn, filepath);
    #ifdef SDKPAYPASS_FILE_OPRETION_DEBUG
    Trace("emvcontactless", "sdkPaypassReadFile = %s\n", fn);
    #endif

    datalen = datasize;

    ret = sdkReadFile(fn, data, addr, &datalen);
    #ifdef SDKPAYPASS_FILE_OPRETION_DEBUG
    Trace("emv", "\r\n sdkPaypassReadFile ret=%d datalen=%d\r\n", ret, datalen);
    #endif

    if(ret != SDK_OK)
    {
        datalen = 0;
    }

    return datalen;
}


unsigned char sdkPaypassFunCB(unsigned char pintype, unsigned char *ASCCreditPwd)
{
    //unsigned char rslt;
    unsigned char tCreditPwd[64], tICCEncryptWay;
    s32 rlt;
    u8 PINTryCount = 0;

    Trace("emv", "pintype=%d\r\n", pintype);
    if(gstEMVBase_TradeParam->InputPWD == NULL)
    {
        Trace("emv", "ERROR!!!USER NOT SET INPUTPIN FUNCTION\r\n");
        return RLT_EMV_ERR;
    }


    tICCEncryptWay = SDK_PED_IC_ONLINE_PIN;


    rlt = gstEMVBase_TradeParam->InputPWD(tICCEncryptWay, PINTryCount, tCreditPwd);
    Trace("emv", "InputPWD=%d\r\n", rlt);
    if(rlt != SDK_OK && rlt != SDK_PED_NOPIN)
    {
        Trace("emv", "WARNING!!!USER CANCEL INPUT PIN\r\n");
        return RLT_ERR_EMV_CancelTrans;
    }

    if(rlt == SDK_OK)
    {
        if(tCreditPwd[0])
        {
            memcpy(ASCCreditPwd, &tCreditPwd[1], tCreditPwd[0]);
            return RLT_EMV_OK;
        }
        else
        {
            return RLT_ERR_EMV_InputBYPASS;
        }
    }
    else if(rlt == SDK_PED_NOPIN)
    {
        return RLT_ERR_EMV_InputBYPASS;
    }
    else
    {
        return RLT_ERR_EMV_CancelTrans;
    }
}

s32 sdkPaypassSetRfCardPowerOff(s32(*fun_setRfCardPowerOff)())
{
    if(gstPaypassTradeUnionStruct == NULL)
    {
        return SDK_PARA_ERR;
    }

    gstPaypassTradeUnionStruct->rf_card_poweroff = fun_setRfCardPowerOff;

    return SDK_OK;
}

s32 sdkPaypassSetDisplayRemoveCard(s32(*fun_setdisplayRemoveCard)())
{
    if(gstPaypassTradeUnionStruct == NULL)
    {
        return SDK_PARA_ERR;
    }

    gstPaypassTradeUnionStruct->displayRemoveCard = fun_setdisplayRemoveCard;

    return SDK_OK;
}

s32 sdkPaypassSetBeforeGPO(s32(*fun_setbeforegpo)())
{
    if(gstsdkPaypassTradeTable)
    {
        gstsdkPaypassTradeTable->setBeforeGPO = fun_setbeforegpo;
        return SDK_OK;
    }
    return SDK_PARA_ERR;
}

int sdkPaypassSetAppExAidParam(SDK_PAYPASS_APPEX_AID_STRUCT *p)
{
    if(p == NULL)
    {
        return -1;
    }
    gstsdkpaypass_appex_aid_list = p;
    return 0;
}


s32 sdkPaypassSetDispPromptData(s32(*fun_dispPromptData)(unsigned char ePromptType))
{
    if(gstsdkPaypassTradeTable)
    {
        gstsdkPaypassTradeTable->DispPromptData = fun_dispPromptData;
        return SDK_OK;
    }
    return SDK_PARA_ERR;
}

s32 sdkPaypassSetRevocationKey(s32(*fun_cmprevocation)(u8 *key))
{
    if(gstPaypassTradeUnionStruct == NULL)
    {
        return SDK_PARA_ERR;
    }

    gstPaypassTradeUnionStruct->termipkrevokecheck = fun_cmprevocation;
    return SDK_OK;
}


s32 sdkPaypassSetSendUserInterfaceRequestData(void(*fun_setSendUserInterfaceRequestData)(u32 step))
{
    if(gstPaypassTradeUnionStruct == NULL)
    {
        return SDK_PARA_ERR;
    }
    gstPaypassTradeUnionStruct->senduserinterfacerequestdata = fun_setSendUserInterfaceRequestData;
    return SDK_OK;
}

s32 sdkPaypassSetSendOutParamset(void(*fun_setSendOutParamset)(u32 step))
{
    if(gstPaypassTradeUnionStruct == NULL)
    {
        return SDK_PARA_ERR;
    }
    gstPaypassTradeUnionStruct->sendoutparamset = fun_setSendOutParamset;
    return SDK_OK;
}

s32 sdkPaypassSetSendDisData(void(*fun_setSendDisData)(int mode))
{
    if(gstPaypassTradeUnionStruct == NULL)
    {
        return SDK_PARA_ERR;
    }
    gstPaypassTradeUnionStruct->sendDisData = fun_setSendDisData;
    return SDK_OK;
}

s32 sdkPaypassSetSendEndApplicationDataRecord(void(*fun_setSendEndApplicationDataRecord)(void))
{
    if(gstPaypassTradeUnionStruct == NULL)
    {
        return SDK_PARA_ERR;
    }
    gstPaypassTradeUnionStruct->sendendapplicationdatarecord = fun_setSendEndApplicationDataRecord;
    return SDK_OK;
}

int BeforeGPORes = -1;
int InputPINRes = -10;
int VerifyCardNoRes = -2;
int VerifyRevocationKeyRes = -2;
int SecondTapCardRes = -1;

void sdkPaypassSetBeforeGPORes(int res)
{
	BeforeGPORes = res;
}

int sdkPaypassSetInputPINRes(int res, unsigned char *pinBuf, int bufLen)
{
	s32 ret;

	if(pinBuf == NULL || bufLen > 256 || bufLen < 0)
	{
		return SDK_PARA_ERR;
	}

	InputPINRes = res;
	ret = sdkEMVBaseConfigTLV(EMVTAG_PIN, pinBuf, bufLen);
	if(ret == SDK_OK)
	{
		return 0;
	}
	else
	{
		return ret;
	}
}

void sdkPaypassSetVerifyCardNoRes(int res)
{
	VerifyCardNoRes = res;
}

void sdkPaypassSetVerifyRevocationKeyRes(int res)
{
	VerifyRevocationKeyRes = res;
}

void sdkPaypassSetSecondTapCardRes(int res)
{
	SecondTapCardRes = res;
}

s32 sdkPaypassGetBeforeGPORes()
{
	//FIXME
//	BeforeGPORes = 0;
	if(BeforeGPORes == 0)
	{
		return SDK_OK;
	}
	else
	{
		return SDK_ERR;
	}
}

u8 sdkPaypassGetInputPINRes()
{
	//FIXME
//	InputPINRes = 0;
	if(InputPINRes == 0)
	{
		return RLT_EMV_OK;
	}
	else if(InputPINRes == -1)
	{
		return RLT_ERR_EMV_CancelTrans;
	}
	else if(InputPINRes == -5)
	{
		return RLT_ERR_EMV_InputBYPASS;
	}
	else
	{
		return RLT_EMV_ERR;
	}
}

s32 sdkPaypassGetVerifyCardNoRes()
{
	//FIXME
//	return 0;
	return VerifyCardNoRes;
}

s32 sdkPaypassGetVerifyRevocationKeyRes()
{
	//FIXME
//	return 0;
	return VerifyRevocationKeyRes;
}

s32 sdkPaypassGetSecondTapCardRes()
{
	//FIXME
//	return 0;
	return SecondTapCardRes;
}

int sdkPaypassLoadParamFromSzztAID()
{
	u8 fn[64]={0};
	s32 size = 0, maxAidLen = 1024, aidNum=0, lenOut=1024;
	s32 i,ret;
	u16 tlvLen=0;
	u8 aidSzzt[1024]={0};
	u8 *TLVGroup=NULL;

	Trace("paypass-info:","start sdkPaypassLoadParamFromSzztAID\r\n");

    sdkSysGetCurAppDir(fn);
    strcat(fn, "emvaids0.par");

	size = sdkGetFileSize(fn);
	if(size % 1024 != 0)
	{
		Trace("emv","Read SzztSDK AID len invalid\r\n");
		return SDK_ERR;
	}

	aidNum = size / 1024;
	for(i = 0; i < aidNum; i++)
	{
		ret = sdkReadFile(fn, aidSzzt, i<<10, &lenOut);
		if(ret == SDK_OK)
		{
			tlvLen = (u16)aidSzzt[1]*256 + (u16)aidSzzt[0];
			TLVGroup = (u8 *)sdkGetMem(tlvLen);
			memset(TLVGroup, 0, tlvLen);
			memcpy(TLVGroup, aidSzzt+4, tlvLen);
			TlvToTERMINFO(TLVGroup, tlvLen);
			sdkFreeMem(TLVGroup);
		}
		else
		{
			Trace("emv","Read SzztSDK AID content Error\r\n");
		}
	}
	return SDK_OK;

}

bool sdkPaypassGetPayMode(u8 *mode)//TRANSFLOW_EMVMODE=1,TRANSFLOW_MSDMODE=2
{
    if(gstPaypassTradeParam)
    {
        *mode = gstPaypassTradeParam->qPBOCOrMSD;
        return 1;
    }
    Trace("emv", "gstPaypassTradeParam is NULL, not initialize");
    return 0;
}

bool sdkPaypassGetPrintReceiptStatus(bool *type)//1 need print
{
    if (gstPaypassTradeParam)
    {
        *type = gstPaypassTradeParam->bPrintReceipt;
        return 1;
    }
    Trace("emv", "gstPaypassTradeParam is NULL, not initialize");
    return 0;
}

bool sdkPaypassSetPrintReceiptStatus(bool type)//TRANSFLOW_EMVMODE=1,TRANSFLOW_MSDMODE=2
{
    if (gstPaypassTradeParam)
    {
        gstPaypassTradeParam->bPrintReceipt = type;
        return 1;
    }
    Trace("emv", "gstPaypassTradeParam is NULL, not initialize");
    return 0;
}


SDK_EMVBASE_CVM_RESULT sdkPaypassGetCVMresult()
{
    SDK_EMVBASE_CVM_RESULT ret = 0;

    switch (gPaypassOutParameterSet.CVM)
    {
        case PAYPASS_OPS_CVM_NOCVMREQ:
            ret = SDKEMVBASE_CVM_NOCVMREQ;
            break;
        case PAYPASS_OPS_CVM_OBTAINSIGNATURE:
            ret = SDKEMVBASE_CVM_OBTAINSIGNATURE;
            break;
        case PAYPASS_OPS_CVM_ONLINEPIN:
            ret = SDKEMVBASE_CVM_ONLINEPIN;
            break;
        case PAYPASS_OPS_CVM_CONFVERIFIED:
            ret = SDKEMVBASE_CVM_CONFVERIFIED;
            break;

        default:
            ret = SDKEMVBASE_CVM_NA;
            break;
    }

    return ret;
}

void sdkPaypassTradeParamDestory(void)
{
    if(gstPaypassTradeParam != NULL)
    {
        if(gstPaypassTradeParam->CAPK != NULL)
        {
            emvbase_free(gstPaypassTradeParam->CAPK);
            gstPaypassTradeParam->CAPK = NULL;
        }

        /*if(gstPaypassTradeParam->SelectedApp != NULL)
        {
            emvbase_free(gstPaypassTradeParam->SelectedApp);
            gstPaypassTradeParam->SelectedApp = NULL;
        }

        if(gstPaypassTradeParam->AppListCandidate != NULL)
        {
            emvbase_free(gstPaypassTradeParam->AppListCandidate);
            gstPaypassTradeParam->AppListCandidate = NULL;
        }*/

        if(gstPaypassTradeParam->AuthData != NULL)
        {
            emvbase_free(gstPaypassTradeParam->AuthData);
            gstPaypassTradeParam->AuthData = NULL;
        }
        if(gstPaypassTradeParam->IPKModul != NULL)
        {
            emvbase_free(gstPaypassTradeParam->IPKModul);
            gstPaypassTradeParam->IPKModul = NULL;
        }
        if(gstPaypassTradeParam->ICCPKModul != NULL)
        {
            emvbase_free(gstPaypassTradeParam->ICCPKModul);
            gstPaypassTradeParam->ICCPKModul = NULL;
        }
        if(gstPaypassTradeParam->RedundantData != NULL)
        {
            emvbase_free(gstPaypassTradeParam->RedundantData);
            gstPaypassTradeParam->RedundantData = NULL;
        }

        emvbase_free(gstPaypassTradeParam);
        gstPaypassTradeParam = NULL;
    }

    if(gstPaypassTradeUnionStruct != NULL)
    {
        emvbase_free(gstPaypassTradeUnionStruct);
        gstPaypassTradeUnionStruct = NULL;
    }

    if(gstsdkPaypassTradeTable != NULL)
    {
        emvbase_free(gstsdkPaypassTradeTable);
        gstsdkPaypassTradeTable = NULL;
    }

}

void sdkPaypassMalloctradememory()
{
    if(gstPaypassTradeUnionStruct == NULL)
    {
        gstPaypassTradeUnionStruct = (PAYPASSTradeUnionStruct *)emvbase_malloc(sizeof(PAYPASSTradeUnionStruct));
    }
    memset(gstPaypassTradeUnionStruct, 0, (sizeof(PAYPASSTradeUnionStruct)));

    if(gstPaypassTradeParam != NULL)
    {
        if(gstPaypassTradeParam->CAPK != NULL)
        {
            emvbase_free(gstPaypassTradeParam->CAPK);
            gstPaypassTradeParam->CAPK = NULL;
        }

        /*if(gstPaypassTradeParam->SelectedApp != NULL)
        {
            emvbase_free(gstPaypassTradeParam->SelectedApp);
            gstPaypassTradeParam->SelectedApp = NULL;
        }

        if(gstPaypassTradeParam->AppListCandidate != NULL)
        {
            emvbase_free(gstPaypassTradeParam->AppListCandidate);
            gstPaypassTradeParam->AppListCandidate = NULL;
        }*/

        if(gstPaypassTradeParam->AuthData != NULL)
        {
            emvbase_free(gstPaypassTradeParam->AuthData);
            gstPaypassTradeParam->AuthData = NULL;
        }
        if(gstPaypassTradeParam->IPKModul != NULL)
        {
            emvbase_free(gstPaypassTradeParam->IPKModul);
            gstPaypassTradeParam->IPKModul = NULL;
        }
        if(gstPaypassTradeParam->ICCPKModul != NULL)
        {
            emvbase_free(gstPaypassTradeParam->ICCPKModul);
            gstPaypassTradeParam->ICCPKModul = NULL;
        }
        if(gstPaypassTradeParam->RedundantData != NULL)
        {
            emvbase_free(gstPaypassTradeParam->RedundantData);
            gstPaypassTradeParam->RedundantData = NULL;
        }

        emvbase_free(gstPaypassTradeParam);
        gstPaypassTradeParam = NULL;
    }

    if(gstPaypassTradeParam == NULL)
    {
        gstPaypassTradeParam = (PAYPASSDLLTRADEPARAMETER *)emvbase_malloc(sizeof(PAYPASSDLLTRADEPARAMETER));
    }
    memset(gstPaypassTradeParam, 0, sizeof(PAYPASSDLLTRADEPARAMETER));

    if(gstsdkPaypassTradeTable == NULL)
    {
        gstsdkPaypassTradeTable = (SDK_PAYPASS_TRADE_PARAM *)emvbase_malloc(sizeof(SDK_PAYPASS_TRADE_PARAM));
    }
    memset(gstsdkPaypassTradeTable, 0, (sizeof(SDK_PAYPASS_TRADE_PARAM)));

    return;
}

static void sdkPaypassIsoCommand(EMVBASE_APDU_SEND *ApduSend, EMVBASE_APDU_RESP *apdu_r)
{
    //int key=0;

    #ifdef PAYPASS_DATAEXCHANGE
    sdkPaypassDateExchangeBeforeSendCMD();
    #endif

    sdkEMVBase_ContactlessIsoCommand(ApduSend, apdu_r);
}

//static void sdkPaypassIsoCommandEx(EMVBASE_APDU_SEND *ApduSend, EMVBASE_APDU_RESP *apdu_r, unsigned short offset)
//{
//	#ifdef PAYPASS_DATAEXCHANGE
//    sdkPaypassDateExchangeBeforeSendCMD();
//    #endif
//
//	sdkEMVBase_ContactlessIsoCommandEx(ApduSend, apdu_r, offset);
//}
//
//static void sdkPaypassHashWithSensitiveData(unsigned int mode, unsigned char cardtype,
//	unsigned int inlen, unsigned char *p_datain, unsigned int *p_outlen, unsigned char *p_outdata)
//{
//	sdkDevGetHashWithSensitiveData(mode, cardtype, inlen, p_datain, p_outlen, p_outdata);
//}
//
//static void sdkPaypassDealSred(u32 cmd, u32 dataInLen, u8 *dataIn, u32 *dataOutLen, u8 *dataOut)
//{
//	sdkEMVBase_DealSred(cmd, dataInLen, dataIn, dataOutLen, dataOut);
//}


static u32 gucPaypassCCCtimer_startid = 0;
u8  gucPaypassCCCcovertimers = 0;

void sdkPaypassCCCtimerGetStartTimeID(void)
{
    gucPaypassCCCtimer_startid = sdkTimerGetId();
//	gucPaypassCCCtimer_startid = sdkTimerGetIdtemp();
}


void sdkPaypassCCCtimerWaitTime(void)
{
    u32 PaypassCCCtimer = 0;
    //u32 aaa=0,bbb=0;
    PaypassCCCtimer = (unsigned int)((1 << gucPaypassCCCcovertimers) * PAYPASS_DLL_CCCTIMER);
	if(1 == gucPaypassCCCcovertimers)
	{
    	PaypassCCCtimer += 400;
	}
	else if(5 == gucPaypassCCCcovertimers)
	{
		PaypassCCCtimer -= 2000;
	}
	else
	{
		PaypassCCCtimer += 3;
	}
    Trace("emv", "gucPaypassCCCcovertimers=%d,PaypassCCCtimer=%u,start id=%d\n", gucPaypassCCCcovertimers, PaypassCCCtimer, gucPaypassCCCtimer_startid);
    //aaa = sdkTimerGetId();
    //Trace("","START cur ms=%u",aaa);
    while(1)
    {
        if( 1 == sdkTimerIsEnd(gucPaypassCCCtimer_startid, PaypassCCCtimer))
        {
            break;
        }
        //ddi_sys_msleep(2);
        sdkmSleep(2);
        //usleep(2*1000);
    }
    //bbb = sdkTimerGetId();
    //Trace("","END cur ms=%u,SPEND TIME = %d \n",bbb,(bbb-aaa));
}

//20130503   20140222
u8 sdkpaypass_deletespecifytornindex(unsigned int index)
{
    TORNTRANSACTIONMAPLIST *tornlistpointer;
    TORNTRANSACTIONMAPLIST *tornlistnextpointer;

    tornlistpointer = gPaypassTornTotalInfo.tornhead;
    tornlistnextpointer = tornlistpointer;
    Trace("emv", "sdkpaypass_deletespecifytornindex,gPaypassTornTotalInfo.tornhead=%p", gPaypassTornTotalInfo.tornhead);

    while(tornlistnextpointer != NULL)
    {
        if(tornlistnextpointer->TornTransactionInfo.index == index)
        {
            gPaypassTornTotalInfo.flashusedflag[index] = 0;
            if(tornlistnextpointer == gPaypassTornTotalInfo.tornhead)
            {
                gPaypassTornTotalInfo.tornhead = tornlistnextpointer->next;
            }
            else
            {
                tornlistpointer->next = tornlistnextpointer->next;
            }
            emvbase_free(tornlistnextpointer);
            return 0;
        }
        tornlistpointer = tornlistnextpointer;
        tornlistnextpointer = tornlistnextpointer->next;
    }
    Trace("emv", "sdkpaypass_deletespecifytornindex out gPaypassTornTotalInfo.tornhead=%p", gPaypassTornTotalInfo.tornhead);
    return 0;
}








//20130503    20140222
void sdkpaypass_addtornmap(u32 index, PaypassTornTransactionLogRecord *torntransactionlogrecord)
{
    TORNTRANSACTIONMAPLIST *tornlistpointer;
    TORNTRANSACTIONMAPLIST *tornlistnextpointer;

    tornlistpointer = gPaypassTornTotalInfo.tornhead;
    tornlistnextpointer = tornlistpointer;

    #ifdef SXL_DEBUG
    Trace("emv", "\r\nindex=%d,tornlistnextpointer=%p,gPaypassTornTotalInfo.tornhead=%p\r\n", index, tornlistnextpointer, gPaypassTornTotalInfo.tornhead);
    #endif
    while(tornlistnextpointer != NULL)
    {
        tornlistpointer = tornlistnextpointer;

        tornlistnextpointer = tornlistnextpointer->next;
    }

    #ifdef SXL_DEBUG
    Trace("emv", "\r\n tornlistnextpointer=%p,gPaypassTornTotalInfo.tornhead=%p\r\n", tornlistnextpointer, gPaypassTornTotalInfo.tornhead);
    #endif
    tornlistnextpointer = (TORNTRANSACTIONMAPLIST *)emvbase_malloc(sizeof(TORNTRANSACTIONMAPLIST));  //sxl ?????????????????????
    if(gPaypassTornTotalInfo.tornhead == NULL)
    {
        gPaypassTornTotalInfo.tornhead = tornlistnextpointer;
        Trace("emv", " HEAD==NULL ,gPaypassTornTotalInfo.tornhead=%p\n", gPaypassTornTotalInfo.tornhead);
    }
    else
    {
        tornlistpointer->next = tornlistnextpointer;
    }

    #ifdef SXL_DEBUG
    Trace("emv", "\r\nadd torn log1,tornlistnextpointer=%p,gPaypassTornTotalInfo.tornhead=%p\r\n", tornlistnextpointer, gPaypassTornTotalInfo.tornhead);
    #endif

    memset(tornlistnextpointer, 0, sizeof(TORNTRANSACTIONMAPLIST));
    tornlistnextpointer->TornTransactionInfo.index = index;
    tornlistnextpointer->TornTransactionInfo.PANLen = torntransactionlogrecord->PANLen;
    tornlistnextpointer->TornTransactionInfo.PANseq = torntransactionlogrecord->PANSeq;
    tornlistnextpointer->TornTransactionInfo.PANseqexist = torntransactionlogrecord->PANSeqexist;
    #ifdef SXL_DEBUG
    Trace("emv", "\r\nadd torn log5\r\n");

    Trace("emv", " gPaypassTornTotalInfo.tornhead=%p\n", gPaypassTornTotalInfo.tornhead);
    #endif

    memcpy(tornlistnextpointer->TornTransactionInfo.PAN, torntransactionlogrecord->PAN, torntransactionlogrecord->PANLen);
    memcpy(tornlistnextpointer->TornTransactionInfo.recordseqnum, torntransactionlogrecord->recordseqnum, 6);


    memcpy(tornlistnextpointer->TornTransactionInfo.transdatetime, torntransactionlogrecord->transdatetime, 7);


    #ifdef SXL_DEBUG
    Trace("emv", "\r\nadd torn log2,tornlistnextpointer=%p\r\n", tornlistnextpointer);

    Trace("emv", " gPaypassTornTotalInfo.tornhead=%p\n", gPaypassTornTotalInfo.tornhead);
    #endif
}



//??????��?torn log     //20140222
unsigned char sdkpaypass_checkifcardintornlog(void)
{
    //u32 index;
    TORNTRANSACTIONMAPLIST *tornlistnextpointer = NULL;
    u8 newstlogseq[6], tmpseq[6];
    u32 newstlogindex = 0;
    u8 newstlogexist = 0;


    u8 PAN[10], PANSeq;
    u16 PANLen;

    emvbase_avl_gettagvalue_all(EMVTAG_PAN, PAN, &PANLen);
    PANSeq = emvbase_avl_gettagvalue(EMVTAG_PANSeq);

    #ifdef EMVB_DEBUG
    Trace("emv", "\r\n paypass_checkifcardintornlog:%d\r\n", PANLen);
    TraceHex("emv", "PAN", PAN, PANLen);
    Trace("emv", "\r\nCardInfo.PANSeq =  %02x \r\n", PANSeq);
    Trace("emv", "\r\n gPaypassTornTotalInfo.tornhead =  %p \r\n", gPaypassTornTotalInfo.tornhead);
    #endif

    tornlistnextpointer = gPaypassTornTotalInfo.tornhead;
    while(tornlistnextpointer != NULL)
    {
        #ifdef EMVB_DEBUG

        Trace("emv", "\r\n tornlistnextpointer =  %p \r\n", tornlistnextpointer);
        Trace("emv", "\r\npaypass_checkifcardintornlog1:%d\r\n", tornlistnextpointer->TornTransactionInfo.PANLen);
        TraceHex("emv", "PAN", tornlistnextpointer->TornTransactionInfo.PAN, tornlistnextpointer->TornTransactionInfo.PANLen);
        Trace("emv", "\r\nCardInfo.PANSeq1 =  %02x %d\r\n", tornlistnextpointer->TornTransactionInfo.PANseq, tornlistnextpointer->TornTransactionInfo.index);
        Trace("emv", "\r\nCardInfo.PANSeq1 =  %02x %d\r\n", tornlistnextpointer->TornTransactionInfo.PANseqexist, tornlistnextpointer->TornTransactionInfo.index);
        #endif

        if(tornlistnextpointer->TornTransactionInfo.PANLen == PANLen)
        {
            if((tornlistnextpointer->TornTransactionInfo.PANseq == PANSeq && tornlistnextpointer->TornTransactionInfo.PANseqexist) || tornlistnextpointer->TornTransactionInfo.PANseqexist == 0)
            {
                if(memcmp(tornlistnextpointer->TornTransactionInfo.PAN, PAN, PANLen) == 0)
                {
                    if(newstlogexist == 0)
                    {
                        newstlogindex = tornlistnextpointer->TornTransactionInfo.index;
                        newstlogexist = 1;
                        memcpy(newstlogseq, tornlistnextpointer->TornTransactionInfo.recordseqnum, 6);
                    }
                    else
                    {
                        if(memcmp(newstlogseq, tornlistnextpointer->TornTransactionInfo.recordseqnum, 6) > 0)
                        {
                            memcpy(tmpseq, newstlogseq, 6);
                            EMVBaseBcdSub(tmpseq, tornlistnextpointer->TornTransactionInfo.recordseqnum, 6);
                            if(memcmp(tmpseq, "\x10\x00\x00\x00\x00\x00", 6) > 0)
                            {
                                memcpy(newstlogseq, tornlistnextpointer->TornTransactionInfo.recordseqnum, 6);
                                newstlogindex = tornlistnextpointer->TornTransactionInfo.index;
                            }
                            else  //oldestseqnum is oldest
                            {

                            }
                        }
                        else
                        {
                            memcpy(tmpseq, tornlistnextpointer->TornTransactionInfo.recordseqnum, 6);
                            EMVBaseBcdSub(tmpseq, newstlogseq, 6);
                            if(memcmp(tmpseq, "\x10\x00\x00\x00\x00\x00", 6) > 0)
                            {

                            }
                            else  //oldestseqnum is oldest
                            {
                                memcpy(newstlogseq, tornlistnextpointer->TornTransactionInfo.recordseqnum, 6);
                                newstlogindex = tornlistnextpointer->TornTransactionInfo.index;
                            }
                        }
                    }
                }
            }
        }
        tornlistnextpointer = tornlistnextpointer->next;
    }


    #ifdef EMVB_DEBUG
    Trace("emv", "\r\nnewstlogexist = %d %d\r\n", newstlogexist, newstlogindex);
    #endif

    if(newstlogexist)
    {
        gPaypassTornTotalInfo.currentusedindex = newstlogindex;
        return 1;
    }

    return 0;
}

unsigned char sdkPaypass_RECOVERAC(EMVBASE_APDU_SEND *apdu_s)
{
    unsigned int ret;


    memcpy(apdu_s->Command, "\x80\xD0\x00\x00", 4);

    if(pPaypasstorntransactionlogrecord == NULL)
    {
        pPaypasstorntransactionlogrecord = (PaypassTornTransactionLogRecord *)emvbase_malloc(sizeof(PaypassTornTransactionLogRecord));
        if(pPaypasstorntransactionlogrecord == NULL)
        {
            Trace("emv", "Paypass-info:pPaypasstorntransactionlogrecord getmem error!\n", ret);
			return RLT_EMV_TERMINATE_TRANSERR;
        }
    }
    //????????��????DRDOL Related Data
    ret = sdkPaypass_TornRecordread(gPaypassTornTotalInfo.currentusedindex, pPaypasstorntransactionlogrecord);
    if(ret != 0)
    {
        return RLT_EMV_ERR;
    }


    if(pPaypasstorntransactionlogrecord != NULL)
    {
        apdu_s->Lc = pPaypasstorntransactionlogrecord->DRDOLRelatedLen;

		if (pPaypasstorntransactionlogrecord->DRDOLRelatedLen > 255)
		{
			apdu_s->Lc=255;
            Trace("emv", "\r\n WARRNING pPaypasstorntransactionlogrecord->DRDOLRelatedLen=%d \r\n",pPaypasstorntransactionlogrecord->DRDOLRelatedLen);
		}
        memcpy(apdu_s->DataIn, pPaypasstorntransactionlogrecord->DRDOLRelatedData, apdu_s->Lc);

        #if 1 /*Modify by luohuidong at 2016.10.05  17:6 */
        apdu_s->Le = 256;
        #else
        apdu_s->Le = 0;
        #endif /* if 0 */

		apdu_s->EnableCancel = 0;
    }

    return RLT_EMV_OK;
}

//20140222
unsigned char sdkPaypass_freetornlogtempdata(void)
{
    if(pPaypasstorntransactionlogrecord != NULL)
    {
        emvbase_free(pPaypasstorntransactionlogrecord);
        pPaypasstorntransactionlogrecord = NULL;
    }
    return 0;
}

//20160824_lhd
unsigned char sdkPaypass_gettornlogtempdata(TornTransactionLogIDSPartRecord *ptornlogrecord)
{

    if(NULL == ptornlogrecord)
    {
        return 2;
    }

    if(pPaypasstorntransactionlogrecord != NULL)
    {
        ptornlogrecord->DSSummary1Len = pPaypasstorntransactionlogrecord->DSSummary1Len;
        ptornlogrecord->IDSStatus = pPaypasstorntransactionlogrecord->IDSStatus;
        memcpy(ptornlogrecord->DSSummary1, pPaypasstorntransactionlogrecord->DSSummary1, 16);

        return 0;
    }
    //PaypassTornTransactionLogRecord *pPaypasstorntransactionlogrecord
    return 1;

}


//sxl20130503  20140222
void sdkPaypass_TornRecorddelete(unsigned int index)
{
    u8 temp[4];
    u8 flag[4];
    u32 addr;
    s32 ret;

    memset(temp, 0xff, sizeof(temp));

    addr = index * PaypassTornTransactionRecordLen;

    while(1)
    {
        ret = sdkPaypassInsertFile( PaypassTornTransaction, (u8 *)temp, 4, addr);
        if(ret != 0)
        {
            Trace("emv", "sdkPaypass_TornRecorddelete err fail = %d\n", ret);
            continue;
        }

        ret = sdkPaypassReadFile( PaypassTornTransaction, flag, 4, addr);
        if(ret != 4)  //??????log
        {
            Trace("emv", "sdkPaypassReadFile err fail = %d\n", ret);
            continue;
        }

        if(memcmp(temp, flag, 4) == 0)
        {
            Trace("emv", "sdkPaypassReadFile err fail = %d\n", ret);
            return;
        }
    }


}


//20140222
u8 sdkpaypass_deletespecifytornlog(unsigned int index)
{
    TORNTRANSACTIONMAPLIST *tornlistpointer;
    TORNTRANSACTIONMAPLIST *tornlistnextpointer;

    tornlistpointer = gPaypassTornTotalInfo.tornhead;
    tornlistnextpointer = tornlistpointer;

    Trace("emv", "sdkpaypass_deletespecifytornlog,gPaypassTornTotalInfo.tornhead=%p\n", gPaypassTornTotalInfo.tornhead);
    while(tornlistnextpointer != NULL)
    {
        if(tornlistnextpointer->TornTransactionInfo.index == index)
        {
            #ifdef EMVB_DEBUG
            Trace("emv", "\r\ndelete torn log %d\r\n", index);
            #endif
            if(gPaypassTornTotalInfo.flashusedflag[index] == 0)
            {
                return 0;
            }

            //sdkPaypass_TornRetriveData(index);
            sdkPaypass_TornRecorddelete(index);
            gPaypassTornTotalInfo.flashusedflag[index] = 0;
            if(tornlistnextpointer == gPaypassTornTotalInfo.tornhead)
            {
                gPaypassTornTotalInfo.tornhead = tornlistnextpointer->next;
            }
            else
            {
                tornlistpointer->next = tornlistnextpointer->next;
            }
            emvbase_free(tornlistnextpointer);
			tornlistnextpointer=NULL;
            return 0;
        }
        tornlistpointer = tornlistnextpointer;
        tornlistnextpointer = tornlistnextpointer->next;
    }

    return 0;
}


// ?????????torn?????


//20140222
u8 sdkpaypass_deletetornlog(void)
{
    s32 ret;

    Trace("emv", "sdkpaypass_deletetornlog,gPaypassTornTotalInfo.tornhead=%p\n", gPaypassTornTotalInfo.tornhead);
    ret = sdkpaypass_deletespecifytornlog(gPaypassTornTotalInfo.currentusedindex);
    if(ret == 0)
    {
        return RLT_EMV_OK;
    }


    return RLT_EMV_ERR;

}

//20140222
unsigned char sdkPaypass_checkifnotprivatetag(unsigned char *tag, unsigned char taglen, EMVBASETAGCVLITEM *retrunitem)
{

    //int i;
    unsigned char tagknown = 0;
    unsigned char present = 0;
    //unsigned char empty = 1;
    //unsigned char tagprivateclass = 0;
    //unsigned char updateRA = 0;
    unsigned char temptag[4];
    unsigned char temptaglen;
    EMVBASETAGCVLITEM *item;
    unsigned char ret = 2;
    int ret1;

    //tag check  ??PAYPASS ?????????


    //???TREE?????????
    memset(temptag, 0, sizeof(temptag));
    temptaglen = (taglen > 3) ? 3 : (taglen);
    memcpy(temptag, tag, temptaglen);
    item = emvbase_avl_gettagitempointer(temptag);
    if(item != NULL)
    {
        present = 1;
        if(item->SupAppType & EMVTAGTYPE_PAYPASS)
        {
            tagknown = 1;
        }

    }

    if(present == 0)  //check if tag known
    {
        ret1 = emvbase_TagBaseLib_read_condition(tag, taglen, EMVTAGTYPE_PAYPASS, 0, 0, retrunitem);
        if(ret1 == 0)
        {
            tagknown = 1;
        }

    }
    else
    {

        memcpy(retrunitem, item, sizeof(EMVBASETAGCVLITEM));

    }


    /*
    if((tag[0]&0xc0)== 0xc0)
    {
    	tagprivateclass = 1;
    }
    */


    if(tagknown)
    {
        ret = 1;
    }


    return ret;
}


//20140222
s32 sdkPaypass_TornRetriveDOLData(u8 *DOL, u32 DOLLen, u8 *RelatedData, u32 RelatedDataLen)
{
    //unsigned char k;
    unsigned short index, len, dataindex;
    //unsigned char tagindex,tmpval,addvalue,ifemvtag;  //tmpdata,
    unsigned char bInTable;
    unsigned char tag[4], taglen;
    unsigned char ret;
    //unsigned char needdonextstep = 1;
    EMVBASETAGCVLITEM tagitem;
    unsigned char  *buf;      //[255]
    #ifdef EMVB_DEBUG
    //unsigned char i;
    #endif



    buf = (unsigned char *)emvbase_malloc(255);

    index = 0;
    dataindex = 0;

    #ifdef EMVB_DEBUG
    TraceHex("emv", "RelatedData", RelatedData, RelatedDataLen);
    #endif
    len = 0;

    while(index < DOLLen)
    {
        bInTable = 0;
        memset(tag, 0, sizeof(tag));
        taglen = 0;
        tag[taglen++] = DOL[index];
        if((tag[0] & 0x1f) == 0x1f)
        {
            tag[taglen++] = DOL[index + 1];
            if(tag[1] & 0x80)
            {
                tag[taglen++] = DOL[index + 2];
            }
        }

        #ifdef EMVB_DEBUG
        TraceHex("emv", "check tag", tag, taglen);
        #endif

        ret = sdkPaypass_checkifnotprivatetag(tag, taglen, &tagitem);
        #ifdef EMVB_DEBUG
        Trace("emv", "sdkPaypass_TornRetriveDOLData ret123 = %d", ret);
        #endif
        if(ret == 0)
        {
            #ifdef EMVB_DEBUG
            Trace("emv", "\r\n%02x %02x wrong tag1\r\n", tag[0], tag[1]);
            #endif
            emvbase_free(buf);
            return RLT_ERR_EMV_IccDataFormat;
        }
        else if(ret == 2)
        {
            //needdonextstep = 0;
        }
        else
        {
            index += taglen;
            if((index + 1) > DOLLen)
            {

                #ifdef EMVB_DEBUG
                Trace("emv", "\r\nEmv_PaypassReadAFLRetData122313 %d\r\n", index);
                #endif
                emvbase_free(buf);
                return RLT_ERR_EMV_IccDataFormat;
            }
            len = DOL[index++];  //sxl  paywave??EMV?????????//TEST qPBOC???????????????

            #ifdef EMVB_DEBUG
            TraceHex("emv", "insert", tag, taglen);
            #endif

            if(len)
            {
                if(len > tagitem.maxlen)
                {
                    if(tagitem.datafomat & EMVTAGFORMAT_N)
                    {
                        emvbase_avl_inserttag(&tagitem, tag, strlen((char *)tag), &RelatedData[dataindex + len - tagitem.maxlen], tagitem.maxlen, NULL);
                    }
                    else
                    {
                        emvbase_avl_inserttag(&tagitem, tag, strlen((char *)tag), &RelatedData[dataindex], tagitem.maxlen, NULL);
                    }

                }
                else if(len < tagitem.minlen)
                {
                    memset(buf, 0, 255);
                    if(tagitem.datafomat & EMVTAGFORMAT_N)
                    {
                        memcpy(&buf[tagitem.minlen - len], &RelatedData[dataindex], len);
                    }
                    else
                    {
                        memcpy(buf, &RelatedData[dataindex], len);
                    }
                    emvbase_avl_inserttag(&tagitem, tag, strlen((char *)tag), buf, tagitem.minlen, NULL);

                }
                else
                {
                    emvbase_avl_inserttag(&tagitem, tag, strlen((char *)tag), &RelatedData[dataindex], len, NULL);
                }
            }

            dataindex += len;

            bInTable = 1;
        }

        #ifdef EMVB_DEBUG
        Trace("emv", "\r\nbInTable = %d\r\n", index);
        #endif


        if(bInTable == 0)
        {
            index += taglen;

            if((index + 1) > DOLLen)
            {

                #ifdef EMVB_DEBUG
                Trace("emv", "\r\nEmv_PaypassReadAFLRetData121313 %d\r\n", index);
                #endif
                emvbase_free(buf);
                return RLT_ERR_EMV_IccDataFormat;
            }
            len = DOL[index++];

            dataindex += len;
        }

        #ifdef EMVB_DEBUG
        Trace("emv", "\r\nindex = %d %d\r\n", dataindex, RelatedDataLen);
        #endif

        if(dataindex > RelatedDataLen)
        {

            emvbase_free(buf);
            return RLT_ERR_EMV_IccDataFormat;
        }


        #ifdef EMVB_DEBUG
        Trace("emv", "\r\nindex = %d %d\r\n", index, bInTable);
        #endif
    }


    emvbase_free(buf);
    return RLT_EMV_OK;


}



//20140222
s32 sdkPaypass_TornRetriveSpecifyData(PaypassTornTransactionLogRecord *torntransactionlogrecord)
{

    s32 ret;
    //u32 i;
    u8 *DOL;             //[300]
    u16 DOLLen;
    u8 TransTypeValue;


    DOL = (u8 *)emvbase_malloc(300);

    emvbase_avl_gettagvalue_all(EMVTAG_PDOL, DOL, &DOLLen);
    ret = sdkPaypass_TornRetriveDOLData(DOL, DOLLen, torntransactionlogrecord->PDOLRelatedData, torntransactionlogrecord->PDOLRelatedLen); //PDOL
    if(ret != 0)
    {
        #ifdef EMVB_DEBUG
        Trace("emv", "\r\nsdkPaypass_TornRetriveSpecifyData err1\r\n");
        #endif
        emvbase_free(DOL);
        return ret;
    }

    emvbase_avl_createsettagvalue(EMVTAG_PDOLData, torntransactionlogrecord->PDOLRelatedData, torntransactionlogrecord->PDOLRelatedLen);


    emvbase_avl_gettagvalue_all(EMVTAG_CDOL1, DOL, &DOLLen);
    if((torntransactionlogrecord->CDOL1Len == DOLLen) && (memcmp(torntransactionlogrecord->CDOL1, DOL, DOLLen) == 0))
    {
        ret = sdkPaypass_TornRetriveDOLData(DOL, DOLLen, torntransactionlogrecord->CDOL1RelatedData, torntransactionlogrecord->CDOL1RelatedLen); //PDOL
    }
    else
    {
        ret = sdkPaypass_TornRetriveDOLData(torntransactionlogrecord->CDOL1, torntransactionlogrecord->CDOL1Len, torntransactionlogrecord->CDOL1RelatedData, torntransactionlogrecord->CDOL1RelatedLen);
    }
    if(ret != 0)
    {
        #ifdef EMVB_DEBUG
        Trace("emv", "\r\nsdkPaypass_TornRetriveSpecifyData err2\r\n");
        #endif
        emvbase_free(DOL);
        return ret;
    }

    emvbase_avl_createsettagvalue(EMVTAG_CDOL1RelatedData, torntransactionlogrecord->CDOL1RelatedData, torntransactionlogrecord->CDOL1RelatedLen);


    emvbase_avl_gettagvalue_all(EMVTAG_DRDOL, DOL, &DOLLen);
    ret = sdkPaypass_TornRetriveDOLData(DOL, DOLLen, torntransactionlogrecord->DRDOLRelatedData, torntransactionlogrecord->DRDOLRelatedLen); //PDOL
    if(ret != 0)
    {
        #ifdef EMVB_DEBUG
        Trace("emv", "\r\nsdkPaypass_TornRetriveSpecifyData err3\r\n");
        #endif
        emvbase_free(DOL);
        return ret;
    }

    //???????
    emvbase_avl_createsettagvalue(EMVTAG_AmtAuthNum, torntransactionlogrecord->AmtAuthNum, 6);
    if(torntransactionlogrecord->AmtOtherNumexist)
    {
        emvbase_avl_createsettagvalue(EMVTAG_AmtOtherNum, torntransactionlogrecord->AmtOtherNum, 6);
    }
    else
    {
        emvbase_avl_createsettagvalue(EMVTAG_AmtOtherNum, NULL, 0);
    }
    //memcpy(gPayData.amount,torntransactionlogrecord->AmtAuthNum,6);
    //memcpy(gPayData.amountother,torntransactionlogrecord->AmtOtherNum,6);


    //??????????????????????????
    if(torntransactionlogrecord->PayPassPreBalanceExist == 1)
    {
        emvbase_avl_createsettagvalue(EMVTAG_BalanceBeforeGAC, torntransactionlogrecord->PayPassPreBalance, 6);
    }
    else if(torntransactionlogrecord->PayPassPreBalanceExist == 2)
    {
        emvbase_avl_createsettagvalue(EMVTAG_BalanceBeforeGAC, NULL, 0);
    }

    emvbase_avl_createsettagvalue(EMVTAG_CVMResult, torntransactionlogrecord->CVMResult, 3);

    if(torntransactionlogrecord->InterfaceDeviceSerialNumberExist)
    {
        emvbase_avl_createsettagvalue(EMVTAG_IFD_SN, torntransactionlogrecord->InterfaceDeviceSerialNumber, 8);
    }
    else
    {
        emvbase_avl_createsettagvalue(EMVTAG_IFD_SN, NULL, 0);
    }

    emvbase_avl_createsettagvalue(EMVTAG_TermCapab, torntransactionlogrecord->TermCapab, 3);

    if(torntransactionlogrecord->CountryCodeexist)
    {
        emvbase_avl_createsettagvalue(EMVTAG_CountryCode, torntransactionlogrecord->CountryCode, 2);
    }
    else
    {
        emvbase_avl_createsettagvalue(EMVTAG_CountryCode, NULL, 0);
    }


    emvbase_avl_settag(EMVTAG_TermType, torntransactionlogrecord->TermType);
    emvbase_avl_createsettagvalue(EMVTAG_TVR, torntransactionlogrecord->TVR, 5);

    #ifdef EMVB_DEBUG
    Trace("emv", "\r\ntorntransactionlogrecord->TransCateCodeexist=%d\r\n", torntransactionlogrecord->TransCateCodeexist);
    if(torntransactionlogrecord->TransCateCodeexist)
    {
        Trace("emv", "\r\ntorntransactionlogrecord->TransCateCode=%d\r\n", torntransactionlogrecord->TransCateCode);
    }
    #endif

    if(torntransactionlogrecord->TransCateCodeexist)
    {
        emvbase_avl_createsettagvalue(EMVTAG_MCHIPTransCateCode, &(torntransactionlogrecord->TransCateCode), 1);
    }
    else
    {
        //20160906_lhd add for case 3MX6-9000 emvbase_avl_createsettagvalue(EMVTAG_MCHIPTransCateCode,NULL,0);
    }

    if(torntransactionlogrecord->TransCurcyCodeexist)
    {
        emvbase_avl_createsettagvalue(EMVTAG_TransCurcyCode, torntransactionlogrecord->TransCurcyCode, 2);
    }
    else
    {
        emvbase_avl_createsettagvalue(EMVTAG_TransCurcyCode, NULL, 0);
    }

    emvbase_avl_createsettagvalue(EMVTAG_TransDate, torntransactionlogrecord->TransDate, 3);
    emvbase_avl_createsettagvalue(EMVTAG_TransTime, torntransactionlogrecord->TransTime, 3);
    TransTypeValue = torntransactionlogrecord->TransTypeValue;
    emvbase_avl_settag(EMVTAG_TransTypeValue, TransTypeValue);
    emvbase_avl_createsettagvalue(EMVTAG_UnpredictNum, torntransactionlogrecord->UnpredictNum, 4);


    emvbase_avl_settag(EMVTAG_ReferenceControlParameter, torntransactionlogrecord->ReferenceControlParameter);

    #ifdef PAYPASS_DATAEXCHANGE

    emvbase_avl_settag(EMVTAG_IDSStatus, torntransactionlogrecord->IDSStatus);
    emvbase_avl_createsettagvalue(EMVTAG_DSSummary1, torntransactionlogrecord->DSSummary1, torntransactionlogrecord->DSSummary1Len);

    #ifdef EMVB_DEBUG
    Trace("emv", "\r\n sdkPaypass_TornRetriveSpecifyData TAG_IDSStatus %x\r\n", torntransactionlogrecord->IDSStatus);
    Trace("emv", "\r\n sdkPaypass_TornRetriveSpecifyData TAG_DSSummary1 %x\r\n", torntransactionlogrecord->DSSummary1Len);
    TraceHex("emv", "data", torntransactionlogrecord->DSSummary1, 16);
    #endif
    #endif

    #ifdef PAYPASS_RRP

    emvbase_avl_createsettagvalue(EMVTAG_TermRREntropy, torntransactionlogrecord->TerminalRelayResistanceEntropy, 4);
    emvbase_avl_createsettagvalue(EMVTAG_DeviceRREntropy, torntransactionlogrecord->DeviceRelayResistanceEntropy, 4);
    emvbase_avl_createsettagvalue(EMVTAG_MinTimeForProcessingRR_APDU, torntransactionlogrecord->MinTimeForProcessingRelayResistanceAPDU, 2);
    emvbase_avl_createsettagvalue(EMVTAG_MaxTimeForProcessingRR_APDU, torntransactionlogrecord->MaxTimeForProcessingRelayResistanceAPDU, 2);
    emvbase_avl_createsettagvalue(EMVTAG_DeviceEstimatedTransmissionTimeForRR_RAPDU, torntransactionlogrecord->DeviceEstimatedTransTForRRRAPDU, 2);
    emvbase_avl_createsettagvalue(EMVTAG_MeasureRRProcessingTime, torntransactionlogrecord->MeasureRelayResistanceProcessingTime, 2);

    emvbase_avl_settag(EMVTAG_RRPCounter, torntransactionlogrecord->RRP_counter);

    #endif

    #if 0
    if(TransTypeValue == PAYPASSTRADE_PURWITHCASH)
    {
        gPayData.nowtask = CASHBACK;
    }
    else if(TransTypeValue == PAYPASSTRADE_REFUND)
    {
        gPayData.nowtask = REFUND;
    }
    else if(TransTypeValue == PAYPASSTRADE_CASH || TransTypeValue == 0x12)
    {
        gPayData.nowtask = CASH;
    }
    else if(TransTypeValue == 0x21)
    {
        gPayData.nowtask = CASHDEPOSIT;
    }
    #if 1//20151211_lhd
    else if((TransTypeValue == 0x17))
    {
        gPayData.nowtask = CASHDISBURSEMENT;
    }
    #endif
    else
    {
        gPayData.nowtask = SALE;
    }
    gPayData.tradetypevalue = TransTypeValue;
    #endif

    emvbase_free(DOL);
    return 0;
}



//20140222
s32 sdkPaypass_TornRetriveData(PaypassTornTransactionLogRecord *torntransactionlogrecord)
{
    s32 ret;


    ret = sdkPaypass_TornRetriveSpecifyData(torntransactionlogrecord);
    if(ret != 0)
    {
        return 1;
    }

    return 0;
}

//20140222
unsigned char sdkPaypass_copytornrecorddata(void)
{
    unsigned char ret;

    if(pPaypasstorntransactionlogrecord == NULL)
    {
        #ifdef EMVB_DEBUG
        Trace("emv", "\r\n sdkPaypass_copytornrecorddata (pPaypasstorntransactionlogrecord == NULL) \r\n");
        #endif

        return 1;
    }
    ret = sdkPaypass_TornRetriveData(pPaypasstorntransactionlogrecord);


    return ret;
}



//20140222
unsigned int sdkPaypass_torn_getoldestseqnum(void)
{
    u32 index = 0;
    u8 oldestseqnum[6];
    u8 tmpseq[6];
    u8 firstflag = 0;

    TORNTRANSACTIONMAPLIST *tornlistnextpointer;

    memset(oldestseqnum, 0, sizeof(oldestseqnum));
    tornlistnextpointer = gPaypassTornTotalInfo.tornhead;

    Trace("emv", "gPaypassTornTotalInfo.tornhead=%p\n", gPaypassTornTotalInfo.tornhead);

    while(tornlistnextpointer != NULL)
    {
        if(firstflag == 0)
        {
            memcpy(oldestseqnum, tornlistnextpointer->TornTransactionInfo.recordseqnum, 6);
            index = tornlistnextpointer->TornTransactionInfo.index;
            firstflag = 1;
        }
        else
        {
            if(memcmp(oldestseqnum, tornlistnextpointer->TornTransactionInfo.recordseqnum, 6) > 0)
            {
                memcpy(tmpseq, oldestseqnum, 6);
                EMVBaseBcdSub(tmpseq, tornlistnextpointer->TornTransactionInfo.recordseqnum, 6);
                if(memcmp(tmpseq, "\x10\x00\x00\x00\x00\x00", 6) > 0) //oldestseqnum is oldest
                {

                }
                else
                {
                    memcpy(oldestseqnum, tornlistnextpointer->TornTransactionInfo.recordseqnum, 6);
                    index = tornlistnextpointer->TornTransactionInfo.index;
                }
            }
            else
            {
                memcpy(tmpseq, tornlistnextpointer->TornTransactionInfo.recordseqnum, 6);
                EMVBaseBcdSub(tmpseq, oldestseqnum, 6);
                if(memcmp(tmpseq, "\x10\x00\x00\x00\x00\x00", 6) > 0)
                {
                    memcpy(oldestseqnum, tornlistnextpointer->TornTransactionInfo.recordseqnum, 6);
                    index = tornlistnextpointer->TornTransactionInfo.index;
                }
                else  //oldestseqnum is oldest
                {

                }
            }


        }

        tornlistnextpointer = tornlistnextpointer->next;

    }

    return index;
}


//20140222
void sdkPaypass_torn_getnewestseqnum(u8 *newestseqnum)
{
    TORNTRANSACTIONMAPLIST *tornlistnextpointer;
    u8 firstflag = 0;
    u8 tmpseq[6];

    memset(newestseqnum, 0, 6);

    tornlistnextpointer = gPaypassTornTotalInfo.tornhead;

    Trace("emv", "gPaypassTornTotalInfo.tornhead=%p\n", gPaypassTornTotalInfo.tornhead);
    while(tornlistnextpointer != NULL)
    {
        if(firstflag == 0)
        {
            firstflag = 1;
            memcpy(newestseqnum, tornlistnextpointer->TornTransactionInfo.recordseqnum, 6);
        }
        else
        {
            if(memcmp(newestseqnum, tornlistnextpointer->TornTransactionInfo.recordseqnum, 6) > 0)
            {
                memcpy(tmpseq, newestseqnum, 6);
                EMVBaseBcdSub(tmpseq, tornlistnextpointer->TornTransactionInfo.recordseqnum, 6);
                if(memcmp(tmpseq, "\x10\x00\x00\x00\x00\x00", 6) > 0)
                {
                    memcpy(newestseqnum, tornlistnextpointer->TornTransactionInfo.recordseqnum, 6);
                }
                else  //oldestseqnum is oldest
                {

                }
            }
            else
            {
                memcpy(tmpseq, tornlistnextpointer->TornTransactionInfo.recordseqnum, 6);
                EMVBaseBcdSub(tmpseq, newestseqnum, 6);
                if(memcmp(tmpseq, "\x10\x00\x00\x00\x00\x00", 6) > 0)
                {

                }
                else  //oldestseqnum is oldest
                {
                    memcpy(newestseqnum, tornlistnextpointer->TornTransactionInfo.recordseqnum, 6);
                }
            }
        }


        tornlistnextpointer = tornlistnextpointer->next;

    }

    EMVBaseBcdAdd(newestseqnum, "\x00\x00\x00\x00\x00\x01", 6);

}


//????????????torn????
//20130503   20140222
unsigned char sdkPaypass_TornRecordsave(unsigned int index, PaypassTornTransactionLogRecord *torntransactionlogrecord)
{
    u32 addr = 0;
    u8 ret;

    memcpy(torntransactionlogrecord->flag, PAYPASS_TORNRECORDFLAG, 4);

    sdkPaypass_Crc16CCITT((u8 *)torntransactionlogrecord, PaypassTornTransactionRecordLen - 2, torntransactionlogrecord->crc);


    addr = index * PaypassTornTransactionRecordLen;


    #ifdef SXL_DEBUG
    Trace("emv", "\r\naddr=%d TornTransactionRecordLen=%d,accessfile=%d\r\n", addr, PaypassTornTransactionRecordLen, sdkPaypassAccessFile( PaypassTornTransaction));
    #endif

    if(sdkPaypassAccessFile( PaypassTornTransaction) != 0)
    {
        ret = sdkPaypassWriteFile( PaypassTornTransaction, (u8 *)torntransactionlogrecord, PaypassTornTransactionRecordLen, 0);
    }
    else
    {
        ret = sdkPaypassInsertFile( PaypassTornTransaction, (u8 *)torntransactionlogrecord, PaypassTornTransactionRecordLen, addr);
    }
    if(ret != 0)
    {
        #ifdef SXL_DEBUG
        Trace("emv", "\r\nsave torn record fail\r\n");
        #endif
        return 1;
    }

    #ifdef SXL_DEBUG
    Trace("emv", "\r\nsave torn record suc%d\r\n", index);
    #endif

    return 0;

}


//sxl 20130503      20140222
unsigned char sdkPaypass_TornRecordread(unsigned int index, PaypassTornTransactionLogRecord *torntransactionlogrecord)
{
    u32 addr;
    //u8 tmp[TornTransactionRecordLen];
    s32 ret;
    u8 checksum[2];


    #ifdef SXL_DEBUG
    Trace("emv", "\r\nTornTransactionRecordLen = %d\r\n", PaypassTornTransactionRecordLen);
    #endif


    if(torntransactionlogrecord == NULL)
    {
        return 1;
    }


    addr = index * PaypassTornTransactionRecordLen;
    ret = sdkPaypassReadFile( PaypassTornTransaction, (u8 *)torntransactionlogrecord, PaypassTornTransactionRecordLen, addr);
    if(ret !=  PaypassTornTransactionRecordLen)
    {
        #ifdef SXL_DEBUG
        Trace("emv", "\r\nsdkPaypass_TornRecordread read file err\r\n");
        #endif
        return 2;    //read end
    }



    sdkPaypass_Crc16CCITT((u8 *)torntransactionlogrecord, PaypassTornTransactionRecordLen - 2, checksum);
    if(memcmp(checksum, torntransactionlogrecord->crc, 2))
    {
        #ifdef SXL_DEBUG
        Trace("emv", "\r\nsdkPaypass_TornRecordread check sum err\r\n");
        #endif
        return 1;
    }


    #ifdef SXL_DEBUG
    Trace("emv", "\r\nsdkPaypass_TornRecordread read success\r\n");
    #endif
    return 0;
}



//20140222
void sdkPaypass_addtornlog(unsigned int AppUnionStructaddr)
{
    unsigned int i;
    PaypassTornTransactionLogRecord *torntransactionlogrecord;
    unsigned short tmplen, DRDOLLen;
    unsigned char seqnum[6];
    unsigned int index;
    int ret;
    PAYPASSTradeUnionStruct *tempApp_UnionStruct;
    unsigned char *DRDOL;    //[255]
    unsigned char MaxNumTornLog;

    tempApp_UnionStruct = (PAYPASSTradeUnionStruct *)AppUnionStructaddr;

    torntransactionlogrecord = (PaypassTornTransactionLogRecord *)emvbase_malloc(sizeof(PaypassTornTransactionLogRecord));
    memset(torntransactionlogrecord, 0, sizeof(PaypassTornTransactionLogRecord));

    emvbase_avl_gettagvalue_all(EMVTAG_PDOLData, torntransactionlogrecord->PDOLRelatedData, &tmplen);
    torntransactionlogrecord->PDOLRelatedLen = (unsigned char)tmplen;


    emvbase_avl_gettagvalue_all(EMVTAG_CDOL1, torntransactionlogrecord->CDOL1, &tmplen);
    torntransactionlogrecord->CDOL1Len = (unsigned char)tmplen;

    emvbase_avl_gettagvalue_all(EMVTAG_CDOL1RelatedData, torntransactionlogrecord->CDOL1RelatedData, &tmplen);
    torntransactionlogrecord->CDOL1RelatedLen = (unsigned char)tmplen;


    tmplen = 0;
    DRDOL = (unsigned char *)emvbase_malloc(255);
    emvbase_avl_gettagvalue_all(EMVTAG_DRDOL, DRDOL, &DRDOLLen);
    Paypass_DOLProcess(typeDRDOL, DRDOL, DRDOLLen, torntransactionlogrecord->DRDOLRelatedData, &tmplen,255, tempApp_UnionStruct);
    torntransactionlogrecord->DRDOLRelatedLen = (unsigned char)tmplen;
    emvbase_free(DRDOL);


    //???????
    emvbase_avl_gettagvalue_spec(EMVTAG_AmtAuthNum, torntransactionlogrecord->AmtAuthNum, 0, 6);
    torntransactionlogrecord->AmtOtherNumexist = !(emvbase_avl_gettagvalue_spec(EMVTAG_AmtOtherNum, torntransactionlogrecord->AmtOtherNum, 0, 6));

    emvbase_avl_gettagvalue_all(EMVTAG_PAN, torntransactionlogrecord->PAN, &tmplen);
    torntransactionlogrecord->PANLen = (unsigned char)tmplen;

    torntransactionlogrecord->PANSeqexist = !(emvbase_avl_gettagvalue_spec(EMVTAG_PANSeq, &(torntransactionlogrecord->PANSeq), 0, 1));


    torntransactionlogrecord->PayPassPreBalanceExist = !(emvbase_avl_gettagvalue_spec(EMVTAG_BalanceBeforeGAC, torntransactionlogrecord->PayPassPreBalance, 0, 6));
    if(torntransactionlogrecord->PayPassPreBalanceExist == 0)
    {
        if(emvbase_avl_gettagitempointer(EMVTAG_BalanceBeforeGAC) != NULL)
        {
            torntransactionlogrecord->PayPassPreBalanceExist = 2;
        }
    }

    emvbase_avl_gettagvalue_spec(EMVTAG_CVMResult, torntransactionlogrecord->CVMResult, 0, 3);

    torntransactionlogrecord->InterfaceDeviceSerialNumberExist = !(emvbase_avl_gettagvalue_spec(EMVTAG_IFD_SN, torntransactionlogrecord->InterfaceDeviceSerialNumber, 0, 8));

    emvbase_avl_gettagvalue_spec(EMVTAG_TermCapab, torntransactionlogrecord->TermCapab, 0, 3);

    torntransactionlogrecord->CountryCodeexist = !(emvbase_avl_gettagvalue_spec(EMVTAG_CountryCode, torntransactionlogrecord->CountryCode, 0, 2));


    torntransactionlogrecord->TermType = emvbase_avl_gettagvalue(EMVTAG_TermType);


    emvbase_avl_gettagvalue_spec(EMVTAG_TVR, torntransactionlogrecord->TVR, 0, 5);

    #ifdef EMVB_DEBUG
    Trace("emv", "\r\n a torntransactionlogrecord->TransCateCodeexist=%d\r\n", torntransactionlogrecord->TransCateCodeexist);
    Trace("emv", "\r\n torntransactionlogrecord->TransCateCode =%d\r\n", torntransactionlogrecord->TransCateCode);

    Trace("emv", "\r\n torntransactionlogrecord->TransCurcyCodeexist =%d\r\n", torntransactionlogrecord->TransCurcyCodeexist);
    Trace("emv", "\r\n torntransactionlogrecord->TransCurcyCode = %02x %02x\r\n", torntransactionlogrecord->TransCurcyCode[0], torntransactionlogrecord->TransCurcyCode[1]);
    #endif

    torntransactionlogrecord->TransCateCodeexist = !(emvbase_avl_gettagvalue_spec(EMVTAG_MCHIPTransCateCode, &(torntransactionlogrecord->TransCateCode), 0, 1));
    torntransactionlogrecord->TransCurcyCodeexist = !(emvbase_avl_gettagvalue_spec(EMVTAG_TransCurcyCode, torntransactionlogrecord->TransCurcyCode, 0, 2));

    #ifdef EMVB_DEBUG
    Trace("emv", "\r\n b torntransactionlogrecord->TransCateCodeexist=%d\r\n", torntransactionlogrecord->TransCateCodeexist);
    Trace("emv", "\r\n torntransactionlogrecord->TransCateCode =%d\r\n", torntransactionlogrecord->TransCateCode);

    Trace("emv", "\r\n torntransactionlogrecord->TransCurcyCodeexist =%d\r\n", torntransactionlogrecord->TransCurcyCodeexist);
    Trace("emv", "\r\n torntransactionlogrecord->TransCurcyCode = %02x %02x\r\n", torntransactionlogrecord->TransCurcyCode[0], torntransactionlogrecord->TransCurcyCode[1]);
    #endif

    emvbase_avl_gettagvalue_spec(EMVTAG_TransDate, torntransactionlogrecord->TransDate, 0, 3);
    emvbase_avl_gettagvalue_spec(EMVTAG_TransTime, torntransactionlogrecord->TransTime, 0, 3);
    torntransactionlogrecord->TransTypeValue = emvbase_avl_gettagvalue(EMVTAG_TransTypeValue);
    emvbase_avl_gettagvalue_spec(EMVTAG_UnpredictNum, torntransactionlogrecord->UnpredictNum, 0, 4);


    //???????,7?????????
    memcpy(&torntransactionlogrecord->transdatetime[1], torntransactionlogrecord->TransDate, 3);
    memcpy(&torntransactionlogrecord->transdatetime[4], torntransactionlogrecord->TransTime, 3);


    torntransactionlogrecord->ReferenceControlParameter = emvbase_avl_gettagvalue(EMVTAG_ReferenceControlParameter);

    #ifdef PAYPASS_DATAEXCHANGE
    //s9.ed12  S11.E14

    //s11.e11
    emvbase_avl_gettagvalue_spec(EMVTAG_IDSStatus, &torntransactionlogrecord->IDSStatus, 0, 1);

    #ifdef EMVB_DEBUG
    Trace("emv", "\r\n sdkPaypass_addtornlog torntransactionlogrecord->IDSStatus:%x\r\n", torntransactionlogrecord->IDSStatus);
    #endif

    if(torntransactionlogrecord->IDSStatus & 0x80)
    {
        EMVBASETAGCVLITEM *TagsToReaditem = emvbase_avl_gettagitempointer(EMVTAG_DSSummary1);
        if(TagsToReaditem != NULL)
        {
            if(TagsToReaditem->len)
            {
                if(TagsToReaditem->len > 16)
                {
                    torntransactionlogrecord->DSSummary1Len = 16;
                }
                else
                {
                    torntransactionlogrecord->DSSummary1Len = TagsToReaditem->len;
                }
                memcpy(torntransactionlogrecord->DSSummary1, TagsToReaditem->data, torntransactionlogrecord->DSSummary1Len);
                #ifdef EMVB_DEBUG
                Trace("emv", "\r\n sdkPaypass_addtornlog TAG_DSSummary1 len:%d\r\n", TagsToReaditem->len);
                TraceHex("emv", "data", TagsToReaditem->data, TagsToReaditem->len);
                #endif
            }
        }
    }
    #endif

    #ifdef PAYPASS_RRP

    emvbase_avl_gettagvalue_spec(EMVTAG_TermRREntropy, torntransactionlogrecord->TerminalRelayResistanceEntropy, 0, 4);
    emvbase_avl_gettagvalue_spec(EMVTAG_DeviceRREntropy, torntransactionlogrecord->DeviceRelayResistanceEntropy, 0, 4);
    emvbase_avl_gettagvalue_spec(EMVTAG_MinTimeForProcessingRR_APDU, torntransactionlogrecord->MinTimeForProcessingRelayResistanceAPDU, 0, 2);
    emvbase_avl_gettagvalue_spec(EMVTAG_MaxTimeForProcessingRR_APDU, torntransactionlogrecord->MaxTimeForProcessingRelayResistanceAPDU, 0, 2);
    emvbase_avl_gettagvalue_spec(EMVTAG_DeviceEstimatedTransmissionTimeForRR_RAPDU, torntransactionlogrecord->DeviceEstimatedTransTForRRRAPDU, 0, 2);
    emvbase_avl_gettagvalue_spec(EMVTAG_MeasureRRProcessingTime, torntransactionlogrecord->MeasureRelayResistanceProcessingTime, 0, 2);

    emvbase_avl_gettagvalue_spec(EMVTAG_RRPCounter, &torntransactionlogrecord->RRP_counter, 0, 1);

    #endif



    #ifdef TORNLOGDEBUG

    TraceHex("emv", "add torn log PDOL data", torntransactionlogrecord->PDOLRelatedData, torntransactionlogrecord->PDOLRelatedLen);

    TraceHex("emv", "add torn log CDOL data", torntransactionlogrecord->CDOL1RelatedData, torntransactionlogrecord->CDOL1RelatedLen);

    TraceHex("emv", "add torn log DDOL data", torntransactionlogrecord->DRDOLRelatedData, torntransactionlogrecord->DRDOLRelatedLen);

    #endif

    #ifdef SXL_DEBUG

    Trace("emv", "gPaypassTornTotalInfo.tornhead=%p\n", gPaypassTornTotalInfo.tornhead);

    TraceHex("emv", "gPaypassTornTotalInfo.flashusedflag", gPaypassTornTotalInfo.flashusedflag, gPaypassTornTotalInfo.TornMaxNum);
    #endif
    for(i = 0; i < gPaypassTornTotalInfo.TornMaxNum; i++)
    {
        if(gPaypassTornTotalInfo.flashusedflag[i] == 0)
        {
            break;
        }
    }


    MaxNumTornLog = emvbase_avl_gettagvalue(EMVTAG_MaxNumTornLog);
    #ifdef EMVB_DEBUG
    Trace("emv", "\r\ni = %d EMVTAG_MaxNumTornLog=%d\r\n", i, MaxNumTornLog);
    #endif


    if(i >= MaxNumTornLog)  //?????????
    {

        index = sdkPaypass_torn_getoldestseqnum();
        #ifdef TORNLOGDEBUG
        Trace("emv", "\r\nsdkPaypass_torn_getoldestseqnum add over max = %d\r\n", index);
        #endif
        if(pPaypasstorntransactionlogrecord == NULL)  //Get Mem
        {
            pPaypasstorntransactionlogrecord = (PaypassTornTransactionLogRecord *)emvbase_malloc(sizeof(PaypassTornTransactionLogRecord));

        }

        if(pPaypasstorntransactionlogrecord != NULL)
        {
            ret = sdkPaypass_TornRecordread(index, pPaypasstorntransactionlogrecord);
            if(ret != 0)
            {
                emvbase_free(pPaypasstorntransactionlogrecord);
                pPaypasstorntransactionlogrecord = NULL;
            }


            #ifdef TORNLOGDEBUG
            Trace("emv", "\r\ntorn log read = %dr\n", pPaypasstorntransactionlogrecord->PDOLRelatedLen);
            #endif

            #ifdef EMVB_DEBUG
            Trace("emv", "\r\nptorntransactionlogrecord->date = %02x %02x %02x\r\n", pPaypasstorntransactionlogrecord->TransDate[0], pPaypasstorntransactionlogrecord->TransDate[1], pPaypasstorntransactionlogrecord->TransDate[2]);
            #endif
        }

        sdkpaypass_deletespecifytornindex(index);  // ????????
        i = index;
        gPaypassTornTotalInfo.flashusedflag[index] = 0;
    }
    else
    {
        if(pPaypasstorntransactionlogrecord != NULL)
        {
            emvbase_free(pPaypasstorntransactionlogrecord);
            pPaypasstorntransactionlogrecord = NULL;
        }
    }

    //??????��??????
    sdkPaypass_torn_getnewestseqnum(seqnum);
    #ifdef SXL_DEBUG
    TraceHex("emv", "add torn log seqnum", seqnum, 6);
    #endif

    #ifdef EMVB_DEBUG
    if(pPaypasstorntransactionlogrecord != NULL)
    {
        Trace("emv", "\r\nptorntransactionlogrecord->date22 = %02x %02x %02x\r\n", pPaypasstorntransactionlogrecord->TransDate[0], pPaypasstorntransactionlogrecord->TransDate[1], pPaypasstorntransactionlogrecord->TransDate[2]);
    }
    #endif

    memcpy(torntransactionlogrecord->recordseqnum, seqnum, 6);
    ret = sdkPaypass_TornRecordsave(i, torntransactionlogrecord);
    #ifdef SXL_DEBUG
    Trace("emv", "\r\nsdkPaypass_addtornlog ret1 index ret = %d,i=%d\r\n", ret, i);
    #endif
    if(ret == 0)
    {
        //check if save success
        ret = sdkPaypass_TornRecordread(i, torntransactionlogrecord);
        #ifdef SXL_DEBUG
        Trace("emv", "\r\nsdkPaypass_addtornlog ret2 = %d\r\n", ret);
        #endif
        if(ret == 0)
        {
            gPaypassTornTotalInfo.flashusedflag[i] = 1;
            sdkpaypass_addtornmap(i, torntransactionlogrecord);

        }
    }

    #ifdef EMVB_DEBUG

    Trace("emv", "gPaypassTornTotalInfo.tornhead=%p\n", gPaypassTornTotalInfo.tornhead);
    if(pPaypasstorntransactionlogrecord != NULL)
    {
        Trace("emv", "\r\nptorntransactionlogrecord->date33 = %02x %02x %02x\r\n", pPaypasstorntransactionlogrecord->TransDate[0], pPaypasstorntransactionlogrecord->TransDate[1], pPaypasstorntransactionlogrecord->TransDate[2]);
    }
    #endif

    emvbase_free(torntransactionlogrecord);
}



//20130503   20140222
unsigned char sdkPaypassTornRecordflagcheck(u32 index)
{
    u32 addr;
    s32 ret;
    u8 flag[4];

    #ifdef SXL_DEBUG
    Trace("emv", "\r\nPaypass_TornRecordflagcheck:%d\r\n", index);
    #endif
    memset(flag, 0, sizeof(flag));
    addr = index * PaypassTornTransactionRecordLen;
    ret = sdkPaypassReadFile( PaypassTornTransaction, flag, 4, addr);

    #ifdef SXL_DEBUG
    Trace("emv", "\r\nPaypass_TornRecordflagcheck1:%d\r\n", ret);
    #endif
    if(ret != 4)  //??????log
    {
        return 2;
    }

    if(memcmp(flag, PAYPASS_TORNRECORDFLAG, 4)) //?????log???????
    {
        return 1;
    }

    return 0;

}



//sxl20130503  20140222
void sdkPaypassTornRecordDeleteAll(void)   // ??????????
{
    TORNTRANSACTIONMAPLIST *tornlistpointer;
    TORNTRANSACTIONMAPLIST *tornlistnextpointer;
    u8 index = 0;
    u8 temp[4];

    tornlistpointer = gPaypassTornTotalInfo.tornhead;

    Trace("emv", "sdkPaypassTornRecordDeleteAll,gPaypassTornTotalInfo.tornhead=%p\n", gPaypassTornTotalInfo.tornhead);
    tornlistnextpointer = tornlistpointer;

    #if 0//20191018_lhd
    if(tornlistnextpointer == NULL)
    {
        return;
    }
    #endif

    Trace("emv", "tornlistnextpointer=%p\n", tornlistnextpointer);

    while(tornlistnextpointer != NULL)
    {
        gPaypassTornTotalInfo.flashusedflag[index] = 0;
        tornlistpointer = tornlistnextpointer;
        tornlistnextpointer = tornlistnextpointer->next;
        index++;

        emvbase_free(tornlistpointer);

    }

    //delete torn file
    memset(temp, 0xff, sizeof(temp));
    sdkPaypassWriteFile( PaypassTornTransaction, (u8 *)temp, 4, 0);


    gPaypassTornTotalInfo.tornhead = NULL;

    Trace("emv", "\r\ngTornTotalInfo.TornMaxNum = %d\r\n", gPaypassTornTotalInfo.TornMaxNum);

    if (gPaypassTornTotalInfo.TornMaxNum)
    {
        memset(&gPaypassTornTotalInfo.flashusedflag[0], 0, gPaypassTornTotalInfo.TornMaxNum);
    }
}


void sdkPaypassReadTornLog(void)
{
    u32 index;
    s32 ret;
    PaypassTornTransactionLogRecord *torntransactionlogrecord;


    Trace("emv", "sdkPaypassReadTornLog,gPaypassTornTotalInfo.tornhead=%p\n", gPaypassTornTotalInfo.tornhead);

    if(pPaypasstorntransactionlogrecord != NULL)
    {
        emvbase_free(pPaypasstorntransactionlogrecord);
        pPaypasstorntransactionlogrecord = NULL;
        Trace("emv", "ABCD (pPaypasstorntransactionlogrecord != NULL)");
    }



    memset(&gPaypassTornTotalInfo, 0, sizeof(PAYPASS_TORNTRANSACTIONMAPTOTALINFO));

    gPaypassTornTotalInfo.TornMaxNum = 5;

    if(gPaypassTornTotalInfo.TornMaxNum > 5)
    {
        gPaypassTornTotalInfo.TornMaxNum = 5;  //???????
    }

    Trace("emv", "gPaypassTornTotalInfo.TornMaxNum =%d \r\n", gPaypassTornTotalInfo.TornMaxNum);


    if(gPaypassTornTotalInfo.TornMaxNum == 0)
    {
        sdkPaypassTornRecordDeleteAll();
        return;
    }

    gPaypassTornTotalInfo.flashusedflag = (u8 *)emvbase_malloc(gPaypassTornTotalInfo.TornMaxNum);
    memset(gPaypassTornTotalInfo.flashusedflag, 0, gPaypassTornTotalInfo.TornMaxNum);

    torntransactionlogrecord = (PaypassTornTransactionLogRecord *)emvbase_malloc(sizeof(PaypassTornTransactionLogRecord));

    for(index = 0 ; index < gPaypassTornTotalInfo.TornMaxNum; index++)
    {
        #ifdef SXL_DEBUG
        Trace("emv", "\r\nindex = %d\r\n", index);
        #endif
        ret = sdkPaypassTornRecordflagcheck(index);
        #ifdef SXL_DEBUG
        Trace("emv", "\r\nPaypass_TornRecordflagcheck = %d\r\n", ret);
        #endif
        if(ret == 2)
        {
            break;
        }
        else if(ret == 0)
        {
            ret = sdkPaypass_TornRecordread(index, torntransactionlogrecord);
            if(ret == 2)
            {
                break;   //???break??????
            }
            else if(ret == 0)
            {
                gPaypassTornTotalInfo.flashusedflag[index] = 1;
                sdkpaypass_addtornmap(index, torntransactionlogrecord);
            }
        }
    }
    emvbase_free(torntransactionlogrecord);
}





//  20131010
u8 sdkPaypass_tornloggreaterthanmaxlifetime(u8 *transdatetime)
{
    u8 tmptime[7];
    u8 datetime[7];
    u8 maxlifetime[7];
    u8 j;
    u32 maxlifevalue;
    unsigned char tagdata[10];//updatecondition;
    //unsigned short tagdatalen;
    #ifdef SXL_DEBUG
    // u8 i;
    #endif

    memcpy(tmptime, transdatetime, 7);
    if(tmptime[1] >= 0x50)
    {
        tmptime[0] = 0x19;
    }
    else
    {
        tmptime[0] = 0x20;
    }

    sdkPaypassGetdatetime(datetime);


    if(datetime[1] >= 0x50)
    {
        datetime[0] = 0x19;
    }
    else
    {
        datetime[0] = 0x20;
    }


    #ifdef SXL_DEBUG
    TraceHex("emv", "tmptime", tmptime, 7);
    TraceHex("emv", "datetime", datetime, 7);
    #endif

    /*
        for(i = 0;i < 7;i++)
        {
        	tmptime[i] = (tmptime[i]>>4)*10 + (tmptime[i]&0x0f);
    		datetime[i] = (datetime[i]>>4)*10 + (datetime[i]&0x0f);
        }
    	*/


    if(memcmp(datetime, tmptime, 7) > 0) //in seconds
    {
        //Paypass_getspectagdata("\xDF\x81\x1C",3,tagdata,&tagdatalen,&updatecondition);

        //if(tagdatalen != 2)
        if(emvbase_avl_gettagvalue_spec(EMVTAG_MaxLifeTimeTornLog, tagdata, 0, 2))
        {
            memcpy(tagdata, PAYPASS_TORNLOG_MAXLIFETIME_DEFAULT, 2);
        }

        maxlifevalue = tagdata[0];
        maxlifevalue = (maxlifevalue << 8) + tagdata[1];

        #ifdef SXL_DEBUG
        Trace("emv", "\\rnmaxlifevalue = %d\r\n", maxlifevalue);
        #endif
        if(maxlifevalue > 65536)
        {
            maxlifevalue = 65536;
        }
        memset(maxlifetime, 0, sizeof(maxlifetime));
        j = maxlifevalue % 60; //s
        maxlifetime[6] = ((j / 10) << 4) + (j % 10);
        maxlifevalue = maxlifevalue / 60;
        j = maxlifevalue % 60; //minute
        maxlifetime[5] = ((j / 10) << 4) + (j % 10);
        maxlifevalue = maxlifevalue / 60;
        j = maxlifevalue % 24; //hours
        maxlifetime[4] = ((j / 10) << 4) + (j % 10);
        maxlifevalue = maxlifevalue / 24;

        EMVBaseBcdAdd(tmptime, maxlifetime, 7);

        #ifdef SXL_DEBUG
        TraceHex("emv", " 7 tmptime", tmptime, 7);
        TraceHex("emv", " 7 datetime", datetime, 7);
        #endif
        if(memcmp(datetime, tmptime, 7) >= 0)
        {
            #ifdef SXL_DEBUG
            Trace("emv", "\r\nMax life time overflow\r\n");
            #endif
            return 1;
        }
    }

    #ifdef SXL_DEBUG
    Trace("emv", "\r\nMax life time not overflow\r\n");
    #endif
    return 0;
}



//20131010
void sdkpaypass_checktornloggreaterthanmaxlifetime(void)
{
    TORNTRANSACTIONMAPLIST *tornlistpointer;
    TORNTRANSACTIONMAPLIST *tornlistnextpointer;
    TORNTRANSACTIONMAPLIST *tornlistprepointer;
    unsigned char ret;
    //unsigned tornlogtosent = 0;


    #ifdef SXL_DEBUG
    //unsigned char i;
    #endif


    tornlistprepointer = NULL;
    tornlistpointer = gPaypassTornTotalInfo.tornhead;

    Trace("emv", "gPaypassTornTotalInfo.tornhead=%p\n", gPaypassTornTotalInfo.tornhead);

    if(tornlistpointer != NULL)
    {
        tornlistnextpointer = tornlistpointer->next;
    }

    sdkpaypassInitOutcomePar();

    gPaypassOutParameterSet.status = PAYPASS_OPS_STATUS_ENDAPPLICATION;

    #ifdef SXL_DEBUG
    if(tornlistprepointer != NULL)
    {
        Trace("emv", "\r\npaypass_checktornloggreaterthanmaxlifetime %d\r\n", tornlistpointer);
    }
    #endif
    while(tornlistpointer != NULL)
    {
        #ifdef SXL_DEBUG
        Trace("emv", "\r\ntornlistpointer->TornTransactionInfo = %d\r\n", tornlistpointer->TornTransactionInfo.index);
        TraceHex("emv", "TRON PAN", tornlistpointer->TornTransactionInfo.PAN, tornlistpointer->TornTransactionInfo.PANLen);
        #endif
        if(sdkPaypass_tornloggreaterthanmaxlifetime(tornlistpointer->TornTransactionInfo.transdatetime))
        {

            if(tornlistpointer == gPaypassTornTotalInfo.tornhead)
            {
                gPaypassTornTotalInfo.tornhead = tornlistnextpointer;
            }

            if(pPaypasstorntransactionlogrecord == NULL)  //Get Mem
            {
                pPaypasstorntransactionlogrecord = (PaypassTornTransactionLogRecord *)emvbase_malloc(sizeof(PaypassTornTransactionLogRecord));

            }

            #ifdef SXL_DEBUG
            Trace("emv", "\r\nget torn log record\r\n");
            #endif


            if(pPaypasstorntransactionlogrecord != NULL)
            {
                ret = sdkPaypass_TornRecordread(tornlistpointer->TornTransactionInfo.index, pPaypasstorntransactionlogrecord);
                if(ret != 0)
                {
                    emvbase_free(pPaypasstorntransactionlogrecord);
                    pPaypasstorntransactionlogrecord = NULL;
                }
            }

            //tornlogtosent = 1;

            #ifdef SXL_DEBUG
            Trace("emv", "\r\nsend torn log record\r\n");
            #endif
            #ifdef PAYPASS_OUTCOME_SUPPORT_FLAG
            sdkpaypassInitOutcomePar();
            gPaypassOutParameterSet.opssendflag = 1;
            gPaypassOutParameterSet.DisDataPresent = 1;
            gPaypassOutParameterSet.status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
            if(gPaypassOutParameterSet.opssendflag)
            {
				if ((gstPaypassTradeUnionStruct)&&(gstPaypassTradeUnionStruct->sendoutparamset))
				{
					gstPaypassTradeUnionStruct->sendoutparamset(2);
				}
                gPaypassOutParameterSet.opssendflag = 0;

                if(gPaypassOutParameterSet.DisDataPresent)
                {
					if ((gstPaypassTradeUnionStruct)&&(gstPaypassTradeUnionStruct->sendDisData))
					{
						gstPaypassTradeUnionStruct->sendDisData(0);
					}
                    gPaypassOutParameterSet.DisDataPresent = 0;
                }
            }
            #endif
            #ifdef SXL_DEBUG
            Trace("emv", "\r\ndelete torn log record\r\n");
            #endif
            sdkPaypass_TornRecorddelete(tornlistpointer->TornTransactionInfo.index);
            gPaypassTornTotalInfo.flashusedflag[tornlistpointer->TornTransactionInfo.index] = 0;
            emvbase_free(tornlistpointer);


            #ifdef SXL_DEBUG
            Trace("emv", "\r\nfree torn log record\r\n");
            #endif
            tornlistpointer = tornlistnextpointer;
            if(tornlistprepointer != NULL)
            {
                tornlistprepointer->next = tornlistnextpointer;
            }
            if(tornlistnextpointer != NULL)
            {
                tornlistnextpointer = tornlistnextpointer->next;
            }

            #ifdef SXL_DEBUG
            Trace("emv", "\r\nselect next3\r\n");
            #endif
        }
        else
        {
            #ifdef SXL_DEBUG
            Trace("emv", "\r\nselect next\r\n");
            #endif
            tornlistprepointer = tornlistpointer;
            tornlistpointer = tornlistnextpointer;
            if(tornlistnextpointer != NULL)
            {
                tornlistnextpointer = tornlistnextpointer->next;
            }
            #ifdef SXL_DEBUG
            Trace("emv", "\r\nselect next1\r\n");
            #endif
        }
    }


    sdkpaypassInitOutcomePar();
    #ifdef PAYPASS_OUTCOME_SUPPORT_FLAG
    gPaypassOutParameterSet.opssendflag = 1;
    gPaypassOutParameterSet.status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
    gPaypassOutParameterSet.DisDataPresent = 1;
    if(gPaypassOutParameterSet.opssendflag)
    {
		if ((gstPaypassTradeUnionStruct)&&(gstPaypassTradeUnionStruct->sendoutparamset))
		{
			gstPaypassTradeUnionStruct->sendoutparamset(2);
		}
        gPaypassOutParameterSet.opssendflag = 0;

        if(gPaypassOutParameterSet.DisDataPresent)
        {
			if ((gstPaypassTradeUnionStruct)&&(gstPaypassTradeUnionStruct->sendDisData))
			{
				gstPaypassTradeUnionStruct->sendDisData(1);
			}
            gPaypassOutParameterSet.DisDataPresent = 0;
        }
    }
    #endif

    //		sdkDispClearScreen();
    //       sdkDispFillRowRam(SDK_DISP_LINE1, 0, "No Torn Log To Send", SDK_DISP_DEFAULT);
    //        sdkDispBrushScreen();

}





void sdkPaypass_phonemessagetable_read(PAYPASS_PHONEMESSAGETABLEINFO *pPhoneMessageTableInfo)
{
    unsigned char i;

    pPhoneMessageTableInfo->phonemessagetablenum = 5;

    memcpy(pPhoneMessageTableInfo->phonemessagetable[0].PCIIMASK, "\x00\x08\x00", 3);
    memcpy(pPhoneMessageTableInfo->phonemessagetable[0].PCIIVALUE, "\x00\x08\x00", 3);
    memcpy(pPhoneMessageTableInfo->phonemessagetable[1].PCIIMASK, "\x00\x04\x00", 3);
    memcpy(pPhoneMessageTableInfo->phonemessagetable[1].PCIIVALUE, "\x00\x04\x00", 3);
    memcpy(pPhoneMessageTableInfo->phonemessagetable[2].PCIIMASK, "\x00\x01\x00", 3);
    memcpy(pPhoneMessageTableInfo->phonemessagetable[2].PCIIVALUE, "\x00\x01\x00", 3);
    memcpy(pPhoneMessageTableInfo->phonemessagetable[3].PCIIMASK, "\x00\x02\x00", 3);
    memcpy(pPhoneMessageTableInfo->phonemessagetable[3].PCIIVALUE, "\x00\x02\x00", 3);
    memcpy(pPhoneMessageTableInfo->phonemessagetable[4].PCIIMASK, "\x00\x00\x00", 3);
    memcpy(pPhoneMessageTableInfo->phonemessagetable[4].PCIIVALUE, "\x00\x00\x00", 3);


    for(i = 0; i < 4; i++)
    {
        pPhoneMessageTableInfo->phonemessagetable[i].MESSAGE = PAYPASS_USERREQDATA_MSGID_SEEPHONE;
        pPhoneMessageTableInfo->phonemessagetable[i].STATUS = PAYPASS_USERREQDATA_STATUS_NOTREADY;
    }
    pPhoneMessageTableInfo->phonemessagetable[4].MESSAGE = PAYPASS_USERREQDATA_MSGID_DECLINED;

    pPhoneMessageTableInfo->phonemessagetable[4].STATUS = PAYPASS_USERREQDATA_STATUS_NOTREADY;

}





void sdkPaypassIntToByteArray(unsigned long var, unsigned char *buf, unsigned char bufLen)
{
    int i;
    unsigned long temp;
    temp = var;
    for(i = 0; i < bufLen; i++)
    {
        buf[bufLen - 1 - i] = temp % 256;
        temp = temp / 256;
    }
}


/*****************************************************************************
** Descriptions:  ????TAG 9F04???
** Parameters:
** Returned value:
** Created By:luohuidong
** Remarks:
****************************************************************************
void sdkPaypassImportOtherAmount(const u8 bcdAmount[6])
{
    u32 tmpsdkAmtAuthBin = 0;
    u8 tmpAmtAuthBin[4] = {0};

    sdkBcdToU32(&tmpsdkAmtAuthBin, bcdAmount, 6);
    sdkPaypassIntToByteArray(tmpsdkAmtAuthBin, tmpAmtAuthBin, 4);

    emvbase_avl_createsettagvalue(EMVTAG_AmtOtherBin, tmpAmtAuthBin, 4);
}
*/

void sdkPaypassGetRandom(unsigned char *randpad,  int randLen)
{
    sdkGetRandom(randpad, randLen);
}


void sdkPaypassCreateUN(void)
{
    u8 i;
    u8 data[8];
    u8 UnpredictNum[4];

    while(1)
    {
        sdkGetRandom(UnpredictNum, 4);
        for(i = 0; i < 4; i++)
        {
            if(UnpredictNum == 0)
            {
                continue;
            }
        }
        sdkBcdToAsc(data, UnpredictNum, 4);
        for(i = 0; i < 8; i++)
        {
            if(data[i] >= 'A')
            {
                data[i] = data[i] - 'A' + 0x30;
            }
        }
        sdkAscToBcd(UnpredictNum, data, 8);

        TraceHex("emv", "AAA TAG_paypassun ccc \n", UnpredictNum, 4);

        emvbase_avl_createsettagvalue(EMVTAG_paypassun, UnpredictNum, 4);

        break;
    }
}

void sdkPaypassGetdatetime(u8 *datetime)
{
    u8 temp[128] = {0};
    memset(temp, 0, sizeof(temp));
    sdkGetRtc(temp);

    if(temp[0] >= 0x50)
    {
        datetime[0] = 0x19;
    }
    else
    {
        datetime[0] = 0x20;
    }
    memcpy(&datetime[1], temp, 6);
    TraceHex("emv", "temp", temp, 8);
}


static u8 paypassBIsSupportDE = 0;
static u8 paypassBIsExsitRecvCMDLog = 0;
static u8 paypassBIsDoNotSaveCurRecvCMDLog = 0;

#ifdef PAYPASS_DISP_DEK_AND_CMD

    static u8 paypassBIsSaveDekExchangeApduLog = 0;

    unsigned char gNeedSaveWritebuf[64];
    unsigned char gNeedSavewritebuflen;
    unsigned char gNeedSaveEMVRxBuf[300];
    unsigned short gNeedSaveEMVRxLen;
	unsigned char gNeedSaveWritebufCAPDU[64];
    unsigned char gNeedSavewritebuflenCAPDU;
    unsigned char gNeedSaveEMVRxBufCAPDU[300];
    unsigned short gNeedSaveEMVRxLenCAPDU;
#endif


static struct _DEKData *gstDEKIndex = NULL; //[DEKDETINDEXNUM];

void sdkPayPassSetSupportDE(unsigned char mode)//0:not support ,1:support
{
    paypassBIsSupportDE = mode;
}
unsigned char sdkPayPassGetSupportDEstatus(void)//0:not support ,1:support
{
    return paypassBIsSupportDE;
}

s32 sdkPayPassManageInitdekdet(void)
{
    int ret1 = 0, ret2 = 0;

    if( NULL == gstDEKIndex )
    {
        gstDEKIndex = (struct _DEKData *)emvbase_malloc(DEKDETINDEXNUM * sizeof(struct _DEKData) + 1);
        if( NULL == gstDEKIndex )
        {

            #ifdef SXL_DEBUG
            Trace("emv", "\r\n sdkPaypassManageinitdekdet malloc err \r\n");
            #endif

            //            while(1);

            return SDK_ERR;

        }
    }
    else
    {
        #ifdef SXL_DEBUG
        Trace("emv", "\r\n sdkPaypassManageinitdekdet malloc if( NULL != gstDEKIndex ) !! \r\n");
        #endif
    }

    memset((unsigned char *)gstDEKIndex, 0, (DEKDETINDEXNUM * sizeof(struct _DEKData)));

    ret1 = sdkPaypassReadFile(DETFILE, (u8 *)gstDEKIndex, (DEKDETINDEXNUM * sizeof(struct _DEKData)), 0);

    if(!(ret1 > 0))
    {
        ret2 = sdkPaypassReadFile(DEKFILE, (u8 *)gstDEKIndex, (DEKDETINDEXNUM * sizeof(struct _DEKData)), 0);
    }

    #ifdef SXL_DEBUG
    Trace("emv", "\r\n sdkPaypassManageinitdekdet ret1 =%d ret2 =%d\r\n", ret1, ret2);
    #endif

    if(!(ret1 > 0))
    {
        return ret2;
    }
    else
    {
        return ret1;
    }
}



#define CAPKCMD_ADD     0x11
#define CAPKCMD_DEL     0x21
#define CAPKCMD_DELALL  0x22
#define CAPKCMD_PAYPASSAIDDEL  0x77
#define CAPKCMD_PAYPASSAIDADD  0x78



s32 sdkPayPassManageSetdekdetParam(unsigned char *dekdetdata, unsigned int dekdetdatalen)
{
    u32 len, needwriteLen = 0;
    s32 ret;
    u8 operatetype;
    //u8 type;
    s32 i, j;
    u8 cur_DEKDET_TYPE = 0;
    u8 cur_DEKDET_NUMLIST = 0;
    u8 cur_DEKDET_SUBNUMLIST = 0;
    u8 cur_DEKDET_TOTALPACKET = 0;
    u8 cur_DEKDET_SUBPACKET = 0;
    u32 tempLen = 0, uioffsetLena = 0;


    if (NULL == gstDEKIndex)
    {
        #ifdef SXL_DEBUG
        Trace("emv", "\r\n (NULL == gstDEKIndex) %p \r\n", gstDEKIndex);
        #endif
        return SDK_ERR;
    }

    len = 0;
    operatetype = dekdetdata[len++];

    #ifdef SXL_DEBUG
    Trace("emv", "\r\n operatetype=0x%x\r\n", operatetype);
    #endif

    if(operatetype == CAPKCMD_ADD || operatetype == CAPKCMD_PAYPASSAIDADD)
    {
        cur_DEKDET_TYPE		 = dekdetdata[len++];
        cur_DEKDET_NUMLIST 	 = dekdetdata[len++];
        cur_DEKDET_SUBNUMLIST	 = dekdetdata[len++];
        cur_DEKDET_TOTALPACKET  = dekdetdata[len++];
        cur_DEKDET_SUBPACKET	 = dekdetdata[len++];

        #ifdef SXL_DEBUG
        Trace("emv", "\r\n cur_DEKDET_TYPE=%d\r\n", cur_DEKDET_TYPE);
        Trace("emv", "\r\n cur_DEKDET_NUMLIST=%d\r\n", cur_DEKDET_NUMLIST);
        Trace("emv", "\r\n cur_DEKDET_SUBNUMLIST=%d\r\n", cur_DEKDET_SUBNUMLIST);
        Trace("emv", "\r\n cur_DEKDET_TOTALPACKET=%d\r\n", cur_DEKDET_TOTALPACKET);
        Trace("emv", "\r\n cur_DEKDET_SUBPACKET=%d\r\n", cur_DEKDET_SUBPACKET);
        Trace("emv", "\r\n dekdetdatalen=%d\r\n", (dekdetdatalen - len));
        TraceHex("emv", "dekdetdata:", dekdetdata + len, (dekdetdatalen - len));
        #endif

        if(cur_DEKDET_NUMLIST <= 0 || cur_DEKDET_NUMLIST > DEKDETINDEXNUM
                || cur_DEKDET_TOTALPACKET <= 0 || cur_DEKDET_TOTALPACKET > DEKDETINDEXNUM  || cur_DEKDET_SUBNUMLIST > DEKSUBDETMAXNUM)
        {
            return SDK_ERR;
        }

        cur_DEKDET_NUMLIST -= 1;
        if( DEK_TYPE == cur_DEKDET_TYPE )
        {
            #if 1 /*Modify by luohuidong at 2017.04.09  16:8 */
            if(sdkPaypassAccessFile(DEKFILE) != 0)
            {
                #ifdef SXL_DEBUG
                Trace("emv", "\r\n (sdkPaypassAccessFile(DEKFILE) != 0) operatetype=%d\r\n", operatetype);
                #endif
                ret = sdkPaypassCreateFile(DEKFILE, (DEKDETINDEXNUM * sizeof(struct _DEKData)), 0xff);
                if(ret != 0)
                {
                    #ifdef SXL_DEBUG
                    Trace("emv", "\r\n sdkPaypassCreateFile(DEKFILE, fall !!!!!!!! ret=%d\r\n", ret);
                    #endif
                    return SDK_ERR;
                }
            }
            #endif /* if 0 */

            needwriteLen = dekdetdatalen - len;
            if( 0 == gstDEKIndex[cur_DEKDET_NUMLIST].bIsexist )//??|?C????-?????DEK ?????o|
            {
                gstDEKIndex[cur_DEKDET_NUMLIST].bIsexist = 0x01;
                gstDEKIndex[cur_DEKDET_NUMLIST].usDekLen = needwriteLen;
            }
            else
            {
                gstDEKIndex[cur_DEKDET_NUMLIST].usDekLen += needwriteLen;
            }

            if(0 == cur_DEKDET_NUMLIST)//??|?C????-?????dek ???????
            {
                gstDEKIndex[cur_DEKDET_NUMLIST].uiDekOffset = (DEKDETINDEXNUM * sizeof(struct _DEKData));
            }
            else
            {
                tempLen = 0;
                for(i = 0; i < cur_DEKDET_NUMLIST; i++)
                {
                    tempLen += gstDEKIndex[i].usDekLen;
                    #ifdef SXL_DEBUG
                    Trace("emv", "\r\n gstDEKIndex[%d].usDekLen=%d tempLen=%d\r\n", i, gstDEKIndex[i].usDekLen, tempLen);
                    #endif
                }
                gstDEKIndex[cur_DEKDET_NUMLIST].uiDekOffset = (DEKDETINDEXNUM * sizeof(struct _DEKData)) + tempLen;
            }
            #ifdef SXL_DEBUG
            Trace("emv", "\r\n gstDEKIndex[%d].uiDekOffset=%d sizeof(gstDEKIndex)=%d\r\n", cur_DEKDET_NUMLIST, gstDEKIndex[cur_DEKDET_NUMLIST].uiDekOffset, (DEKDETINDEXNUM * sizeof(struct _DEKData)));
            #endif


            //????-????????
            ret = sdkPaypassInsertFile(DEKFILE, (u8 *)gstDEKIndex, (DEKDETINDEXNUM * sizeof(struct _DEKData)), 0);
            if(ret == 0)
            {
                #ifdef SXL_DEBUG
                Trace("emv", "\r\n save DEK gstDEKIndex suceess\r\n");
                #endif
            }
            else
            {
                #ifdef SXL_DEBUG
                Trace("emv", "\r\n save DEK gstDEKIndex fail\r\n");
                #endif
                return SDK_ERR;
            }

            //????-???????1
            ret = 0;
            if(cur_DEKDET_TOTALPACKET == 0x01)
            {
                if(cur_DEKDET_TOTALPACKET == cur_DEKDET_SUBPACKET)//??????????????????????????????????-??
                {
                    //ret = sdkPaypassInsertFile(DEKFILE,&dekdetdata[len],needwriteLen,gstDEKIndex[cur_DEKDET_NUMLIST].uiDekOffset);
                    uioffsetLena = gstDEKIndex[cur_DEKDET_NUMLIST].uiDekOffset;
                }
                else
                {
                    ret = 2;
                }
            }
            else
            {
                if( 0x01 == cur_DEKDET_SUBPACKET )//????????   ??|?C?????????????????
                {
                    //ret = sdkPaypassInsertFile(DEKFILE,&dekdetdata[len],needwriteLen,gstDEKIndex[cur_DEKDET_NUMLIST].uiDekOffset);
                    uioffsetLena = gstDEKIndex[cur_DEKDET_NUMLIST].uiDekOffset;
                }
                else
                {
                    uioffsetLena = (gstDEKIndex[cur_DEKDET_NUMLIST].uiDekOffset + gstDEKIndex[cur_DEKDET_NUMLIST].usDekLen - needwriteLen);
                    //ret = kern_append_file(DEKFILE,&dekdetdata[len],needwriteLen);
                    //ret = sdkPaypassInsertFile(DEKFILE,&dekdetdata[len],needwriteLen,uioffsetLena);
                }
            }
            if(2 != ret)
            {
                #ifdef SXL_DEBUG
                Trace("emv", "\r\n DEK_TYPE uiDekOffset	=%d \r\n", gstDEKIndex[cur_DEKDET_NUMLIST].uiDekOffset);
                Trace("emv", "\r\n DEK_TYPE cur Offset	=%d \r\n", uioffsetLena);
                Trace("emv", "\r\n DEK_TYPE total len	=%d \r\n", gstDEKIndex[cur_DEKDET_NUMLIST].usDekLen);
                Trace("emv", "\r\n DEK_TYPE cur write len=%d \r\n", needwriteLen);
                #endif
                ret = sdkPaypassInsertFile(DEKFILE, &dekdetdata[len], needwriteLen, uioffsetLena);
            }
            if(ret == 0)
            {
                #ifdef SXL_DEBUG
                Trace("emv", "\r\n save DEKFILE	suceess\r\n");
                #endif
            }
            else
            {
                #ifdef SXL_DEBUG
                Trace("emv", "\r\n DEK_TYPE save DEKFILE fail ret=%d\r\n", ret);
                #endif
                gstDEKIndex[cur_DEKDET_NUMLIST].bIsexist = 0x00;//??C????????????????o?  ??????????????
                ret = sdkPaypassInsertFile(DEKFILE, (u8 *)gstDEKIndex, (DEKDETINDEXNUM * sizeof(struct _DEKData)), 0);
                #ifdef SXL_DEBUG
                Trace("emv", "\r\n DEK_TYPE save DEKDET fail 222 ret=%d\r\n", ret);
                #endif
                return SDK_ERR;
            }
        }
        else if(( DET_TYPE == cur_DEKDET_TYPE ))
        {

            #if 1 /*Modify by luohuidong at 2017.04.09  16:8 */
            if(sdkPaypassAccessFile(DETFILE) != 0)
            {
                #ifdef SXL_DEBUG
                Trace("emv", "\r\n (sdkPaypassAccessFile(DETFILE) != 0) operatetype=%d\r\n", operatetype);
                #endif
                ret = sdkPaypassCreateFile(DETFILE, (DEKDETINDEXNUM * sizeof(struct _DEKData)), 0xff);
                if(ret != 0)
                {
                    #ifdef SXL_DEBUG
                    Trace("emv", "\r\n sdkPaypassCreateFile(DETFILE, fall !!!!!!!! ret=%d\r\n", ret);
                    #endif
                    return SDK_ERR;
                }
            }
            #endif /* if 0 */


            if( 0 == gstDEKIndex[cur_DEKDET_NUMLIST].bIsexist )//??|?C????-?????DEK ?????o|
            {
                #ifdef SXL_DEBUG
                Trace("emv", "\r\n input DET_TYPE  fail because the dek is empty ,so det don't save \r\n");
                #endif
                return SDK_ERR;
            }
            cur_DEKDET_SUBNUMLIST -= 1;
            needwriteLen = dekdetdatalen - len;

            if( 0 == gstDEKIndex[cur_DEKDET_NUMLIST].detdata[cur_DEKDET_SUBNUMLIST].bIsexist )//??|?C????-?????DEK ?????o|
            {
                gstDEKIndex[cur_DEKDET_NUMLIST].detdata[cur_DEKDET_SUBNUMLIST].bIsexist = 0x01;
                gstDEKIndex[cur_DEKDET_NUMLIST].detdata[cur_DEKDET_SUBNUMLIST].usDetLen = needwriteLen;
            }
            else
            {
                gstDEKIndex[cur_DEKDET_NUMLIST].detdata[cur_DEKDET_SUBNUMLIST].usDetLen += needwriteLen;
            }
            #ifdef SXL_DEBUG
            Trace("emv", "\r\n DET gstDEKIndex[%d].detdata[%d].bIsexist=%d \r\n", cur_DEKDET_NUMLIST, cur_DEKDET_SUBNUMLIST, gstDEKIndex[cur_DEKDET_NUMLIST].detdata[cur_DEKDET_SUBNUMLIST].bIsexist);
            #endif
            if(0 == cur_DEKDET_SUBNUMLIST)//??|?C????-?????det ???????
            {
                gstDEKIndex[cur_DEKDET_NUMLIST].detdata[cur_DEKDET_SUBNUMLIST].uiDetOffset = (DEKDETINDEXNUM * sizeof(struct _DEKData));
                #ifdef SXL_DEBUG
                Trace("emv", "\r\n 00 DET_TYPE save det usDetLen=%d\r\n", gstDEKIndex[cur_DEKDET_NUMLIST].detdata[cur_DEKDET_SUBNUMLIST].uiDetOffset);
                #endif
            }
            else
            {
                tempLen = 0;
                for(i = 0; i < cur_DEKDET_SUBNUMLIST; i++)
                {
                    tempLen += gstDEKIndex[cur_DEKDET_NUMLIST].detdata[i].usDetLen;
                    #ifdef SXL_DEBUG
                    Trace("emv", "\r\n 11 DET_TYPE save det gstDEKIndex[%d].detdata[%d].usDetLen=%d\r\n", cur_DEKDET_NUMLIST, i, gstDEKIndex[cur_DEKDET_NUMLIST].detdata[i].usDetLen);
                    #endif
                }
                gstDEKIndex[cur_DEKDET_NUMLIST].detdata[cur_DEKDET_SUBNUMLIST].uiDetOffset = (DEKDETINDEXNUM * sizeof(struct _DEKData)) + tempLen;
            }
            //???????????????DEK?o??????????????DET
            if(cur_DEKDET_NUMLIST > 0)
            {
                tempLen = 0;
                for(j = 0; j < cur_DEKDET_NUMLIST; j++)
                {
                    //for(i=0;i<= cur_DEKDET_SUBNUMLIST;i++)
                    for(i = 0; i < DEKSUBDETMAXNUM; i++)
                    {
                        if(0 == gstDEKIndex[j].detdata[i].usDetLen)
                        {
                            break;
                        }
                        tempLen += gstDEKIndex[j].detdata[i].usDetLen;
                        #ifdef SXL_DEBUG
                        Trace("emv", "\r\n 22 DET_TYPE save det gstDEKIndex[%d].detdata[%d].usDetLen=%d\r\n", j, i, gstDEKIndex[j].detdata[i].usDetLen);
                        #endif
                    }
                }
                gstDEKIndex[cur_DEKDET_NUMLIST].detdata[cur_DEKDET_SUBNUMLIST].uiDetOffset += tempLen;
            }

            //????-????????
            ret = sdkPaypassInsertFile(DETFILE, (u8 *)gstDEKIndex, (DEKDETINDEXNUM * sizeof(struct _DEKData)), 0);
            if(ret == 0)
            {
                #ifdef SXL_DEBUG
                Trace("emv", "\r\n DET_TYPE save gstDEKIndex suceess\r\n");
                #endif
            }
            else
            {
                #ifdef SXL_DEBUG
                Trace("emv", "\r\n DET_TYPE save gstDEKIndex fail\r\n");
                #endif
                return SDK_ERR;
            }


            //????-???????1
            ret = 0;
            if(cur_DEKDET_TOTALPACKET == 0x01)
            {
                if(cur_DEKDET_TOTALPACKET == cur_DEKDET_SUBPACKET)//??????????????????????????????????-??
                {
                    //ret = sdkPaypassInsertFile(DETFILE,&dekdetdata[len],needwriteLen,gstDEKIndex[cur_DEKDET_NUMLIST].detdata[cur_DEKDET_SUBNUMLIST].uiDekOffset);
                    uioffsetLena = gstDEKIndex[cur_DEKDET_NUMLIST].detdata[cur_DEKDET_SUBNUMLIST].uiDetOffset;
                }
                else
                {
                    ret = 2;
                }
            }
            else
            {
                if( 0x01 == cur_DEKDET_SUBPACKET )//????????   ??|?C?????????????????
                {
                    //ret = sdkPaypassInsertFile(DETFILE,&dekdetdata[len],needwriteLen,gstDEKIndex[cur_DEKDET_NUMLIST].detdata[cur_DEKDET_SUBNUMLIST].uiDekOffset);
                    uioffsetLena = gstDEKIndex[cur_DEKDET_NUMLIST].detdata[cur_DEKDET_SUBNUMLIST].uiDetOffset;
                }
                else
                {
                    //ret = kern_append_file(DETFILE,&dekdetdata[len],needwriteLen);
                    uioffsetLena = gstDEKIndex[cur_DEKDET_NUMLIST].detdata[cur_DEKDET_SUBNUMLIST].uiDetOffset + gstDEKIndex[cur_DEKDET_NUMLIST].detdata[cur_DEKDET_SUBNUMLIST].usDetLen - needwriteLen;
                }
            }

            if(2 != ret)
            {
                #ifdef SXL_DEBUG
                Trace("emv", "\r\n DET_TYPE uiDekOffset	=%d \r\n", gstDEKIndex[cur_DEKDET_NUMLIST].detdata[cur_DEKDET_SUBNUMLIST].uiDetOffset);
                Trace("emv", "\r\n DET_TYPE cur Offset	=%d \r\n", uioffsetLena);
                Trace("emv", "\r\n DET_TYPE total len	=%d \r\n", gstDEKIndex[cur_DEKDET_NUMLIST].detdata[cur_DEKDET_SUBNUMLIST].usDetLen);
                Trace("emv", "\r\n DET_TYPE cur write len=%d \r\n", needwriteLen);
                #endif
                ret = sdkPaypassInsertFile(DETFILE, &dekdetdata[len], needwriteLen, uioffsetLena);
            }

            if(ret == 0)
            {
                #ifdef SXL_DEBUG
                Trace("emv", "\r\n DET_TYPE save DEKDET suceess\r\n");
                #endif
            }
            else
            {
                #ifdef SXL_DEBUG
                Trace("emv", "\r\n DET_TYPE save DEKDET fail ret=%d\r\n", ret);
                #endif
                gstDEKIndex[cur_DEKDET_NUMLIST].detdata[cur_DEKDET_SUBNUMLIST].bIsexist = 0x00;//??C????????????????o?  ??????????????
                ret = sdkPaypassInsertFile(DETFILE, (u8 *)gstDEKIndex, (DEKDETINDEXNUM * sizeof(struct _DEKData)), 0);
                #ifdef SXL_DEBUG
                Trace("emv", "\r\n DET_TYPE save DEKDET fail 333 ret=%d\r\n", ret);
                #endif
                return SDK_ERR;
            }
        }

        return SDK_OK;

    }
    else if((operatetype == CAPKCMD_DEL || operatetype == CAPKCMD_PAYPASSAIDDEL)\
            || (operatetype == CAPKCMD_DELALL || operatetype == 0x23))
    {
        memset(gstDEKIndex, 0, (DEKDETINDEXNUM * sizeof(struct _DEKData)) );
        if(0 == sdkPaypassAccessFile(DEKFILE))
        {
            ret = sdkPaypassDelFile(DEKFILE);
            if(ret == 1)
            {
                #ifdef SXL_DEBUG
                Trace("emv", "\r\n delete gstDEKIndex DEKFILE suceess\r\n");
                #endif
                //return 0;
            }
            else
            {
                #ifdef SXL_DEBUG
                Trace("emv", "\r\n delete gstDEKIndex DEKFILE fail ret=%d -----------  \r\n", ret);
                #endif
                return SDK_ERR;
            }
        }
        else
        {
            #ifdef SXL_DEBUG
            Trace("emv", "\r\n delete gstDEKIndex DEKFILE fail sdkPaypassAccessFile(DEKFILE)=%d -----------  \r\n", sdkPaypassAccessFile(DEKFILE));
            #endif
        }

        if(0 == sdkPaypassAccessFile(DETFILE))
        {
            ret = sdkPaypassDelFile(DETFILE);
            if(ret == 1)
            {
                #ifdef SXL_DEBUG
                Trace("emv", "\r\n delete gstDEKIndex DETFILE suceess11\r\n");
                #endif
                return SDK_OK;
            }
            else
            {
                #ifdef SXL_DEBUG
                Trace("emv", "\r\n delete gstDEKIndex DETFILE fail ret=%d -----111------  \r\n", ret);
                #endif
                return SDK_ERR;
            }
        }
        else
        {

            #ifdef SXL_DEBUG
            Trace("emv", "\r\n delete gstDEKIndex DEKFILE fail sdkPaypassAccessFile(DETFILE)=%d -----------  \r\n", sdkPaypassAccessFile(DETFILE));
            #endif
            return SDK_OK;
        }
    }
    else
    {
        return SDK_OK;
    }

    return SDK_ERR;

}

#define GETDEKDET_FILE_CONTENT_MAXLEN	(970)
s32 sdkPayPassManageGetDE_Apdu_LOG(unsigned char *dekdetdata, int *dekdetdatalen)
{

    s32 ret;
    unsigned int totalLen = 0, offset = 0, needreadlen = 0, bufoffer = 0, addroffer = 0;
    unsigned char *buf = NULL; //ǰ��Ԥ��4���ֽ�: //1���ֽ� �ܰ���  1���ֽڵ�ǰ����  2���ֽڵ�ǰ���ݳ���  nbyte��������
    //nbyte�������� ��ԭ���Ĵ�ascii���?> 1�ֽ�����(0x00:ascii,0x01:hex)+ 2�ֽڳ��� + nbyte���� , 1�ֽ�����(0x00:ascii,0x01:hex)+ 2�ֽڳ��� + nbyte���� ����ظ�������?

    unsigned char	bufForLen[24] = {0};
    int curlen = 0;
    memset(bufForLen, 0, sizeof(bufForLen));

    *dekdetdatalen = 0;
    if(0 != sdkPaypassAccessFile(DETDEKEXCHANGELOG))
    {
        *dekdetdatalen = 4;
        memcpy(dekdetdata, "\x00\x00\x00\x00", *dekdetdatalen);
        return SDK_ERR;
    }

    ret = sdkPaypassReadFile(DETDEKEXCHANGELOG, bufForLen, 12, 0);

    #ifdef EMVB_DEBUG
    Trace("emv", "\r\n read buflen ret = %d\r\n", ret);
    TraceHex("emv", "read buf:", bufForLen, 12);
    #endif

    bufoffer = 4;
    addroffer = 12;


    totalLen = bufForLen[0];
    totalLen <<= 8;
    totalLen |= bufForLen[1];
    totalLen <<= 8;
    totalLen |= bufForLen[2];
    totalLen <<= 8;
    totalLen |= bufForLen[3];

    totalLen -= addroffer;


    offset = bufForLen[4];
    offset <<= 8;
    offset |= bufForLen[5];
    offset <<= 8;
    offset |= bufForLen[6];
    offset <<= 8;
    offset |= bufForLen[7];

    curlen = totalLen - offset;


    #ifdef EMVB_DEBUG
    Trace("emv", "\r\n totalLen=%d offset = %d curlen=%d\r\n", totalLen, offset, curlen);
    #endif

    if(curlen > 0 )
    {

        buf = (unsigned char *)emvbase_malloc(1024);
        memset(buf, 0, (1024));
        if( curlen > GETDEKDET_FILE_CONTENT_MAXLEN )
        {
            needreadlen = GETDEKDET_FILE_CONTENT_MAXLEN;

            ret = sdkPaypassReadFile(DETDEKEXCHANGELOG, buf + bufoffer, needreadlen, offset + addroffer);

            #ifdef EMVB_DEBUG
            Trace("emv", "\r\n read buflen 1 ret = %d\r\n", ret);
            #endif

            offset += GETDEKDET_FILE_CONTENT_MAXLEN;

            bufForLen[4] = (unsigned char)(offset >> 24);
            bufForLen[5] = (unsigned char)(offset >> 16);
            bufForLen[6] = (unsigned char)(offset >> 8);
            bufForLen[7] = (unsigned char)(offset);

            #ifdef EMVB_DEBUG
            Trace("emv", "\r\n bufForLen = %02x %02x %02x %02x \r\n", bufForLen[4], bufForLen[5], bufForLen[6], bufForLen[7]);
            #endif

            ret = sdkPaypassInsertFile(DETDEKEXCHANGELOG, bufForLen, 12, 0);
            #ifdef EMVB_DEBUG
            Trace("emv", "\r\n read buflen 2 ret = %d\r\n", ret);
            #endif

        }
        else
        {
            needreadlen = curlen;
            ret = sdkPaypassReadFile(DETDEKEXCHANGELOG, buf + bufoffer, needreadlen, offset + addroffer);
            #ifdef EMVB_DEBUG
            Trace("emv", "\r\n read buflen 3 ret = %d\r\n", ret);
            #endif
            ret = sdkPaypassDelFile(DETDEKEXCHANGELOG);
            #ifdef EMVB_DEBUG
            Trace("emv", "\r\n read buflen 4 ret = %d\r\n", ret);
            #endif
        }

        buf[2] = (unsigned char)(needreadlen >> 8);
        buf[3] = (unsigned char)(needreadlen);

        needreadlen += bufoffer;

        *dekdetdatalen = needreadlen;
        memcpy(dekdetdata, buf, *dekdetdatalen);

        emvbase_free(buf);
        return SDK_OK;

    }
    else
    {
        *dekdetdatalen = 4;
        memcpy(dekdetdata, "\x00\x00\x00\x00", *dekdetdatalen);
        return SDK_ERR;
    }

}


int sdkPaypassManageGetDekTotalNum(void)
{
    int i = 0, dektotalnum = 0;

    dektotalnum = 0;

    if (NULL == gstDEKIndex)
    {
        return -1;
    }

    for(i = 0; i < DEKDETINDEXNUM; i++)
    {
        if(gstDEKIndex[i].bIsexist)
        {
            dektotalnum++;
        }
    }

    #ifdef SXL_DEBUG
    Trace("emv", "\r\n sdkPaypassManageGetDekTotalNum dektotalnum=%d\r\n", dektotalnum);
    #endif

    return dektotalnum;
}

int  sdkPaypassManageBisSupportDe(void)
{
    int ret = 0;

    if (0 == sdkPayPassGetSupportDEstatus())
    {
        return 0;
    }

    ret = sdkPaypassManageGetDekTotalNum();
    if(0 >= ret)
    {
        return 0;
    }

    return 1;

}

s32 sdkPaypassManageGetDeksSubDetTotalNum(struct _DEKData *dek)
{
    int i = 0, dettotalnum = 0;

    dettotalnum = 0;

    for(i = 0; i < DEKSUBDETMAXNUM; i++)
    {
        if(dek->detdata[i].bIsexist)
        {
            dettotalnum++;
        }
    }

    #ifdef SXL_DEBUG
    Trace("emv", "\r\n sdkPaypassManageGetDeksSubDetTotalNum dettotalnum=%d\r\n", dettotalnum);
    #endif

    return dettotalnum;
}

s32 sdkPaypassManagereadDek(u8 index, u8 *data, u32 *dataLen)
{
    s32 ret = 0;
    u32 readlen = 0;

    if (NULL == gstDEKIndex)
    {
        return -1;
    }


    if(0 == gstDEKIndex[index].bIsexist)
    {
        return -1;
    }
    if(gstDEKIndex[index].usDekLen > XML_FILE_DEK_LEN)
    {
        readlen = XML_FILE_DEK_LEN;
    }
    else
    {
        readlen = gstDEKIndex[index].usDekLen;
    }
    ret = sdkPaypassReadFile(DEKFILE, data, readlen, gstDEKIndex[index].uiDekOffset);
    #ifdef SXL_DEBUG
    //Trace("emv","\r\n sdkPaypassManagereadDek uiDekOffset=%d readlen=%d want readlen=%d \r\n",gstDEKIndex[index].uiDekOffset,ret,gstDEKIndex[index].usDekLen);
    #endif
    if(0 == ret)
    {
        *dataLen = 0;
    }
    else
    {
        *dataLen = gstDEKIndex[index].usDekLen;
    }
    return ret;
}

//retrurn value:  read data len
s32 sdkPaypassManagereadDet(struct _DEKData *dek, u8 detIndex, u8 *data, u32 *dataLen)
{
    s32 ret = 0;
    u32 readlen = 0;

    if(NULL == dek)
    {
        return -1;
    }

    if(0 == dek->detdata[detIndex].bIsexist)
    {
        return -1;
    }

    if(dek->detdata[detIndex].usDetLen > XML_FILE_DET_LEN)
    {
        readlen = XML_FILE_DET_LEN;
    }
    else
    {
        readlen = dek->detdata[detIndex].usDetLen;
    }

    ret = sdkPaypassReadFile(DETFILE, data, readlen, dek->detdata[detIndex].uiDetOffset);
    #ifdef SXL_DEBUG
    //Trace("emv","\r\n sdkPaypassManagereadDet uiDetOffset=%d readlen=%d want readlen=%d \r\n",dek->detdata[detIndex].uiDetOffset,ret,readlen);
    #endif
    if(0 == ret)
    {
        *dataLen = 0;
    }
    else
    {
        *dataLen = readlen;
    }
    return ret;
}

//match return 1,not match return 0
s32 sdkPaypassManageCompareddata(const u8 *data1, const u8 *data2, const u32 cmpLen)
{
    int i = 0;

    if( ( 0 >= cmpLen ) || ( NULL == data1 ) || ( NULL == data2 ) )
    {
        #ifdef SXL_DEBUG
        Trace("emv", "\r\n sdkPaypassManageCompareddata INPUT DATA fail %d %p %p \r\n", cmpLen, data1, data2);
        #endif
        return -1;
    }

    for(i = 0; i < cmpLen; i++)
    {

        if(data1[i] != data2[i])
        {
            return 0;
        }
    }
    return 1;
}

//return index or -1
s32 sdkPaypassManageIfMatchDek(u8 startSearchNum, u8 *dekdetdata, u32 dekdetdatalen)
{
    s32 i = 0, j = 0, dektotalnum = 0;
    u8 *temp = NULL;
    u32 tempLen = 0;
    s32 ret = 0;

    dektotalnum = sdkPaypassManageGetDekTotalNum();
    if( 0 >= dektotalnum )
    {
        #ifdef SXL_DEBUG
        Trace("emv", "\r\n sdkPaypassManageIfMatchDek dek total num is empty\r\n");
        #endif
        return -1;
    }

    temp = (u8 *)emvbase_malloc(XML_FILE_DEK_LEN + 1);
    if(NULL == temp)
    {
        return -1;
    }

    if(startSearchNum != 0)
    {
        i = startSearchNum;
    }
    else
    {
        i = 0;
    }
    j = 0;
    for(; i < dektotalnum; i++)
    {
        memset(temp, 0, XML_FILE_DEK_LEN);
        tempLen = 0;
        ret = sdkPaypassManagereadDek(i, temp, &tempLen);
        if(0 == ret)
        {
            #ifdef SXL_DEBUG
            Trace("emv", "\r\n sdkPaypassManageIfMatchDek sdkPaypassManagereadDek fail i=%d\r\n", i);
            #endif

            emvbase_free(temp);
            return -1;
        }

        if( 1 == sdkPaypassManageCompareddata(dekdetdata, temp, dekdetdatalen) )
        {
            #ifdef SXL_DEBUG
            Trace("emv", "\r\n cmp dekdetdatalen =%d\r\n", dekdetdatalen);
            #endif
            j = 1;
            break;
        }
    }

    emvbase_free(temp);



    if(0 == j)
    {
        #ifdef SXL_DEBUG
        Trace("emv", "\r\n sdkPaypassManageIfMatchDek NOT FIND XML i=%d \r\n", i);
        #endif
        return -1;
    }
    else
    {
        #ifdef SXL_DEBUG
        Trace("emv", "\r\n sdkPaypassManageIfMatchDek IS	FIND XML i=%d \r\n", i);
        #endif
        return i;
    }

}



s32 sdkPaypassManageIsMatchDekReturnDet(u8 *dekdata, u32 dekdatalen, u8 *DekCurIdnex, u8 *detdata, u32 *detdatalen, u8 *DetTotalNum, u8 *DetCurNum)
{
    s32 dekindex = 0, detTotalNum = 0, ret = 0;


    dekindex = sdkPaypassManageIfMatchDek(0, dekdata, dekdatalen);
    if(0 > dekindex)
    {
        #ifdef SXL_DEBUG
        Trace("emv", "\r\n sdkPaypassManageIsMatchDekReturnDet sdkPaypassManageIfMatchDek fail dekindex=%d\r\n", dekindex);
        #endif
        return -1;
    }

    *DekCurIdnex = dekindex;

    detTotalNum = sdkPaypassManageGetDeksSubDetTotalNum(&gstDEKIndex[dekindex]);

    ret = sdkPaypassManagereadDet(&gstDEKIndex[dekindex], 0, detdata, detdatalen);

    *DetCurNum = 0;
    *DetTotalNum = detTotalNum;
    return ret;
}

s32 sdkPayPassManagePrintfAllDekDet(void)
{
    s32 i = 0, j = 0, dektotalnum = 0, dettotalnum = 0;
    u8 *temp = NULL;
    u32 tempLen = 0;
    s32 ret = 0;

    dektotalnum = sdkPaypassManageGetDekTotalNum();
    if( 0 >= dektotalnum )
    {
        Trace("emv", "\r\n sdkPaypassManageprintf_all_dekdet dek total num is empty\r\n");

        return -1;
    }

    temp = (u8 *)emvbase_malloc(XML_FILE_DEK_LEN + 1);
    if(NULL == temp)
    {
        Trace("emv", "\r\n sdkPaypassManageprintf_all_dekdet emvbase_malloc err \r\n");
        return -1;
    }

    for(i = 0; i < dektotalnum; i++)
    {
        memset(temp, 0, XML_FILE_DEK_LEN);
        tempLen = 0;
        ret = sdkPaypassManagereadDek(i, temp, &tempLen);
        if(0 == ret)
        {
            Trace("emv", "\r\n sdkPaypassManageprintf_all_dekdet sdkPaypassManagereadDek fail i=%d ret=%d\r\n", i, ret);
        }

        Trace("emv", "\r\n sdkPaypassManageprintf_all_dekdet i=%d  ret=%d tempLen=%d\r\n", i, ret, tempLen);
        TraceHex("emv", "temp:", temp, tempLen);

        dettotalnum = sdkPaypassManageGetDeksSubDetTotalNum(&gstDEKIndex[i]);
        for(j = 0; j < dettotalnum; j++)
        {
            memset(temp, 0, XML_FILE_DEK_LEN);
            tempLen = 0;
            ret = sdkPaypassManagereadDet(&gstDEKIndex[i], j, temp, &tempLen);
            if(0 == ret)
            {
                Trace("emv", "\r\n sdkPaypassManageprintf_all_dekdet fail i=%d j=%d ret=%d\r\n", i, j, ret);
            }

            Trace("emv", "\r\n sdkPaypassManageprintf_all_dekdet i=%d j=%d  ret=%d tempLen=%d\r\n", i, j, ret, tempLen);
            TraceHex("emv", "temp:", temp, tempLen);
        }
    }
    emvbase_free(temp);
    return 0;
}


#ifdef PAYPASS_DISP_DEK_AND_CMD

void sdkPaypassFormatCopyAsciiToHexBuf(unsigned char *wbuf, unsigned char wlen, unsigned char *OutBuf, unsigned int *OutBufLen, unsigned char mode)
{
    unsigned short Len = 0x00;

    Len = wlen;
    OutBuf[0] = 0x00;//0x00:ascii, 0x01:hex
    OutBuf[1] = 0x00;//2?-???????????o|
    OutBuf[2] = wlen;

    memcpy(&OutBuf[3], wbuf, Len);
    *OutBufLen = 0x00;
    *OutBufLen = wlen + 3;

}
void sdkPaypassFormatCopyHexToAsciiBuf(unsigned char *wbuf, unsigned short wlen, unsigned char *OutBuf, unsigned int *OutBufLen, unsigned char mode)
{
    OutBuf[0] = 0x01;
    OutBuf[1] = (unsigned char)(wlen >> 8);
    OutBuf[2] = (unsigned char)(wlen);
    memcpy(&OutBuf[3], wbuf, wlen);
    *OutBufLen = wlen + 3;
}

static unsigned int  gPaypassDEKandCMDSaveLen = 0;

unsigned int sdkPaypassManage_dekdet_get_save_file_temp_index(void)
{
    return gPaypassDEKandCMDSaveLen;
}

int sdkPaypassManage_dekdet_set_save_file_temp_index(unsigned int SaveLen)
{
    gPaypassDEKandCMDSaveLen = SaveLen;
    return 0;
}

int sdkPaypassManage_dekdet_index_save(void)
{
    unsigned char	writebuf[64];
    int ret = 0;
    memset(writebuf, 0, sizeof(writebuf));
    writebuf[0] = (unsigned char)(gPaypassDEKandCMDSaveLen >> 24);
    writebuf[1] = (unsigned char)(gPaypassDEKandCMDSaveLen >> 16);
    writebuf[2] = (unsigned char)(gPaypassDEKandCMDSaveLen >> 8);
    writebuf[3] = (unsigned char)(gPaypassDEKandCMDSaveLen);
    #ifdef EMVB_DEBUG
    //Trace("emv","\r\n sdkPaypassManage_dekdet_index_save len=%d bufForLen = %02x %02x %02x %02x \r\n",\
    //gPaypassDEKandCMDSaveLen,writebuf[0],writebuf[1],writebuf[2],writebuf[3]);
    #endif

    ret = sdkPaypassInsertFile(DETDEKEXCHANGELOG, writebuf, 4, 0);
    #ifdef EMVB_DEBUG
    //Trace("emv","\r\n sdkPaypassManage_dekdet_index_save ret = %d\r\n",ret);
    #endif
    return ret;
}



#define SAVEDEKDETFILEMAXLEN	(300)
#define DEKDET_DATAEXCHANGE_BUF_LEN     (512)

int sdkPaypassManage_dekdet_datachange_save(unsigned char *headascii, unsigned char headasciilen,
        unsigned char *temp_bcd, unsigned short temp_bcdlen)
{
    unsigned char	*asciibuf = NULL;
    unsigned int asciiBufLen = 0, dekdetindex = 0, BufLentemph = 0, i, BufLentempl = 0, needsavelen = 0;
    int ret = 0;


    asciibuf = (unsigned char *)emvbase_malloc(DEKDET_DATAEXCHANGE_BUF_LEN);
    if(NULL == asciibuf)
    {
        //Assert(0);
        #ifdef SXL_DEBUG
        Trace("emv", "\r\n sdkPaypassManage_dekdet_datachange_save Assert(0)\n");
        #endif
        //while(1);
        return -1;
    }
    memset(asciibuf, 0x00, DEKDET_DATAEXCHANGE_BUF_LEN);

    //SAVE HEAD

    //change head len
    sdkPaypassFormatCopyAsciiToHexBuf(headascii, headasciilen, asciibuf, &needsavelen, 0);

    dekdetindex = sdkPaypassManage_dekdet_get_save_file_temp_index();
    ret = sdkPaypassInsertFile(DETDEKEXCHANGELOG, (u8 *)asciibuf, needsavelen, dekdetindex);
    #ifdef EMVB_DEBUG
    Trace("emv", "\r\n savehead ret =%d needsavelen=%d\r\n", ret, needsavelen);
    #endif
    dekdetindex += needsavelen;
    sdkPaypassManage_dekdet_set_save_file_temp_index(dekdetindex);


    //save content
    if(temp_bcdlen > SAVEDEKDETFILEMAXLEN)
    {
        BufLentemph = temp_bcdlen / SAVEDEKDETFILEMAXLEN;
        BufLentempl = temp_bcdlen % SAVEDEKDETFILEMAXLEN;

        for(i = 0; i < BufLentemph; i++)
        {
            memset(asciibuf, 0x20, DEKDET_DATAEXCHANGE_BUF_LEN);

            asciiBufLen = 0;
            sdkPaypassFormatCopyHexToAsciiBuf(temp_bcd + i * SAVEDEKDETFILEMAXLEN, SAVEDEKDETFILEMAXLEN, asciibuf, &asciiBufLen, 1);

            dekdetindex = sdkPaypassManage_dekdet_get_save_file_temp_index();
            ret = sdkPaypassInsertFile(DETDEKEXCHANGELOG, (u8 *)asciibuf, asciiBufLen, dekdetindex);
            if(0 != ret)
            {
                #ifdef EMVB_DEBUG
                Trace("emv", "\r\n sdkPaypassInsertFile err ret1=%d\r\n", ret);
                #endif

                break;
            }

            #ifdef EMVB_DEBUG
            Trace("emv", "\r\nsdkPaypassManage_dekdet_datachange_save dekdetindex=%d ret=%d asciiBufLen2=%d\r\n", dekdetindex, ret, asciiBufLen);
            #endif

            dekdetindex += asciiBufLen;
            sdkPaypassManage_dekdet_set_save_file_temp_index(dekdetindex);
        }

        if(BufLentempl > 0)
        {
            memset(asciibuf, 0x20, DEKDET_DATAEXCHANGE_BUF_LEN);

            asciiBufLen = 0;
            sdkPaypassFormatCopyHexToAsciiBuf(temp_bcd + i * SAVEDEKDETFILEMAXLEN, BufLentempl, asciibuf, &asciiBufLen, 0);

            dekdetindex = sdkPaypassManage_dekdet_get_save_file_temp_index();
            ret = sdkPaypassInsertFile(DETDEKEXCHANGELOG, (u8 *)asciibuf, asciiBufLen, dekdetindex);
            if(0 != ret)
            {
                #ifdef EMVB_DEBUG
                Trace("emv", "\r\n sdkPaypassInsertFile err ret=%d\r\n", ret);
                #endif
            }

            #ifdef EMVB_DEBUG
            Trace("emv", "\r\n sdkPaypassManage_dekdet_datachange_save dekdetindex ggg =%d ret=%d asciiBufLen2=%d\r\n", dekdetindex, ret, asciiBufLen);
            #endif

            dekdetindex += asciiBufLen;
            sdkPaypassManage_dekdet_set_save_file_temp_index(dekdetindex);
        }
    }
    else
    {
        if(temp_bcdlen > 0)
        {
            memset(asciibuf, 0x20, DEKDET_DATAEXCHANGE_BUF_LEN);

            asciiBufLen = 0;
            sdkPaypassFormatCopyHexToAsciiBuf(temp_bcd, temp_bcdlen, asciibuf, &asciiBufLen, 0);
            dekdetindex = sdkPaypassManage_dekdet_get_save_file_temp_index();
            ret = sdkPaypassInsertFile(DETDEKEXCHANGELOG, (u8 *)asciibuf, asciiBufLen, dekdetindex);

            #ifdef EMVB_DEBUG
            Trace("emv", "\r\nsdkPaypassManage_dekdet_datachange_save dekdetindex fffa =%d ret=%d asciiBufLen2=%d\r\n", dekdetindex, ret, asciiBufLen);
            #endif

            dekdetindex += asciiBufLen;
            sdkPaypassManage_dekdet_set_save_file_temp_index(dekdetindex);
        }
    }
    sdkPaypassManage_dekdet_index_save();
    emvbase_free(asciibuf);
	asciibuf=NULL;
    return 0;
}
#endif

int sdkPaypassManage_savedatachange_cmd(int mode)
{

    #ifdef EMVB_DEBUG
    //	 Trace("emv","\r\n sdkPaypassManage_savedatachange_cmd mode=%d aaaa bIsExsitRecvCMDLog=%d %d %d %d \r\n",
    //		 mode,paypassBIsExsitRecvCMDLog,gNeedSavewritebuflen,gNeedSaveEMVRxLen,sdkPayPassSupportSaveDekLogGetStatus());
    #endif

    #ifdef PAYPASS_DISP_DEK_AND_CMD
    if(sdkPayPassSupportSaveDekLogGetStatus())
    {
    	if(gNeedSavewritebuflenCAPDU > 0 && gNeedSaveEMVRxLenCAPDU > 0)
        {
            sdkPaypassManage_dekdet_datachange_save(gNeedSaveWritebufCAPDU, gNeedSavewritebuflenCAPDU, gNeedSaveEMVRxBufCAPDU, gNeedSaveEMVRxLenCAPDU);
        }

        if(paypassBIsExsitRecvCMDLog)
        {
            paypassBIsExsitRecvCMDLog = 0;

            if(gNeedSavewritebuflen > 0 && gNeedSaveEMVRxLen > 0)
            {
                sdkPaypassManage_dekdet_datachange_save(gNeedSaveWritebuf, gNeedSavewritebuflen, gNeedSaveEMVRxBuf, gNeedSaveEMVRxLen);
            }
        }

    }
    #endif

    return 0;
}




u8 sdkPaypassdekCurIndex = 0;
u8 sdkPaypassdetCurIndex = 0;
u8 sdkPaypassdetTotalNum = 0;

#ifdef PAYPASS_DISP_DEK_AND_CMD
    int sdkPaypassdekdetsendcmt = 0;
    int sdkPaypassdekdetresvcmt = 0;
#endif

//send return value : 1 succ,0 fall
//resv return value : >0 read datalen ,  =0:fall
int sdkPayPassDataExchangeCmd(unsigned char cmd, unsigned char *data, unsigned int *dataLen)
{
    int ret = 0;
    unsigned int readdataLen = 0;

    #ifdef PAYPASS_DISP_DEK_AND_CMD
    unsigned char  writebuf[76];//,*asciibuf;
    unsigned int writebuflen = 0; //,asciiBufLen;
    unsigned short needsavelen = 0;
    #endif
	Trace("emv", "\r\n sdkPayPassDataExchangeCmd cmd = %d\r\n ", cmd);

    if(1 == cmd)
    {
        #ifdef PAYPASS_DISP_DEK_AND_CMD
        if(sdkPayPassSupportSaveDekLogGetStatus())
        {
            sdkPaypassdekdetsendcmt++;

            memset(writebuf, 0, sizeof(writebuf));
            sprintf(writebuf, "--- send DEK: Len=%03d, cnt=%d ---", *dataLen, sdkPaypassdekdetsendcmt);
            writebuflen = strlen(writebuf);
            needsavelen = (unsigned short)dataLen[0];
            sdkPaypassManage_dekdet_datachange_save(writebuf, writebuflen, data, needsavelen);
        }


        Trace("emv", "\r\n ------------------*******the cnt send %d ,resv %d******---------------------\r\n", sdkPaypassdekdetsendcmt, sdkPaypassdekdetresvcmt);
        Trace("emv", "\r\n send DEK : len=%d, send data= \r\n ", dataLen[0]);
        TraceHex("emv", "sdk send dek : ", data, dataLen[0]);
        Trace("emv", "\r\n sdkPayPassDataExchangeCmd cmd=%d dekindex=%d =%d =%d\r\n", cmd, sdkPaypassdekCurIndex, sdkPaypassdetCurIndex, sdkPaypassdetTotalNum);

        #endif
        ret = sdkPaypassManageIfMatchDek(0, data, dataLen[0]);
        if(0 > ret)
        {
            sdkPaypassdekCurIndex = 0;
            sdkPaypassdetCurIndex = 0;
            sdkPaypassdetTotalNum = 0;
            return 0;
        }
        sdkPaypassdetCurIndex = 0;
        sdkPaypassdekCurIndex = ret;
        sdkPaypassdetTotalNum = sdkPaypassManageGetDeksSubDetTotalNum(&gstDEKIndex[sdkPaypassdekCurIndex]);
        #ifdef SXL_DEBUG
        Trace("emv", "\r\n sdkPayPassDataExchangeCmdSendDEK =%d =%d =%d\r\n", sdkPaypassdekCurIndex, sdkPaypassdetTotalNum, sdkPaypassdetCurIndex);
        #endif
        ret = 1;
    }
    else if(2 == cmd)//??????????? det??????????a?????????
    {
        if((sdkPaypassdetCurIndex > sdkPaypassdetTotalNum) || (0 >= sdkPaypassdetTotalNum))
        {
            #ifdef SXL_DEBUG
            Trace("emv", "\r\n sdkPayPassDataExchangeCmd read det, but do not macth dek, return 0 %d %d \r\n", sdkPaypassdetCurIndex, sdkPaypassdetTotalNum);
            #endif
            return 0;
        }
        readdataLen = 0x00;
        ret = sdkPaypassManagereadDet(&gstDEKIndex[sdkPaypassdekCurIndex], sdkPaypassdetCurIndex, data, &readdataLen);
        if(ret > 0)
        {
            sdkPaypassdetCurIndex++;
            *dataLen = ret;

            #ifdef PAYPASS_DISP_DEK_AND_CMD
            if(sdkPayPassSupportSaveDekLogGetStatus())
            {
                sdkPaypassdekdetresvcmt++;

                memset(writebuf, 0, sizeof(writebuf));
                sprintf(writebuf, "--- resv DET: Len=%03d, cnt=%d ---", readdataLen, sdkPaypassdekdetresvcmt);
                writebuflen = strlen(writebuf);

                needsavelen = (unsigned short)readdataLen;
                sdkPaypassManage_dekdet_datachange_save(writebuf, writebuflen, data, needsavelen);

            }

            Trace("emv", "\r\n ------------------*******the cnt send %d ,resv %d******---------------------\r\n", sdkPaypassdekdetsendcmt, sdkPaypassdekdetresvcmt);
            Trace("emv", "\r\n resv DET :len=%d,ret=%d \r\n ", readdataLen, ret);
            TraceHex("emv", "resv data= ", data, readdataLen);
            Trace("emv", "\r\n sdkPayPassDataExchangeCmd cmd=%d dekindex=%d =%d =%d\r\n", cmd, sdkPaypassdekCurIndex, sdkPaypassdetCurIndex, sdkPaypassdetTotalNum);

            #endif
        }
        else
        {
            *dataLen = 0;
        }
    }
    return ret;
}

int sdkPaypassDateExchangeBeforeSendCMD(void)
{
    if((sdkPaypassdetTotalNum > 0) && (sdkPaypassdekCurIndex > 0) && (sdkPaypassdetCurIndex <= sdkPaypassdetTotalNum))
    {
        #if DDI_RF_EXCHANGE_EMVBASE_APDU_CMD_TIMER_CNT /*Modify by luohuidong at 2016.11.24  19:18 */
        Trace("emv", "\r\n (sdkPaypassdetTotalNum > 0) \r\n");
        #endif

        unsigned char *detdata = NULL;
        unsigned int detdatalen = 0;
        int deret = 0;
        detdata = (unsigned char *)emvbase_malloc(DEKDETSENDDATALEN);
        if(NULL == detdata)
        {
            #ifdef SXL_DEBUG
            Trace("emv", "\r\n ContactlessIccIsoCommand emvbase_malloc 2048 err !!!!! \r\n");
            #endif
            return -1;
        }
        memset(detdata, 0, DEKDETSENDDATALEN);
        detdatalen = 0;
        deret = sdkPayPassDataExchangeCmd(2, detdata, &detdatalen);
        if(deret > 0)
        {
            #ifdef SXL_DEBUG
            Trace("emv", "\r\n sdkPayPassDataExchangeCmd update DET's TLV bb \r\n");
            #endif

            u16 index, paydatalen, lena;
            u8 tag[4], taglen;
            u8 *paydata = NULL;

            index = 0;
            paydata = &detdata[0];
            paydatalen = detdatalen;

            #ifdef SXL_DEBUG
            Trace("emv", "\r\n cc paydatalen=%d paydata[0]=%02x %02x %02x %02x \r\n", paydatalen, paydata[0], paydata[1], paydata[2], paydata[3]);
            TraceHex("emv", "data", detdata, detdatalen);
            // TraceHex("emv","cc paydata:",detdata,detdatalen);
            #endif

            while(index < paydatalen)
            {
                taglen = 0;
                memset(tag, 0, sizeof(tag));
                tag[taglen++] = paydata[index++];
                if((tag[0] & 0x1f) == 0x1f)
                {
                    tag[taglen++] = paydata[index++];
                    if(tag[1] & 0x80)
                    {
                        tag[taglen++] = paydata[index++];
                    }
                }
                if(EMVBase_ParseExtLen(paydata, &index, &lena))
                {
                    #ifdef SXL_DEBUG
                    Trace("emv", "\r\n (EMVBase_ParseExtLen(paydata,&index,&len)) err !!! line 3544 index=%d len=%d \r\n", index, lena);
                    #endif
                    break;
                }
                #ifdef EMVB_DEBUG
                TraceHex("emv", "data", tag, taglen);
                TraceHex("emv", "data", &paydata[index], lena);
                #endif
                emvbase_avl_createsettagvalue(tag, &paydata[index], lena);

                index += lena;
            }
            if(index != paydatalen)
            {
                #ifdef SXL_DEBUG
                Trace("emv", "\r\npay data err index=%d paydatalen=%d\r\n", index, paydatalen);
                #endif
            }
            paydata = NULL;
        }
        emvbase_free(detdata);
        detdata = NULL;
    }
}

#ifdef PAYPASS_DISP_DEK_AND_CMD

void sdkPaypassDEsaveSendApdulog(unsigned char *tempsenddata, unsigned short tempdatalen)
{
    unsigned char  writebuf[64];
    unsigned char writebuflen = 0;

    if(sdkPayPassSupportSaveDekLogGetStatus())
    {
        Trace("emv", "sdkPayPassSupportSaveDekLogGetStatus");

        memset(writebuf, 0, sizeof(writebuf));				 //
        sprintf(writebuf, "send CA: Len=%d ", tempdatalen); //1?-?????????????  2?-???????????o|
        writebuflen = strlen(writebuf);

		if(0 != memcmp("\x80\xEA", tempsenddata, 2))
		{
			gNeedSavewritebuflenCAPDU = 0;
			gNeedSaveEMVRxLenCAPDU = 0;
	        sdkPaypassManage_dekdet_datachange_save(writebuf, writebuflen, tempsenddata, tempdatalen);
		}
		else
		{
			memset(gNeedSaveWritebufCAPDU, 0, sizeof(gNeedSaveWritebufCAPDU));
            if(writebuflen > 64)
            {
                gNeedSavewritebuflenCAPDU = 64;
            }
            else
            {
                gNeedSavewritebuflenCAPDU = writebuflen;
            }
			memcpy(gNeedSaveWritebufCAPDU, writebuf, gNeedSavewritebuflenCAPDU);

			memset(gNeedSaveEMVRxBufCAPDU, 0, sizeof(gNeedSaveEMVRxBufCAPDU));
            if(tempdatalen > 300)
            {
                gNeedSaveEMVRxLenCAPDU = 300;
            }
            else
            {
                gNeedSaveEMVRxLenCAPDU = tempdatalen;
            }
            memcpy(gNeedSaveEMVRxBufCAPDU, tempsenddata, gNeedSaveEMVRxLenCAPDU);
		}
    }
}

void sdkPaypassDEsaveRecvApdulog(unsigned char *EMVRxBuf, unsigned short EMVRxLen)
{
    unsigned char  writebuf[64];
    unsigned char writebuflen = 0;

    if(sdkPayPassSupportSaveDekLogGetStatus())
    {
        memset(writebuf, 0, sizeof(writebuf));                //
        sprintf(writebuf, "resv RA: Len=%d ", EMVRxLen);; //1?-?????????????  2?-???????????o|
        writebuflen = strlen(writebuf);
        if(paypassBIsDoNotSaveCurRecvCMDLog)
        {
            paypassBIsDoNotSaveCurRecvCMDLog = 0;

            paypassBIsExsitRecvCMDLog = 1;

            memset(gNeedSaveWritebuf, 0, sizeof(gNeedSaveWritebuf));
            if(writebuflen > 64)
            {
                gNeedSavewritebuflen = 64;
            }
            else
            {
                gNeedSavewritebuflen = writebuflen;
            }
            memcpy(gNeedSaveWritebuf, writebuf, gNeedSavewritebuflen);

            memset(gNeedSaveEMVRxBuf, 0, sizeof(gNeedSaveEMVRxBuf));
            if(EMVRxLen > 300)
            {
                gNeedSaveEMVRxLen = 300;
            }
            else
            {
                gNeedSaveEMVRxLen = EMVRxLen;
            }
            memcpy(gNeedSaveEMVRxBuf, EMVRxBuf, gNeedSaveEMVRxLen);
        }
        else
        {
            sdkPaypassManage_dekdet_datachange_save(writebuf, writebuflen, EMVRxBuf, EMVRxLen);
        }
    }
}

//20190202 lhd this dekLog function only for debug use
void sdkPayPassSupportSaveDekLogSet(void)//this function need use before sdkPaypassTransInit func
{
    paypassBIsSaveDekExchangeApduLog = 1;
}

void sdkPayPassSupportSaveDekLogClear(void)
{
    paypassBIsSaveDekExchangeApduLog = 0;
}

unsigned char sdkPayPassSupportSaveDekLogGetStatus(void)
{
    return paypassBIsSaveDekExchangeApduLog;
}
//end
#endif


/*=======BEGIN: luohuidong 2017.04.06  14:10 modify===========*/
int sdkpaypassCreateDefTagValue(unsigned char *tag, unsigned char *tagdata, unsigned short tagdatalen, unsigned char updatecondition)
{
    EMVBASETAGCVLITEM storeditem;


    #ifdef EMVB_DEBUG
    Trace("emv", "sdkpaypassCreateDefTagValue tag %02x %02x %02x updatecondition=0x%02x\r\n", tag[0], tag[1], tag[2], updatecondition);
    #endif


    memset((unsigned char *)&storeditem, 0, sizeof(EMVBASETAGCVLITEM));

    storeditem.updatecondition = updatecondition;

    unsigned char taglen = strlen((char *)tag);

    memcpy(storeditem.Tag, tag, ((taglen > 3) ? 3 : (taglen)));

    return emvbase_avl_inserttag(&storeditem, tag, strlen((char *)tag), tagdata, tagdatalen, NULL);
}
/*====================== END======================== */


void  sdkPaypassInitDEparam(void)
{
    paypassBIsExsitRecvCMDLog = 0;
    paypassBIsDoNotSaveCurRecvCMDLog = 0;

    #ifdef PAYPASS_DISP_DEK_AND_CMD
    gNeedSavewritebuflen = 0;
    gNeedSaveEMVRxLen = 0;
    sdkPaypassdekdetsendcmt = 0;
    sdkPaypassdekdetresvcmt = 0;
    #endif
}


u8 sdkPaypassCheckifCancelCLtrade(void)
{
    return false;
}

u8 gsdkPaypassSaveApduCMDfileInitflag = 1; //1:need clear file,0:do not clear file
u8 sdkPaypassSaveApduCMDfileInitflagSet(u8 mode)
{
    Trace("emv", "mode=%d \r\n", mode);
    gsdkPaypassSaveApduCMDfileInitflag = mode;
    return 1;
}

u8 sdkPaypassSaveApduCMDfileInitflagGet(void)
{
    Trace("emv", "gsdkPaypassSaveApduCMDfileInitflag=%d \r\n", gsdkPaypassSaveApduCMDfileInitflag);
    return gsdkPaypassSaveApduCMDfileInitflag;
}

void sdkPaypassSaveApduCMDfileInit(void)
{
    if(sdkPaypassAccessFile(DETDEKEXCHANGELOG) == 0)
    {
        #ifdef EMVB_DEBUG
        Trace("emv", "\r\n DETDEKEXCHANGELOG file exist ,now delet it \r\n");
        #endif
        sdkPaypassDelFile(DETDEKEXCHANGELOG);
    }
    #ifdef PAYPASS_DISP_DEK_AND_CMD

    sdkPaypassManage_dekdet_set_save_file_temp_index(12);

    #endif
    //s32 transresult =
    sdkPaypassInsertFile(DETDEKEXCHANGELOG, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12, 0);
    #ifdef EMVB_DEBUG
    //	Trace("emv","\r\n read buflen 88 ret = %d\r\n",transresult);
    #endif
}


s32 sdkPaypassDifferEMVBase()
{
    unsigned char tagbExist = 0;
    EMVBASETAGCVLITEM *item = NULL;


    emvbase_avl_settagtype(EMVTAGTYPE_PAYPASS);

    emvbase_avl_createsettagvalue(EMVTAG_CVMResult, "\x00\x00\x00", 3);

    emvbase_avl_deletetag(EMVTAG_ThirdPartyData);//20200102_lhd AEPAY already INIT THIS TAG:9F6E
    emvbase_avl_deletetag(EMVTAG_MCLSecureCardFrame);//20210408_lhd NDOT AFC DATA STRUCTURE

	if(!emvbase_avl_checkiftagexist(EMVTAG_TermCapab))
	{
		emvbase_avl_createsettagvalue(EMVTAG_TermCapab, "\xE0\x60\x08", 3);
	}
	if(!emvbase_avl_checkiftagexist(EMVTAG_TermAddCapab))
	{
		emvbase_avl_createsettagvalue(EMVTAG_TermAddCapab, "\x61\x00\xD0\xB0\x01", 5);
	}
//	emvbase_avl_settag(EMVTAG_KernelConfiguration, 0x30);
	if(!emvbase_avl_checkiftagexist(EMVTAG_KernelConfiguration))
	{
		emvbase_avl_settag(EMVTAG_KernelConfiguration, 0xB0);	//for J03 team
	}
	if(!emvbase_avl_checkiftagexist(EMVTAG_RTCL_OndeviceCVM))
	{
		emvbase_avl_createsettagvalue(EMVTAG_RTCL_OndeviceCVM, "\x00\x09\x99\x99\x99\x99", 6);
	}
	if(!emvbase_avl_checkiftagexist(EMVTAG_magcvmrequirer))
	{
		emvbase_avl_settag(EMVTAG_magcvmrequirer, 0x10);
	}
	if(!emvbase_avl_checkiftagexist(EMVTAG_magnocvmrequirer))
	{
		emvbase_avl_settag(EMVTAG_magnocvmrequirer, 0x00);
	}
	if(!emvbase_avl_checkiftagexist(EMVTAG_SecurityCapability))
	{
		emvbase_avl_settag(EMVTAG_SecurityCapability, 0x08);
	}
	if(!emvbase_avl_checkiftagexist(EMVTAG_CardDataInputCapability))
	{
		emvbase_avl_settag(EMVTAG_CardDataInputCapability, 0xE0);
	}
//	emvbase_avl_settag(EMVTAG_CVMCapabCVMRequired, 0x60);
	if(!emvbase_avl_checkiftagexist(EMVTAG_CVMCapabCVMRequired))
	{
		emvbase_avl_settag(EMVTAG_CVMCapabCVMRequired, 0x2C);	//for J03 team
	}
	if(!emvbase_avl_checkiftagexist(EMVTAG_CVMCapabNOCVMRequired))
	{
		emvbase_avl_settag(EMVTAG_CVMCapabNOCVMRequired, 0x08);
	}

    emvbase_avl_createsettagvalue(EMVTAG_TVR, "\x00\x00\x00\x00\x00", 5);


    if(emvbase_avl_checkiftagexist(EMVTAG_TermUDOL) == 0)
    {
        emvbase_avl_createsettagvalue(EMVTAG_TermUDOL, "\x9F\x6A\x04", 3);
    }

    if(emvbase_avl_checkiftagexist(EMVTAG_paypassMagAppVer) == 0)
    {
        emvbase_avl_createsettagvalue(EMVTAG_paypassMagAppVer, "\x00\x01", 2);
    }


    if(emvbase_avl_checkiftagexist(EMVTAG_AppVerNum) == 0)
    {
        emvbase_avl_createsettagvalue(EMVTAG_AppVerNum, "\x00\x02", 2);

        Trace("emv", "set EMVTAG_TerminalExpectedTTFRRPAPDU df8135 to 0018\n");
    }

    if(emvbase_avl_checkiftagexist(EMVTAG_TermExpectedTransmissionTimeForRR_CAPDU) == 0)
    {
        emvbase_avl_createsettagvalue(EMVTAG_TermExpectedTransmissionTimeForRR_CAPDU, "\x00\x12", 2);
    }
    if(emvbase_avl_checkiftagexist(EMVTAG_TermExpectedTransmissionTimeForRR_RAPDU) == 0)
    {
//        emvbase_avl_createsettagvalue(EMVTAG_TermExpectedTransmissionTimeForRR_RAPDU, "\x00\x18", 2);
		  emvbase_avl_createsettagvalue(EMVTAG_TermExpectedTransmissionTimeForRR_RAPDU, "\x00\x12", 2);
    }
    if(emvbase_avl_checkiftagexist(EMVTAG_RRAccuracyThreshold) == 0)
    {
        emvbase_avl_createsettagvalue(EMVTAG_RRAccuracyThreshold, "\x01\x2C", 2);
    }
    if(emvbase_avl_checkiftagexist(EMVTAG_RRTransmissionTimeMismatchThreshold) == 0)
    {
//        emvbase_avl_createsettagvalue(EMVTAG_RRTransmissionTimeMismatchThreshold, "\x32", 1);
		  emvbase_avl_createsettagvalue(EMVTAG_RRTransmissionTimeMismatchThreshold, "\x05", 1);
    }
    if(emvbase_avl_checkiftagexist(EMVTAG_MaxRRGracePeriod) == 0)
    {
        emvbase_avl_createsettagvalue(EMVTAG_MaxRRGracePeriod, "\x00\x32", 2);
    }
    if(emvbase_avl_checkiftagexist(EMVTAG_MinRRGracePeriod) == 0)
    {
        emvbase_avl_createsettagvalue(EMVTAG_MinRRGracePeriod, "\x00\x14", 2);
    }

	emvbase_avl_printtagallvalue("in sdkPaypassDifferEMVBase", EMVTAG_MinRRGracePeriod);

    if(emvbase_avl_checkiftagexist(EMVTAG_PaypassPunatcTrack2))
    {
        emvbase_avl_deletetag(EMVTAG_PaypassPunatcTrack2);
    }

	if(0 == sdkPaypassGetCertificationStatus()) // removed for SB261 but reserved in production
	{
	    if(emvbase_avl_checkiftagexist(EMVTAG_BalanceBeforeGAC))
	    {
	        tagbExist = 0;
	        item = emvbase_avl_gettagitempointer(EMVTAG_BalanceBeforeGAC);
	        if(item != NULL)
	        {
	            if((6 == item->len) && (0 == memcmp("\x00\x00\x00\x00\x00\x00", item->data, 6)))
	            {
	                tagbExist = 1;
	            }
	        }
	        if(0 == tagbExist)
	        {
	            emvbase_avl_createsettagvalue(EMVTAG_BalanceBeforeGAC, NULL, 0);
	        }
 	    }

	    if(emvbase_avl_checkiftagexist(EMVTAG_BalanceAfterGAC))
	    {
	        tagbExist = 0;
	        item = emvbase_avl_gettagitempointer(EMVTAG_BalanceAfterGAC);
	        if(item != NULL)
	        {
	            if((6 == item->len) && (0 == memcmp("\x00\x00\x00\x00\x00\x00", item->data, 6)))
	            {
	                tagbExist = 1;
	            }
	        }
	        if(0 == tagbExist)
	        {
	            emvbase_avl_createsettagvalue(EMVTAG_BalanceAfterGAC, NULL, 0);
	        }
	    }
	}

    if(emvbase_avl_checkiftagexist(EMVTAG_MaxLifeTimeTornLog) == 0)
    {
        emvbase_avl_createsettagvalue(EMVTAG_MaxLifeTimeTornLog, PAYPASS_TORNLOG_MAXLIFETIME_DEFAULT, 3);
    }

    if(emvbase_avl_checkiftagexist(EMVTAG_MaxNumTornLog) == 0)
    {
        emvbase_avl_settag(EMVTAG_MaxNumTornLog, 0);
    }

    if(emvbase_avl_checkiftagexist(EMVTAG_MessageHoldTime) == 0)
    {
        emvbase_avl_createsettagvalue(EMVTAG_MessageHoldTime, PAYPASS_USERREQDATA_MSGHOLDTIME_DEFAULT, 3);
    }

    if(emvbase_avl_checkiftagexist(EMVTAG_HoldTimeValue) == 0)
    {
        emvbase_avl_settag(EMVTAG_HoldTimeValue, 0x0d);
    }

	gstPaypassTradeUnionStruct->PaypassTradeParam->bretforminit = sdkPaypassGetCertificationStatus();

	return SDK_OK;
}

void sdkPaypassAidToTermInfo(unsigned char i, EMVBASE_AID_STRUCT *termaidparam, unsigned char onlinepinsup)
{
    emvbase_avl_createsettagvalue(EMVTAG_TermAID, (termaidparam + i)->Aid, (termaidparam + i)->AidLen);

    emvbase_avl_printtagallvalue("before sdkPaypassAidToTermInfo EMVTAG_AppVerNum-", EMVTAG_AppVerNum);

    emvbase_avl_createsettagvalue(EMVTAG_AppVerNum, (termaidparam + i)->AppVerNum, 2);

    emvbase_avl_printtagallvalue("after sdkPaypassAidToTermInfo EMVTAG_AppVerNum-", EMVTAG_AppVerNum);

    emvbase_avl_createsettagvalue(EMVTAG_TACDefault, (termaidparam + i)->TacDefault, 5);
    emvbase_avl_createsettagvalue(EMVTAG_TACOnline, (termaidparam + i)->TacOnline, 5);
    emvbase_avl_createsettagvalue(EMVTAG_TACDenial, (termaidparam + i)->TacDecline, 5);

    emvbase_avl_createsettagvalue(EMVTAG_termcvm_limit, (termaidparam + i)->cl_cvmlimit, 6);
	TraceHex("paypass-info", "CVM Limit", (termaidparam + i)->cl_cvmlimit, 6);
	emvbase_avl_createsettagvalue(EMVTAG_clessofflineamt, (termaidparam + i)->cl_translimit, 6);
	TraceHex("paypass-info", "Contactless trans Limit", (termaidparam + i)->cl_translimit, 6);
	emvbase_avl_createsettagvalue(EMVTAG_clessofflinelimitamt, (termaidparam + i)->cl_offlinelimit, 6);
	TraceHex("paypass-info", "Contactless floor Limit", (termaidparam + i)->cl_offlinelimit, 6);

    emvbase_avl_createsettagvalue(EMVTAG_FloorLimit, (termaidparam + i)->FloorLimit, 4);
    emvbase_avl_createsettagvalue(EMVTAG_Threshold, (termaidparam + i)->Threshold, 4);
    emvbase_avl_settag(EMVTAG_MaxTargetPercent, (termaidparam + i)->MaxTargetPercent);
    emvbase_avl_settag(EMVTAG_TargetPercent, (termaidparam + i)->TargetPercent);
    emvbase_avl_createsettagvalue(EMVTAG_TermDDOL, (termaidparam + i)->TermDDOL, (termaidparam + i)->TermDDOLLen);
    //20210408_lhd for NDOT AFC DATA STRUCTURE  emvbase_avl_createsettagvalue(EMVTAG_VLPTransLimit, (termaidparam + i)->ectranslimit, 6);

	return;
}

s32 sdkPaypassReadAIDParameters(PAYPASSTradeUnionStruct *tempsdkPaypass_UnionStruct)
{
    s32 ret;
    u8 type = 0;

    EMVBASE_AID_STRUCT *tempaidparam;
    EMVBASE_LISTAPPDATA *tempappdata;
    unsigned char tempselectedappno;

    tempappdata = tempsdkPaypass_UnionStruct->PaypassTradeParam->AppListCandidate;
    tempselectedappno = tempsdkPaypass_UnionStruct->PaypassTradeParam->SelectedAppNo;
    tempaidparam = (EMVBASE_AID_STRUCT *)emvbase_malloc(sizeof(EMVBASE_AID_STRUCT));

    type = emvbase_avl_gettagvalue(EMVTAG_TransTypeValue);
    Trace("emv", "type=0x%x\n", type);
    ret = sdkEMVBaseReadAIDParameters((tempappdata + tempselectedappno)->AidInTerm, (tempappdata + tempselectedappno)->AidInTermLen, tempaidparam, type, 2);
    Trace("emv", "sdkEMVBaseReadAIDParameters ret=%d,%d\n", ret);
    if(ret != SDK_OK)
    {
        emvbase_free(tempaidparam);
        return ret;
    }
    sdkPaypassAidToTermInfo(0, tempaidparam, 0);

    emvbase_free(tempaidparam);

    #if 1
    if(emvbase_avl_checkiftagexist(EMVTAG_CardAID) == 0)
    {
        if((tempappdata + tempselectedappno)->AIDLen)
        {
            emvbase_avl_createsettagvalue(EMVTAG_CardAID, (tempappdata + tempselectedappno)->AID, (tempappdata + tempselectedappno)->AIDLen);
        }
    }
    #endif

    #if 0//20191014_lhd for case 3B02-0101(03-PPSE_WITHOUTOPTIONAL)
    if(emvbase_avl_checkiftagexist(EMVTAG_AppLabel) == 0)
    {
        if((tempappdata + tempselectedappno)->AppLabelLen)
        {
            emvbase_avl_createsettagvalue(EMVTAG_AppLabel, (tempappdata + tempselectedappno)->AppLabel, (tempappdata + tempselectedappno)->AppLabelLen);
        }
    }
    #endif

    #if 1
    if(emvbase_avl_checkiftagexist(EMVTAG_AppPreferName) == 0)
    {
        if((tempappdata + tempselectedappno)->PreferNameLen)
        {
            emvbase_avl_createsettagvalue(EMVTAG_AppPreferName, (tempappdata + tempselectedappno)->PreferName, (tempappdata + tempselectedappno)->PreferNameLen);
        }
    }

    if(emvbase_avl_checkiftagexist(EMVTAG_LangPrefer) == 0)
    {
        if((tempappdata + tempselectedappno)->LangPreferLen)
        {
            emvbase_avl_createsettagvalue(EMVTAG_LangPrefer, (tempappdata + tempselectedappno)->LangPrefer, (tempappdata + tempselectedappno)->LangPreferLen);
        }
    }
    #endif

    return SDK_OK;
}

s32 sdkPaypassPreTrans()
{
    u8 i, j = 0;
    u8 ifexistusedaid = 0;
    SDK_EMVBASE_AID_STRUCT *tempaidparam = NULL;
    unsigned char AmtAuthNum[6];
    s32 num = 0, aidnum = 0;
    unsigned char TransTypevalue;

    memset(AmtAuthNum, 0, sizeof(AmtAuthNum));
    emvbase_avl_gettagvalue_spec(EMVTAG_AmtAuthNum, AmtAuthNum, 0, 6);

    if( gstPaypassTradeParam == NULL || NULL == gstPaypassTradeParam->gPaypassProceFlag
            || gstPaypassTradeUnionStruct == NULL)
    {
        Trace("emv", "input err: %d,%d\r\n", (gstPaypassTradeParam == NULL), ( NULL == gstPaypassTradeParam->gPaypassProceFlag) );
        return SDK_ERR;
    }

    TransTypevalue = emvbase_avl_gettagvalue(EMVTAG_TransTypeValue);
    Trace("emv", "TAG_TransTypeValue = 0x%02x \r\n", TransTypevalue);


    tempaidparam = (SDK_EMVBASE_AID_STRUCT *)emvbase_malloc(sizeof(SDK_EMVBASE_AID_STRUCT));


    sdkEMVBaseGetAIDListNum(&aidnum);
    for(i = 0; i < aidnum; i++)
    {
        num = 0;
        memset(tempaidparam, 0, (sizeof(SDK_EMVBASE_AID_STRUCT)));
        sdkEMVBaseGetAnyAIDList(i, 1, tempaidparam, &num);

        if(num == 0)
        {
            continue;
        }

        if( tempaidparam->transvaule == TransTypevalue )
        {
            Trace("emv", "find i =%d ", i);
        }
        else
        {
            Trace("emv", "not find,need fine type=0x%x,actually type=0x%x, i =%d ", TransTypevalue, tempaidparam->transvaule, i);
            continue;
        }

        TraceHex("emv", "tempaidparam->cl_translimit", tempaidparam->cl_translimit, 6);
        TraceHex("emv", "tempaidparam->cl_offlinelimit", tempaidparam->cl_offlinelimit, 6);
        TraceHex("emv", "tempaidparam->cl_cvmlimit", tempaidparam->cl_cvmlimit, 6);
        TraceHex("emv", "AmtAuthNum", AmtAuthNum, 6);
        Trace("emv", "i=%d", i);


        if (NULL == gstsdkpaypass_appex_aid_list)
        {
            Trace("emv", "  get appex aid  err a !!!!,use default value. i=%d \r\n", i);
            TraceHex("emv", "tempaidparam->Aid", tempaidparam->Aid, tempaidparam->AidLen);
        }
        else
        {
            for(j = 0; j < aidnum; j++)
            {
                if( (memcmp(tempaidparam->Aid, gstsdkpaypass_appex_aid_list[j].Aid, gstsdkpaypass_appex_aid_list[j].AidLen) == 0)
                        && (tempaidparam->AidLen ==  gstsdkpaypass_appex_aid_list[j].AidLen)
                        && (tempaidparam->transvaule == gstsdkpaypass_appex_aid_list[j].tradetypevalue ) )
                {
                    Trace("emv", "find j =%d ", j);
                    break;
                }
                else
                {
                    continue;
                }
            }
            Trace("emv", "  b j=%d,iTotal=%d\r\n", j, aidnum);

            if(j >= aidnum)
            {
                emvbase_free(tempaidparam);//20191217_lhd add
                return SDK_ERR;
            }
        }


        //Reader Contactless Transaction Limit
        if(memcmp(AmtAuthNum, tempaidparam->cl_translimit, 6) > 0 )
        {
            gstPaypassTradeParam->gPaypassProceFlag[i].TermCTLEF = 1;
        }
        //20191106_lhd for config PPS_MCnoDefault_1 , else
        {
            ifexistusedaid = 1;
        }

        if(memcmp(AmtAuthNum, tempaidparam->cl_offlinelimit, 6) > 0 )
        {
            gstPaypassTradeParam->gPaypassProceFlag[i].TermCFLEF = 1;
        }


        if(memcmp(AmtAuthNum, tempaidparam->cl_cvmlimit, 6) > 0 )
        {
            gstPaypassTradeParam->gPaypassProceFlag[i].TermCVMRLEF = 1;
        }

        if((NULL != gstsdkpaypass_appex_aid_list))
        {
            if(memcmp(AmtAuthNum, gstsdkpaypass_appex_aid_list[j].ondevRTCL, 6) > 0 )
            {
                gstPaypassTradeParam->gPaypassProceFlag[i].OndevCTLEF = 1;
            }
            #if 1//for case 3G07-0100 A01
            else
            {
                ifexistusedaid = 1;
            }
            #endif
        }

    }

    emvbase_free(tempaidparam);


    if(ifexistusedaid)
    {

        return SDK_OK;
    }
    else
    {
        dllpaypass_SetErrPar(gstPaypassTradeUnionStruct, PAYPASSSTEP_S52_14);
        dllpaypass_SetErrPar(gstPaypassTradeUnionStruct, PAYPASSSTEP_S52_18);
        dllpaypass_packoutsignal(gstPaypassTradeUnionStruct, PAYPASSSTEP_S52_19);
        if (1 == sdkPaypassGetCertificationStatus())
        {
            return SDK_EMV_ENDAPPLICATION;
        }
        else
        {
            return SDK_EMV_NoAppSel;
        }
    }

}

s32 sdkPaypassFinalSelectedApp()
{
    u8 retCode = RLT_EMV_ERR;
    s32 rlt = SDK_ERR;

    retCode = Paypass_FinalSelectedApp(gstPaypassTradeUnionStruct);
    if(retCode == PAYPASS_READAIDPARAMETERS)
    {
        rlt = sdkPaypassReadAIDParameters(gstPaypassTradeUnionStruct);
        if(rlt != SDK_OK)
        {
            retCode = RLT_ERR_EMV_SWITCHINTERFACE;
        }
        else
        {
            return rlt;
        }
    }

    sdkEMVBaseRltToSdkRlt(retCode, &rlt);

    return rlt;
}

s32 sdkPaypassPreProcessing()
{
	EMVBASETAGCVLITEM *item;
    unsigned char tagbExist = 0;
    unsigned char ucAmtAuthNum[8] = {0};
    u8 MaxNumTornLog = 0;
	unsigned char ucAmtLimit[8] = {0};


	if(0 == sdkPaypassGetCertificationStatus()) // removed for SB261 but reserved in production
	{
	    if(emvbase_avl_checkiftagexist(EMVTAG_BalanceBeforeGAC))
	    {
	        tagbExist = 0;
	        item = emvbase_avl_gettagitempointer(EMVTAG_BalanceBeforeGAC);
	        if(item != NULL)
	        {
	            if((6 == item->len) && (0 == memcmp("\x00\x00\x00\x00\x00\x00", item->data, 6)))
	            {
	                tagbExist = 1;
	            }
	        }
	        if(0 == tagbExist)
	        {
	            emvbase_avl_createsettagvalue(EMVTAG_BalanceBeforeGAC, NULL, 0);
	        }
	    }

    	if(emvbase_avl_checkiftagexist(EMVTAG_BalanceAfterGAC))
	    {
	        tagbExist = 0;
	        item = emvbase_avl_gettagitempointer(EMVTAG_BalanceAfterGAC);
	        if(item != NULL)
	        {
	            if((6 == item->len) && (0 == memcmp("\x00\x00\x00\x00\x00\x00", item->data, 6)))
	            {
	                tagbExist = 1;
	            }
	        }
	        if(0 == tagbExist)
	        {
	            emvbase_avl_createsettagvalue(EMVTAG_BalanceAfterGAC, NULL, 0);
	        }
	    }
	}

	#if 1//20191021_lhd
    MaxNumTornLog = emvbase_avl_gettagvalue(EMVTAG_MaxNumTornLog);
    Trace("paypass-info", "Max Number of Torn Transaction Log Records: %d\r\n", MaxNumTornLog);
    if(MaxNumTornLog == 0)
    {
        sdkPaypassTornRecordDeleteAll();
    }
    #endif


    emvbase_avl_gettagvalue_spec(EMVTAG_AmtAuthNum, ucAmtAuthNum, 0, 6);
    TraceHex("paypass-info", "Amount: ", ucAmtAuthNum, 6);

	emvbase_avl_gettagvalue_spec(EMVTAG_clessofflinelimitamt, ucAmtLimit, 0, 6);
    TraceHex("paypass-info", "CL Floor Limit: ", ucAmtLimit, 6);
	if(memcmp(ucAmtAuthNum, ucAmtLimit, 6) > 0)//20200421_lhd
    {
        gstPaypassTradeUnionStruct->PaypassTradeParam->PaypassTermEFLEF = 1;
    }

	emvbase_avl_gettagvalue_spec(EMVTAG_termcvm_limit, ucAmtLimit, 0, 6);
    TraceHex("paypass-info", "CVM Limit: ", ucAmtLimit, 6);
    if(memcmp(ucAmtAuthNum, ucAmtLimit, 6) > 0)
    {
		Trace("paypass-warning", "Amount greater than CVM Limit\r\n");
        gstPaypassTradeUnionStruct->PaypassTradeParam->PaypassCVMRLEF = 1;
    }

	return SDK_OK;
}

s32 sdkPaypassInitialApp()
{
    s32 rlt = SDK_ERR;
	u8 retCode = RLT_EMV_OK;
//	static u8 callbackFlag = 0;

//	if(callbackFlag == 0)
//	{
//		if(0 == sdkPaypassGetCertificationStatus()) // history issue, retain normally
//		{
//			unsigned char TRMData[8] = {0};
//			unsigned char TermCap[3] = {0};
//
//			memset(TRMData, 0, sizeof(TRMData));
//			TermCap[1] = emvbase_avl_gettagvalue(EMVTAG_CVMCapabCVMRequired);
//			Trace("paypass-info", "CVMCapabCVMRequired: 0x%02X\r\n", TermCap[1]);
//			TermCap[1] &= 0x6F;
//			Trace("paypass-info", "EMVTAG_KernelConfiguration=0x%02x", emvbase_avl_gettagvalue(EMVTAG_KernelConfiguration));
//			TraceHex("paypass-info", "TermCap", TermCap, 3);
//			if(emvbase_avl_gettagvalue(EMVTAG_KernelConfiguration) & 0x20)
//			{
//				TermCap[1] |= 0x04;
//			}
//
//			TRMData[0] = TermCap[1];
//			TraceHex("paypass-info", "PAYPASS AID", gstPaypassTradeUnionStruct->PaypassTradeParam->SelectedApp->AID,
//					 gstPaypassTradeUnionStruct->PaypassTradeParam->SelectedApp->AIDLen);
//			if(memcmp(gstPaypassTradeUnionStruct->PaypassTradeParam->SelectedApp->AID, "\xA0\x00\x00\x00\x04\x30\x60", 7) == 0
//					|| memcmp(gstPaypassTradeUnionStruct->PaypassTradeParam->SelectedApp->AID, "\xA0\x00\x00\x00\x04\x22\x03", 7) == 0
//					|| memcmp(gstPaypassTradeUnionStruct->PaypassTradeParam->SelectedApp->AID, "\xA0\x00\x00\x00\x04\x10\x10\xC1\x23\x45\x67\x89", 12) == 0 )	//for L3 test case specially
//			{
//				TRMData[2] = 0x80;
//			}
//			TraceHex("paypass-info", "TRMData", TRMData, 8);
//
//			emvbase_avl_createsettagvalue(EMVTAG_TRMData, TRMData, 8);
//		}
//
////		sdkPaypassLoadParamFromSzztAID();
//
//		callbackFlag++;
//		return EMV_REQ_SET_BEFOREGPO;
//	}
//	else
//	{
//		rlt = sdkPaypassGetBeforeGPORes();
//		Trace("paypass-info", "sdkPaypassGetBeforeGPORes ret = %d\r\n", rlt);
//		callbackFlag = 0;
//	}

	rlt = sdkPaypassPreProcessing();
	Trace("paypass-info", "sdkPaypassPreProcessing ret = %d\r\n", rlt);
	if(rlt != SDK_OK)
	{
		return rlt;
	}

    retCode = Paypass_InitialApp(gstPaypassTradeUnionStruct);
	Trace("paypass-info", "Paypass_InitialApp ret = %d\r\n", retCode);

    sdkEMVBaseRltToSdkRlt(retCode, &rlt);
	Trace("paypass-info", "sdkPaypassInitialApp ret = %d\r\n", rlt);

    return rlt;
}

s32 sdkPaypassRelayResistanceProtocol()
{
    u8 retCode = RLT_EMV_OK;
    s32 rlt = SDK_ERR;


    retCode = Paypass_RelayResistanceProtocol(gstPaypassTradeUnionStruct);
	Trace("paypass-info", "Paypass_RelayResistanceProtocol ret = %d\r\n", retCode);

	sdkEMVBaseRltToSdkRlt(retCode, &rlt);
	Trace("paypass-info", "sdkPaypassRelayResistanceProtocol ret = %d\r\n", rlt);

    return rlt;

}

s32 sdkPaypassReadAppData()
{
    u8 retCode = RLT_EMV_OK;
    s32 rlt = SDK_ERR;


    retCode = Paypass_ReadAppData(gstPaypassTradeUnionStruct);
	Trace("paypass-info", "Paypass_ReadAppData ret = %d\r\n", retCode);

	sdkEMVBaseRltToSdkRlt(retCode, &rlt);
	Trace("paypass-info", "sdkPaypassReadAppData ret = %d\r\n", rlt);

    return rlt;
}

s32 sdkPaypassComputeCryptographicChecksum()
{
    u8 retCode = RLT_EMV_OK;
    s32 rlt = SDK_ERR;


    retCode = Paypass_ComputeCryptographicChecksum(gstPaypassTradeUnionStruct);
	Trace("paypass-info", "Paypass_ComputeCryptographicChecksum ret = %d\r\n", retCode);

	if (gstPaypassTradeUnionStruct->PaypassTradeParam->bPrintReceipt)//20190531_lhd
    {
        gstemvbaseneedsign = 1;
    }
    //gstemvbaseTransFlowModeStatus = (SDK_EMVBASE_TRANSFLOW_MODE)(gstPaypassTradeParam->qPBOCOrMSD);//20190531_lhd
    gstemvbaseCVMresult = sdkPaypassGetCVMresult();

	if((retCode == RLT_EMV_ONLINE_WAIT) || (retCode == RLT_EMV_OFFLINE_DECLINE))
    {
        gstPaypassTradeUnionStruct->PaypassTradeParam->TransResult = retCode;
    }

	sdkEMVBaseRltToSdkRlt(retCode, &rlt);
	Trace("paypass-info", "sdkPaypassComputeCryptographicChecksum ret = %d\r\n", rlt);

    return rlt;
}

s32 sdkPaypassReadCAPK(PAYPASSTradeUnionStruct *pstPaypassTradeParam)
{
    EMVBASE_CAPK_STRUCT *tempcapk;
    EMVBASE_LISTAPPDATA *tempselectedapp;
    unsigned char  RID[5];
    unsigned char CAPKI;
    unsigned char ret;


    if(pstPaypassTradeParam->PaypassTradeParam->CAPK == NULL)
    {
        pstPaypassTradeParam->PaypassTradeParam->CAPK = (EMVBASE_CAPK_STRUCT *)emvbase_malloc(sizeof(EMVBASE_CAPK_STRUCT));
        memset(pstPaypassTradeParam->PaypassTradeParam->CAPK, 0, sizeof(EMVBASE_CAPK_STRUCT));

    }

    tempcapk = pstPaypassTradeParam->PaypassTradeParam->CAPK;
    tempselectedapp = pstPaypassTradeParam->PaypassTradeParam->SelectedApp;

    tempcapk->ModulLen = 0;

    ret = emvbase_avl_gettagvalue_spec(EMVTAG_CAPKI, &CAPKI, 0, 1);
    if(ret != 0)
    {
    	Trace("emv","EMVTAG_CAPKI(8F) does not exsit err !!! ,LINE=%d\r\n",__LINE__);
		TraceHex("emv","RID:", (unsigned char *)tempselectedapp->AID, 5);
        return -1;
    }

    if(tempcapk->ModulLen == 0)
    {
        memcpy(RID, (unsigned char *)tempselectedapp->AID, 5);

        sdkEMVBase_ReadCAPK(RID, CAPKI, tempcapk);
    }

    return SDK_OK;
}

s32 sdkPaypassDataAuth()
{
	u8 retCode = RLT_EMV_ERR;
	s32 rlt = SDK_ERR;


	sdkPaypassReadCAPK(gstPaypassTradeUnionStruct);

	retCode = Paypass_DataAuth(gstPaypassTradeUnionStruct);
	Trace("paypass-info", "Paypass_DataAuth retCode = %d\r\n", retCode);

	sdkEMVBaseRltToSdkRlt(retCode, &rlt);
	Trace("paypass-info", "sdkPaypassDataAuth ret = %d\r\n", rlt);

	return rlt;
}

s32 sdkPaypassPreGenACBalanceRead()
{
	u8 retCode = RLT_EMV_OK;
    s32 rlt = SDK_ERR;

	if(0 == sdkPaypassGetCertificationStatus()) // removed for SB261 but retained in production
	{
	    retCode = Paypass_PreGenACBalanceRead(gstPaypassTradeUnionStruct);
	    Trace("paypass-info", "Paypass_preGenACBalanceRead ret=%d\r\n", retCode);
	}

	sdkEMVBaseRltToSdkRlt(retCode, &rlt);
	Trace("paypass-info","sdkPaypassPreGenACBalanceRead ret = %d\r\n",rlt);

	return rlt;
}

s32 sdkPaypassProcessRestrict()
{
	u8 retCode = RLT_EMV_ERR;
    s32 rlt = SDK_ERR;
	u8 tvr[5];

	retCode = Paypass_ProcessRestrict(gstPaypassTradeUnionStruct);
	Trace("paypass-info","Paypass_ProcessRestrict ret = %d\r\n",retCode);

	if(RLT_EMV_OK != retCode)
	{
		retCode = RLT_EMV_TERMINATE_TRANSERR;
	}

	emvbase_avl_gettagvalue_spec(EMVTAG_TVR, tvr, 0, 5);
	TraceHex("paypass-info", "after ProcessRestrict TVR", tvr,5);

    sdkEMVBaseRltToSdkRlt(retCode, &rlt);
	Trace("paypass-info","sdkPaypassProcessRestrict ret = %d\r\n",rlt);

    return rlt;
}

s32 sdkPaypassCheckFloorLimit()
{
	u8 retCode = RLT_EMV_ERR;
	s32 rlt = SDK_ERR;


	retCode = Paypass_CheckFloorLimit(gstPaypassTradeUnionStruct);
	Trace("paypass-info", "Paypass_CheckFloorLimit retCode = %d\r\n", retCode);

	sdkEMVBaseRltToSdkRlt(retCode, &rlt);
	Trace("paypass-info", "sdkPaypassCheckFloorLimit ret = %d\r\n", rlt);

	return rlt;
}

s32 sdkPaypassCardHolderVerf()
{
    u8 retCode = RLT_EMV_ERR;
    s32 rlt = SDK_ERR;
	//FIXME:temp test
	u8 CVMres[3] = {0};
	s32 len;

    retCode = Paypass_CardHolderVerf(gstPaypassTradeUnionStruct);
    Trace("paypass-info", "Paypass_CardHolderVerf retCode = %d\r\n", retCode);

	if(gstPaypassTradeUnionStruct->PaypassTradeParam->bPrintReceipt)
	{
		gstemvbaseneedsign = 1;
	}

    sdkEMVBaseRltToSdkRlt(retCode, &rlt);
	Trace("paypass-info", "sdkPaypassCardHolderVerf ret = %d\r\n", rlt);

	sdkEMVBaseReadTLV("\x9F\x34", CVMres, &len);
	TraceHex("test", "after Paypass_CardHolderVerf 9F34", CVMres, 3);

	return rlt;
}

s32 sdkPaypassPreGenACPutData()
{
    u8 retCode = RLT_EMV_ERR;
    s32 rlt = SDK_ERR;


    retCode = Paypass_PreGenACPutData(gstPaypassTradeUnionStruct);
    Trace("paypass-info", "Paypass_PreGenACPutData retCode = %d\r\n", retCode);

    sdkEMVBaseRltToSdkRlt(retCode, &rlt);
	Trace("paypass-info", "sdkPaypassPreGenACPutData ret = %d\r\n", rlt);

    return rlt;
}

s32 sdkPaypassTermiAnalys()
{
    u8 retCode = RLT_EMV_ERR;
    s32 rlt = SDK_ERR;
	u8 tvr[5];
	u8 TermAnaResult = 0;


	while(1)
	{
		TermAnaResult = Paypass_TermActAnaly(gstPaypassTradeUnionStruct);

	    if (0 == gsdkPaypassRefundrequestAAC)
	    {
	        if(0x20 == emvbase_avl_gettagvalue(EMVTAG_TransTypeValue))		//refund
	        {
	            TermAnaResult = 0x00;
	            Trace("paypass-info", "Refund-->AAC\r\n");
	        }
	    }

		//gstPaypassTradeUnionStruct->PaypassTradeParam->bretforminit = sdkPaypassGetCertificationStatus();

		if(0 == sdkPaypassGetCertificationStatus()) // removed for SB261 but reserved in production
		{
			retCode = Paypass_Recovery(gstPaypassTradeUnionStruct);
			Trace("paypass-info", "Paypass_Recovery retCode=%d\r\n", retCode);
			if(RLT_EMV_OK != retCode)
			{
				if(RLT_EMV_OTHER == retCode)
				{
					retCode = RLT_EMV_OK;
				}

				break;
		    }
		}

	    retCode = Paypass_TermiAnalys(gstPaypassTradeUnionStruct, TermAnaResult);
	    Trace("paypass-info", "Paypass_TermiAnalys retCode=%d\r\n", retCode);

		break;
	}


	emvbase_avl_gettagvalue_spec(EMVTAG_TVR, tvr, 0, 5);
	TraceHex("paypass-info", "after TermiAnalys TVR", tvr, 5);

	sdkEMVBaseRltToSdkRlt(retCode, &rlt);
	Trace("paypass-info","sdkPaypassTermiAnalys ret = %d\r\n",rlt);

	return rlt;
}

s32 sdkPaypassPostGenACBalanceRead()
{
	u8 retCode = RLT_EMV_OK;
    s32 rlt = SDK_ERR;
	unsigned char bisexist = 0;


	if(0 == sdkPaypassGetCertificationStatus()) // removed for SB261 but reserved in production
	{
	    retCode = Paypass_PostGenACBalanceRead(gstPaypassTradeUnionStruct);
	    Trace("paypass-info", "Paypass_preGenACBalanceRead ret=%d\r\n", retCode);
	}

	sdkEMVBaseRltToSdkRlt(retCode, &rlt);
	Trace("paypass-info","sdkPaypassPreGenACBalanceRead ret = %d\r\n",rlt);

	return rlt;
}

s32 sdkPaypassPostGenACPutData()
{
    u8 retCode = RLT_EMV_ERR;
    s32 rlt = SDK_ERR;


	while(1)
	{
		retCode = Paypass_TransProcess1(gstPaypassTradeUnionStruct);
		Trace("paypass-info", "Paypass_TransProcess1 retCode = %d\r\n", retCode);
		if(RLT_EMV_OK != retCode)
		{
			if(retCode == PAYPASS_REQ_SETCALLBACK_REVOKEY)
			{
				return EMV_REQ_SET_REVOCATIONKEY_CHECK;
			}
			break;
	    }

	    retCode = Paypass_PostGenACPutData(gstPaypassTradeUnionStruct);
	    Trace("paypass-info", "Paypass_PostGenACPutData retCode = %d\r\n", retCode);

		break;
	}

    sdkEMVBaseRltToSdkRlt(retCode, &rlt);
	Trace("paypass-info", "sdkPaypassPostGenACPutData ret = %d\r\n", rlt);

    return rlt;
}

s32 sdkPaypassTransAnalys()
{
	u8 retCode = RLT_EMV_ERR;
	s32 rlt = SDK_ERR;


	while(1)
	{
		retCode = Paypass_TransProcess2(gstPaypassTradeUnionStruct);
		Trace("paypass-info", "Paypass_TransProcess retCode = %d\r\n", retCode);
		if(RLT_EMV_OK != retCode)
		{
			break;
	    }

		retCode = Paypass_OnlinePIN(gstPaypassTradeUnionStruct);
	    Trace("paypass-info", "Paypass_OnlinePIN retCode = %d\r\n", retCode);
		if(RLT_EMV_OK != retCode)
		{
			if(retCode == PAYPASS_REQ_SETCALLBACK_INPUTPIN)
			{
				return EMV_REQ_ONLINE_PIN;
			}
			break;
	    }

		if (gstPaypassTradeUnionStruct->PaypassTradeParam->bPrintReceipt)//20190531_lhd
        {
            gstemvbaseneedsign = 1;
        }
        //gstemvbaseTransFlowModeStatus = (SDK_EMVBASE_TRANSFLOW_MODE)(gstPaypassTradeParam->qPBOCOrMSD);//20190531_lhd
        gstemvbaseCVMresult = sdkPaypassGetCVMresult();

	    Trace("paypass-info", "TransResult after Paypass_TransProcess2 = %d\r\n", gstPaypassTradeParam->TransResult);

		if(gstPaypassTradeParam->TransResult == RLT_EMV_ONLINE_WAIT)
	    {
	        retCode = RLT_EMV_ONLINE_WAIT;
	    }
		else if(gstPaypassTradeParam->TransResult == RLT_EMV_OFFLINE_APPROVE)
        {
            retCode = RLT_EMV_OFFLINE_APPROVE;
        }
        else if(gstPaypassTradeParam->TransResult == RLT_EMV_OFFLINE_DECLINE)
        {
            retCode = RLT_EMV_OFFLINE_DECLINE;
        }
		else if(gstPaypassTradeParam->TransResult == RLT_ERR_EMV_SWITCHINTERFACE)
        {
            retCode = RLT_ERR_EMV_SWITCHINTERFACE;
        }
		else
        {
            retCode = RLT_EMV_TERMINATE_TRANSERR;//disp endapplication, sjz20220815
        }

//		if(gPaypassOutParameterSet.status == PAYPASS_OPS_STATUS_ENDAPPLICATION)//case:3m06-9295_c/3m23-2800_a10;failed to send record twice.
//		{
//			retCode = PAYPASS_ENDAPPLICATION;
//		}
		if(0 == sdkPaypassGetRefundrequestAAC())
        {
            if(emvbase_avl_gettagvalue(EMVTAG_TransTypeValue) == 0x20)		//refund
            {
				gstPaypassTradeParam->TransResult = RLT_EMV_ONLINE_WAIT;
                retCode = RLT_EMV_ONLINE_WAIT;
            }
        }

		break;
	}

	sdkEMVBaseRltToSdkRlt(retCode, &rlt);
	Trace("paypass-info", "sdkPaypassTransAnalys ret = %d\r\n", rlt);

	return rlt;
}

void sdkPaypassTransTermDataInit(void)
{
    gstPaypassTradeParam->SelectedAppNo = 0xff;

    gstPaypassTradeUnionStruct->PaypassTradeParam = gstPaypassTradeParam;
    gstPaypassTradeUnionStruct->IsoCommand = sdkPaypassIsoCommand;
//	gstPaypassTradeUnionStruct->IsoCommandEx = NULL;
    gstPaypassTradeUnionStruct->EMVB_InputCreditPwd = sdkPaypassFunCB;
    gstPaypassTradeUnionStruct->EMVB_RandomNum = sdkPaypassGetRandom;
//	gstPaypassTradeUnionStruct->HashWithSensitiveData = sdkPaypassHashWithSensitiveData;
//	gstPaypassTradeUnionStruct->DealSred = sdkPaypassDealSred;
	gstPaypassTradeUnionStruct->GetPinRes = sdkPaypassGetInputPINRes;
	gstPaypassTradeUnionStruct->GetVerifyCardNoRes = sdkPaypassGetVerifyCardNoRes;
	gstPaypassTradeUnionStruct->GetVerifyCAPKRes = sdkPaypassGetVerifyRevocationKeyRes;

    gstPaypassTradeUnionStruct->getCurMsTimer = sdkTimerGetId;
//    gstPaypassTradeUnionStruct->getCurMsTimer = sdkTimerGetIdtemp;

    gstPaypassTradeUnionStruct->checkifcardintornlog = sdkpaypass_checkifcardintornlog;
    gstPaypassTradeUnionStruct->RECOVERAC = sdkPaypass_RECOVERAC;
    gstPaypassTradeUnionStruct->freetornlogtempdata = sdkPaypass_freetornlogtempdata;
    gstPaypassTradeUnionStruct->deletetornlog = sdkpaypass_deletetornlog;
    gstPaypassTradeUnionStruct->copytornrecorddata = sdkPaypass_copytornrecorddata;
    gstPaypassTradeUnionStruct->addtornlog = sdkPaypass_addtornlog;
    gstPaypassTradeUnionStruct->gettornlogtempdata = sdkPaypass_gettornlogtempdata;

    gucPaypassCCCtimer_startid = 0x00;
    gstPaypassTradeUnionStruct->paypassccctimecnt = &gucPaypassCCCcovertimers;
    gstPaypassTradeUnionStruct->paypassdelay = sdkPaypassCCCtimerWaitTime;
    gstPaypassTradeUnionStruct->paypassCcctimerGetStartId = sdkPaypassCCCtimerGetStartTimeID;//luohuidong 2017.03.21 20:1


    sdkPaypass_phonemessagetable_read(&gPaypassPhoneMessageTableInfo);
    gstPaypassTradeUnionStruct->pphonemsgtable = &gPaypassPhoneMessageTableInfo;


    memset(&gPaypassApduErrorID, 0, sizeof(PAYPASS_APDU_ERRORID));
    memset(&gstPaypassOutComeErrID, 0, sizeof(PAYPASS_OUTCOME_ERRORID));

    gstPaypassTradeUnionStruct->EmvErrID = &gPaypassApduErrorID;
    gstPaypassTradeUnionStruct->pErrorID = &gstPaypassOutComeErrID;

    #ifdef PAYPASS_OUTCOME_SUPPORT_FLAG

    memset(&gPaypassUserInterfaceReqData, 0, sizeof(PAYPASS_USERINTERFACEREQDATA));
    gstPaypassTradeUnionStruct->pUserInterfaceReqData = &gPaypassUserInterfaceReqData;
    #endif
    memset(&gPaypassOutParameterSet, 0, sizeof(PAYPASS_OUTPARAMETERSET));
    gstPaypassTradeUnionStruct->pOutParameterSet = &gPaypassOutParameterSet;

    #ifdef PAYPASS_DATAEXCHANGE
    sdkPaypassInitDEparam();
//    gstPaypassTradeUnionStruct->paypassResvStopSignal = sdkPaypassCheckifCancelCLtrade;
    gstPaypassTradeUnionStruct->paypassResvStopSignal = NULL;
    gstPaypassTradeUnionStruct->ppaypassbIsDoNotSaveCurRecvCMDLog = &(paypassBIsDoNotSaveCurRecvCMDLog);
    gstPaypassTradeUnionStruct->paypassSaveDataExchangeCmd = sdkPaypassManage_savedatachange_cmd;
    gstPaypassTradeUnionStruct->paypassDataExchange = sdkPayPassDataExchangeCmd;
    gstPaypassTradeUnionStruct->paypassGetDekTotalNum = sdkPaypassManageGetDekTotalNum;
    gstPaypassTradeUnionStruct->paypassBisSupportDe = sdkPaypassManageBisSupportDe;
    if (sdkPayPassGetSupportDEstatus())
    {

        #ifdef PAYPASS_DISP_DEK_AND_CMD
        if(sdkPayPassSupportSaveDekLogGetStatus())
        {
            if(sdkPaypassSaveApduCMDfileInitflagGet())
            {
                sdkPaypassSaveApduCMDfileInit();
            }
        }
        #endif
    }
    #endif
}


s32 sdkPaypassTransInitPrivate()
{
    sdkPaypassMalloctradememory();
    sdkPaypassTransTermDataInit();

    return SDK_OK;
}

s32 sdkPaypassTransInit()
{
    static u8 bIsinitTorn = 0;

	gPaypassTransStuatus = 0;
    sdkPaypassTradeParamDestory();
    sdkPaypassTransInitPrivate();

    if(gstEMVBase_TradeParam && gstEMVBase_TradeParam->termipkrevokecheck)
    {
        sdkPaypassSetRevocationKey(gstEMVBase_TradeParam->termipkrevokecheck);
    }

    if (!bIsinitTorn)
    {
        bIsinitTorn = 1;
        sdkPaypassReadTornLog();//20200414_lhd add
    }

    sdkPaypassCreateUN();

    sdkPaypassKernalOutComeInit();

    return SDK_OK;
}


s32 sdkPaypassTransFlow1()
{
    u8 retCode = 0;
    int rlt = 0;

	Trace("emv","goto sdkPaypassTransFlow1\r\n");

    if((gstsdkPaypassTradeTable == NULL) || (gstPaypassTradeParam == NULL) || (gstPaypassTradeUnionStruct == NULL))
    {
        return SDK_PARA_ERR;
    }

    if(gstPaypassTradeUnionStruct->PaypassTradeParam == NULL)
    {
        return SDK_PARA_ERR;
    }

    if(gstEMVBase_UnionStruct != NULL)
    {
        if(gstEMVBase_UnionStruct->rapdu != NULL)
        {
            emvbase_free(gstEMVBase_UnionStruct->rapdu);
            gstEMVBase_UnionStruct->rapdu = NULL;
        }
    }

    Trace("emv", "gstEMVBase_EntryPoint->SelectedApp =%p\r\n", gstEMVBase_EntryPoint->SelectedApp);
    Trace("emv", "gstEMVBase_EntryPoint->AppListCandidate =%p\r\n", gstEMVBase_EntryPoint->AppListCandidate);
    Trace("emv", "gstEMVBase_EntryPoint->AppListCandidatenum =%d\r\n", gstEMVBase_EntryPoint->AppListCandidatenum);
    Trace("emv", "gstEMVBase_UnionStruct->SelectedAppNo =%d\r\n", gstEMVBase_UnionStruct->SelectedAppNo);

    gstPaypassTradeUnionStruct->PaypassTradeParam->SelectedApp = gstEMVBase_EntryPoint->SelectedApp;
    gstPaypassTradeUnionStruct->PaypassTradeParam->AppListCandidate = gstEMVBase_EntryPoint->AppListCandidate;
    gstPaypassTradeUnionStruct->PaypassTradeParam->AppListCandidatenum = gstEMVBase_EntryPoint->AppListCandidatenum;
    gstPaypassTradeUnionStruct->PaypassTradeParam->SelectedAppNo = gstEMVBase_UnionStruct->SelectedAppNo;

	rlt = sdkPaypassDifferEMVBase();

	rlt = sdkPaypassFinalSelectedApp();
	if(rlt != SDK_OK)
    {
        return rlt;
    }

	rlt = sdkPaypassInitialApp();
	if(rlt != SDK_OK)
    {
        return rlt;
    }

	if(TRANSFLOW_EMVMODE == gstPaypassTradeParam->qPBOCOrMSD)
    {
		rlt = sdkPaypassRelayResistanceProtocol();
		if(rlt != SDK_OK)
	    {
	        return rlt;
	    }
	}

	rlt = sdkPaypassReadAppData();
	if(rlt != SDK_OK)
    {
        return rlt;
    }

	if(TRANSFLOW_MSDMODE == gstPaypassTradeParam->qPBOCOrMSD)
    {
		rlt = sdkPaypassComputeCryptographicChecksum();

	    return rlt;
	}

	rlt = sdkPaypassDataAuth();
	if(rlt != SDK_OK)
    {
        return rlt;
    }

	rlt = sdkPaypassPreGenACBalanceRead();
	if(rlt != SDK_OK)
    {
        return rlt;
    }

	rlt = sdkPaypassProcessRestrict();
	if(rlt != SDK_OK)
    {
        return rlt;
    }

	rlt = sdkPaypassCheckFloorLimit();
	if(rlt != SDK_OK)
    {
        return rlt;
    }

	rlt = sdkPaypassCardHolderVerf();
	if(rlt != SDK_OK)
    {
        return rlt;
    }

	rlt = sdkPaypassPreGenACPutData();
	if(rlt != SDK_OK)
    {
        return rlt;
    }

	rlt = sdkPaypassTermiAnalys();
	if(rlt != SDK_OK)
    {
        return rlt;
    }

	rlt = sdkPaypassPostGenACBalanceRead();
	if(rlt != SDK_OK)
    {
        return rlt;
    }

	rlt = sdkPaypassPostGenACPutData();
	if(rlt != SDK_OK)
    {
        return rlt;
    }

	rlt = sdkPaypassTransAnalys();

    return rlt;
}

s32 sdkPaypassImportOnlineResult(s32 ucOnlineResult, u8 *pheRspCode)
{
    if(NULL == gstPaypassTradeParam)
    {
        return SDK_ERR;
    }

    if(ucOnlineResult == SDK_OK)
    {
        gstPaypassTradeParam->onlinetradestatus = RLT_EMV_OK;
        if(pheRspCode != NULL)
        {
            emvbase_avl_createsettagvalue(EMVTAG_AuthRespCode, pheRspCode, 2);
            TraceHex("emv", "App Set AuthCode 8A:", pheRspCode, 2);
        }
    }
    else
    {
        gstPaypassTradeParam->onlinetradestatus = RLT_EMV_ERR;
    }

    Trace("emv", "App Set Online Result:%d,ucOnlineResult=%d\r\n", gstPaypassTradeParam->onlinetradestatus, ucOnlineResult);
    return SDK_OK;
}

s32 sdkPaypassTransFlow2()
{
    u8 retCode = 0;
    int rlt = 0;


	Trace("emv","goto sdkPaypassTransFlow2\r\n");
    Trace("emv", "onlinetradestatus:%d,TransResult=%d\r\n", gstPaypassTradeParam->onlinetradestatus, gstPaypassTradeParam->TransResult);
    if(gstPaypassTradeParam->TransResult == RLT_EMV_ONLINE_WAIT)
    {
        if(gstPaypassTradeParam->onlinetradestatus != RLT_EMV_OK)
        {
            gstPaypassTradeParam->TransResult = RLT_EMV_OFFLINE_DECLINE;
            retCode = RLT_EMV_OFFLINE_DECLINE;
        }
        else
        {
            retCode = Paypass_OnLineDeal(gstPaypassTradeUnionStruct);

            Trace("emv", "ret:%d,TransResult=%d\r\n", retCode, gstPaypassTradeParam->TransResult);

            if(RLT_EMV_OK == retCode)
            {
                retCode = gstPaypassTradeParam->TransResult;
            }
        }
    }
	else
	{
    	retCode = RLT_EMV_OFFLINE_DECLINE;
	}

    sdkEMVBaseRltToSdkRlt(retCode, &rlt);

    return rlt;
}

s32 sdkPaypassTransReadAppData()
{
    u8 retCode=0;
    s32 ret = 0;


    if((gstsdkPaypassTradeTable == NULL) || (gstPaypassTradeParam == NULL)
        || (gstPaypassTradeUnionStruct == NULL) || (gstEMVBase_UnionStruct == NULL)
        || (gstPaypassTradeUnionStruct->PaypassTradeParam == NULL) || (gstEMVBase_EntryPoint == NULL))
    {
        return SDK_PARA_ERR;
    }

    if(gstEMVBase_UnionStruct->rapdu)
    {
        emvbase_free(gstEMVBase_UnionStruct->rapdu);
        gstEMVBase_UnionStruct->rapdu = NULL;
    }

    Trace("emv", "gstEMVBase_EntryPoint->SelectedApp =%p\r\n", gstEMVBase_EntryPoint->SelectedApp);
    Trace("emv", "gstEMVBase_EntryPoint->AppListCandidate =%p\r\n", gstEMVBase_EntryPoint->AppListCandidate);
    Trace("emv", "gstEMVBase_EntryPoint->AppListCandidatenum =%d\r\n", gstEMVBase_EntryPoint->AppListCandidatenum);
    Trace("emv", "gstEMVBase_UnionStruct->SelectedAppNo =%d\r\n", gstEMVBase_UnionStruct->SelectedAppNo);

    gstPaypassTradeUnionStruct->PaypassTradeParam->SelectedApp = gstEMVBase_EntryPoint->SelectedApp;
    gstPaypassTradeUnionStruct->PaypassTradeParam->AppListCandidate = gstEMVBase_EntryPoint->AppListCandidate;
    gstPaypassTradeUnionStruct->PaypassTradeParam->AppListCandidatenum = gstEMVBase_EntryPoint->AppListCandidatenum;
    gstPaypassTradeUnionStruct->PaypassTradeParam->SelectedAppNo = gstEMVBase_UnionStruct->SelectedAppNo;

	ret = sdkPaypassDifferEMVBase();

	ret = sdkPaypassFinalSelectedApp();
	if(ret != SDK_OK)
    {
        return ret;
    }

	ret = sdkPaypassInitialApp();
	if(ret != SDK_OK)
    {
        return ret;
    }

	ret = sdkPaypassRelayResistanceProtocol();
	if(ret != SDK_OK)
    {
        return ret;
    }

	ret = sdkPaypassReadAppData();
	if(ret != SDK_OK)
    {
        return ret;
    }

	return SDK_EMV_TransReadAppDataComplete;
}

void sdkPaypass_OutCome_SetErrPar(unsigned int step)
{
    if (gstPaypassTradeUnionStruct)
    {
        dllpaypass_SetErrPar(gstPaypassTradeUnionStruct, step);
    }
}

void sdkPaypass_OutCome_packoutsignal(unsigned int step)
{
    if (gstPaypassTradeUnionStruct)
    {
        dllpaypass_packoutsignal(gstPaypassTradeUnionStruct, step);
    }
}

s32 sdkPaypassGetLibCompileTime(u8 *version)
{
	Paypass_GetLibCompileTime(version);
	return SDK_OK;
}

s32 sdkPaypassGetLibVerson(u8 *version)
{
	Paypass_GetLibVersion(version);
	return SDK_OK;
}

s32 sdkPaypassTransFlow()
{
    SDK_EMVBASE_CL_HIGHESTAID tempHighestAID;
	static u8 callbackFlag = 0;
    u8 retCode = 0;
    int ret = 0;

	if(gPaypassTransStuatus == SDK_PAYPASS_STATUS_PPSE)
	{
		ret = sdkEMVBaseEntryPoint(&tempHighestAID);
		if (SDK_EMV_IccReturnErr == ret ||
				SDK_EMV_CardBlock == ret ||
				SDK_EMV_PPSERev6A82 == ret )// ret != 9000
		{
			sdkPaypass_OutCome_SetErrPar(PAYPASSSTEP_S52_7);
			sdkPaypass_OutCome_SetErrPar(PAYPASSSTEP_S52_8);
			sdkPaypass_OutCome_packoutsignal(PAYPASSSTEP_S52_9);
			return AS_ERR;
		}
		else if(SDK_EMV_IccDataFormatErr == ret)
		{
			sdkPaypass_OutCome_SetErrPar(PAYPASSSTEP_S52_11);
			sdkPaypass_OutCome_SetErrPar(PAYPASSSTEP_S52_18);
			sdkPaypass_OutCome_packoutsignal(PAYPASSSTEP_S52_19);
			return AS_FCIFMTILL;
		}
		else if(SDK_EMV_AppTimeOut == ret)
		{
			Trace("emv", "sdkEMVBaseEntryPoint ,ret = %d,L3=%d\r\n", ret, gPaypassApduErrorID.L3);

			if(gPaypassApduErrorID.L3 == EMVB_ERRID_L3_STOP)
			{
				sdkPaypass_OutCome_packoutsignal(PAYPASSSTEP_S52_13);
				return EMV_ENDAPPLICATION;
			}
			else
			{
				Trace("emv", "sdkEMVBaseEntryPoint ,err!!!!  ret = %d\r\n", ret);
				sdkPaypass_OutCome_packoutsignal(PAYPASSSTEP_S52_5);
				return EMV_REQ_READCAARD_AGAIN;
			}
		}
		else if(SDK_EMV_NoAppSel == ret )
		{
			sdkPaypass_OutCome_SetErrPar(PAYPASSSTEP_S51_11_0);
			sdkPaypass_OutCome_packoutsignal(PAYPASSSTEP_S51_12_0);
			return AS_NULCANDIDATE;
		}

		if(SDK_OK == ret)
		{
			if(0 == tempHighestAID.aidlen)
			{
                sdkPaypass_OutCome_SetErrPar(PAYPASSSTEP_S52_14);
                sdkPaypass_OutCome_SetErrPar(PAYPASSSTEP_S52_18);
                sdkPaypass_OutCome_packoutsignal(PAYPASSSTEP_S52_19);
				return EMV_ENDAPPLICATION;
			}

			gPaypassTransStuatus = SDK_PAYPASS_DIFFEMVBASE;
			return EMV_STA_CANDIDATES_BUILT;
		}
		else
		{
			return EMV_GETTERMAPPSERR;
		}
	}


	switch (gPaypassTransStuatus)
	{
		case SDK_PAYPASS_DIFFEMVBASE:
			if((gstsdkPaypassTradeTable == NULL) || (gstPaypassTradeParam == NULL) || (gstPaypassTradeUnionStruct == NULL))
			{
				return EMV_UNINITIALIZED;
			}

			if(gstPaypassTradeUnionStruct->PaypassTradeParam == NULL)
			{
				return EMV_UNINITIALIZED;
			}

			if(gstEMVBase_UnionStruct != NULL)
			{
				if(gstEMVBase_UnionStruct->rapdu != NULL)
				{
					emvbase_free(gstEMVBase_UnionStruct->rapdu);
					gstEMVBase_UnionStruct->rapdu = NULL;
				}
			}

			Trace("emv", "gstEMVBase_EntryPoint->SelectedApp =%p\r\n", gstEMVBase_EntryPoint->SelectedApp);
			Trace("emv", "gstEMVBase_EntryPoint->AppListCandidate =%p\r\n", gstEMVBase_EntryPoint->AppListCandidate);
			Trace("emv", "gstEMVBase_EntryPoint->AppListCandidatenum =%d\r\n", gstEMVBase_EntryPoint->AppListCandidatenum);
			Trace("emv", "gstEMVBase_UnionStruct->SelectedAppNo =%d\r\n", gstEMVBase_UnionStruct->SelectedAppNo);

			gstPaypassTradeUnionStruct->PaypassTradeParam->SelectedApp = gstEMVBase_EntryPoint->SelectedApp;
			gstPaypassTradeUnionStruct->PaypassTradeParam->AppListCandidate = gstEMVBase_EntryPoint->AppListCandidate;
			gstPaypassTradeUnionStruct->PaypassTradeParam->AppListCandidatenum = gstEMVBase_EntryPoint->AppListCandidatenum;
			gstPaypassTradeUnionStruct->PaypassTradeParam->SelectedAppNo = gstEMVBase_UnionStruct->SelectedAppNo;


			ret = sdkPaypassDifferEMVBase();
			Trace("test", "finish sdkPaypassDifferEMVBase\r\n");
			gPaypassTransStuatus = SDK_PAYPASS_STATUS_SELECTAID;
			return EMV_STA_IDLE;

		case SDK_PAYPASS_STATUS_RESELECTAID:
			sdkEMVBaseTransInit();
			//如果支持SPI命令，就把POI information存到8B里去
			sdkPaypassTransInit();
			sdkEMVBaseCreateUnpredictNum();
			memset(&tempHighestAID, 0, sizeof(SDK_EMVBASE_CL_HIGHESTAID));
			ret = sdkEMVBaseReSelectApp(&tempHighestAID);
			if(ret == SDK_OK)
			{
			}
			else if(SDK_EMV_NoAppSel == ret)
			{
				sdkPaypass_OutCome_SetErrPar(PAYPASSSTEP_S51_11_0);
				sdkPaypass_OutCome_packoutsignal(PAYPASSSTEP_S51_12_0);
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ENDAPPLICATION;
			}
			else
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ENDAPPLICATION;
			}

		case SDK_PAYPASS_STATUS_SELECTAID:
			ret = sdkPaypassFinalSelectedApp();
			if(SDK_OK == ret)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_GPO;
				return EMV_STA_APP_SELECTED;
			}
			else if(ret == SDK_EMV_TransTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_REQ_READCAARD_AGAIN;
			}
			else if(ret == SDK_EMV_AppSelectTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_RESELECTAID;
				return EMV_REQ_SELECT_NEXT;
			}
			else if(ret == SDK_EMV_SwitchInterface)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_SWITCH_INTERFACE;
			}
			else if(ret == SDK_PARA_ERR)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_UNINITIALIZED;
			}
			else if(ret == SDK_EMV_TransTerminate)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ENDAPPLICATION;
			}
			else if(ret == SDK_EMV_UseOtherCard)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_USE_OTHERCARD;
			}
			else
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return AS_ERR;
			}

		case SDK_PAYPASS_STATUS_GPO:
			ret = sdkPaypassInitialApp();
			if(SDK_OK == ret)
			{
				if(TRANSFLOW_EMVMODE == gstPaypassTradeParam->qPBOCOrMSD)
				{
					gPaypassTransStuatus = SDK_PAYPASS_STATUS_RRP;
				}
				else
				{
					gPaypassTransStuatus = SDK_PAYPASS_STATUS_READRECORD;
				}
				return EMV_STA_APP_INITIALIZED;
			}
//			else if(ret == EMV_REQ_SET_BEFOREGPO)
//			{
//				return EMV_REQ_SET_BEFOREGPO;
//			}
			else if(ret == SDK_EMV_TransTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_REQ_READCAARD_AGAIN;
			}
			else if(ret == SDK_EMV_AppSelectTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_RESELECTAID;
				return EMV_REQ_SELECT_NEXT;
			}
			else if(ret == SDK_EMV_SwitchInterface)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_SWITCH_INTERFACE;
			}
			else if(ret == SDK_PARA_ERR)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_UNINITIALIZED;
			}
			else if(ret == SDK_EMV_TransTerminate)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ENDAPPLICATION;
			}
			else if(ret == SDK_EMV_UseOtherCard)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_USE_OTHERCARD;
			}
			else
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return IA_ERR;
			}

		case SDK_PAYPASS_STATUS_RRP:
			ret = sdkPaypassRelayResistanceProtocol();
			if(SDK_OK == ret)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_READRECORD;
				return EMV_STA_RRP_COMPLETED;
			}
			else if(ret == EMV_REQ_SET_BEFOREGPO)
			{
				return EMV_REQ_SET_BEFOREGPO;
			}
			else if(ret == SDK_EMV_TransTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_REQ_READCAARD_AGAIN;
			}
			else if(ret == SDK_EMV_AppSelectTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_RESELECTAID;
				return EMV_REQ_SELECT_NEXT;
			}
			else if(ret == SDK_EMV_SwitchInterface)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_SWITCH_INTERFACE;
			}
			else if(ret == SDK_PARA_ERR)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_UNINITIALIZED;
			}
			else if(ret == SDK_EMV_TransTerminate)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ENDAPPLICATION;
			}
			else if(ret == SDK_EMV_UseOtherCard)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_USE_OTHERCARD;
			}
			else
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return CO_RRPERR;
			}

		case SDK_PAYPASS_STATUS_READRECORD:
			ret = sdkPaypassReadAppData();
			if(SDK_OK == ret)
			{
				if(TRANSFLOW_MSDMODE == gstPaypassTradeParam->qPBOCOrMSD)
				{
					gPaypassTransStuatus = SDK_PAYPASS_STATUS_CCC;
				}
				else
				{
					gPaypassTransStuatus = SDK_PAYPASS_STATUS_ODA;
				}
				return EMV_REQ_GETCARDNO;
			}
			else if(ret == SDK_EMV_TransTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_REQ_READCAARD_AGAIN;
			}
			else if(ret == SDK_EMV_AppSelectTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_RESELECTAID;
				return EMV_REQ_SELECT_NEXT;
			}
			else if(ret == SDK_EMV_SwitchInterface)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_SWITCH_INTERFACE;
			}
			else if(ret == SDK_PARA_ERR)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_UNINITIALIZED;
			}
			else if(ret == SDK_EMV_TransTerminate)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ENDAPPLICATION;
			}
			else if(ret == SDK_EMV_UseOtherCard)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_USE_OTHERCARD;
			}
			else
			{
				if(ret == SDK_EMV_TransOfflineDecline)	//Torn trans fail,send out trans result
				{
					gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
					return EMV_DENIALED_OFFLINE;
				}
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return RD_ERR;
			}

		case SDK_PAYPASS_STATUS_CCC:
			ret = sdkPaypassComputeCryptographicChecksum();
			if(SDK_EMV_TransOnlineWait == ret)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_REQ_GO_ONLINE;
			}
			else if(ret == SDK_EMV_TransOfflineDecline)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_DENIALED_OFFLINE;
			}
			else if(ret == SDK_EMV_TransTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_REQ_READCAARD_AGAIN;
			}
			else if(ret == SDK_EMV_AppSelectTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_RESELECTAID;
				return EMV_REQ_SELECT_NEXT;
			}
			else if(ret == SDK_EMV_SwitchInterface)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_SWITCH_INTERFACE;
			}
			else if(ret == SDK_PARA_ERR)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_UNINITIALIZED;
			}
			else if(ret == SDK_EMV_TransTerminate)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ENDAPPLICATION;
			}
			else if(ret == SDK_EMV_UseOtherCard)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_USE_OTHERCARD;
			}
			else
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return CO_CCCERR;
			}

		case SDK_PAYPASS_STATUS_ODA:
			ret = sdkPaypassDataAuth();
			if(SDK_OK == ret)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PREGACBALANCEREAD;
				return EMV_STA_DATA_AUTH_COMPLETED;
			}
			else if(ret == EMV_REQ_SET_BEFOREGPO)
			{
				return EMV_REQ_SET_BEFOREGPO;
			}
			else if(ret == SDK_EMV_TransTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_REQ_READCAARD_AGAIN;
			}
			else if(ret == SDK_EMV_AppSelectTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_RESELECTAID;
				return EMV_REQ_SELECT_NEXT;
			}
			else if(ret == SDK_EMV_SwitchInterface)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_SWITCH_INTERFACE;
			}
			else if(ret == SDK_PARA_ERR)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_UNINITIALIZED;
			}
			else if(ret == SDK_EMV_TransTerminate)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ENDAPPLICATION;
			}
			else if(ret == SDK_EMV_UseOtherCard)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_USE_OTHERCARD;
			}
			else
			{
				if(ret == SDK_EMV_TransOfflineDecline)
				{
					gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
					return EMV_DENIALED_OFFLINE;
				}
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return DA_ERR;
			}

		case SDK_PAYPASS_STATUS_PREGACBALANCEREAD:
			ret = sdkPaypassPreGenACBalanceRead();
			if(SDK_OK == ret)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PROCESSREST;
				return EMV_STA_PREGACBAlANCEREAD_COMPLETED;
			}
			else if(ret == SDK_EMV_TransTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_REQ_READCAARD_AGAIN;
			}
			else if(ret == SDK_EMV_AppSelectTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_RESELECTAID;
				return EMV_REQ_SELECT_NEXT;
			}
			else if(ret == SDK_EMV_SwitchInterface)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_SWITCH_INTERFACE;
			}
			else if(ret == SDK_PARA_ERR)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_UNINITIALIZED;
			}
			else if(ret == SDK_EMV_TransTerminate)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ENDAPPLICATION;
			}
			else if(ret == SDK_EMV_UseOtherCard)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_USE_OTHERCARD;
			}
			else
			{
				if(ret == SDK_EMV_TransOfflineDecline)
				{
					gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
					return EMV_DENIALED_OFFLINE;
				}
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return CO_PREGACBALANCEREADERR;
			}

		case SDK_PAYPASS_STATUS_PROCESSREST:
			ret = sdkPaypassProcessRestrict();
			if(SDK_OK == ret)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_CHECKFLOORLMT;
				return EMV_STA_PROCESS_RESTRICT_COMPLETED;
			}
			else if(ret == SDK_EMV_TransTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_REQ_READCAARD_AGAIN;
			}
			else if(ret == SDK_EMV_AppSelectTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_RESELECTAID;
				return EMV_REQ_SELECT_NEXT;
			}
			else if(ret == SDK_EMV_SwitchInterface)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_SWITCH_INTERFACE;
			}
			else if(ret == SDK_PARA_ERR)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_UNINITIALIZED;
			}
			else if(ret == SDK_EMV_TransTerminate)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ENDAPPLICATION;
			}
			else if(ret == SDK_EMV_UseOtherCard)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_USE_OTHERCARD;
			}
			else
			{
				if(ret == SDK_EMV_TransOfflineDecline)
				{
					gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
					return EMV_DENIALED_OFFLINE;
				}
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return RE_ERR;
			}

		case SDK_PAYPASS_STATUS_CHECKFLOORLMT:
			ret = sdkPaypassCheckFloorLimit();
		case SDK_PAYPASS_STATUS_CVM:
			ret = sdkPaypassCardHolderVerf();
			if(SDK_OK == ret)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PREGACPUTDATA;
				return EMV_STA_CARDHOLDER_VERIFY_COMPLETED;
			}
			else if(ret == SDK_EMV_TransTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_REQ_READCAARD_AGAIN;
			}
			else if(ret == SDK_EMV_AppSelectTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_RESELECTAID;
				return EMV_REQ_SELECT_NEXT;
			}
			else if(ret == SDK_EMV_SwitchInterface)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_SWITCH_INTERFACE;
			}
			else if(ret == SDK_PARA_ERR)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_UNINITIALIZED;
			}
			else if(ret == SDK_EMV_TransTerminate)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ENDAPPLICATION;
			}
			else if(ret == SDK_EMV_UseOtherCard)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_USE_OTHERCARD;
			}
			else
			{
				if(ret == SDK_EMV_TransOfflineDecline)
				{
					gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
					return EMV_DENIALED_OFFLINE;
				}
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return CV_ERR;
			}

		case SDK_PAYPASS_STATUS_PREGACPUTDATA:
			ret = sdkPaypassPreGenACPutData();
		case SDK_PAYPASS_STATUS_TAA:
			ret = sdkPaypassTermiAnalys();
			if(ret == SDK_OK)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_POSTGACBALANCEREAD;
				return EMV_STA_ACTION_ANALYSE_COMPLETED;
			}
			else if(ret == SDK_EMV_TransOnlineWait)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_ISSUERUPDATE;
				return EMV_REQ_GO_ONLINE;
			}
			else if(ret == SDK_EMV_TransOfflineApprove)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ACCEPTED_OFFLINE;
			}
			else if(ret == SDK_EMV_TransOfflineDecline)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ACCEPTED_OFFLINE;
			}
			else if(ret == SDK_EMV_TransOnlineApprove)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ACCEPTED_ONLINE;
			}
			else if(ret == SDK_EMV_TransOnlineDecline)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_DENIALED_ONLINE;
			}
			else if(ret == SDK_EMV_SeePhone)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_SEEPHONE;
			}
			else if(ret == SDK_EMV_TransTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_REQ_READCAARD_AGAIN;
			}
			else if(ret == SDK_EMV_AppSelectTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_RESELECTAID;
				return EMV_REQ_SELECT_NEXT;
			}
			else if(ret == SDK_EMV_SwitchInterface)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_SWITCH_INTERFACE;
			}
			else if(ret == SDK_PARA_ERR)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_UNINITIALIZED;
			}
			else if(ret == SDK_EMV_TransTerminate)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ENDAPPLICATION;
			}
			else if(ret == SDK_EMV_UseOtherCard)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_USE_OTHERCARD;
			}
			else
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return AA_ERR;
			}

		case SDK_PAYPASS_STATUS_POSTGACBALANCEREAD:
			ret = sdkPaypassPostGenACBalanceRead();
			gPaypassTransStuatus = SDK_PAYPASS_STATUS_POSTGACPUTDATA;
		case SDK_PAYPASS_STATUS_POSTGACPUTDATA:
			ret = sdkPaypassPostGenACPutData();
			if(ret == SDK_OK)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_TRANSANALYS;
				return EMV_STA_POSTGACPUTDATA_COMPLETED;
			}
			else if(ret == EMV_REQ_SET_REVOCATIONKEY_CHECK)
			{
				return EMV_REQ_SET_REVOCATIONKEY_CHECK;
			}
			else if(ret == SDK_EMV_TransOnlineWait)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_ISSUERUPDATE;
				return EMV_REQ_GO_ONLINE;
			}
			else if(ret == SDK_EMV_TransOfflineApprove)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ACCEPTED_OFFLINE;
			}
			else if(ret == SDK_EMV_TransOfflineDecline)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ACCEPTED_OFFLINE;
			}
			else if(ret == SDK_EMV_TransOnlineApprove)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ACCEPTED_ONLINE;
			}
			else if(ret == SDK_EMV_TransOnlineDecline)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_DENIALED_ONLINE;
			}
			else if(ret == SDK_EMV_SeePhone)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_SEEPHONE;
			}
			else if(ret == SDK_EMV_TransTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_REQ_READCAARD_AGAIN;
			}
			else if(ret == SDK_EMV_AppSelectTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_RESELECTAID;
				return EMV_REQ_SELECT_NEXT;
			}
			else if(ret == SDK_EMV_SwitchInterface)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_SWITCH_INTERFACE;
			}
			else if(ret == SDK_PARA_ERR)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_UNINITIALIZED;
			}
			else if(ret == SDK_EMV_TransTerminate)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ENDAPPLICATION;
			}
			else if(ret == SDK_EMV_UseOtherCard)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_USE_OTHERCARD;
			}
			else
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return CO_POSTGACPUTDATAERR;
			}

		case SDK_PAYPASS_STATUS_TRANSANALYS:
			ret = sdkPaypassTransAnalys();
			if(ret == SDK_EMV_TransOnlineWait)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_ISSUERUPDATE;
				return EMV_REQ_GO_ONLINE;
			}
			else if(ret == EMV_REQ_ONLINE_PIN)
			{
				return EMV_REQ_ONLINE_PIN;
			}
			else if(ret == SDK_EMV_TransOfflineApprove)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ACCEPTED_OFFLINE;
			}
			else if(ret == SDK_EMV_TransOfflineDecline)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ACCEPTED_OFFLINE;
			}
			else if(ret == SDK_EMV_TransOnlineApprove)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ACCEPTED_ONLINE;
			}
			else if(ret == SDK_EMV_TransOnlineDecline)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_DENIALED_ONLINE;
			}
			else if(ret == SDK_EMV_SeePhone)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_SEEPHONE;
			}
			else if(ret == SDK_EMV_TransTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_REQ_READCAARD_AGAIN;
			}
			else if(ret == SDK_EMV_AppSelectTryAgain)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_RESELECTAID;
				return EMV_REQ_SELECT_NEXT;
			}
			else if(ret == SDK_EMV_SwitchInterface)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_SWITCH_INTERFACE;
			}
			else if(ret == SDK_PARA_ERR)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_UNINITIALIZED;
			}
			else if(ret == SDK_EMV_TransTerminate)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ENDAPPLICATION;
			}
			else if(ret == SDK_EMV_UseOtherCard)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_USE_OTHERCARD;
			}
			else
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return CO_TRANSANALYSERR;
			}

		case SDK_PAYPASS_STATUS_ISSUERUPDATE:
			ret = sdkPaypassTransFlow2();
			if(ret == SDK_EMV_TransOfflineApprove)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ACCEPTED_OFFLINE;
			}
			else if(ret == SDK_EMV_TransOfflineDecline)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_DENIALED_OFFLINE;
			}
			else if(ret == SDK_EMV_TransOnlineApprove)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ACCEPTED_ONLINE;
			}
			else if(ret == SDK_EMV_TransOnlineDecline)
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_DENIALED_ONLINE;
			}
			else
			{
				gPaypassTransStuatus = SDK_PAYPASS_STATUS_PPSE;
				return EMV_ENDAPPLICATION;
			}
	}

}
