#include "dllemvbase.h"
#include "dllpaypass.h"
#include "dllpaypassAes.h"
#include "dllpaypassprivate.h"
#include <math.h>
//FIXME:temp test
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>
//FIXME:temp test


#define SpecificationBulletinNo239	(1)	//0:support 1:not support

#define NEWTAGNAMELEN (3)

#define KERNEL_VERSION "ZT_PAYPASS_Core_V0.0.7"

const unsigned char PaypassReadRecordErrorTag[][2] =
{
    {0x9F, 0x36},
    // {0x9f,0x5d},
    {0, 0}
};

const unsigned char PaypassGACErrorTag[][2] =
{
    {0x9F, 0x6B},
    {0x9F, 0x6E},
    {0, 0}
};

const unsigned char PaypassnotemplatedTag[][2] =
{
    {0x9F, 0x4C},
    {0x9F, 0x70},
    {0x9F, 0x71},
    {0x9F, 0x72},
    {0x9F, 0x73},
    {0x9F, 0x74},
    {0x9F, 0x75},
    {0x9F, 0x76},
    {0x9F, 0x77},
    {0x9F, 0x78},
    {0x9F, 0x79},
    {0, 0}
};

const unsigned char PaypassSupportedValuesForGetDataCmd[][2] =
{
    {0x9F, 0x50},
    {0x9F, 0x70},
    {0x9F, 0x71},
    {0x9F, 0x72},
    {0x9F, 0x73},
    {0x9F, 0x74},
    {0x9F, 0x75},
    {0x9F, 0x76},
    {0x9F, 0x77},
    {0x9F, 0x78},
    {0x9F, 0x79},
    {0x00, 0x00}
};

static const EMVBASETAG PaypassCDAmandatoryTag[] =
{
    {EMVTAG_CAPKI},
    {EMVTAG_ICCPKCert},
    {EMVTAG_ICCPKExp},
    {EMVTAG_IPKCert},
    {EMVTAG_IPKExp},
    {EMVTAG_SDATagList},
    {"\x00\x00\x00"}
};


unsigned char Paypass_GetKernelVerInfo(int machine_code, unsigned char ver[32])
{
	unsigned char ret = RLT_EMV_ERR;

	if(NULL == ver) return ret;

	switch(machine_code)
	{
		case 0xAD://G2+
		case 0x99://G2
		case 0x35://G2M
		case 0x36://G2N
		case 0x1F://G2Q
			strcpy(ver, "MCLKERNEL V3.1.1");
			ret = RLT_EMV_OK;
			break;


		case 0x26://K300
		case 0x28://K300+
		case 0x29://K300+ American
			strcpy(ver, "MCLKERNEL V3.1.1");
			ret = RLT_EMV_OK;
			break;

		default:

			break;
	}

	return ret;
}

static int GetCompileTime(char *pasCompileTime, const char *pasDate, const char *pasTime)
{
	char temp_date[64] = {0},str_year[5] = {0}, str_month[4] = {0}, str_day[3] = {0};
	char temp_time[64] = {0},str_hour[2] = {0}, str_min[2] = {0}, str_sec[2] = {0};
	char en_month[12][4]={ "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	char tempCompileTime[20] = {0};
	int i = 0;

	if(pasCompileTime == NULL || pasDate == NULL || pasTime == NULL)
	{
		return 0;
	}

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

	if(strlen(str_day)==1)
	{
		str_day[1]=str_day[0];
		str_day[0]=0x30;
	}

	sprintf(pasCompileTime, "%s%s%s", str_year, str_month, str_day);
	sprintf(tempCompileTime, " %s", pasTime);
	strcat(pasCompileTime, tempCompileTime);

	return 0;
}

void Paypass_GetLibCompileTime(unsigned char *version)
{
    GetCompileTime(version, __DATE__, __TIME__);
}

void Paypass_GetLibVersion(unsigned char *version)
{
	unsigned char cst_k_ver[] = KERNEL_VERSION;
	memcpy(version, cst_k_ver, sizeof(cst_k_ver));
}

unsigned char Paypass_checkifwrongtag(PAYPASSTradeUnionStruct *tempApp_UnionStruct, unsigned char *tag, unsigned char taglen, EMVBASETAGCVLITEM *retrunitem)
{
    unsigned char tagknown = 0;
    unsigned char present = 0;
    unsigned char empty = 1;
    unsigned char tagprivateclass = 0;
    unsigned char updateRA = 0;
    unsigned char temptag[4];
    unsigned char temptaglen;
    EMVBASETAGCVLITEM *item;
    unsigned char ret = 2;
    int ret1;


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
        updateRA = item->updatecondition & EMVTAGUPDATECONDITION_RA;
        if(item->len)
        {
            empty = 0;
        }
    }

    if(present == 0)  //check if tag known
    {
        ret1 = emvbase_TagBaseLib_read_condition(tag, taglen, EMVTAGTYPE_PAYPASS, 0, 0, retrunitem);
        if(ret1 == 0)
        {
            tagknown = 1;
            updateRA = retrunitem->updatecondition & EMVTAGUPDATECONDITION_RA;
        }
    }
    else
    {
        memcpy(retrunitem, item, sizeof(EMVBASETAGCVLITEM));
    }

    if((tag[0] & 0xc0) == 0xc0)
    {
        tagprivateclass = 1;
    }

	EMVBase_Trace("paypass-info: updateRA = %02x, tagprivateclass = %02x, present = %02x, empty = %02x\n\n", updateRA, tagprivateclass, present, empty);

    if(!(tagknown && tagprivateclass && (updateRA == 0)))
    {
        if(tagknown)
        {
            if((present == 0 || empty) && updateRA)
            {
                ret = 1;
            }
            else
            {
				EMVBase_Trace("paypass-error: Tag already existed1\r\n");
                ret = 0;
            }
        }
        else
        {
            if(present)
            {
                if(empty && updateRA)
                {
                    ret = 1;
                }
                else
                {
					EMVBase_Trace("paypass-error: Tag already existed2\r\n");
                    ret = 0;
                }
            }
        }
    }

    return ret;
}


//0x10是A5模板FinalReadSelectRetData
//0x11是A5底下�?BF0C模板
//0x6F�?F自己的模�?
//0x12 �?7的模�?
//0x70是读记录的模�?
//0x02是CryptochecksumRetData 77的模�?也是GenerateRAC�?7的模板，
//0x00是swaitingforgetdata 直接存的getdata
unsigned char Paypass_ParseAndStoreCardResponse(PAYPASSTradeUnionStruct *tempApp_UnionStruct, unsigned char *DataOut, unsigned short *DataOutIndex, unsigned char *parsebInTable, unsigned char readstep)
{
    unsigned short index, len;
    unsigned char tagindex;	//tmpdata,tmpval,,addvalue,ifemvtag
    unsigned char bInTable;
    unsigned char tag[4], taglen;
    unsigned char ret;
    //unsigned char needdonextstep = 1;
    EMVBASETAGCVLITEM tagitem;


    index = *DataOutIndex;
    bInTable = 0;

    memset(tag, 0, sizeof(tag));
    taglen = 0;
    tag[taglen++] = DataOut[index];
    if((tag[0] & 0x1f) == 0x1f)
    {
        tag[taglen++] = DataOut[index + 1];
        if(tag[1] & 0x80)
        {
            tag[taglen++] = DataOut[index + 2];
        }
    }

	EMVBase_TraceHex("paypass-info: parsing tag: ", tag, taglen);

    if(0x10 == readstep)
    {
        if(0 == memcmp(tag, "\x84\x00\x00\x00", 1))
        {
			EMVBase_Trace("paypass-error: Tag84 included in TagA5\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }
    }
    else if(0x11 == readstep)
    {
        if(0 == memcmp(tag, "\x5F\x2D\x00\x00", 2))
        {
			EMVBase_Trace("paypass-error: Tag5F2D included in TagBF0C\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }
    }
    else if(0x12 == readstep)
    {
        if(0 == memcmp(tag, "\x9F\x69\x00\x00", 2))
        {
			EMVBase_Trace("paypass-error: Tag9F69 shall not return\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }
    }
    else if( 0x6F == readstep )
    {
        if(0 == memcmp(tag, "\x9F\x4D\x00\x00", 2))
        {
			EMVBase_Trace("paypass-error: Tag9F4D included in Tag6F\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }
    }
    else if( 0x70 == readstep )
    {
        if(0 == memcmp(tag, "\x9F\x10\x00\x00", 2))
        {
			EMVBase_Trace("paypass-error: Tag9F10 shall not return\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }
    }
    else if(readstep == 1)
    {
        tagindex = 0;
        while((PaypassReadRecordErrorTag[tagindex][0] != 0) || (PaypassReadRecordErrorTag[tagindex][1] != 0) )
        {
            if((PaypassReadRecordErrorTag[tagindex][0] == tag[0]) && (PaypassReadRecordErrorTag[tagindex][1] == tag[1]) )
            {
            	EMVBase_Trace("paypass-error: Tag%02x%02x shall not return\r\n", tag[0], tag[1]);
                return PAYPASS_CARDDATAERR;
            }
            tagindex++;
        }
    }
    else if(readstep == 2)
    {
        tagindex = 0;
        while((PaypassGACErrorTag[tagindex][0] != 0) || (PaypassGACErrorTag[tagindex][1] != 0) )
        {
            if((PaypassGACErrorTag[tagindex][0] == tag[0]) && (PaypassGACErrorTag[tagindex][1] == tag[1]) )
            {
            	EMVBase_Trace("paypass-error: Tag%02x%02x shall not return\r\n", tag[0], tag[1]);
                return RLT_ERR_EMV_IccDataFormat;
            }
            tagindex++;
        }
    }

    if(0 != readstep )
    {
        tagindex = 0;
        while((PaypassnotemplatedTag[tagindex][0] != 0) || (PaypassnotemplatedTag[tagindex][1] != 0) )
        {
            if((PaypassnotemplatedTag[tagindex][0] == tag[0]) && (PaypassnotemplatedTag[tagindex][1] == tag[1]) )
            {
				EMVBase_Trace("paypass-error: Tag%02x%02x shall not return\r\n", tag[0], tag[1]);
                return RLT_ERR_EMV_IccDataFormat;
            }
            tagindex++;
        }
    }

    ret = Paypass_checkifwrongtag(tempApp_UnionStruct, tag, taglen, &tagitem);
    if(ret == 0)
    {
        return RLT_ERR_EMV_IccDataFormat;
    }
    else if(ret == 2)
    {
        //needdonextstep = 0;
    }
    else
    {
        index += taglen;
        if(EMVBase_ParseExtLen(DataOut, &index, &len))
        {
            EMVBase_Trace("paypass-error: Tag parsing error\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }

         //20160714_lhd forbidon for	3m23-9200-33 (CID len=0)  //if(len)
        {
            if((len > tagitem.maxlen || len < tagitem.minlen) && (tagitem.SupAppType & EMVTAGTYPE_PAYPASS))
            {
            	EMVBase_Trace("paypass-info: Tag value len range: min(%d)-max(%d)\r\n", tagitem.minlen, tagitem.maxlen);
				EMVBase_Trace("paypass-info: Tag value len in card: %d\r\n", len);
                EMVBase_Trace("paypass-error: Tag value len error\r\n");
                return RLT_ERR_EMV_IccDataFormat;
            }

            #if 1//20160714_lhd add for case
            if( (0 == memcmp(tag, "\x94\x00\x00\x00", 1) ) && (len % 4))
            {
				EMVBase_Trace("paypass-error: AFL's len != Multiple of 4\r\n");
                return RLT_ERR_EMV_IccDataFormat;
            }
            #endif
        }

        if( (0 == memcmp(tag, "\x5F\x53\x00\x00", 2) ) && ( 0x6F == readstep ) )
        {
            ;
        }
        else
        {
            emvbase_avl_inserttag(&tagitem, tag, strlen((char *)tag), &DataOut[index], len, NULL);
        }
        index += len;
        bInTable = 1;
    }

    if(bInTable == 0)
    {
        index += taglen;

        if(EMVBase_ParseExtLen(DataOut, &index, &len))
        {
             EMVBase_Trace("paypass-error: Tag parsing error\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }
        index += len;

        bInTable = 1;
    }

    *DataOutIndex = index;
    *parsebInTable = bInTable;

    return RLT_EMV_OK;
}

void Paypass_PackberTLVdata(unsigned char *dstdata,unsigned int *dstdatalen,unsigned char *tag,unsigned char taglen,unsigned char *srcdata,unsigned int srcdatalen)
{
    unsigned int len = 0, pos = 0;
    unsigned int t, j;


    pos = *dstdatalen;
    memcpy((unsigned char *)&dstdata[pos], (unsigned char *)&tag[0], taglen);
    pos += taglen;

    len = srcdatalen;
    if(len < 128)
    {
        dstdata[pos++] = len;
    }
    else
    {
        t = 0;
        while(len)
        {
            t++;
            len = len >> 8;
        }
        dstdata[pos++] = 0x80 | t;
        len = srcdatalen;
        for(j = t; j > 0; j--)
        {
            dstdata[pos + j - 1] = len % 256;
            len = len >> 8;
        }

        pos += t;
    }
    if(srcdata != NULL)
    {
        memcpy(&dstdata[pos], srcdata, srcdatalen);
        pos += srcdatalen;
    }

    *dstdatalen = pos;
}

void dllpaypass_direcionaryiteminit(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
	PAYPASS_OUTPARAMETERSET *tempoutparameterset;

	tempoutparameterset = tempApp_UnionStruct->pOutParameterSet;

	memset(&tempoutparameterset->discretionaryitem, 0, sizeof(DISCRETIONARYITEMITEM_OUTCOME));
}

//20140222
void dllpaypass_packoutsignal(PAYPASSTradeUnionStruct *tempApp_UnionStruct, unsigned int step)
{
    unsigned char temp[10];
    PAYPASS_OUTPARAMETERSET *tempoutparameterset;
    PAYPASS_OUTCOME_ERRORID *temperrid;
    PAYPASS_USERINTERFACEREQDATA *tempuserinterfacereqdata;
    PAYPASS_APDU_ERRORID *tempEmvErrID;


    tempoutparameterset = tempApp_UnionStruct->pOutParameterSet;
    temperrid = tempApp_UnionStruct->pErrorID;
    if( NULL == tempApp_UnionStruct->pOutParameterSet)
    {
        EMVBase_Trace("\r\n paypass-error: dllpaypass_packoutsignal tempApp_UnionStruct->pOutParameterSet == null ! err ! ");
    }
    if( NULL == tempApp_UnionStruct->pErrorID)
    {
        EMVBase_Trace("\r\n paypass-error: dllpaypass_packoutsignal tempApp_UnionStruct->pErrorID == null ! err ! ");
    }
    tempuserinterfacereqdata = tempApp_UnionStruct->pUserInterfaceReqData;
    #ifdef PAYPASS_OUTCOME_SUPPORT_FLAG

    tempEmvErrID = tempApp_UnionStruct->EmvErrID;
    #endif
    memset(temp, 0, sizeof(temp));

    EMVBase_Trace("\r\n paypass-info: dllpaypass_packoutsignal signal step = %d\r\n", step);


    #ifndef	PAYPASS_OUTCOME_SUPPORT_FLAG

    if(step == PAYPASSSTEP_S4_E13_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->L3 = PAYPASS_ERRID_L3_AMOUNTNOTPRESENT;
    }
    else if(step == PAYPASSSTEP_S11_E120_1)
    {
        //20160812_lhd for case 3M25-5702(A03)
        tempuserinterfacereqdata->MessageID = tempApp_UnionStruct->pphonemsgtable->phonemessagetable[tempApp_UnionStruct->PaypassTradeParam->PaypassSelectedPOSCIIID].MESSAGE;

        //		tempoutparameterset->UIRequestOnRestartPresent = 1;
        //		tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_READYTOREAD;
        //		memcpy(tempuserinterfacereqdata->HoldTime, "\x00\x00\x00", 3);
    }
    else if(step == PAYPASSSTEP_S11_E114_5)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
    }
    else if(step == PAYPASSSTEP_S11_E114_4)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_TRYANOTHERINTERFACE;
    }
    else if(step == PAYPASSSTEP_S11_E114_3)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_DECLINED;
    }
    else if(step == PAYPASSSTEP_S11_E114_2)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ONLINE;
    }
    else if(step == PAYPASSSTEP_S11_E114_1)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_APPROVED;
    }
    else if(step == PAYPASSSTEP_S11_E112_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        tempoutparameterset->start = PAYPASS_OPS_START_B;
    }
    else if(step == PAYPASSSTEP_S11_E110_0)
    {
    }
    else if(step == PAYPASSSTEP_S5_E27_2)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAERR;
    }
    else if(step == PAYPASSSTEP_S51_12_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->L2 = PAYPASS_ERRID_L2_EMPTYCANLIST;

    }
    else if(step == PAYPASSSTEP_S52_13)
    {
        temperrid->L3 = PAYPASS_ERRID_L3_STOP;
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;

    }
    else if(step == PAYPASSSTEP_S7_8)
    {
        temperrid->L3 = PAYPASS_ERRID_L3_STOP;
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
    }
    else if(step == PAYPASSSTEP_S16_E7)
    {
        temperrid->L3 = PAYPASS_ERRID_L3_STOP;
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;

    }
    else if(step == PAYPASSSTEP_S9_E15_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        tempoutparameterset->start = PAYPASS_OPS_START_B;

    }
    else if(step == PAYPASSSTEP_S11_E95_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;

    }
    else if(step == PAYPASSSTEP_S9_E10_0)
    {

        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        tempoutparameterset->start = PAYPASS_OPS_START_B;
        //tempoutparameterset->UIRequestOnRestartPresent = 1;
        temperrid->L1 = 1;

    }
    else if(step == PAYPASSSTEP_S10_E53_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;


    }
    else if(step == PAYPASSSTEP_S10_E6_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        tempoutparameterset->start = PAYPASS_OPS_START_B;
        temperrid->L1 = 1;
    }
    else if(step == PAYPASSSTEP_S14_33_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ONLINE;
        tempoutparameterset->CVM = PAYPASS_OPS_CVM_NOCVMREQ;
    }
    else if(step == PAYPASSSTEP_S14_34_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ONLINE;
        tempoutparameterset->CVM = PAYPASS_OPS_CVM_CONFVERIFIED;
        if(tempApp_UnionStruct->PaypassTradeParam->PaypassCVMRLEF)
        {
            //tempoutparameterset->ReceiptinoutParameter = 1;
        }

    }
    else if(step == PAYPASSSTEP_S13_26_0)
    {

        tempoutparameterset->status = PAYPASS_OPS_STATUS_ONLINE;
        emvbase_avl_gettagvalue_spec(EMVTAG_TermCapab, temp, 1, 1);
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error:tempterminfo->TermCapab[1]1 = %02x\r\n", temp[0]);
        #endif
        tempoutparameterset->CVM = temp[0] >> 4;		 //这里需要取实际的CVM方式
        //tempoutparameterset->ReceiptinoutParameter = 1;

    }
    else if(step == PAYPASSSTEP_S13_25_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ONLINE;
        emvbase_avl_gettagvalue_spec(EMVTAG_TermCapab, temp, 1, 1);
        tempoutparameterset->CVM = temp[0] >> 4;
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error:tempterminfo->TermCapab[1] = %02x\r\n", temp[0]);
        #endif
        if((temp[0] & 0xf0) == 0x10)
        {
            //tempoutparameterset->ReceiptinoutParameter = 1;
            tempoutparameterset->CVM = PAYPASS_OPS_CVM_OBTAINSIGNATURE;
            tempApp_UnionStruct->PaypassTradeParam->bPrintReceipt = 1;
        }

    }
    else if(step == PAYPASSSTEP_S13_33_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;

    }
    else if(step == PAYPASSSTEP_S13_45_0)
    {

        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        tempoutparameterset->start = PAYPASS_OPS_START_B;


        //tempuserinterfacereqdata->MessageID = tempApp_UnionStruct->pphonemsgtable->phonemessagetable[tempApp_UnionStruct->PaypassTradeParam->PaypassSelectedPOSCIIID].MESSAGE;

    }
    else if(step == PAYPASSSTEP_S13_43_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_DECLINED;

    }
    else if(step == PAYPASSSTEP_S13_5_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        tempoutparameterset->start = PAYPASS_OPS_START_B;
        temperrid->L1 = 1;

    }
    else if(step == PAYPASSSTEP_S16_E3_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        tempoutparameterset->start = PAYPASS_OPS_START_B;
        temperrid->L1 = 1;

    }
    else if(step == PAYPASSSTEP_S5_E17_2)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAMISSING;
    }
    else if(step == PAYPASSSTEP_S4_E27_2)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->L2 = PAYPASS_ERRID_L2_PARSINGERROR;

    }
    else if(step == PAYPASSSTEP_S4_E15_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_SELECTNEXT;
        tempoutparameterset->start = PAYPASS_OPS_START_C;

        temperrid->L2 = PAYPASS_ERRID_L2_MAXLIMITEXCEEDED;

    }
    else if(step == PAYPASSSTEP_S4_E10_2)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->L2 = PAYPASS_ERRID_L2_STATUSBYTES;
        temperrid->SW12[0] = tempEmvErrID->SW12[0];
        temperrid->SW12[1] = tempEmvErrID->SW12[1];

    }
    else if(step == PAYPASSSTEP_S4_E6_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        tempoutparameterset->start = PAYPASS_OPS_START_B;
        temperrid->L1 = 1;

    }
    else if(step == PAYPASSSTEP_S3_90_2)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;

    }
    else if(step == PAYPASSSTEP_S3_9_2)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_SELECTNEXT;
        tempoutparameterset->start = PAYPASS_OPS_START_C;

    }
    else if(step == PAYPASSSTEP_S3_5_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_TRYAGAIN;
        tempoutparameterset->start = PAYPASS_OPS_START_B;

    }
    else if(step == PAYPASSSTEP_S1_8_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_SELECTNEXT;
        tempoutparameterset->start = PAYPASS_OPS_START_C;
    }
    else if(step == PAYPASSSTEP_S53_15)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;//20170712_lhd
    }
    else if(step == PAYPASSSTEP_S53_5)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_TRYAGAIN;
        tempoutparameterset->start = PAYPASS_OPS_START_B;
    }
    else if(step == PAYPASSSTEP_S52_19)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
    }
    else if(step == PAYPASSSTEP_S52_9)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
    }
    else if(step == PAYPASSSTEP_S52_5)
    {
        temperrid->L1 = 1;
        tempoutparameterset->status = PAYPASS_OPS_STATUS_TRYAGAIN;
        tempoutparameterset->start = PAYPASS_OPS_START_B;
    }
    else if(PAYPASSSTEP_S9_ED8_0 == step)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->L1 = 1;//20160901_lhd add for case 3MX6-5609
        temperrid->SW12[0] = tempEmvErrID->SW12[0];
        temperrid->SW12[1] = tempEmvErrID->SW12[1];

    }
    else if( PAYPASSSTEP_S9_8 == step )//20191123_lhd add for case 3MX6-5609
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->L1 = PAYPASS_ERRID_L1_TIMEOUTERR;//20160901_lhd add for case 3MX6-5609
        temperrid->SW12[0] = tempEmvErrID->SW12[0];
        temperrid->SW12[1] = tempEmvErrID->SW12[1];
    }
    else if( PAYPASSSTEP_S11_E77_0 == step )////20160825_LHD  FOR CASE 3MX6 7099
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAERR;
        temperrid->SW12[0] = tempEmvErrID->SW12[0];
        temperrid->SW12[1] = tempEmvErrID->SW12[1];
    }
    else if(PAYPASSSTEP_S6_3 == step)
    {
        temperrid->L3 = PAYPASS_ERRID_L3_TIMEOUT;
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
    }
    else if(PAYPASSSTEP_S910_ED52_0 == step)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
    }
    else if(PAYPASSSTEP_S910_ED53_0 == step)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
    }
    else if(PAYPASSSTEP_SGAC_ED6 == step)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_IDSDATAERR;
        {
            tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        }
    }
    else if(PAYPASSSTEP_SGAC_ED11 == step)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_IDSNOMATCHAC;
        {
            tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        }
    }

    #else
    if(step == PAYPASSSTEP_S53_8)
    {
        /*if(tempApp_UnionStruct->PaypassTradeParam->SelectSuccessoutmsg)
        {
            tempoutparameterset->opssendflag = 1;
            tempoutparameterset->DisDataPresent = 1;
        }*/
    }
    else if(step == PAYPASSSTEP_S11_E120_2)
    {
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;
        tempoutparameterset->DataRecordPresent = 1;
        #if 1//20160705_lhd add
        tempoutparameterset->UIRequestOnOutPresent = 1;
        #endif

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.emvdataflag = 1;
    }
    else  if(step == PAYPASSSTEP_S4_E13_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->L3 = PAYPASS_ERRID_L3_AMOUNTNOTPRESENT;

        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;
        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.emvdataflag = 1;
    }
    else if(step == PAYPASSSTEP_S11_E120_1)
    {
        #if 1//20160812_lhd for case 3M25-5702(A03)
        tempuserinterfacereqdata->MessageID = tempApp_UnionStruct->pphonemsgtable->phonemessagetable[tempApp_UnionStruct->PaypassTradeParam->PaypassSelectedPOSCIIID].MESSAGE;
        #else
        tempuserinterfacereqdata->MessageID = tempoutparameterset->status;
        #endif
        tempoutparameterset->UIRequestOnRestartPresent = 1;
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_READYTOREAD;
        memcpy(tempuserinterfacereqdata->HoldTime, "\x00\x00\x00", 3);

        tempuserinterfacereqdata->sendMsgFlag = 1;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;
        tempoutparameterset->DataRecordPresent = 1;

        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error:tempuserinterfacereqdata->sendMsgFlagaaa\r\n = %d\r\n", tempuserinterfacereqdata->sendMsgFlag);
        #endif
    }
    else if(step == PAYPASSSTEP_S11_E114_5)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
    }
    else if(step == PAYPASSSTEP_S11_E114_4)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_TRYANOTHERINTERFACE;
    }
    else if(step == PAYPASSSTEP_S11_E114_3)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_DECLINED;
    }
    else if(step == PAYPASSSTEP_S11_E114_2)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ONLINE;
    }
    else if(step == PAYPASSSTEP_S11_E114_1)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_APPROVED;
    }
    else if(step == PAYPASSSTEP_S11_E112_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        tempoutparameterset->start = PAYPASS_OPS_START_B;
    }
    else if(step == PAYPASSSTEP_S11_E110_0)
    {
        tempoutparameterset->DataRecordPresent = 1;
    }
    else if(step == PAYPASSSTEP_S5_E27_2)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAERR;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.errindicationflag = 1;

        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;
        tempuserinterfacereqdata->sendMsgFlag = 1; //20160812_lhd add

    }
    else if(step == PAYPASSSTEP_S51_12_0)
    {

        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        temperrid->L2 = PAYPASS_ERRID_L2_EMPTYCANLIST;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.errindicationflag = 1;

    }
    else if(step == PAYPASSSTEP_S52_13)
    {
        temperrid->L3 = PAYPASS_ERRID_L3_STOP;
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        tempoutparameterset->discretionaryitem.errindicationflag = 0;

    }
    else if(step == PAYPASSSTEP_S7_8)
    {
        temperrid->L3 = PAYPASS_ERRID_L3_STOP;
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.msgdataflag = 1;
    }
    else if(step == PAYPASSSTEP_S16_E7)
    {
        temperrid->L3 = PAYPASS_ERRID_L3_STOP;
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.emvdataflag = 1;
    }
    else if(step == PAYPASSSTEP_S9_E15_0)
    {
        #ifdef TORNLOGDEBUG
        EMVBase_Trace("\r\n paypass-error:send torn log\r\n");
        #endif
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        tempoutparameterset->start = PAYPASS_OPS_START_B;
        tempoutparameterset->UIRequestOnRestartPresent = 1;
        temperrid->L1 = 1;
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_TRYAGAIN;

        tempuserinterfacereqdata->sendMsgFlag = 1;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.emvdataflag = 1;
    }
    else if(step == PAYPASSSTEP_S11_E95_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;

        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;
        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.emvdataflag = 1;
    }
    else if(step == PAYPASSSTEP_S9_E10_0)
    {

        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        tempoutparameterset->start = PAYPASS_OPS_START_B;
        tempoutparameterset->UIRequestOnRestartPresent = 1;
        temperrid->L1 = 1;
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_TRYAGAIN;

        tempuserinterfacereqdata->sendMsgFlag = 1;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.emvdataflag = 1;

    }
    else if(step == PAYPASSSTEP_S10_E53_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.emvdataflag = 1;

    }
    else if(step == PAYPASSSTEP_S10_E6_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        tempoutparameterset->start = PAYPASS_OPS_START_B;
        tempoutparameterset->UIRequestOnRestartPresent = 1;
        temperrid->L1 = 1;
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_TRYAGAIN;

        //temperrid->MsgOnError = PAYPASS_ERRID_MSGONERR_TRYAGAIN;
        tempuserinterfacereqdata->sendMsgFlag = 1;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.emvdataflag = 1;
    }
    else if(step == PAYPASSSTEP_S14_33_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ONLINE;
        tempoutparameterset->CVM = PAYPASS_OPS_CVM_NOCVMREQ;
        tempoutparameterset->DataRecordPresent = 1;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.msgdataflag = 1;
    }
    else if(step == PAYPASSSTEP_S14_34_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ONLINE;
        tempoutparameterset->CVM = PAYPASS_OPS_CVM_CONFVERIFIED;
        if(tempApp_UnionStruct->PaypassTradeParam->PaypassCVMRLEF)
        {
            tempoutparameterset->ReceiptinoutParameter = 1;
        }
        tempoutparameterset->DataRecordPresent = 1;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.msgdataflag = 1;
    }
    else if(step == PAYPASSSTEP_S13_26_0)
    {

        tempoutparameterset->status = PAYPASS_OPS_STATUS_ONLINE;
        emvbase_avl_gettagvalue_spec(EMVTAG_TermCapab, temp, 1, 1);
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error:tempterminfo->TermCapab[1]1 = %02x\r\n", temp[0]);
        #endif
        tempoutparameterset->CVM = temp[0] >> 4;		 //这里需要取实际的CVM方式
        tempoutparameterset->ReceiptinoutParameter = 1;
        tempoutparameterset->DataRecordPresent = 1;

        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;
        tempApp_UnionStruct->PaypassTradeParam->bPrintReceipt = 1;
        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.msgdataflag = 1;

    }
    else if(step == PAYPASSSTEP_S13_25_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ONLINE;
        emvbase_avl_gettagvalue_spec(EMVTAG_TermCapab, temp, 1, 1);
        tempoutparameterset->CVM = temp[0] >> 4;
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error:tempterminfo->TermCapab[1] = %02x\r\n", temp[0]);
        #endif
        if((temp[0] & 0xf0) == 0x10)
        {
            tempoutparameterset->ReceiptinoutParameter = 1;
            tempoutparameterset->CVM = PAYPASS_OPS_CVM_OBTAINSIGNATURE;
            tempApp_UnionStruct->PaypassTradeParam->bPrintReceipt = 1;
        }
        tempoutparameterset->DataRecordPresent = 1;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.msgdataflag = 1;
    }
    else if(step == PAYPASSSTEP_S13_33_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->MsgOnError = tempuserinterfacereqdata->MessageID;

        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.msgdataflag = 1;
    }
    else if(step == PAYPASSSTEP_S13_45_0)
    {

        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        tempoutparameterset->start = PAYPASS_OPS_START_B;
        tempoutparameterset->UIRequestOnRestartPresent = 1;
        tempoutparameterset->DataRecordPresent = 1;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        tempuserinterfacereqdata->sendMsgFlag = 1;

        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_READYTOREAD;
        memcpy(tempuserinterfacereqdata->HoldTime, "\x00\x00\x00", 3);
        #if 0//20160715_lhd add for case 3G27-1010(05)
        tempuserinterfacereqdata->MessageID = tempoutparameterset->status;
        #else
        tempuserinterfacereqdata->MessageID = tempApp_UnionStruct->pphonemsgtable->phonemessagetable[tempApp_UnionStruct->PaypassTradeParam->PaypassSelectedPOSCIIID].MESSAGE;
        //memcpy(tempuserinterfacereqdata->HoldTime,"\x00\x00\x00",3);
        //tempuserinterfacereqdata->Status = tempApp_UnionStruct->pphonemsgtable->phonemessagetable[tempApp_UnionStruct->PaypassTradeParam->PaypassSelectedPOSCIIID].STATUS;
        //tempuserinterfacereqdata->sendMsgFlag = 1;
        #endif

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.msgdataflag = 1;
    }
    else if(step == PAYPASSSTEP_S13_43_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_DECLINED;
        tempoutparameterset->DataRecordPresent = 1;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.msgdataflag = 1;
    }
    else if(step == PAYPASSSTEP_S13_5_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        tempoutparameterset->start = PAYPASS_OPS_START_B;
        tempoutparameterset->UIRequestOnRestartPresent = 1;
        temperrid->L1 = 1;
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_TRYAGAIN;

        tempuserinterfacereqdata->sendMsgFlag = 1;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;
    }
    else if(step == PAYPASSSTEP_S16_E3_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        tempoutparameterset->start = PAYPASS_OPS_START_B;
        tempoutparameterset->UIRequestOnRestartPresent = 1;
        temperrid->L1 = 1;
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_TRYAGAIN;


        tempuserinterfacereqdata->sendMsgFlag = 1;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.emvdataflag = 1;
    }
    else if(step == PAYPASSSTEP_S5_E17_2)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAMISSING;

        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.emvdataflag = 1;
    }
    else if(step == PAYPASSSTEP_S4_E27_2)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        temperrid->L2 = PAYPASS_ERRID_L2_PARSINGERROR;

        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.emvdataflag = 1;
    }
    else if(step == PAYPASSSTEP_S4_E15_0)
    {
        tempoutparameterset->FieldoffinoutParameter = 0xff;
        tempoutparameterset->status = PAYPASS_OPS_STATUS_SELECTNEXT;
        tempoutparameterset->start = PAYPASS_OPS_START_C;

        temperrid->L2 = PAYPASS_ERRID_L2_MAXLIMITEXCEEDED;

        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.emvdataflag = 1;
    }
    else if(step == PAYPASSSTEP_S4_E10_2)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        temperrid->L2 = PAYPASS_ERRID_L2_STATUSBYTES;
        temperrid->SW12[0] = tempEmvErrID->SW12[0];
        temperrid->SW12[1] = tempEmvErrID->SW12[1];

        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.emvdataflag = 1;
    }
    else if(step == PAYPASSSTEP_S4_E6_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        tempoutparameterset->start = PAYPASS_OPS_START_B;
        tempoutparameterset->UIRequestOnRestartPresent = 1;
        temperrid->L1 = 1;
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_TRYAGAIN;
        tempuserinterfacereqdata->sendMsgFlag = 1;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.emvdataflag = 1;
    }
    else if(step == PAYPASSSTEP_S3_90_2)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.errindicationflag = 1;
    }
    else if(step == PAYPASSSTEP_S3_9_2)
    {
        tempoutparameterset->FieldoffinoutParameter = 0xFF;
        tempoutparameterset->status = PAYPASS_OPS_STATUS_SELECTNEXT;
        tempoutparameterset->start = PAYPASS_OPS_START_C;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.errindicationflag = 1;
    }
    else if(step == PAYPASSSTEP_S3_5_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_TRYAGAIN;
        tempoutparameterset->start = PAYPASS_OPS_START_B;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.errindicationflag = 1;
    }
    else if(step == PAYPASSSTEP_S1_8_0)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_SELECTNEXT;
        tempoutparameterset->start = PAYPASS_OPS_START_C;

        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;

        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.errindicationflag = 1;
    }
    else if(step == PAYPASSSTEP_S53_15)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;//20170712_lhd
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;
    }
    else if(step == PAYPASSSTEP_S53_5)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_TRYAGAIN;
        tempoutparameterset->start = PAYPASS_OPS_START_B;
        temperrid->L1 = 1;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;
    }
    else if(step == PAYPASSSTEP_S52_19)
    {
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;
    }
    else if(step == PAYPASSSTEP_S52_9)
    {
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;
    }
    else if(step == PAYPASSSTEP_S52_5)
    {
        temperrid->L1 = 1;
        tempoutparameterset->status = PAYPASS_OPS_STATUS_TRYAGAIN;
        tempoutparameterset->start = PAYPASS_OPS_START_B;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;
    }
    #ifdef PAYPASS_DATAEXCHANGE
    else if(PAYPASSSTEP_S9_ED8_0 == step)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        temperrid->L1 = 1;//20160901_lhd add for case 3MX6-5609
        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAERR;
        temperrid->SW12[0] = tempEmvErrID->SW12[0];
        temperrid->SW12[1] = tempEmvErrID->SW12[1];
        tempoutparameterset->DataRecordPresent = 1;

        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;
        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.emvdataflag = 1;
        //GetTLV(TagOf(Data Record)),
    }
    else if( PAYPASSSTEP_S9_8 == step )//20191123_lhd add for case 3MX6-5609
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        temperrid->L1 = PAYPASS_ERRID_L1_TIMEOUTERR;//20160901_lhd add for case 3MX6-5609
        temperrid->SW12[0] = tempEmvErrID->SW12[0];
        temperrid->SW12[1] = tempEmvErrID->SW12[1];
        tempoutparameterset->DataRecordPresent = 1;

        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;
        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.emvdataflag = 1;
    }
    else if( PAYPASSSTEP_S11_E77_0 == step )////20160825_LHD  FOR CASE 3MX6 7099
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAERR;
        temperrid->SW12[0] = tempEmvErrID->SW12[0];
        temperrid->SW12[1] = tempEmvErrID->SW12[1];
        tempoutparameterset->DataRecordPresent = 1;

        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;
        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.emvdataflag = 1;
        //GetTLV(TagOf(Data Record)),
    }
    else if(PAYPASSSTEP_S6_3 == step)
    {
        temperrid->L3 = PAYPASS_ERRID_L3_TIMEOUT;
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_DECLINED;
        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.msgdataflag = 1;
    }
    else if(PAYPASSSTEP_S910_ED52_0 == step)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;

        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;
        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.emvdataflag = 1;

        tempoutparameterset->DataRecordPresent = 1;

        tempoutparameterset->UIRequestOnOutPresent = 1;
    }
    else if(PAYPASSSTEP_S910_ED53_0 == step)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
        temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;

        tempoutparameterset->opssendflag = 1;
        tempoutparameterset->DisDataPresent = 1;
        dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
        tempoutparameterset->discretionaryitem.emvdataflag = 1;

        tempoutparameterset->UIRequestOnOutPresent = 1;
    }
    else if(PAYPASSSTEP_SGAC_ED6 == step)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_IDSDATAERR;
        {
            tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
            temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
            tempoutparameterset->opssendflag = 1;
            tempoutparameterset->DisDataPresent = 1;

            dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
            tempoutparameterset->discretionaryitem.emvdataflag = 1;

        }
    }
    else if(PAYPASSSTEP_SGAC_ED11 == step)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_IDSNOMATCHAC;
        {
            tempoutparameterset->status = PAYPASS_OPS_STATUS_ENDAPPLICATION;
            temperrid->MsgOnError = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
            tempoutparameterset->opssendflag = 1;
            tempoutparameterset->DisDataPresent = 1;

            dllpaypass_direcionaryiteminit(tempApp_UnionStruct);
            tempoutparameterset->discretionaryitem.emvdataflag = 1;

        }
    }
    #endif




    #if 1//20160705_lhd add 案例 PPS_Mchip1 3M24-5351 do not
    if(tempoutparameterset->DisDataPresent)
    {

        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error: lhd step=%d,pUserInterfaceReqData->MessageID=%d ,", step, tempApp_UnionStruct->pUserInterfaceReqData->MessageID);
        #endif

        if( temperrid->MsgOnError == PAYPASS_USERREQDATA_MSGID_TRYAGAIN )
        {
            tempoutparameterset->UIRequestOnOutPresent = 0;
        }
        else
        {
            tempoutparameterset->UIRequestOnOutPresent = 1;
        }

    }
    #endif

    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-error: DataRecordPresent=%d , status= %d opssendflag=%d DisDataPresent=%d sendMsgFlag=%d\r\n", \
                  tempoutparameterset->DataRecordPresent, tempoutparameterset->status, \
                  tempoutparameterset->opssendflag, tempoutparameterset->DisDataPresent, tempuserinterfacereqdata->sendMsgFlag);
    #endif
    if(tempoutparameterset->DataRecordPresent == 0)
    {
        if(tempoutparameterset->opssendflag)
        {
        	if(tempApp_UnionStruct->sendoutparamset)
            tempApp_UnionStruct->sendoutparamset(step);

            #if 1//20160707_lhd add
            if(tempuserinterfacereqdata->sendMsgFlag)
            {
            	if(tempApp_UnionStruct->senduserinterfacerequestdata)
                tempApp_UnionStruct->senduserinterfacerequestdata(step);
                tempuserinterfacereqdata->sendMsgFlag = 0;
            }
            #endif

            if(tempoutparameterset->DisDataPresent)
            {
            	if(tempApp_UnionStruct->sendDisData)
                tempApp_UnionStruct->sendDisData(0);
            }

            tempoutparameterset->opssendflag = 0;
            tempoutparameterset->DisDataPresent = 0;
        }

        if(tempuserinterfacereqdata->sendMsgFlag)
        {
        	if(tempApp_UnionStruct->senduserinterfacerequestdata)
            tempApp_UnionStruct->senduserinterfacerequestdata(step);
        }
        tempuserinterfacereqdata->sendMsgFlag = 0;


    }
    else
    {
        if(tempoutparameterset->status != PAYPASS_OPS_STATUS_APPROVED && tempoutparameterset->status != PAYPASS_OPS_STATUS_DECLINED &&
                tempoutparameterset->status != PAYPASS_OPS_STATUS_ONLINE)
        {
            #ifdef EMVB_DEBUG
            EMVBase_Trace("\r\n paypass-error:sendMsgFlagkkk\r\n = %d\r\n", tempuserinterfacereqdata->sendMsgFlag);
            #endif
            if(tempoutparameterset->opssendflag)
            {
				if(tempApp_UnionStruct->sendoutparamset)
                tempApp_UnionStruct->sendoutparamset(step);

                #if 1//20160706_lhd add

                if(tempuserinterfacereqdata->sendMsgFlag)
                {
                	if(tempApp_UnionStruct->senduserinterfacerequestdata)
                    tempApp_UnionStruct->senduserinterfacerequestdata(step);
                    tempuserinterfacereqdata->sendMsgFlag = 0;
                }

                #endif

                #ifdef EMVB_DEBUG
                EMVBase_Trace("\r\n paypass-error:tempoutparameterset->DisDataPresent = %d\r\n", tempoutparameterset->DisDataPresent);
                #endif
                if(tempoutparameterset->DisDataPresent)
                {
                	if(tempApp_UnionStruct->sendDisData)
                    tempApp_UnionStruct->sendDisData(0);
                }


                #ifdef EMVB_DEBUG
                EMVBase_Trace("\r\n paypass-error:tempoutparameterset->DataRecordPresent = %d\r\n", tempoutparameterset->DataRecordPresent);
                #endif
                if(tempoutparameterset->DataRecordPresent)
                {
                	if(tempApp_UnionStruct->sendendapplicationdatarecord)
                    tempApp_UnionStruct->sendendapplicationdatarecord();
                }
                #ifdef EMVB_DEBUG
                EMVBase_Trace("\r\n paypass-error:tempuserinterfacereqdata->sendMsgFlageeee = %d\r\n", tempuserinterfacereqdata->sendMsgFlag);
                #endif
                tempoutparameterset->opssendflag = 0;
                tempoutparameterset->DisDataPresent = 0;
                tempoutparameterset->DataRecordPresent = 0;
            }

            #ifdef EMVB_DEBUG
            EMVBase_Trace("\r\n paypass-error:tempuserinterfacereqdata->sendMsgFlag1 = %d\r\n", tempuserinterfacereqdata->sendMsgFlag);
            #endif


            if(tempuserinterfacereqdata->sendMsgFlag)
            {
            	if(tempApp_UnionStruct->senduserinterfacerequestdata)
                tempApp_UnionStruct->senduserinterfacerequestdata(step);

            }
            tempuserinterfacereqdata->sendMsgFlag = 0;
        }
    }
    #endif

}



/*******************************************************************
�? �? sxl
函数名称:
函数功能: 状态码错误时置�?
入口参数: SWA  SWB
�?�?�?
相关调用:
�? �?
修改信息:	   20140222
********************************************************************/
void dllpaypass_SetErrPar(PAYPASSTradeUnionStruct *tempApp_UnionStruct, unsigned int step)
{

    PAYPASS_OUTPARAMETERSET *tempoutparameterset;
    PAYPASS_OUTCOME_ERRORID *temperrid;

    #ifdef PAYPASS_OUTCOME_SUPPORT_FLAG
    PAYPASS_USERINTERFACEREQDATA *tempuserinterfacereqdata;
    PAYPASS_PHONEMESSAGETABLEINFO *tempphonemsgtbl;
    unsigned char temp[10];
    #endif
    PAYPASS_APDU_ERRORID *tempEmvErrID;
    tempoutparameterset = tempApp_UnionStruct->pOutParameterSet;
    temperrid = tempApp_UnionStruct->pErrorID;

    #ifdef PAYPASS_OUTCOME_SUPPORT_FLAG

    tempuserinterfacereqdata = tempApp_UnionStruct->pUserInterfaceReqData;
    tempphonemsgtbl = tempApp_UnionStruct->pphonemsgtable;
    tempEmvErrID = tempApp_UnionStruct->EmvErrID;
    memset(temp, 0, sizeof(temp));
    #endif
    if( NULL == tempApp_UnionStruct->pOutParameterSet)
    {
        EMVBase_Trace("\r\n paypass-error: dllpaypass_SetErrPar tempApp_UnionStruct->pOutParameterSet == null ! err ! ");
    }
    if( NULL == tempApp_UnionStruct->pErrorID)
    {
        EMVBase_Trace("\r\n paypass-error: dllpaypass_SetErrPar tempApp_UnionStruct->pErrorID == null ! err ! ");
    }

    EMVBase_Trace("\r\n paypass-info:dllpaypass_SetErrPar, step = %d\r\n", step);


    #ifndef PAYPASS_OUTCOME_SUPPORT_FLAG
		    if(step == PAYPASSSTEP_S53_13)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_EMPTYCANLIST;
		    }
		    else if(step == PAYPASSSTEP_S52_11)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_PARSINGERROR;
		    }
		    else if(step == PAYPASSSTEP_S52_14)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_EMPTYCANLIST;
		    }
		    else if(step == PAYPASSSTEP_S53_7)
		    {
		        tempoutparameterset->status = PAYPASS_OPS_STATUS_APPROVED;
		    }
		    else if(step == PAYPASSSTEP_S1_7_0)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_PARSINGERROR;
		    }
		    else if(step == PAYPASSSTEP_S1_7_1)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAMISSING;
		    }
		    else if(step == PAYPASSSTEP_S3_5_0)
		    {
		        temperrid->L1 = 1;  //L1 error
		    }
		    else if(step == PAYPASSSTEP_S3_9_1)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_STATUSBYTES;
		        temperrid->SW12[0] = tempEmvErrID->SW12[0];
		        temperrid->SW12[1] = tempEmvErrID->SW12[1];
		    }
		    else if(step == PAYPASSSTEP_S9_E17_0)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_STATUSBYTES;
		        temperrid->SW12[0] = tempEmvErrID->SW12[0];
		        temperrid->SW12[1] = tempEmvErrID->SW12[1];
		    }
		    else if(step == PAYPASSSTEP_S3_12_0)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_PARSINGERROR;
		    }
		    else if(step == PAYPASSSTEP_S3_14_0)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAMISSING;
		    }
		    else if(step == PAYPASSSTEP_S3_E41_0)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAERR;
		    }
		    else if(step == PAYPASSSTEP_S3_E18_0)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_MAGNOTSUP;
		    }
		    else if(step == PAYPASSSTEP_S4_E4_0)
		    {
		        temperrid->L1 = 1;  //L1 error
		    }
		    else if(step == PAYPASSSTEP_S16_E1_0)
		    {
		        temperrid->L1 = 1;  //L1 error
		    }
		    else if(step == PAYPASSSTEP_CVM_E18_0)
		    {
		        tempApp_UnionStruct->PaypassTradeParam->bPrintReceipt = 1;
		        tempoutparameterset->CVM = PAYPASS_OPS_CVM_OBTAINSIGNATURE;
		    }
		    else if(step == PAYPASSSTEP_CVM_E18_1)
		    {
		        tempoutparameterset->CVM = PAYPASS_OPS_CVM_ONLINEPIN;
		    }
		    else if(step == PAYPASSSTEP_CVM_E8_0)
		    {
		        tempoutparameterset->CVM = PAYPASS_OPS_CVM_NOCVMREQ;
		    }
		    else if(step == PAYPASSSTEP_CVM_E4_0)
		    {
		        tempoutparameterset->CVM = PAYPASS_OPS_CVM_CONFVERIFIED;
		    }
		    else if(step == PAYPASSSTEP_S78_20_0)
		    {
		        tempoutparameterset->CVM = PAYPASS_OPS_CVM_CONFVERIFIED;
		    }
		    else if(step == PAYPASSSTEP_S13_10_0)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_STATUSBYTES;
		        temperrid->SW12[0] = tempEmvErrID->SW12[0];
		        temperrid->SW12[1] = tempEmvErrID->SW12[1];
		    }
		    else if(step == PAYPASSSTEP_S13_13_0)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_PARSINGERROR;
		    }
		    else if(step == PAYPASSSTEP_S13_14_4)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAMISSING;
		    }
		    else if(step == PAYPASSSTEP_S14_21_1)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAERR;
		    }
		    else if(step == PAYPASSSTEP_S10_E14_0)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_PARSINGERROR;
		    }
		    else if(step == PAYPASSSTEP_S10_E16_0)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAMISSING;
		    }
		    else if(step == PAYPASSSTEP_S10_E18_0)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAERR;
		    }
		    else if(step == PAYPASSSTEP_S10_E20_0)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_PARSINGERROR;
		    }
		    else if(step == PAYPASSSTEP_S10_E22_0)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAMISSING;
		    }
		    else if(step == PAYPASSSTEP_S10_E24_0)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAERR;
		    }
		    else if(step == PAYPASSSTEP_S11_E46_0)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_CAMFAILED;
		    }
		    else if(step == PAYPASSSTEP_S52_7)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_STATUSBYTES;
		        temperrid->SW12[0] = tempEmvErrID->SW12[0];
		        temperrid->SW12[1] = tempEmvErrID->SW12[1];

		    }
		    else if(step == PAYPASSSTEP_S910_ED9_0)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAMISSING;
		    }
		    else if(step == PAYPASSSTEP_S910_ED11_0)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_IDSREADERR;
		    }
		    else if(step == PAYPASSSTEP_S910_ED19_0)
		    {
		        temperrid->L2 = PAYPASS_ERRID_L2_IDSWRITEERR;
		    }

    #else
    if(step == PAYPASSSTEP_S13_44_0)
    {
        tempuserinterfacereqdata->MessageID = tempphonemsgtbl->phonemessagetable[tempApp_UnionStruct->PaypassTradeParam->PaypassSelectedPOSCIIID].MESSAGE;
        memcpy(tempuserinterfacereqdata->HoldTime, "\x00\x00\x00", 3);
        tempuserinterfacereqdata->Status = tempphonemsgtbl->phonemessagetable[tempApp_UnionStruct->PaypassTradeParam->PaypassSelectedPOSCIIID].STATUS;
        tempuserinterfacereqdata->sendMsgFlag = 1;
    }
    else if(step == PAYPASSSTEP_S13_12_1)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_CLEARDISP;
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_CARDREADSUCCESS;
        memcpy(tempuserinterfacereqdata->HoldTime, "\x00\x00\x00", 3);
        tempuserinterfacereqdata->sendMsgFlag = 1;
    }
    else if(step == PAYPASSSTEP_S11_E113_0)	//message hold time
    {
        tempuserinterfacereqdata->MessageID = tempphonemsgtbl->phonemessagetable[tempApp_UnionStruct->PaypassTradeParam->PaypassSelectedPOSCIIID].MESSAGE;
        memcpy(tempuserinterfacereqdata->HoldTime, PAYPASS_USERREQDATA_MSGHOLDTIME_DEFAULT, 3); //HoldTime的值不�?
        tempuserinterfacereqdata->Status = tempphonemsgtbl->phonemessagetable[tempApp_UnionStruct->PaypassTradeParam->PaypassSelectedPOSCIIID].STATUS;

    }
    else if(step == PAYPASSSTEP_S11_E119_0)
    {
        tempuserinterfacereqdata->sendMsgFlag = 1;
    }
    else if(step == PAYPASSSTEP_S11_E120_3)
    {
        tempuserinterfacereqdata->sendMsgFlag = 1;
    }
    else if(step == PAYPASSSTEP_S11_E115_9)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_CLEARDISP;
        memcpy(tempuserinterfacereqdata->HoldTime, "\x00\x00\x00", 3);
    }
    else if(step == PAYPASSSTEP_S11_E115_8)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_INSERTCARD;
    }
    else if(step == PAYPASSSTEP_S11_E115_7)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_DECLINED;
    }
    else if(step == PAYPASSSTEP_S11_E115_6)
    {
        memcpy(tempuserinterfacereqdata->HoldTime, "\x00\x00\x00", 3);
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_AUTHORISINGPLS;
    }
    else if(step == PAYPASSSTEP_S11_E115_5)
    {
        if(tempoutparameterset->CVM == PAYPASS_OPS_CVM_OBTAINSIGNATURE)
        {
            tempApp_UnionStruct->PaypassTradeParam->bPrintReceipt = 1;
            tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_APPROVEDSIGN;
        }
        else
        {
            tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_APPROVED;
        }
    }
    else if(step == PAYPASSSTEP_S11_E115_4)
    {
        emvbase_avl_gettagvalue_spec(EMVTAG_AppCurcyCode, temp, 0, 2);
        memcpy(tempuserinterfacereqdata->CurrencyCode, temp, 2);
    }
    else if(step == PAYPASSSTEP_S11_E115_3)
    {
    	#if 0 // removed on 2022.01.01
        emvbase_avl_gettagvalue_spec(EMVTAG_BalanceAfterGAC, temp, 0, 6);
        tempuserinterfacereqdata->ValueQualifier = PAYPASS_USERREQDATA_VQ_BALANCE;
        memcpy(tempuserinterfacereqdata->value, temp, 6);
		#endif
    }
    else if(step == PAYPASSSTEP_S11_E115_2)
    {
        memcpy(tempuserinterfacereqdata->HoldTime, PAYPASS_USERREQDATA_MSGHOLDTIME_DEFAULT, 3);
    }
    else if(step == PAYPASSSTEP_S11_E115_1)
    {
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_NOTREADY;

    }
    else if(step == PAYPASSSTEP_S11_E112_0)
    {
        tempuserinterfacereqdata->MessageID = tempphonemsgtbl->phonemessagetable[tempApp_UnionStruct->PaypassTradeParam->PaypassSelectedPOSCIIID].MESSAGE;
        memcpy(tempuserinterfacereqdata->HoldTime, PAYPASS_USERREQDATA_MSGHOLDTIME_DEFAULT, 3);
        tempuserinterfacereqdata->Status = tempphonemsgtbl->phonemessagetable[tempApp_UnionStruct->PaypassTradeParam->PaypassSelectedPOSCIIID].STATUS;
    }
    else if(step == PAYPASSSTEP_S53_13)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_EMPTYCANLIST;
    }
    else if(step == PAYPASSSTEP_S52_11)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_PARSINGERROR;
    }
    else if(step == PAYPASSSTEP_S52_14)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_EMPTYCANLIST;
    }
    else if(step == PAYPASSSTEP_S53_7)
    {
        tempoutparameterset->status = PAYPASS_OPS_STATUS_APPROVED;
    }
    else if(step == PAYPASSSTEP_S1_7_0)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_PARSINGERROR;
    }
    else if(step == PAYPASSSTEP_S1_7_1)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAMISSING;
    }
    else if(step == PAYPASSSTEP_S3_5_0)
    {
        temperrid->L1 = 1;  //L1 error
    }
    else if(step == PAYPASSSTEP_S3_9_1)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_STATUSBYTES;
        temperrid->SW12[0] = tempEmvErrID->SW12[0];
        temperrid->SW12[1] = tempEmvErrID->SW12[1];
    }
    else if(step == PAYPASSSTEP_S9_E17_0)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_STATUSBYTES;
        temperrid->SW12[0] = tempEmvErrID->SW12[0];
        temperrid->SW12[1] = tempEmvErrID->SW12[1];
    }
    else if(step == PAYPASSSTEP_S3_12_0)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_PARSINGERROR;
    }
    else if(step == PAYPASSSTEP_S3_14_0)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAMISSING;
    }
    else if(step == PAYPASSSTEP_S3_E41_0)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAERR;
    }
    else if(step == PAYPASSSTEP_S3_E18_0)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_MAGNOTSUP;
    }
    else if(step == PAYPASSSTEP_S4_E4_0)
    {
        temperrid->L1 = 1;  //L1 error
    }
    else if(step == PAYPASSSTEP_S16_E1_0)
    {
        temperrid->L1 = 1;  //L1 error
    }
    else if(step == PAYPASSSTEP_S3_90_1)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_NOTREADY;
        tempuserinterfacereqdata->sendMsgFlag = 1;
    }
    else if(step == PAYPASSSTEP_S13_32_0)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_NOTREADY;
        tempuserinterfacereqdata->sendMsgFlag = 1;
        memcpy(tempuserinterfacereqdata->HoldTime, PAYPASS_USERREQDATA_MSGHOLDTIME_DEFAULT, 3); //20160714_lhd
    }
    else if(step == PAYPASSSTEP_S4_E5_0)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_TRYAGAIN;
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_READYTOREAD;
        #if 1//20160714_lhd add for case 3m09 9010 a
        tempuserinterfacereqdata->sendMsgFlag = 0;
        #else
        tempuserinterfacereqdata->sendMsgFlag = 1;
        #endif
        memcpy(tempuserinterfacereqdata->HoldTime, "\x00\x00\x00", 3);
    }
    else if(step == PAYPASSSTEP_S16_E2_0)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_TRYAGAIN;
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_READYTOREAD;
        //tempuserinterfacereqdata->sendMsgFlag = 1;
        memcpy(tempuserinterfacereqdata->HoldTime, "\x00\x00\x00", 3);
    }
    else if(step == PAYPASSSTEP_S4_E10_1)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_NOTREADY;
        tempuserinterfacereqdata->sendMsgFlag = 1;
    }
    else if(step == PAYPASSSTEP_S4_E27_1)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_NOTREADY;
        tempuserinterfacereqdata->sendMsgFlag = 1;
    }
    else if(step == PAYPASSSTEP_S5_E17_1)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_NOTREADY;
        tempuserinterfacereqdata->sendMsgFlag = 1;
    }
    else if(step == PAYPASSSTEP_S5_E27_1)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_NOTREADY;
        #if 1//20160714_lhd add for case 3m09 9300 a01
        tempuserinterfacereqdata->sendMsgFlag = 0;
        #else
        tempuserinterfacereqdata->sendMsgFlag = 1;
        #endif
    }
    else if(step == PAYPASSSTEP_S5_E31_0)
    {
        //tempoutparameterset->CVM = PAYPASS_OPS_CVM_OBTAINSIGNATURE;
        tempoutparameterset->ReceiptinoutParameter = 1;
    }
    else if(step == PAYPASSSTEP_CVM_E18_0)
    {
        tempApp_UnionStruct->PaypassTradeParam->bPrintReceipt = 1;
        tempoutparameterset->CVM = PAYPASS_OPS_CVM_OBTAINSIGNATURE;
        tempoutparameterset->ReceiptinoutParameter = 1;
    }
    else if(step == PAYPASSSTEP_CVM_E18_1)
    {
        tempoutparameterset->CVM = PAYPASS_OPS_CVM_ONLINEPIN;
    }
    else if(step == PAYPASSSTEP_CVM_E8_0)
    {
        tempoutparameterset->CVM = PAYPASS_OPS_CVM_NOCVMREQ;
    }
    else if(step == PAYPASSSTEP_CVM_E4_0)
    {
        tempoutparameterset->CVM = PAYPASS_OPS_CVM_CONFVERIFIED;
    }
    else if(step == PAYPASSSTEP_S78_20_0)
    {
        tempoutparameterset->CVM = PAYPASS_OPS_CVM_CONFVERIFIED;
    }
    else if(step == PAYPASSSTEP_S13_4_0)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_TRYAGAIN;
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_READYTOREAD;
        memcpy(tempuserinterfacereqdata->HoldTime, "\x00\x00\x00", 3);
    }
    else if(step == PAYPASSSTEP_S13_10_0)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_STATUSBYTES;
        temperrid->SW12[0] = tempEmvErrID->SW12[0];
        temperrid->SW12[1] = tempEmvErrID->SW12[1];
    }
    else if(step == PAYPASSSTEP_S13_13_0)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_PARSINGERROR;
    }
    else if(step == PAYPASSSTEP_S13_14_4)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAMISSING;
    }
    else if(step == PAYPASSSTEP_S14_21_1)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAERR;
    }
    else if(step == PAYPASSSTEP_S13_42_0)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_DECLINED;
        memcpy(tempuserinterfacereqdata->HoldTime, PAYPASS_USERREQDATA_MSGHOLDTIME_DEFAULT, 3);
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_NOTREADY;
        tempuserinterfacereqdata->sendMsgFlag = 1;
    }
    else if(step == PAYPASSSTEP_S13_23_0)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_CLEARDISP;
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_CARDREADSUCCESS;
        memcpy(tempuserinterfacereqdata->HoldTime, "\x00\x00\x00", 3);
        tempuserinterfacereqdata->sendMsgFlag = 1;
    }
    else if(step == PAYPASSSTEP_S9_E27_0)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_CLEARDISP;
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_CARDREADSUCCESS;
        memcpy(tempuserinterfacereqdata->HoldTime, "\x00\x00\x00", 3);
        tempuserinterfacereqdata->sendMsgFlag = 1;
    }
    else if(step == PAYPASSSTEP_S10_E5_0)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_TRYAGAIN;
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_READYTOREAD;
        memcpy(tempuserinterfacereqdata->HoldTime, "\x00\x00\x00", 3);

    }
    else if(step == PAYPASSSTEP_S10_E14_0)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_PARSINGERROR;
    }
    else if(step == PAYPASSSTEP_S10_E16_0)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAMISSING;
    }
    else if(step == PAYPASSSTEP_S10_E18_0)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAERR;
    }
    else if(step == PAYPASSSTEP_S10_E50_0)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_NOTREADY;
        tempuserinterfacereqdata->sendMsgFlag = 1;
        #if 1//20160713_lhd add for case 3M24-9371(A)
        memcpy(tempuserinterfacereqdata->HoldTime, PAYPASS_USERREQDATA_MSGHOLDTIME_DEFAULT, 3); //HoldTime的值不�?
        #endif
    }
    else if(step == PAYPASSSTEP_S9_E9_0)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_TRYAGAIN;
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_READYTOREAD;
        memcpy(tempuserinterfacereqdata->HoldTime, "\x00\x00\x00", 3);
    }
    else if(step == PAYPASSSTEP_S9_E14_0)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_TRYAGAIN;
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_READYTOREAD;
        memcpy(tempuserinterfacereqdata->HoldTime, "\x00\x00\x00", 3);
    }
    else if(step == PAYPASSSTEP_S10_E20_0)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_PARSINGERROR;
    }
    else if(step == PAYPASSSTEP_S10_E22_0)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAMISSING;
    }
    else if(step == PAYPASSSTEP_S10_E24_0)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAERR;
    }
    else if(step == PAYPASSSTEP_S11_E46_0)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_CAMFAILED;
    }
    else if(step == PAYPASSSTEP_S11_E90_0)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_NOTREADY;
        tempuserinterfacereqdata->sendMsgFlag = 1;
        #if 1//20160707_lhd add for case 3M25 7305 01
        //memcpy(tempuserinterfacereqdata->HoldTime,PAYPASS_USERREQDATA_MSGHOLDTIME_DEFAULT,3);
        #endif
    }
    else if(step == PAYPASSSTEP_S51_11_0)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        memcpy(tempuserinterfacereqdata->HoldTime, PAYPASS_USERREQDATA_MSGHOLDTIME_DEFAULT, 3);
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_NOTREADY;
        tempuserinterfacereqdata->sendMsgFlag = 1;
    }
    else if(step == PAYPASSSTEP_S52_7)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_STATUSBYTES;
        temperrid->SW12[0] = tempEmvErrID->SW12[0];
        temperrid->SW12[1] = tempEmvErrID->SW12[1];

    }
    else if(step == PAYPASSSTEP_S52_18)	//sxl?20150612要显�?个字�?F2D
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        //memcpy(tempuserinterfacereqdata->HoldTime,PAYPASS_USERREQDATA_MSGHOLDTIME_DEFAULT,3);
        #ifdef PAYPASSV303
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_NOTREADY;
        #else
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_IDLE;
        #endif
        tempuserinterfacereqdata->sendMsgFlag = 1;


    }
    else if(step == PAYPASSSTEP_S53_14)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        #ifdef PAYPASSV303
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_NOTREADY;
        #else
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_IDLE;
        #endif
        tempuserinterfacereqdata->sendMsgFlag = 1;
    }
    #ifdef PAYPASSV303
    else if(step == PAYPASSSTEP_S52_8)
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        memcpy(tempuserinterfacereqdata->HoldTime, PAYPASS_USERREQDATA_MSGHOLDTIME_DEFAULT, 3);
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_NOTREADY;
        tempuserinterfacereqdata->sendMsgFlag = 1;
    }
    #endif
    #ifdef PAYPASS_DATAEXCHANGE
    else if(step == PAYPASSSTEP_S9_ED7_0 )
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_NOTREADY;
        tempuserinterfacereqdata->sendMsgFlag = 1;
        //memcpy(tempuserinterfacereqdata->HoldTime,PAYPASS_USERREQDATA_MSGHOLDTIME_DEFAULT,3);//20160707_lhd
    }
    else if(step == PAYPASSSTEP_S910_ED9_0)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_CARDDATAMISSING;
    }
    else if(step == PAYPASSSTEP_S910_ED11_0)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_IDSREADERR;
    }
    else if(step == PAYPASSSTEP_S910_ED19_0)
    {
        temperrid->L2 = PAYPASS_ERRID_L2_IDSWRITEERR;
    }
    else if(step == PAYPASSSTEP_S15_E10)
    {
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_CARDREADSUCCESS;
    }
    else if(PAYPASSSTEP_SGAC_ED20 == step)
    {
        #if 1
        //tempuserinterfacereqdata->MessageID = tempphonemsgtbl->phonemessagetable[tempApp_UnionStruct->PaypassTradeParam->PaypassSelectedPOSCIIID].MESSAGE;
        memcpy(tempuserinterfacereqdata->HoldTime, PAYPASS_USERREQDATA_MSGHOLDTIME_DEFAULT, 3); //HoldTime的值不�?
        tempuserinterfacereqdata->Status = tempphonemsgtbl->phonemessagetable[tempApp_UnionStruct->PaypassTradeParam->PaypassSelectedPOSCIIID].STATUS;
        #else
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_APPROVED;
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_NOTREADY;
        memcpy(tempuserinterfacereqdata->HoldTime, PAYPASS_USERREQDATA_MSGHOLDTIME_DEFAULT, 3);
        #endif
        tempuserinterfacereqdata->sendMsgFlag = 1;
    }
    #endif
    else
    {
        tempuserinterfacereqdata->MessageID = PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD;
        memcpy(tempuserinterfacereqdata->HoldTime, PAYPASS_USERREQDATA_MSGHOLDTIME_DEFAULT, 3);
        tempuserinterfacereqdata->Status = PAYPASS_USERREQDATA_STATUS_IDLE;
        tempuserinterfacereqdata->sendMsgFlag = 1;

    }

    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-info: start send msg process,step=%d sendMsgFlag=%d,MessageID=%d\r\n", step, tempuserinterfacereqdata->sendMsgFlag, tempuserinterfacereqdata->MessageID);
    #endif


    //	for case 3G10-9192-01
    //if((PAYPASSSTEP_S13_44_0 == step)||(PAYPASSSTEP_S13_42_0 == step)||( PAYPASSSTEP_S11_E120_3 == step )||((1 == tempuserinterfacereqdata->sendMsgFlag)&&(tempuserinterfacereqdata->MessageID == PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD)))
    if((PAYPASSSTEP_SGAC_ED20 == step) || (PAYPASSSTEP_S13_42_0 == step) ||
            ( PAYPASSSTEP_S11_E120_3 == step ) || ( (1 == tempuserinterfacereqdata->sendMsgFlag) &&
                    (tempuserinterfacereqdata->MessageID == PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD)))
    {
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-info: do not sendMsgFlag \r\n");
        #endif
        ;
    }
    else if(tempuserinterfacereqdata->sendMsgFlag)
    {
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-info: sendMsgFlag step=%d\r\n", step);
        #endif
		if(tempApp_UnionStruct->senduserinterfacerequestdata)
        tempApp_UnionStruct->senduserinterfacerequestdata(step);
        tempuserinterfacereqdata->sendMsgFlag = 0;
    }

    #endif

}

/*****************************************************************************
** Descriptions:
** Parameters:	   unsigned char *buf
							  unsigned short bufLen
							  unsigned char *T
							  unsigned short *TVLstartOffer
							  unsigned short *TVLlen
							  unsigned char mode
** Returned value:
** Created By:	   luohuidong  2016.03.17
** Remarks: //find =1, not find =0 ,mode=0: tag content have len,=1:not len
*****************************************************************************/
int checkTagIsExistInBuf(unsigned char *buf, unsigned short bufLen, unsigned char *T, unsigned short *TVLstartOffer, unsigned short *TVLlen, unsigned char mode)
{
    unsigned char *PDOL = NULL, tag[4], taglen = 0;
    unsigned short index = 0, len = 0, offer = 0;


    PDOL = buf;

    index = 0;
    while(index < bufLen)
    {
        if(PDOL[index] == 0x00)
        {
            index++;
            continue;
        }

        memset(tag, 0, sizeof(tag));
        taglen = 0;
        offer = index;
        tag[taglen++] = PDOL[index];
        if((tag[0] & 0x1f) == 0x1f)
        {
            tag[taglen++] = PDOL[index + 1];
            if(tag[1] & 0x80)
            {
                tag[taglen++] = PDOL[index + 2];
            }
        }
        index += taglen;
        if( index >= bufLen )
        {
            if(0 == memcmp(tag, T, taglen))
            {
                *TVLlen = (index - offer);
                *TVLstartOffer = offer;
                return 1;
            }
            break;
        }
        len = 0;

        if(1 == mode)
        {
            ;
        }
        else
        {
            EMVBase_ParseExtLenWithMaxLen(PDOL, &index, &len, bufLen);
        }

        if(0 == memcmp(tag, T, taglen))
        {
            *TVLlen = len + (index - offer);
            *TVLstartOffer = offer;
            return 1;
        }
        index += len;
    }

    return 0;
}

int AddToList(unsigned char mode, unsigned char *T, unsigned short L, unsigned char *V, unsigned char *taglist)
{
    EMVBASETAGCVLITEM *item = NULL;
    unsigned char ttryexist, *temp = NULL, addtaglen, inputmode = 0;
    unsigned short tempLen = 0, index = 0, oldtagvlen = 0;
    unsigned short offer = 0, tagvalueLen = 0, iimaxLen = 0;
    unsigned int len = 0, t = 0, j = 0, bisflag = 0;
    int retaddtolist = 0;


    if(NULL == T || NULL == taglist )
    {
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error: AddToList err!!!!!!!!!!!!1\r\n");
        #endif
        return -1;
    }

    addtaglen = (strlen((char *)T) > NEWTAGNAMELEN) ? NEWTAGNAMELEN : (strlen((char *)T));


    temp = (unsigned char *)emvbase_malloc(PAYPASS_DE_LIST_BUF_MAX_LEN + 1);
    if( NULL == temp )
    {
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error: emvbase_malloc 1 err!!!!!!!!!!!!1\r\n");
        #endif
        return -1;
    }


    iimaxLen = ((strlen((char *)taglist) > NEWTAGNAMELEN) ? NEWTAGNAMELEN : (strlen((char *)taglist)));

    #ifdef EMVB_DEBUG
    EMVBase_TraceHex("\r\n paypass-error: AddToList taglist", taglist, iimaxLen);
    #endif



    memset(temp, 0, PAYPASS_DE_LIST_BUF_MAX_LEN);
    tempLen = 0;
    ttryexist = 0;
    item = emvbase_avl_gettagitempointer(taglist);
    if(item != NULL)
    {
        if(item->len > 1 || ((1 == item->len) && (0x00 != item->data[0])))
        {
            ttryexist = 1;
        }
    }
    bisflag = 0;
    tagvalueLen = 0;
    index = 0;
    if(ttryexist)
    {
        #ifdef EMVB_DEBUG
        EMVBase_TraceHex("\r\n paypass-error: AddToList taglist content", item->data, item->len);
        #endif

        if(item->len > PAYPASS_DE_LIST_BUF_MAX_LEN)
        {
            oldtagvlen = PAYPASS_DE_LIST_BUF_MAX_LEN;
        }
        else
        {
            oldtagvlen = item->len;
        }

        #if 0 /*Modify by luohuidong at 2016.10.05  17:50 */
        iimaxLen = ((strlen((char *)taglist) > NEWTAGNAMELEN) ? NEWTAGNAMELEN : (strlen((char *)taglist)));

        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error: AddToList taglist name:\r\n");
        EMVBase_TraceHex("\r\n paypass-error: printFormat", taglist, iimaxLen);
        #endif
        #endif /* if 0 */

        if((0 == memcmp((unsigned char *)EMVTAG_DataNeeded, (unsigned char *)taglist, iimaxLen)) ||
                ((0 == memcmp((unsigned char *)EMVTAG_TagsToRead, (unsigned char *)taglist, iimaxLen))))
        {
            inputmode = 1;
        }
        else
        {
            inputmode = 0;
        }
        offer = 0;
        bisflag = checkTagIsExistInBuf(&item->data[index], oldtagvlen, T, &offer, &tagvalueLen, inputmode);
        if( 1 == bisflag )
        {
            if(offer > 0)
            {
                if(offer > PAYPASS_DE_LIST_BUF_MAX_LEN)
                {
                    offer = PAYPASS_DE_LIST_BUF_MAX_LEN;
                }
                memcpy(temp, &item->data[index], offer);
                tempLen += offer;
            }
            else
            {
                tempLen = 0;
            }
        }
        else
        {
            memcpy(temp, &item->data[index], oldtagvlen);
            tempLen += oldtagvlen;
        }
    }

    if(0x01 & mode)
    {
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error: AddToList need add to list tag ,tag len=%d tempLen=%dT: ", addtaglen, tempLen);
        EMVBase_TraceHex("\r\n paypass-error: printFormat", T, addtaglen);
        #endif
        memcpy(&temp[tempLen], T, addtaglen);
        tempLen += addtaglen;
    }
    if(0x02 & mode)
    {
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error:	AddToList need add to list len , L=%d ", L);
        #endif
        len = L;
        if(len < 128)
        {
            temp[tempLen++] = len;
        }
        else
        {
            t = 0;
            while(len)
            {
                t++;
                len = len >> 8;
            }
            temp[tempLen++] = 0x80 | t;
            len = L;
            for(j = t; j > 0; j--)
            {
                temp[tempLen + j - 1] = len % 256;
                len = len >> 8;
            }
            tempLen += t;
        }
    }
    if(0x04 & mode)
    {
        if(V == NULL)
        {
            emvbase_free(temp);

            #ifdef EMVB_DEBUG
            EMVBase_Trace("\r\n paypass-error: AddToList int put mode=4,but no value, err !!!!!!!2\r\n");
            #endif
            return -1;
        }
        #ifdef EMVB_DEBUG
        EMVBase_TraceHex("\r\n paypass-error: AddToList need add to list value", V, L);
        #endif
        #if 0
        Packbertlvdatatemp(&temp[0], &tempLen, V, L);
        #else
        if((tempLen + L) > PAYPASS_DE_LIST_BUF_MAX_LEN)
        {
            #ifdef EMVB_DEBUG
            EMVBase_Trace("\r\n paypass-error: AddToList LEN TOO LONG err !!!!!!!(tempLen+L)=%d\r\n", (tempLen + L));
            #endif
            L = PAYPASS_DE_LIST_BUF_MAX_LEN - tempLen;
        }
        memcpy(&temp[tempLen], V, L);
        tempLen += L;
        #endif
    }

    if((ttryexist) && ( 1 == bisflag ))
    {
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error: index%d offer%d tagvalueLen%d oldtagvlen=%d\r\n", index, offer, tagvalueLen, oldtagvlen);
        EMVBase_Trace("\r\n paypass-error: AddToList (oldtagvlen-offer-tagvalueLen)=%d ", (oldtagvlen - offer - tagvalueLen));
        #endif
        if( ( (oldtagvlen - offer - tagvalueLen) + tempLen) > PAYPASS_DE_LIST_BUF_MAX_LEN )
        {
            #ifdef EMVB_DEBUG
            EMVBase_Trace("\r\n paypass-error:	AddToList LEN TOO LONG err !!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
            #endif
            memcpy(&temp[tempLen], &item->data[index + offer + tagvalueLen], PAYPASS_DE_LIST_BUF_MAX_LEN - tempLen);
            tempLen = PAYPASS_DE_LIST_BUF_MAX_LEN;
        }
        else
        {
            memcpy(&temp[tempLen], &item->data[index + offer + tagvalueLen], (oldtagvlen - offer - tagvalueLen));
            tempLen += (oldtagvlen - offer - tagvalueLen);
        }
    }

    item = NULL; //20160908_lhd add

    #if 0
    if( 0 == paypassbase_checkIFlongtagdata((unsigned char *)taglist, iimaxLen, EMVTAGTYPE_PAYPASS))
    {
        retaddtolist = paypassbase_avl_createsettagvalue(taglist, temp, tempLen);
    }
    else
    {
        retaddtolist = emvbase_avl_createsettagvalue(taglist, temp, tempLen);
    }
    #else
    retaddtolist = emvbase_avl_createsettagvalue(taglist, temp, tempLen);
    #endif


    emvbase_free(temp);
    temp = NULL;


    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-error:	AddToList inset ret retaddtolist=%d\r\n", retaddtolist);
    #endif


    return 0;

}

/*****************************************************************************
** Descriptions:
** Parameters:	   unsigned char mode
							  unsigned char *taglist1
							  unsigned char *taglist2
** Returned value:
** Created By:	   luohuidong  2016.03.17
** Remarks:
*****************************************************************************/
int AddListToList(unsigned char mode, unsigned char *taglist1, unsigned char *taglist2)
{
    EMVBASETAGCVLITEM *item1 = NULL;
    unsigned char ttryexist1 = 0, *PDOL = NULL, tag[4], taglen = 0, bisonlytag = 0, taglist1len = 0;
    unsigned short tadalen = 0, oldtagvlen = 0, index = 0, len = 0;
    int tagisupdate = 0;
    unsigned char tagexistflag = 0;
    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-error: AddListToList ");
    #endif

    if(NULL == taglist1 || NULL == taglist2 )
    {
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error: AddListToList err!!!!!!!!!!!!1\r\n");
        #endif
        return -1;
    }

    ttryexist1 = 0;
    item1 = emvbase_avl_gettagitempointer(taglist1);
    if(item1 != NULL)
    {
        if(item1->len > 0)
        {
            ttryexist1 = 1;
        }
    }

    if(0 == ttryexist1)
    {
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error: AddListToList taglist1 is empty !!!!!!!!!!!!1 %d\r\n", item1->len);
        #endif
        return -1;
    }

    if(item1->len > PAYPASS_DE_LIST_BUF_MAX_LEN)
    {
        tadalen = PAYPASS_DE_LIST_BUF_MAX_LEN;
    }
    else
    {
        tadalen = item1->len;
    }

    index = 0;
    oldtagvlen = 0;
    bisonlytag = 0;
    taglist1len = (strlen((char *)taglist1) > NEWTAGNAMELEN) ? NEWTAGNAMELEN : (strlen((char *)taglist1));
    if( (3 == taglist1len) && ( 0 == memcmp(taglist1, EMVTAG_TagsToRead, 3) ) )
    {
        bisonlytag = 1;
    }

    PDOL = &item1->data[0];
    while(index < tadalen)
    {
        if( PDOL[index] == 0x00)
        {
            index++;
            continue;
        }

        memset(tag, 0, sizeof(tag));
        taglen = 0;
        tag[taglen++] = PDOL[index];
        if((tag[0] & 0x1f) == 0x1f)
        {
            tag[taglen++] = PDOL[index + 1];
            if(tag[1] & 0x80)
            {
                tag[taglen++] = PDOL[index + 2];
            }
        }
        index += taglen;
        if( index > tadalen )
        {
            break;
        }
        len = 0;
        if(bisonlytag)
        {
            #if 1//for case 3GX2-0830
            tagisupdate = emvbase_TagBaseLib_read_condition(tag, taglen, EMVTAGTYPE_PAYPASS, 0, 0, NULL);
            if(0 == tagisupdate)
            {
                AddToList(0x01, tag, len, &PDOL[index], taglist2);
            }
            #else
            AddToList(0x01, tag, len, &PDOL[index], taglist2);
            #endif
        }
        else
        {
            EMVBase_ParseExtLenWithMaxLen(PDOL, &index, &len, tadalen);
            AddToList(0x07, tag, len, &PDOL[index], taglist2);
        }

        index += len;
    }

    return 0;
}

/*****************************************************************************
** Descriptions:
** Parameters:	   unsigned char mode
							  unsigned char *T
							  unsigned char *taglist
** Returned value:
** Created By:	   luohuidong  2016.03.17
** Remarks:
*****************************************************************************/
int RemoveFromList(unsigned char mode, unsigned char *T, unsigned char *taglist)
{

    EMVBASETAGCVLITEM *item = NULL;
    unsigned char ttryexist, *temp = NULL,/*temp1=NULL,*/addtaglen;
    int tempLen = 0, i = 0;
    unsigned int uitemplen = 0, uilen = 0;
    unsigned short needcopyLen = 0, temp1Len = 0;
    unsigned char *PDOL = NULL, tag[4], taglen = 0;
    unsigned short index = 0, len = 0, tempPDOLlen = 0;
    unsigned char bisneedcpy = 0;

    if(NULL == T || NULL == taglist )
    {
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error: RemoveFromList err!!!!!!!!!!!!1\r\n");
        EMVBase_Trace("\r\n paypass-error: RemoveFromList  taglist=\r\n");
        EMVBase_TraceHex("\r\n paypass-error: printFormat", taglist, ((strlen((char *)taglist) > NEWTAGNAMELEN) ? NEWTAGNAMELEN : (strlen((char *)taglist))));
        #endif
        return -1;
    }



    ttryexist = 0;
    item = emvbase_avl_gettagitempointer(taglist);
    if(item != NULL)
    {
        if(item->len > 0)
        {
            ttryexist = 1;
        }
    }
    addtaglen = (strlen((char *)T) > NEWTAGNAMELEN) ? NEWTAGNAMELEN : (strlen((char *)T));

    if(ttryexist)
    {
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error: RemoveFromList item->len=%d taglist	BUF=\r\n", item->len);
        EMVBase_TraceHex("\r\n paypass-error: printFormat", item->data, item->len);
        #endif
    }
    else
    {
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error: RemoveFromList  taglist=\r\n");
        EMVBase_TraceHex("\r\n paypass-error: printFormat", taglist, ((strlen((char *)taglist) > NEWTAGNAMELEN) ? NEWTAGNAMELEN : (strlen((char *)taglist))));
        EMVBase_Trace("\r\n paypass-error: RemoveFromList aaa taglist is not exsit \r\n");
        #endif
        return -1;
    }

    temp = (unsigned char *)emvbase_malloc(PAYPASS_DE_LIST_BUF_MAX_LEN + 1);
    if( NULL == temp )
    {
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error: emvbase_malloc 4 err!!!!!!!!!!!!1\r\n");
        #endif
        return -1;
    }

    memset(temp, 0, PAYPASS_DE_LIST_BUF_MAX_LEN);


    tempLen = ((item->len > PAYPASS_DE_LIST_BUF_MAX_LEN) ? PAYPASS_DE_LIST_BUF_MAX_LEN : item->len);

    if(1 == mode)
    {
        index = 0;
        len = 0;
        PDOL = &item->data[index];
        tempPDOLlen = tempLen;

        needcopyLen = 0;
        index = 0;
        temp1Len = 0;
        bisneedcpy = 0;
        while(index < tempPDOLlen)
        {
            if( PDOL[index] == 0x00)
            {
                index++;
                continue;
            }

            memset(tag, 0, sizeof(tag));
            taglen = 0;
            tag[taglen++] = PDOL[index];
            if((tag[0] & 0x1f) == 0x1f)
            {
                tag[taglen++] = PDOL[index + 1];
                if(tag[1] & 0x80)
                {
                    tag[taglen++] = PDOL[index + 2];
                }
            }
            index += taglen;
            if(!(index < tempPDOLlen))
            {
                #ifdef EMVB_DEBUG
                EMVBase_Trace("\r\n paypass-error: RemoveFromList (!(index<tempPDOLlen)) ERR !!! \r\n");
                #endif
                break;
            }
            len = 0;

            #ifdef EMVB_DEBUG
            EMVBase_Trace("\r\n paypass-error: remove this tag tempPDOLlen a PDOL=%p %d %d %d \r\n", PDOL, tempPDOLlen, index, len);
            #endif

            EMVBase_ParseExtLenWithMaxLen(PDOL, &index, &len, tempLen);

            #ifdef EMVB_DEBUG
            EMVBase_Trace("\r\n paypass-error: remove this tag tempPDOLlen b PDOL=%p %d %d %d \r\n", PDOL, tempPDOLlen, index, len);
            #endif

            if(0 != memcmp(tag, T, taglen))
            {
                bisneedcpy = 1;
                uitemplen = 0x00;
                uitemplen = temp1Len;
                uilen = 0x00;
                uilen = len;
                Paypass_PackberTLVdata(&temp[0], &uitemplen, tag, taglen, &PDOL[index], uilen);
                temp1Len = (unsigned short)uitemplen;
            }
            else
            {
                #ifdef EMVB_DEBUG
                EMVBase_Trace("\r\n paypass-error: remove this tag tempPDOLlen%d %d %d \r\n", tempPDOLlen, index, len);
                #endif
            }
            index += len;
        }
        if(bisneedcpy)
        {
            needcopyLen = temp1Len;
        }
        else
        {
            temp[0] = 0;
            needcopyLen = 0;
        }
    }
    else
    {
        for(i = 0; i < tempLen; i++)
        {
            if(0 == memcmp(&item->data[i], T, addtaglen))
            {
                break;
            }
        }
        if(i >= tempLen)
        {
            #ifdef EMVB_DEBUG
            EMVBase_Trace("\r\n paypass-error: RemoveFromList bbb  tempLen=%d, i=%d\r\n", tempLen, i);
            #endif
            emvbase_free(temp);


            return -1;
        }
        needcopyLen = 0;
        if(i > 0)
        {
            //20160825_lhd item->data[0] -= addtaglen;
            memcpy(&temp[needcopyLen], &item->data[0], i);
            needcopyLen = i;
            //tempLen -= addtaglen;
        }
        tempLen -= (i + addtaglen);

        if(tempLen > 0)
        {
            memcpy(&temp[needcopyLen], &item->data[i + addtaglen], tempLen);
            needcopyLen += tempLen;
        }
        else if(tempLen == 0)
        {
            //emvbase_avl_deletetag(taglist);
            //needcopyLen=1;
            //temp[0]=0x00;
        }
    }

    //emvbase_avl_deletetag(taglist);  //20160908_lhd add

    if(needcopyLen > 0)
    {
        emvbase_avl_createsettagvalue(taglist, temp, needcopyLen);
    }
    else if(0 == needcopyLen)
    {
        emvbase_avl_createsettagvalue(taglist, NULL, 0);
    }


    #ifdef EMVB_DEBUG

    EMVBase_Trace("\r\n paypass-error: Remove tag name\r\n");
    EMVBase_TraceHex("\r\n paypass-error: printFormat", T, ((strlen((char *)T) > NEWTAGNAMELEN) ? NEWTAGNAMELEN : (strlen((char *)T))));

    EMVBase_Trace("\r\n paypass-error: taglist name=\r\n");
    EMVBase_TraceHex("\r\n paypass-error: printFormat", taglist, ((strlen((char *)taglist) > NEWTAGNAMELEN) ? NEWTAGNAMELEN : (strlen((char *)taglist))));

    EMVBase_Trace("\r\n paypass-error: save this buf back to taglist, tempLen=%d needcopyLen=%d temp:\r\n", tempLen, needcopyLen);
    EMVBase_TraceHex("\r\n paypass-error: printFormat", temp, needcopyLen);
    #endif

    emvbase_free(temp);


    return 0;

}

unsigned char FOReveryTinTagsToReadYet(unsigned char mode)//S3.ED49 SAME S1.D15
{
    unsigned char *TagsToReadYet = NULL;
    EMVBASETAGCVLITEM *item = NULL;
    EMVBASETAGCVLITEM *TagsToReadYetItem = NULL;
    unsigned short index, TagsToReadYetLen, bInTable;
    unsigned char tag[4], taglen, bisdelflag = 0;


    TagsToReadYetItem = emvbase_avl_gettagitempointer(EMVTAG_TagsToReadYet);
    if(TagsToReadYetItem != NULL)
    {
        if(TagsToReadYetItem->len > 0)
        {
            EMVBase_TraceHex("paypass-info: TagsToReadYet: ", TagsToReadYetItem->data, TagsToReadYetItem->len);

            TagsToReadYetLen = TagsToReadYetItem->len;
            if(TagsToReadYetLen > PAYPASS_DE_LIST_BUF_MAX_LEN)
            {
                TagsToReadYetLen = PAYPASS_DE_LIST_BUF_MAX_LEN;
            }

			TagsToReadYet = (unsigned char *)emvbase_malloc(TagsToReadYetLen + 1);
            if(TagsToReadYet == NULL)
            {
                EMVBase_Trace("paypass-info: Memory error\r\n");
                return 0;
            }

            memset(TagsToReadYet, 0, TagsToReadYetLen);
            memcpy(TagsToReadYet, &TagsToReadYetItem->data[0], TagsToReadYetLen);

            index = 0;
            while(index < TagsToReadYetLen)
            {
                if(TagsToReadYet[index] == 0x00)
                {
                    index++;
                    continue;
                }

                memset(tag, 0, sizeof(tag));
                taglen = 0;
                tag[taglen++] = TagsToReadYet[index];
                if((tag[0] & 0x1f) == 0x1f)
                {
                    tag[taglen++] = TagsToReadYet[index + 1];
                    if(tag[1] & 0x80)
                    {
                        tag[taglen++] = TagsToReadYet[index + 2];
                    }
                }

				EMVBase_TraceHex("paypass-info: Tag in TagsToReadYet: ", tag, taglen);

                index += taglen;

                bInTable = 0;

                item = emvbase_avl_gettagitempointer(tag);
                if(item != NULL)
                {
                    if(item->len)//[IsNotEmpty(T)]
                    {
                    	EMVBase_Trace("paypass-info: Is Not Empty\r\n");
                        bInTable = 1;
                    }
                }

                bisdelflag = 0;
                if(bInTable)
                {
                    AddToList(0x07, tag, item->len, item->data, EMVTAG_DataToSend);
                    RemoveFromList(0, tag, EMVTAG_TagsToReadYet);
                    bisdelflag = 1;
                }
                else if(1 == mode)
                {
                    if(emvbase_avl_checkiftagknown(tag))
                    {
                        AddToList(0x05, tag, 1, "\x00", EMVTAG_DataToSend);
                        RemoveFromList(0, tag, EMVTAG_TagsToReadYet);
                        bisdelflag = 1;
                    }
                }
                if( ( 1 == mode ) && ( 0 == bisdelflag ) )
                {
                    RemoveFromList(0, tag, EMVTAG_TagsToReadYet);
                }
            }

            emvbase_free(TagsToReadYet);
            TagsToReadYet = NULL;
        }
    }

    return 0;
}

unsigned char FindaTagInTagList(unsigned char *InputList, unsigned char *Tag, unsigned char TagLen)
{
    EMVBASETAGCVLITEM *item = NULL;
    unsigned char *DOL = NULL;
    unsigned short index = 0;
    unsigned short tempLen = 0;
    unsigned char  tag[3] = {0};
    unsigned char taglen = 0;


    EMVBase_Trace("paypass-info: Find a Tag In TagList\r\n");

    EMVBase_TraceHex("paypass-info: DOL Tag: ", InputList, 3);
	EMVBase_TraceHex("paypass-info: Sub Tag: ", Tag, TagLen);

    item = emvbase_avl_gettagitempointer(InputList);
    if((item != NULL) && item->len)
    {

		EMVBase_TraceHex("paypass-info: DOL: ", item->data, item->data);

        DOL = item->data;
        if(item->len > 1024)
        {
            tempLen = 1024;
        }
        else
        {
            tempLen = item->len;
        }

        index = 0;
        while(index < tempLen)
        {
            if(DOL[index] == 0x00)
            {
                index++;
                continue;
            }

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
            index += taglen;

            index++;//add len

            if(0 == memcmp(tag, Tag, TagLen))
            {
                EMVBase_Trace("paypass-info: included\r\n");
                return 1;
            }
        }
    }

    return 0;
}

unsigned char ForEveryTLinTagList(unsigned char *InputList)
{
    EMVBASETAGCVLITEM *item = NULL;
    unsigned char *DOL = NULL;
    unsigned int index = 0;
    unsigned int tempPDOLlen = 0;
    unsigned char tag[3] = {0};
    unsigned char taglen = 0;
    int tagisupdate = 0;


    EMVBase_Trace("paypass-info: FOR every TL entry in the %02x02x02x\r\n", InputList[0], InputList[1], InputList[2]);

    item = emvbase_avl_gettagitempointer(InputList);
    if((NULL == item) || (0 == item->len))
    {
        return RLT_EMV_OK;
    }

    EMVBase_TraceHex("paypass-info: DOL data: \r\n", item->data, item->len);

    DOL = item->data;

    if(item->len > 1024)
    {
        tempPDOLlen = 1024;
    }
    else
    {
        tempPDOLlen = item->len;
    }

    index = 0;
    while(index < tempPDOLlen)
    {
        if(DOL[index] == 0x00)
        {
            index++;
            continue;
        }

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
        index += taglen;
        index++;//add len

		EMVBase_TraceHex("paypass-info: Tag in DOL: ", tag, taglen);

        if(emvbase_avl_checkiftagempty(tag))
        {
            tagisupdate = emvbase_TagBaseLib_read_condition(tag, taglen, EMVTAGTYPE_PAYPASS, 0, 0, NULL);

            if(0 == tagisupdate)
            {
            	EMVBase_Trace("paypass-info: Tag Is Empty!\r\n");

                AddToList(0x01, tag, 0, NULL, EMVTAG_DataNeeded);
            }
        }
    }

    return RLT_EMV_OK;
}


/*****************************************************************************
** Descriptions:
** Parameters:	   unsigned char *buf1
							  unsigned char *buf2
							  unsigned int len
** Returned value:
** Created By:	   luohuidong  2016.03.17
** Remarks:
*****************************************************************************/
void emv_maths_xor(unsigned char *buf1, unsigned char *buf2, unsigned int len)
{
    unsigned int i;

    for(i = 0; i < len; i++)
    {
        buf1[i] = buf1[i] ^ buf2[i];
    }
}


/*****************************************************************************
** Descriptions:
** Parameters:	   unsigned char input[8]
							  unsigned char output[8]
** Returned value:
** Created By:	   luohuidong  2016.03.17
** Remarks:
*****************************************************************************/
int OWHF2(unsigned char input[8], unsigned char output[8])
{
    unsigned char PL, PD[8], DSID[12], DSPKL[8], DSPKR[8], OID[8], KL[8], KR[8], xorresult[8];
    unsigned char DSSlotManageBisexist = 0, DSSlotManage = 0;
    unsigned char DSODSInfo = 0, DSODSInfoBisexist = 0;

    int i;
    EMVBASETAGCVLITEM *itembisexist = NULL;

    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-error: in OWHF2 \r\n");
    EMVBase_Trace("\r\n paypass-error: input data\r\n");
    EMVBase_TraceHex("\r\n paypass-error: printFormat", input, 8);
    #endif
    memset(DSID, 0, sizeof(DSID));
    memset(DSPKL, 0, sizeof(DSPKL));
    memset(DSPKR, 0, sizeof(DSPKR));

    memset(xorresult, 0, sizeof(xorresult));

    PL = 0;
    memcpy(PD, input, 8);

    itembisexist = emvbase_avl_gettagitempointer(EMVTAG_DSID);
    if(itembisexist != NULL)
    {
        if(itembisexist->len > 0)
        {
            #ifdef EMVB_DEBUG
            EMVBase_Trace("\r\n paypass-error: TAG_DSID get data\r\n");
            EMVBase_TraceHex("\r\n paypass-error: printFormat", itembisexist->data, itembisexist->len);
            #endif
            if(itembisexist->len > 11)
            {
                PL = 11;
            }
            else
            {
                PL = itembisexist->len;
            }
            memcpy(DSID, itembisexist->data, PL);
        }
    }
    if(0 == PL)
    {
        return 1;
    }

    for(i = 0; i < 6; i++)
    {
        DSPKL[i] = ( (DSID[i] / 16) * 10 + (DSID[i] % 16) ) * 2;
        DSPKR[i] = ( (DSID[PL - 6 + i] / 16) * 10 + (DSID[PL - 6 + i] % 16) ) * 2;
    }

    itembisexist = emvbase_avl_gettagitempointer(EMVTAG_DSSlotManagementControl);
    if(itembisexist != NULL)
    {
        if(itembisexist->len > 0)
        {
            DSSlotManageBisexist = 1;
            DSSlotManage = itembisexist->data[0];
        }
    }
    itembisexist = emvbase_avl_gettagitempointer(EMVTAG_DSODSInfo);
    if(itembisexist != NULL)
    {
        if(itembisexist->len > 0)
        {
            DSODSInfoBisexist = 1;
            DSODSInfo = itembisexist->data[0];
        }
    }
    if( (DSSlotManageBisexist) &&
            (0x80 == (0x80 & DSSlotManage)) &&
            (DSODSInfoBisexist && (0x40 == (0x40 & DSODSInfo)) ) )
    {
        memset(OID, 0x00, sizeof(OID));
    }
    else
    {
        itembisexist = emvbase_avl_gettagitempointer(EMVTAG_DSOperatorID);
        if(itembisexist != NULL)
        {
            if(itembisexist->len > 0)
            {
                memcpy(OID, itembisexist->data, (itembisexist->len > 8 ? 8 : itembisexist->len));
            }
        }
    }
    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-error: OID:");
    EMVBase_TraceHex("\r\n paypass-error: printFormat", OID, 8);
    #endif
    for(i = 0; i < 6; i++)
    {
        KL[i] = DSPKL[i];
        KR[i] = DSPKR[i];
    }

    for(i = 6; i < 8; i++)
    {
        KL[i] = OID[i - 2];
        KR[i] = OID[i];
    }
    memcpy(xorresult, OID, 8);

    emv_maths_xor(xorresult, PD, 8);
    dllpaypassDesEncrypt(1, xorresult, KL);

    dllpaypassDesEncrypt(0, xorresult, KR);

    dllpaypassDesEncrypt(1, xorresult, KL);

    emv_maths_xor(xorresult, PD, 8);

    memcpy(output, xorresult, 8);

    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-error: xorresult:");
    EMVBase_TraceHex("\r\n paypass-error: printFormat", xorresult, 8);
    #endif

    return 0;

}

/*****************************************************************************
** Descriptions:
** Parameters:	   unsigned char input[8]
							  unsigned char output[8]
** Returned value:
** Created By:	   luohuidong  2016.03.17
** Remarks:
*****************************************************************************/
int OWHF2AES(unsigned char input[8], unsigned char output[8])
{
    EMVBASETAGCVLITEM *itembisexist = NULL;
    unsigned char DSSlotManageBisexist = 0, DSSlotManage = 0, DSODSInfoBisexist = 0, DSODSInfo = 0;
    unsigned char OID[8], M[16], C[8], Y[11], DSID[12], PL, K[16], T[16];
    AES_KEY aeskey;


    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-error: in OWHF2AES \r\n");
    EMVBase_Trace("\r\n paypass-error: input data\r\n");
    EMVBase_TraceHex("\r\n paypass-error: printFormat", input, 8);
    #endif

    memset(OID, 0, sizeof(OID));

    memset(M, 0, sizeof(M));
    memset(C, 0, sizeof(C));
    memset(K, 0, sizeof(K));
    memset(Y, 0, sizeof(Y));
    memset(T, 0, sizeof(T));

    itembisexist = emvbase_avl_gettagitempointer(EMVTAG_DSSlotManagementControl);
    if(itembisexist != NULL)
    {
        if(itembisexist->len > 0)
        {
            DSSlotManageBisexist = 1;
            DSSlotManage = itembisexist->data[0];
        }
    }
    itembisexist = emvbase_avl_gettagitempointer(EMVTAG_DSODSInfo);
    if(itembisexist != NULL)
    {
        if(itembisexist->len > 0)
        {
            DSODSInfoBisexist = 1;
            DSODSInfo = itembisexist->data[0];
        }
    }
    if( (DSSlotManageBisexist) &&
            (0x80 == (0x80 & DSSlotManage)) &&
            (DSODSInfoBisexist && (0x40 == (0x40 & DSODSInfo)) ) )
    {
        memset(OID, 0x00, sizeof(OID));
    }
    else
    {
        itembisexist = emvbase_avl_gettagitempointer(EMVTAG_DSOperatorID);
        if(itembisexist != NULL)
        {
            if(itembisexist->len > 0)
            {
                memcpy(OID, itembisexist->data, (itembisexist->len > 8 ? 8 : itembisexist->len));
            }
        }
    }
    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-error: OID:");
    EMVBase_TraceHex("\r\n paypass-error: printFormat", OID, 8);
    #endif

    memcpy(C, input, 8);
    memcpy(M, C, 8);
    memcpy(&M[8], OID, 8);

    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-error: M:");
    EMVBase_TraceHex("\r\n paypass-error: printFormat", M, 16);
    #endif

    memset(Y, 0, sizeof(Y));
    itembisexist = emvbase_avl_gettagitempointer(EMVTAG_DSID);
    if(itembisexist != NULL)
    {
        if(itembisexist->len > 0)
        {
            #ifdef EMVB_DEBUG
            EMVBase_Trace("\r\n paypass-error: TAG_DSID get data\r\n");
            EMVBase_TraceHex("\r\n paypass-error: printFormat", itembisexist->data, itembisexist->len);
            #endif
            if(itembisexist->len > 11)
            {
                PL = 11;
            }
            else
            {
                PL = itembisexist->len;
            }
            memcpy(DSID, itembisexist->data, PL);
        }
    }

    if(11 > PL)
    {
        memcpy(&Y[11 - PL], DSID, PL);
    }
    else
    {
        memcpy(Y, DSID, PL);
    }

    memcpy(K, Y, 11);
    memcpy(&K[11], &OID[4], 4);
    K[15] = 0x3F;

    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-error: Y:");
    EMVBase_TraceHex("\r\n paypass-error: printFormat", Y, 11);
    #endif

    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-error: K:");
    EMVBase_TraceHex("\r\n paypass-error: printFormat", K, 16);
    #endif

    memset(&aeskey, 0, sizeof(AES_KEY));
    memset(T, 0, sizeof(T));
    dllpaypassAES_set_encrypt_key(K, 128, &aeskey);
    dllpaypassAES_encrypt(M, T, &aeskey);

    emv_maths_xor(T, M, 16);

    memcpy(output, T, 8);
    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-error: T:");
    EMVBase_TraceHex("\r\n paypass-error: printFormat", T, sizeof(T));
    #endif

    return 0;
}

unsigned char Paypass_FinalReadSelectRetData(EMVBASE_SELECT_RET *selectRet, unsigned char *DataOut, unsigned short LenOut, PAYPASSTradeUnionStruct *tempAppUnionStruct)
{
    unsigned char k;
    unsigned short index, indexFCI, indexFCIProp, len, lenFCI, lenFCIProp, templen;
    unsigned short indexIssuerDiscret;//,lenIssuerDiscret;
    unsigned char ret;
    unsigned char bIntable;
    EMVBASETAGCVLITEM *emvitem;
    unsigned char *temp;   //[300]


	EMVBase_Trace("paypass-info: ---response message of final select parsing---\r\n");

    index = 0;
    if(DataOut[index] != 0x6F)  //FCI模板标识
    {
    	EMVBase_Trace("paypass-error: FCI template error(r-apdu Byte1 != 6F)\r\n");
        return RLT_ERR_EMV_IccDataFormat; //FCI template
    }
    index++;

    if(EMVBase_ParseExtLen(DataOut, &index, &lenFCI))
    {
    	EMVBase_Trace("paypass-error: FCI template(Tag6F) parsing error\r\n");
        return RLT_ERR_EMV_IccDataFormat;
    }
    indexFCI = index;

    while(index < indexFCI + lenFCI)
    {
    	EMVBase_Trace("paypass-info: sub Tags in FCI template(Tag6F) parsing\r\n");

        if(index >= LenOut)
		{
			EMVBase_Trace("paypass-error: r-apdu parsing error\r\n");
			EMVBase_Trace("paypass-error: sub Tags' len error\r\n");
			return RLT_ERR_EMV_IccDataFormat;
		}
        if(DataOut[index] == 0xFF || DataOut[index] == 0x00)
        {
            index++;
            continue;
        }
        else if(DataOut[index] == 0x84) //DF�?
        {
        	EMVBase_Trace("paypass-info: DF Name(Tag84) parsing\r\n");
            if(selectRet->DFNameExist == 1)
			{
				EMVBase_Trace("paypass-error: DF Name(Tag84) already existed\r\n");
				return RLT_ERR_EMV_IccDataFormat;
			}
            index++;

            if(EMVBase_ParseExtLen(DataOut, &index, &templen))
            {
            	EMVBase_Trace("paypass-error: DF Name(Tag84) parsing error\r\n");
                return RLT_ERR_EMV_IccDataFormat;
            }


            selectRet->DFNameLen = templen;
            if(selectRet->DFNameLen > 16)
			{
				EMVBase_Trace("paypass-error: DF Name(Tag84)'s len is %d > 16\r\n", selectRet->DFNameLen);
				return RLT_ERR_EMV_IccDataFormat;
			}
            memcpy(selectRet->DFName, DataOut + index, templen);
            selectRet->DFNameExist = 1;

            index += templen;

            emvbase_avl_createsettagvalue(EMVTAG_DFName, selectRet->DFName, selectRet->DFNameLen);
        }
        else if(DataOut[index] == 0xA5) //FCI数据专用模板
        {
        	EMVBase_Trace("paypass-info: FCI Proprietary Template(TagA5) parsing\r\n");

            selectRet->FCIPropExist = 1;
            index++;

            if(EMVBase_ParseExtLen(DataOut, &index, &lenFCIProp))
            {
            	EMVBase_Trace("paypass-error: FCI Proprietary Template(TagA5) parsing error\r\n");
                return RLT_ERR_EMV_IccDataFormat;
            }
            indexFCIProp = index;

            while(index < indexFCIProp + lenFCIProp)
            {
            	EMVBase_Trace("paypass-info: sub Tags in FCI Proprietary Template(TagA5) parsing\r\n");

                ret = Paypass_ParseAndStoreCardResponse(tempAppUnionStruct, DataOut, &index, &bIntable, 0x10);
                if(ret != RLT_EMV_OK)
                {
                	EMVBase_Trace("paypass-error: sub Tags in FCI Proprietary Template(TagA5) parsing error\r\n");
                    return RLT_ERR_EMV_IccDataFormat;
                }
            }
            if(index != indexFCIProp + lenFCIProp)
            {
            	EMVBase_Trace("paypass-error: FCI Proprietary Template(TagA5)'s value parsing error\r\n");
				EMVBase_Trace("paypass-error: sub Tags' len error\r\n");
            	return RLT_ERR_EMV_IccDataFormat;
			}

            indexIssuerDiscret = 0;
            emvitem = emvbase_avl_gettagitempointer(EMVTAG_FCIDisData);
            if(emvitem != NULL)
            {
				EMVBase_Trace("paypass-info: sub Tags in FCI Issuer Discretionary Data(TagBF0C) parsing\r\n");

                temp = (unsigned char *)emvbase_malloc(300);
                memset(temp, 0, 300);
                memcpy(temp, emvitem->data, emvitem->len);
                while(indexIssuerDiscret < emvitem->len)
                {
                    ret = Paypass_ParseAndStoreCardResponse(tempAppUnionStruct, temp, &indexIssuerDiscret, &bIntable, 0x11);
                    if(ret != RLT_EMV_OK)
                    {
						EMVBase_Trace("paypass-error: sub Tags in FCI Issuer Discretionary Data(TagBF0C) parsing error\r\n");
                        emvbase_free(temp);
                        return RLT_ERR_EMV_IccDataFormat;
                    }
                }

                emvbase_free(temp);

                if(indexIssuerDiscret != emvitem->len)
                {
                	EMVBase_Trace("paypass-error: FCI Issuer Discretionary Data(TagBF0C)'s value parsing error\r\n");
					EMVBase_Trace("paypass-error: sub Tags' len error\r\n");
                    return RLT_ERR_EMV_IccDataFormat;
                }

                emvbase_avl_deletetag(EMVTAG_FCIDisData);
            }
        }
        else
        {
            ret = Paypass_ParseAndStoreCardResponse(tempAppUnionStruct, DataOut, &index, &bIntable, 0x6F);
            if(ret != RLT_EMV_OK)
            {
				EMVBase_Trace("paypass-error: sub Tags in FCI template(Tag6F) parsing error\r\n");
                return RLT_ERR_EMV_IccDataFormat;
            }
        }
    }
    if(index != indexFCI + lenFCI)
    {
    	EMVBase_Trace("paypass-error: FCI template(Tag6F)'s value parsing error\r\n");
		EMVBase_Trace("paypass-error: sub Tags' len error\r\n");
    	return RLT_ERR_EMV_IccDataFormat;
	}

    return RLT_EMV_OK;
}

unsigned char Paypass_FinalSelectRetData(EMVBASE_APDU_RESP *apdu_r, PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    EMVBASE_SELECT_RET selectRet;
    EMVBASE_LISTAPPDATA *tempselectedapp;
    EMVBASE_LISTAPPDATA *tempappdata;
    unsigned char tempselectedappno;
    unsigned char *tempappnum;


    tempappnum = &(tempApp_UnionStruct->PaypassTradeParam->AppListCandidatenum);
    tempselectedapp = tempApp_UnionStruct->PaypassTradeParam->SelectedApp;
    tempappdata = tempApp_UnionStruct->PaypassTradeParam->AppListCandidate;
    tempselectedappno = tempApp_UnionStruct->PaypassTradeParam->SelectedAppNo;

    if(apdu_r->SW1 == 0x90 && apdu_r->SW2 == 0x00) //Select OK
    {
        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S53_7);

        memset((unsigned char *)&selectRet, 0, sizeof(EMVBASE_SELECT_RET));

        if(Paypass_FinalReadSelectRetData(&selectRet, apdu_r->DataOut, apdu_r->LenOut, tempApp_UnionStruct) != RLT_EMV_OK)
        {
            return RLT_ERR_EMV_IccDataFormat;
        }

        if(selectRet.DFNameExist == 0 || selectRet.FCIPropExist == 0)
        {
        	EMVBase_Trace("paypass-info: DF Name(Tag84) or FCI Proprietary Template(TagA5) missing\r\n");
            return RLT_ERR_EMV_IccDataMissing;
        }

        if((tempappdata + tempselectedappno)->AIDLen != selectRet.DFNameLen)
        {
        	EMVBase_Trace("paypass-info: DFName(Tag84)'s len(%d) != terminal aid's len(%d)\r\n", (tempappdata + tempselectedappno)->AIDLen, selectRet.DFNameLen);
            return RLT_ERR_EMV_IccDataFormat;
        }

        if(memcmp((tempappdata + tempselectedappno)->AID, selectRet.DFName, selectRet.DFNameLen))
        {
        	EMVBase_Trace("paypass-info: DFName(Tag84)'s value does not match with the selected aid\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }

        memcpy(tempselectedapp->AID, (tempappdata + tempselectedappno)->AID, (tempappdata + tempselectedappno)->AIDLen);
        tempselectedapp->AIDLen = (tempappdata + tempselectedappno)->AIDLen;
        tempselectedapp->ReqKernelID = (tempappdata + tempselectedappno)->ReqKernelID;
    }
    else//current app selected fail,delete it from app list and select again.
    {
    	EMVBase_Trace("paypass-info: SW != 9000\r\n");
        if(*tempappnum > 1)
        {
        	EMVBase_Trace("paypass-info: select next aid\r\n");
            return RLT_EMV_APPSELECTTRYAGAIN;
        }
        else
        {
        	EMVBase_Trace("paypass-error: no more aid\r\n");
            return RLT_ERR_EMV_SWITCHINTERFACE;	 //20110804
        }
    }

    dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S53_8);

    return PAYPASS_READAIDPARAMETERS;
}

unsigned char Paypass_FinalSelect(PAYPASSTradeUnionStruct *App_UnionStruct)
{
    unsigned char retCode = 0;
    EMVBASE_APDU_SEND *apdu_s = NULL;
    EMVBASE_APDU_RESP *apdu_r = NULL;
    unsigned char tempselectappno = 0;
    EMVBASE_LISTAPPDATA *tempAppAppData = NULL;


	apdu_s = (EMVBASE_APDU_SEND *)emvbase_malloc(sizeof(EMVBASE_APDU_SEND));
	apdu_r = (EMVBASE_APDU_RESP *)emvbase_malloc(sizeof(EMVBASE_APDU_RESP));
	memset(apdu_s, 0, sizeof(EMVBASE_APDU_SEND));
	memset(apdu_r, 0, sizeof(EMVBASE_APDU_RESP));

    tempAppAppData = App_UnionStruct->PaypassTradeParam->AppListCandidate;
    tempselectappno = App_UnionStruct->PaypassTradeParam->SelectedAppNo;

    EMVBase_COMMAND_SELECT((tempAppAppData + tempselectappno)->AID, (tempAppAppData + tempselectappno)->AIDLen, 0, apdu_s);
    App_UnionStruct->IsoCommand(apdu_s, apdu_r);

	emvbase_free(apdu_s);

    if(apdu_r->ReadCardDataOk != RLT_EMV_OK)
    {
    	EMVBase_Trace("paypass-error: apdu timeout or other error\r\n");

		emvbase_free(apdu_r);

        if(App_UnionStruct->EmvErrID->L3 != EMVB_ERRID_L3_STOP)
        {
            dllpaypass_packoutsignal(App_UnionStruct, PAYPASSSTEP_S53_5);
			return RLT_ERR_EMV_APDUTIMEOUT;
        }
		else
		{
			dllpaypass_packoutsignal(App_UnionStruct, PAYPASSSTEP_S52_13);
			return RLT_EMV_TERMINATE_TRANSERR;
		}

    }

    retCode = Paypass_FinalSelectRetData(apdu_r, App_UnionStruct);

	emvbase_free(apdu_r);

    if(retCode == RLT_ERR_EMV_SWITCHINTERFACE)
    {
        dllpaypass_SetErrPar(App_UnionStruct, PAYPASSSTEP_S53_13);
        dllpaypass_SetErrPar(App_UnionStruct, PAYPASSSTEP_S53_14);
        dllpaypass_packoutsignal(App_UnionStruct, PAYPASSSTEP_S53_15);
        return RLT_EMV_TERMINATE_TRANSERR;
    }
	else if(retCode == RLT_ERR_EMV_IccDataFormat)
    {
        dllpaypass_SetErrPar(App_UnionStruct, PAYPASSSTEP_S1_7_0);
        dllpaypass_packoutsignal(App_UnionStruct, PAYPASSSTEP_S1_8_0);

        return RLT_EMV_APPSELECTTRYAGAIN;
    }
	else if(retCode == RLT_ERR_EMV_IccDataMissing)
    {
        dllpaypass_SetErrPar(App_UnionStruct, PAYPASSSTEP_S1_7_1);
        dllpaypass_packoutsignal(App_UnionStruct, PAYPASSSTEP_S1_8_0);

        return RLT_EMV_APPSELECTTRYAGAIN;
    }

    return retCode;
}

unsigned char Paypass_FinalSelectedApp(PAYPASSTradeUnionStruct *App_UnionStruct)
{
    unsigned char retCode;


	EMVBase_Trace("paypass-info: ---final select---\r\n");

    retCode = Paypass_FinalSelect(App_UnionStruct);
	if(RLT_ERR_EMV_APDUTIMEOUT == retCode)
	{
		retCode = RLT_ERR_EMV_TransRestart;
	}

    return retCode;
}

void Paypass_InitEMVDataObjects(PAYPASSTradeUnionStruct *App_UnionStruct)
{
    unsigned char TermCapab[3];
    EMVBASETAGCVLITEM *item;
    unsigned char HoldTimeValue;


    emvbase_avl_settag(EMVTAG_MobileSupportIndicator, 0x01);
    emvbase_avl_createsettagvalue(EMVTAG_CVMResult, "\x00\x00\x00", 3);

    App_UnionStruct->PaypassTradeParam->ODAstaus = 0;

    memset(TermCapab, 0, sizeof(TermCapab));
    TermCapab[0] = emvbase_avl_gettagvalue(EMVTAG_CardDataInputCapability);
    TermCapab[2] = emvbase_avl_gettagvalue(EMVTAG_SecurityCapability);
    emvbase_avl_createsettagvalue(EMVTAG_TermCapab, TermCapab, 3);

    #ifdef EMVB_DEBUG
    emvbase_avl_printtagallvalue("init pre GPO EMVTAG_TermCapab", EMVTAG_TermCapab);
    #endif


    #ifdef PAYPASS_OUTCOME_SUPPORT_FLAG

    App_UnionStruct->pUserInterfaceReqData->LanguagePreferenceLen = 0;
    item = emvbase_avl_gettagitempointer(EMVTAG_LangPrefer);
    if(item != NULL)
    {
        if(item->len)
        {
            memcpy(App_UnionStruct->pUserInterfaceReqData->LanguagePreference, item->data, item->len);
            App_UnionStruct->pUserInterfaceReqData->LanguagePreferenceLen = item->len;
        }
    }

    App_UnionStruct->pOutParameterSet->FieldoffinoutParameter = 0xFF;

    item = emvbase_avl_gettagitempointer(EMVTAG_AppCapabilitiesInfor);
    if(item != NULL)
    {
        EMVBase_TraceHex("\r\n paypass-error: 9F 5D,EMVTAG_AppCapabilitiesInfor ,item->data", item->data, item->len);
        if((item->len) > 1)
        {
            if(item->data[1] & 0x04)
            {
                HoldTimeValue = emvbase_avl_gettagvalue(EMVTAG_HoldTimeValue);
                App_UnionStruct->pOutParameterSet->FieldoffinoutParameter = HoldTimeValue;
            }
        }
    }
    #endif

	return;
}

unsigned char Paypass_InitDEDataObjects()
{
	unsigned char tagisexsit = 0;
    EMVBASETAGCVLITEM *TagsToReaditem = NULL;


	emvbase_avl_createsettagvalue(EMVTAG_PAYPASS_AccountType, NULL, 0); //20160716_lhd add  // ����

	emvbase_avl_createsettagvalue(EMVTAG_DataNeeded, NULL, 0);
	emvbase_avl_createsettagvalue(EMVTAG_DataToSend, NULL, 0);
	emvbase_avl_createsettagvalue(EMVTAG_TagsToReadYet, NULL, 0);

	tagisexsit = 0;
    TagsToReaditem = emvbase_avl_gettagitempointer(EMVTAG_TagsToRead);
    if(TagsToReaditem != NULL)
    {
        tagisexsit = 1; // empty
        if(TagsToReaditem->len)
        {
            if((TagsToReaditem->len == 1) && (0x00 == TagsToReaditem->data[0]))
            {
                ;
            }
            else
            {
                tagisexsit = 2; // not empty
            }
        }
    }

	if(2 == tagisexsit)
    {
        emvbase_avl_createsettagvalue(EMVTAG_TagsToReadYet, &TagsToReaditem->data[0], TagsToReaditem->len);
    }
    else if(1 == tagisexsit)
    {
        emvbase_avl_createsettagvalue(EMVTAG_DataNeeded, EMVTAG_TagsToRead, 3);
    }

	return 0;
}

unsigned char Paypass_InitEMV_DEDataObjects()
{
	EMVBase_Trace("paypass-info: ---Paypass_InitEMV_DEDataObjects---\r\n");

	emvbase_avl_createsettagvalue(EMVTAG_PostGACPutDataStatus, "\x00", 1);
    emvbase_avl_createsettagvalue(EMVTAG_PreGACPutDataStatus, "\x00", 1);

	emvbase_avl_createsettagvalue(EMVTAG_TagsToWriteYetAfterGenAC, NULL, 0);
    emvbase_avl_createsettagvalue(EMVTAG_TagsToWriteYetBeforeGenAC, NULL, 0);

	if(emvbase_avl_checkiftagexist(EMVTAG_TagsToWriteBeforeGAC))
	{
		AddListToList(0, EMVTAG_TagsToWriteBeforeGAC, EMVTAG_TagsToWriteYetBeforeGenAC);
	}
	if(emvbase_avl_checkiftagexist(EMVTAG_TagsToWriteAfterGAC))
	{
		AddListToList(0, EMVTAG_TagsToWriteAfterGAC, EMVTAG_TagsToWriteYetAfterGenAC);
	}

	if(emvbase_avl_checkiftagempty(EMVTAG_TagsToWriteBeforeGAC))
	{
        AddToList(0x01, EMVTAG_TagsToWriteBeforeGAC, 0, NULL, EMVTAG_DataNeeded);
    }
	if(emvbase_avl_checkiftagempty(EMVTAG_TagsToWriteAfterGAC))
	{
        AddToList(0x01, EMVTAG_TagsToWriteAfterGAC, 0, NULL, EMVTAG_DataNeeded);
    }

    return RLT_EMV_OK;
}

unsigned char Paypass_InitIDSDataObjects()
{
    emvbase_avl_createsettagvalue(EMVTAG_IDSStatus, "\x00", 1);
    emvbase_avl_createsettagvalue(EMVTAG_DSSummaryStatus, "\x00", 1);
    emvbase_avl_createsettagvalue(EMVTAG_DSDigestH, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);

    return RLT_EMV_OK;
}

unsigned char Paypass_IDSSetting()
{
    unsigned char DataStorageVersionNumber = 0;
    EMVBASETAGCVLITEM *item = NULL;
    unsigned char bDSIDExist = 0;
	unsigned char bAppCapabilitiesInfoExist = 0;
	unsigned char IDSStatus = 0;


	#if SpecificationBulletinNo239//20200701_LHD
    bDSIDExist = 0;
    if(emvbase_avl_checkiftagexist(EMVTAG_DSVNTerm) && emvbase_avl_checkiftagpresent(EMVTAG_DSOperatorID)) //Reader supports IDS
    {
		EMVBase_Trace("paypass-info: EMVTAG_DSVNTerm exist, EMVTAG_DSOperatorID exist\r\n");
        item = emvbase_avl_gettagitempointer(EMVTAG_DSID);
        if(item != NULL)
        {
            if(item->len)
            {
                bDSIDExist = 1;
				AddToList(0x07, EMVTAG_DSID, item->len, item->data, EMVTAG_DataToSend);
            }
			else
            {
                AddToList(0x01, EMVTAG_DSID, 0, NULL, EMVTAG_DataToSend);
            }
        }
		else
        {
            AddToList(0x05, EMVTAG_DSID, 1, "\x00", EMVTAG_DataToSend);
        }

        bAppCapabilitiesInfoExist = 0;
        item = emvbase_avl_gettagitempointer(EMVTAG_AppCapabilitiesInfor);
        if(item != NULL)
        {
            if(item->len)
            {
                bAppCapabilitiesInfoExist = 1;
                DataStorageVersionNumber = item->data[0];

				AddToList(0x07, EMVTAG_AppCapabilitiesInfor, item->len, item->data, EMVTAG_DataToSend);
            }
			else
            {
                AddToList(0x01, EMVTAG_AppCapabilitiesInfor, 0, NULL, EMVTAG_DataToSend);
            }
        }
		else
        {
            AddToList(0x05, EMVTAG_AppCapabilitiesInfor, 1, "\x00", EMVTAG_DataToSend);
        }

		EMVBase_Trace("paypass-info: bAppCapabilitiesInfoExist=%d,DataStorageVersionNumber=%02x,bDSIDExist=%d\r\n",bAppCapabilitiesInfoExist,DataStorageVersionNumber,bDSIDExist);
        if(	bAppCapabilitiesInfoExist && ((0x01 == (DataStorageVersionNumber & 0x0F)) || (0x02 == (DataStorageVersionNumber & 0x0F))) && bDSIDExist)
        {
        	IDSStatus = emvbase_avl_gettagvalue(EMVTAG_IDSStatus);
			IDSStatus |= 0x80;
			emvbase_avl_settag(EMVTAG_IDSStatus, IDSStatus);
        }
    }
	#endif

    return 0;
}

unsigned char Paypass_CheckPDOLdataMissing(unsigned char flag) // 1:S1.12,    2:S2.7
{
    EMVBASETAGCVLITEM *item = NULL;
    unsigned char *PDOL = NULL;
    unsigned int index = 0;
    unsigned int tempPDOLlen = 0;
    unsigned char tag[3];
    unsigned short taglen = 0;
	unsigned char L = 0;


	emvbase_avl_createsettagvalue(EMVTAG_TagsMissingPDOLDataFlag, "\x00", 1);

    item = emvbase_avl_gettagitempointer(EMVTAG_PDOL);
    if((item != NULL) && item->len)
    {
        PDOL = &item->data[0];

        if(item->len > 1024)
        {
            tempPDOLlen = 1024;
        }
        else
        {
            tempPDOLlen = item->len;
        }

        index = 0;
        while(index < tempPDOLlen)
        {
            if(PDOL[index] == 0x00)
            {
                index++;
                continue;
            }

            memset(tag, 0, sizeof(tag));
            taglen = 0;
            tag[taglen++] = PDOL[index];
            if((tag[0] & 0x1f) == 0x1f)
            {
                tag[taglen++] = PDOL[index + 1];
                if(tag[1] & 0x80)
                {
                    tag[taglen++] = PDOL[index + 2];
                }
            }

            index += taglen;
            L = PDOL[index++];

            EMVBase_TraceHex("paypass-info: tag in PDOL: ", tag, taglen);

			if(1 == flag)
			{
				if((L > 0) && emvbase_avl_checkiftagempty(tag)
					&& (0 == emvbase_TagBaseLib_read_condition(tag, taglen, EMVTAGTYPE_PAYPASS, 1, EMVTAGUPDATECONDITION_DET, NULL)))
	            {
	            	EMVBase_Trace("paypass-info: S1.12 Missing PDOL Data!\r\n");

	                emvbase_avl_createsettagvalue(EMVTAG_TagsMissingPDOLDataFlag, "\x01", 1);
					AddToList(0x01, tag, 0, NULL, EMVTAG_DataNeeded);
	                return RLT_EMV_ERR;
	            }
			}
			else
			{
	            if(emvbase_avl_checkiftagempty(tag) && (0 == emvbase_TagBaseLib_read_condition(tag, taglen, EMVTAGTYPE_PAYPASS, 1, EMVTAGUPDATECONDITION_DET, NULL)))
	            {
	            	EMVBase_Trace("paypass-info: S2.7 Missing PDOL Data!\r\n");

	                emvbase_avl_createsettagvalue(EMVTAG_TagsMissingPDOLDataFlag, "\x01", 1);
	                return RLT_EMV_ERR;
	            }
			}
        }
    }

    return RLT_EMV_OK;
}

unsigned char Paypass_SendDEKSignalAndInitialize(unsigned char bitmap, PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    EMVBASETAGCVLITEM *item = NULL;
    int ret = 0;
    unsigned char  *dataNeedBuff = NULL; //[256];
    unsigned int dataNeedBuffLen = 0;


    #if 1	//20160628_lhd  add for performance
    ret = tempApp_UnionStruct->paypassBisSupportDe();
    if(0 >= ret)
    {
        return RLT_EMV_ERR;
    }
    #endif

    dataNeedBuff = (unsigned char *)emvbase_malloc(DEKDETSENDDATALEN);
    if(NULL == dataNeedBuff)
    {
        EMVBase_Trace("paypass-info: Memory error\r\n");
        return RLT_EMV_ERR;
    }
    memset(dataNeedBuff, 0, DEKDETSENDDATALEN);
    dataNeedBuffLen = 0;

    if(0x01 & bitmap)
    {
        item = emvbase_avl_gettagitempointer(EMVTAG_DataToSend);
        if(item != NULL)
        {
            EMVBase_Trace("paypass-info: DEK DataToSend len: %d\r\n", item->len);
            EMVBase_TraceHex("paypass-info: DEK DataToSend: ", item->data, item->len);

            if(item->len)
            {
				Paypass_PackberTLVdata(&dataNeedBuff[0], &dataNeedBuffLen, EMVTAG_DataToSend, 3, item->data, item->len);
            }
            else
            {
            	memcpy(&dataNeedBuff[dataNeedBuffLen], EMVTAG_DataToSend, 3);
        		dataNeedBuffLen += 3;
                dataNeedBuff[dataNeedBuffLen++] = 0x00;
            }
        }
    }

    if(0x02 & bitmap)
    {
        item = emvbase_avl_gettagitempointer(EMVTAG_DataNeeded);
        if(item != NULL)
        {
        	EMVBase_Trace("paypass-info: DEK DataNeeded len: %d\r\n", item->len);
            EMVBase_TraceHex("paypass-info: DEK DataNeeded: ", item->data, item->len);

            if(item->len)
            {
				Paypass_PackberTLVdata(&dataNeedBuff[0], &dataNeedBuffLen, EMVTAG_DataNeeded, 3, item->data, item->len);
            }
            else//20160805_lhd add need test
            {
            	memcpy(&dataNeedBuff[dataNeedBuffLen], EMVTAG_DataNeeded, 3);
        		dataNeedBuffLen += 3;
                dataNeedBuff[dataNeedBuffLen++] = 0x00;
            }
        }
    }

    if(NULL != tempApp_UnionStruct->paypassDataExchange)
    {
        tempApp_UnionStruct->paypassDataExchange(1, dataNeedBuff, &dataNeedBuffLen);
    }

    emvbase_free(dataNeedBuff);
    dataNeedBuff = NULL;

    if(0x01 & bitmap)
    {
		emvbase_avl_deletetag(EMVTAG_DataToSend);
		emvbase_avl_createsettagvalue(EMVTAG_DataToSend, NULL, 0);
    }
    if(0x02 & bitmap)
    {
		emvbase_avl_deletetag(EMVTAG_DataNeeded);
		emvbase_avl_createsettagvalue(EMVTAG_DataNeeded, NULL, 0);
    }

    return RLT_EMV_OK;
}

unsigned char Paypass_UpdateWithDetData(unsigned char *data, unsigned int dataLen)
{
    unsigned short index, paydatalen, len;
    unsigned char tag[4], taglen, tagisupdate = 0;
    unsigned char *paydata = NULL;
    EMVBASETAGCVLITEM *item = NULL;
    EMVBASETAGCVLITEM emvtagitem;


    index = 0;
    paydata = &data[0];
    paydatalen = dataLen;

    while(index < paydatalen)
    {
        len = 0;
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

        #ifdef EMVB_DEBUG
        EMVBase_TraceHex("paypass-info: UpdateWithDetData tag: ", tag, taglen);
        #endif

        if(EMVBase_ParseExtLenWithMaxLen(paydata, &index, &len, paydatalen))
        {
            break;
        }

        if(0 == memcmp(tag, EMVTAG_TagsToRead, 3))
        {
            emvbase_avl_createsettagvalue(tag, &paydata[index], len);
            AddListToList(0, tag, EMVTAG_TagsToReadYet);
        }
        else if(0 == memcmp(tag, EMVTAG_TagsToWriteBeforeGAC, 3))
        {
            emvbase_avl_createsettagvalue(tag, &paydata[index], len);
            AddListToList(0, tag, EMVTAG_TagsToWriteYetBeforeGenAC);
        }
        else if(0 == memcmp(tag, EMVTAG_TagsToWriteAfterGAC, 3))
        {
            emvbase_avl_createsettagvalue(tag, &paydata[index], len);
            AddListToList(0, tag, EMVTAG_TagsToWriteYetAfterGenAC);
        }
        else
        {
            tagisupdate = emvbase_TagBaseLib_read_condition(tag, taglen, EMVTAGTYPE_PAYPASS, 1, EMVTAGUPDATECONDITION_DET, NULL);
            if(0 == tagisupdate)
            {
            	#ifdef EMVB_DEBUG
		        EMVBase_Trace("paypass-info: tag IsKnown and include DET Signal\r\n");
		        #endif

                emvbase_avl_createsettagvalue(tag, &paydata[index], len);
            }
            else
            {
                item = emvbase_avl_gettagitempointer(tag);
                if((item != NULL) && item->len)
                {
                    memset(&emvtagitem, 0, sizeof(EMVBASETAGCVLITEM));
                    tagisupdate = emvbase_TagBaseLib_read_condition(tag, taglen, EMVTAGTYPE_PAYPASS, 0, 0, &emvtagitem);
                    if( (tagisupdate == 0) && (!( emvtagitem.updatecondition & EMVTAGUPDATECONDITION_DET)) )
                    {
                        #ifdef EMVB_DEBUG
				        EMVBase_Trace("paypass-info: tag don't include DET Signal\r\n");
				        #endif
                    }
                    else
                    {
                    	#ifdef EMVB_DEBUG
				        EMVBase_Trace("paypass-info: tag IsPresent and include DET Signal\r\n");
				        #endif

                        emvbase_avl_createsettagvalue(tag, &paydata[index], len);
                    }
                }
            }
        }

        index += len;
    }

    if(index != paydatalen)
    {
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error:pay data err index=%d paydatalen=%d\r\n", index, paydatalen);
        #endif
    }

    paydata = NULL;
    return RLT_EMV_OK;
}

unsigned char Paypass_ResvDEKSignal(unsigned char bitmap, unsigned char mode, PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    EMVBASETAGCVLITEM *item = NULL;
    unsigned char  *dataNeedBuff = NULL;
    unsigned int dataNeedBuffLen = 0;
    unsigned char BisContinue = 0;
    unsigned char retCode = 0;
    unsigned int curTimerID = 0, EndTimerID = 0;
    int ret = 0;


    if((NULL == tempApp_UnionStruct->paypassDataExchange ) || (NULL == tempApp_UnionStruct->paypassResvStopSignal))
    {
        return RLT_EMV_ERR;
    }

    if (NULL == tempApp_UnionStruct->getCurMsTimer)
    {
        return RLT_EMV_ERR;
    }


    #if 1	//20160628_lhd  add for performance
    ret = tempApp_UnionStruct->paypassBisSupportDe();
    if(0 >= ret)
    {
        return RLT_EMV_OK;
    }
    #endif

    dataNeedBuff = (unsigned char *)emvbase_malloc(DEKDETSENDDATALEN);
    if(NULL == dataNeedBuff)
    {
        EMVBase_Trace("paypass-info: Memory error\r\n");
        return RLT_EMV_ERR;
    }
    memset(dataNeedBuff, 0, DEKDETSENDDATALEN);
    dataNeedBuffLen = 0;

    BisContinue = 0;
    curTimerID = tempApp_UnionStruct->getCurMsTimer();
	EndTimerID = 500;
    item = emvbase_avl_gettagitempointer(EMVTAG_TimeOutValue);
    if((item != NULL) && item->len)
    {
        EndTimerID = item->data[0];
        EndTimerID <<= 8;
        EndTimerID |= item->data[1];
    }

    #ifdef EMVB_DEBUG
    EMVBase_Trace("paypass-info: curTimerID=%d EndTimerID=%d\r\n", curTimerID, EndTimerID);
    #endif

    while(1)
    {
        dataNeedBuffLen = 0;
        ret = tempApp_UnionStruct->paypassDataExchange(2, dataNeedBuff, &dataNeedBuffLen);

        if(ret > 0)
        {
            Paypass_UpdateWithDetData(dataNeedBuff, dataNeedBuffLen);

            BisContinue = 1;
            continue;
        }
        else if(1 == BisContinue)
        {
            retCode = RLT_EMV_OK;
            break;
        }

        if(0x01 == mode)
        {
            retCode = RLT_EMV_OK;
            break;
        }

		if(tempApp_UnionStruct->paypassResvStopSignal)
		{
			retCode = tempApp_UnionStruct->paypassResvStopSignal();
			if(retCode)
			{
				EMVBase_Trace("paypass-error: waiting for PDOL data STOP\r\n");
				tempApp_UnionStruct->pErrorID->L3 = EMVB_ERRID_L3_STOP;
				retCode = RLT_ERR_EMV_CancelTrans;
				break;
			}
		}

        if((tempApp_UnionStruct->getCurMsTimer() - curTimerID) > EndTimerID)
        {
            EMVBase_Trace("paypass-error: waiting for PDOL data TIMEOUT\r\n");
            retCode = RLT_ERR_EMV_APDUTIMEOUT;
            break;
        }
    }

    emvbase_free(dataNeedBuff);
    dataNeedBuff = NULL;

    return retCode;
}

unsigned char Paypass_PDOLProcess(PAYPASSTradeUnionStruct *tempApp_UnionStruct, EMVBASE_APDU_SEND *apdu_s)
{
    unsigned char *pdolData;   //[255]
    unsigned short index, indexOut;
    unsigned char k, m, *buf, bInTable; //[255]
    unsigned char len;
    EMVBASETAGCVLITEM *item;
    unsigned char *PDOL;   //[300]
    unsigned char tag[4], taglen;
    EMVBASETAGCVLITEM *pdoldataitem;
    unsigned short templen, tempPDOLlen;
    unsigned char PDOLbexist = 0;
	unsigned short DOLDataMaxLen = 255;
	unsigned char errflag = 0;


    item = emvbase_avl_gettagitempointer(EMVTAG_PDOL);
    if(item != NULL)
    {
        if(item->len)
        {
            PDOLbexist = 1;
        }
    }


    #ifdef PAYPASS_DATAEXCHANGE
    unsigned char retCode = 0;


    Paypass_InitDEDataObjects();

    emvbase_avl_createsettagvalue(EMVTAG_TagsMissingPDOLDataFlag, "\x00", 1);

    while(PDOLbexist)
    {
    	Paypass_CheckPDOLdataMissing(1);

		// S1.15-Add known data listed in Tags To Read Yet to Data To Send
		FOReveryTinTagsToReadYet(0);

		Paypass_InitEMV_DEDataObjects();

		//DEK support IDS, modified by  sjz20220815
//		if(0 == tempApp_UnionStruct->PaypassTradeParam->bretforminit) // removed for SB261 but retained in production
		{
			Paypass_InitIDSDataObjects();
		}

        Paypass_IDSSetting(); // S1.17-S1.20

        if(!emvbase_avl_gettagvalue(EMVTAG_TagsMissingPDOLDataFlag)) // S1.21
        {
        	break;
        }

        Paypass_SendDEKSignalAndInitialize(0x03, tempApp_UnionStruct); // S1.22

        while(1)
        {
            retCode = Paypass_ResvDEKSignal(0x03, 0, tempApp_UnionStruct);
            if(RLT_EMV_OK != retCode)
            {
                if(RLT_ERR_EMV_APDUTIMEOUT == retCode)
                {
                    dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S6_3);
                }
                else if(RLT_ERR_EMV_CancelTrans == retCode)
                {
                    dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S7_8);
                }

                return RLT_EMV_TERMINATE_TRANSERR;
            }

            if(RLT_EMV_OK == Paypass_CheckPDOLdataMissing(2)) // S2.7
            {
                break;
            }
        }

		break;
    }
    #endif


    if(PDOLbexist == 0)
    {
    	FOReveryTinTagsToReadYet(0);

		Paypass_InitEMV_DEDataObjects();
		//DEK support IDS, modified by  sjz20220815
//		if(0 == tempApp_UnionStruct->PaypassTradeParam->bretforminit) // removed for SB261 but retained in production
		{
			Paypass_InitIDSDataObjects();
		}

        Paypass_IDSSetting(); // S1.17-S1.20

    	EMVBase_Trace("paypass-info: PDOL(Tag9F38) missing\r\n");

        memcpy(apdu_s->Command, "\x80\xA8\x00\x00", 4);
        apdu_s->Lc = 2;
        memcpy(apdu_s->DataIn, "\x83\x00", 2);
        apdu_s->Le = 256;
        apdu_s->EnableCancel = 1;
        return RLT_EMV_OK;
    }

    pdolData = (unsigned char *)emvbase_malloc(255);
    buf = (unsigned char *)emvbase_malloc(255);
    PDOL = (unsigned char *)emvbase_malloc(300);
    memset(pdolData, 0, 255);

	EMVBase_TraceHex("paypass-info: PDOL: ", item->data, item->len);

    index = 0;
    indexOut = 0;

    memset(PDOL, 0, 300);
    if(item->len > 255)
    {
        tempPDOLlen = 255;
    }
    else
    {
        tempPDOLlen = item->len;
    }
    memcpy(PDOL, item->data, tempPDOLlen);

	errflag = 0;

    while(index < tempPDOLlen) //Process PDOL
    {
        if(PDOL[index] == 0xFF || PDOL[index] == 0x00)
        {
            index++;
            continue;
        }
        memset(buf, 0, 255);
        bInTable = 0;

        memset(tag, 0, sizeof(tag));
        taglen = 0;
        tag[taglen++] = PDOL[index];
        if((tag[0] & 0x1f) == 0x1f)
        {
            tag[taglen++] = PDOL[index + 1];
            if(tag[1] & 0x80)
            {
                tag[taglen++] = PDOL[index + 2];
            }
        }

		EMVBase_TraceHex("PDOL tag:", tag, 3);
        pdoldataitem = emvbase_avl_gettagitempointer(tag);
        if(pdoldataitem != NULL)
        {
			if(!memcmp(tag, "\x9F\x1D", 2))
			{
				EMVBase_TraceHex("PDOL tag content:", pdoldataitem->data, 8);
			}
            index += taglen;

            if((index + 1) > tempPDOLlen)
            {
            	EMVBase_Trace("paypass-error: PDOL(Tag9F38)'s value error\r\n");

                emvbase_free(pdolData);
                emvbase_free(buf);
                emvbase_free(PDOL);
                return RLT_ERR_EMV_IccDataFormat;
            }
            templen = PDOL[index++];

            k = templen;   // 1
            m = pdoldataitem->len; // 4
			if ((indexOut + k) > DOLDataMaxLen)
			{
				errflag = 1;
				break;
			}

            if(pdoldataitem->datafomat & EMVTAGFORMAT_N) //numeric
            {
                if(k >= m)
                {
                    if(m)
                    {
                        memcpy(&buf[k - m], pdoldataitem->data, m);
                    }
                    memcpy(&pdolData[indexOut], buf, k);
                }
                else
                {
                    if(m)
                    {
                        memcpy(buf, pdoldataitem->data, m);
                    }
                    memcpy(&pdolData[indexOut], &buf[m - k], k);
                }
            }
            else if(pdoldataitem->datafomat & EMVTAGFORMAT_CN) //compact numeric
            {
                if(m)
                {
                    memset(buf, 0xFF, 255);
                    memcpy(buf, pdoldataitem->data, m);
                    memcpy(&pdolData[indexOut], buf, k);
                }
                else
                {
                    memset(buf, 0x00, 255);
                    memcpy(&pdolData[indexOut], buf, k);
                }
            }
            else//other formats
            {
                if(m)
                {
                    memcpy(buf, pdoldataitem->data, m);
                }
                memcpy(&pdolData[indexOut], buf, k);
            }

            indexOut += k;
            bInTable = 1;
        }

        if(!bInTable)
        {
            index += taglen;
            if((index + 1) > tempPDOLlen)
            {
            	EMVBase_Trace("paypass-error: PDOL(Tag9F38)'s value error\r\n");

                emvbase_free(pdolData);
                emvbase_free(buf);
                emvbase_free(PDOL);
                return RLT_ERR_EMV_IccDataFormat;
            }

            templen = PDOL[index++];
            k = templen;
			if ((indexOut + k) > DOLDataMaxLen)
			{
				errflag = 1;
				break;
			}
            memcpy(&pdolData[indexOut], buf, k);
            indexOut += k;
        }
    }

    // Process PDOL end here
	if(errflag)
	{
		emvbase_free(pdolData);
		emvbase_free(buf);
		emvbase_free(PDOL);
		EMVBase_Trace("paypass-error: PDOL(Tag9F38)'s value error\r\n");
		return RLT_EMV_ERR;
	}

    emvbase_avl_createsettagvalue(EMVTAG_PDOLData, pdolData, indexOut);

    memcpy(apdu_s->Command, "\x80\xA8\x00\x00", 4);
    apdu_s->Lc = indexOut + 2;
    len = 0;
    apdu_s->DataIn[len++] = 0x83;

    if((indexOut & 0x80) > 0)
    {
        (apdu_s->Lc)++;
        apdu_s->DataIn[len++] = 0x81;
        apdu_s->DataIn[len++] = indexOut;
    }
    else
    {
        apdu_s->DataIn[len++] = indexOut;
    }

    memcpy((unsigned char *)&apdu_s->DataIn[len], pdolData, indexOut);
    apdu_s->Le = 256;
    apdu_s->EnableCancel = 1;

    emvbase_free(pdolData);
    emvbase_free(buf);
    emvbase_free(PDOL);

    return RLT_EMV_OK;
}

unsigned char Paypass_GPORetData(PAYPASSTradeUnionStruct *tempApp_UnionStruct, EMVBASE_APDU_RESP *apdu_r)
{
    unsigned short index, indexTemp, len, lenTemp, cplen, j;
    unsigned char k;  //i,
    unsigned char bInTable;
    unsigned char ret;
    EMVBASETAGCVLITEM *tagitem;
    unsigned char aipexsit, aflexsit;


	EMVBase_Trace("paypass-info: ---response message of GPO parsing---\r\n");

    if(apdu_r->SW1 == 0x90 && apdu_r->SW2 == 0x00) //Get Processing Options OK
    {
        index = 0;

        if(apdu_r->DataOut[index] == 0x80) //primitive data
        {
        	EMVBase_Trace("paypass-info: GPO template 80\r\n");

            if(apdu_r->LenOut < 8)
            {
            	EMVBase_Trace("paypass-error: r-apdu's len < 8\r\n");
                return RLT_ERR_EMV_IccDataFormat;
            }
            index++;
            if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &len))
            {
            	EMVBase_Trace("paypass-error: Tag80 parsing error\r\n");
                return RLT_ERR_EMV_IccDataFormat;
            }

            if(index + len != apdu_r->LenOut)
            {
            	EMVBase_Trace("paypass-error: Tag80's len error\r\n");
            	return RLT_ERR_EMV_IccDataFormat;
			}

            if(len < 6)
            {
            	EMVBase_Trace("paypass-error: Tag80's len < 6\r\n");
                return RLT_ERR_EMV_IccDataFormat;
            }

            if(emvbase_avl_checkiftagexist(EMVTAG_AIP))
            {
            	EMVBase_Trace("paypass-error: AIP(Tag82) already exist\r\n");
                return RLT_ERR_EMV_IccDataFormat;
            }

            emvbase_avl_createsettagvalue(EMVTAG_AIP, apdu_r->DataOut + index, 2);


            index += 2;
            k = (len - 2) % 4;
            if(k != 0)
            {
            	EMVBase_Trace("paypass-error: AFL's len != Multiple of 4 (Tag80's len != 4*x + 2)\r\n");
                return RLT_ERR_EMV_IccDataFormat;
            }
            k = (len - 2) / 4;
            cplen = ((unsigned short)k) * 4;

            if(emvbase_avl_checkiftagexist(EMVTAG_AFL))
            {
				EMVBase_Trace("paypass-error: AFL(Tag94) already exist\r\n");
                return RLT_ERR_EMV_IccDataFormat;
            }

            emvbase_avl_createsettagvalue(EMVTAG_AFL, apdu_r->DataOut + index, cplen);

            for(j = 0; j < k; j++)
            {
                if(apdu_r->DataOut[index + j * 4] == 0)
                {
                	EMVBase_Trace("paypass-error: AFL[%d]'s SFI == 0\r\n", j+1);
                    return RLT_ERR_EMV_IccDataFormat;
                }
            }

			index += cplen;
        }
        else if(apdu_r->DataOut[index] == 0x77) //TLV coded data
        {
        	EMVBase_Trace("paypass-info: GPO template 77\r\n");

            index++;
            if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &lenTemp))
            {
            	EMVBase_Trace("paypass-error: Tag77 parsing error\r\n");
                return RLT_ERR_EMV_IccDataFormat;
            }

            indexTemp = index;
            while(index < indexTemp + lenTemp)
            {
                if(index >= apdu_r->LenOut)
                {
                	EMVBase_Trace("paypass-error: r-apdu parsing error\r\n");
					EMVBase_Trace("paypass-error: sub Tags' len error\r\n");
                    return RLT_ERR_EMV_IccDataFormat;
                }
                if(apdu_r->DataOut[index] == 0xFF || apdu_r->DataOut[index] == 0x00)
                {
                    index++;
                    continue;
                }
                ret = Paypass_ParseAndStoreCardResponse(tempApp_UnionStruct, apdu_r->DataOut, &index, &bInTable, 0x12); //20160713_lhd for 3M23-9200(12,25) 1->0
                if(ret != RLT_EMV_OK)
                {
                	EMVBase_Trace("paypass-error: sub Tags in Tag77 parsing error\r\n");
                    return RLT_ERR_EMV_IccDataFormat;
                }
            }
            if(index != indexTemp + lenTemp)
            {
            	EMVBase_Trace("paypass-error: Tag77's value parsing error\r\n");
				EMVBase_Trace("paypass-error: sub Tags' len error\r\n");
                return RLT_ERR_EMV_IccDataFormat;
            }
        }
        else
		{
			EMVBase_Trace("paypass-error: GPO template error (Byte1 != 77 or 80)\r\n");
			return RLT_ERR_EMV_IccDataFormat;
		}
    }
    else
    {
    	EMVBase_Trace("paypass-error: SW != 9000\r\n");
        return RLT_EMV_TRANSNOTACCEPT;
    }

    if(index != apdu_r->LenOut)
    {
    	EMVBase_Trace("paypass-info: len = %d\r\n", index);
    	EMVBase_Trace("paypass-info: apdu_r->LenOut = %d\r\n", apdu_r->LenOut);
    	EMVBase_Trace("paypass-error: r-apdu data error\r\n");
        return RLT_ERR_EMV_IccDataFormat;
    }

    #if 0 // �������࣬��Ϊ���´�������֤û����ɾ��
    aipexsit = 0;
    aflexsit = 0;

    tagitem = emvbase_avl_gettagitempointer(EMVTAG_AFL);
    if(tagitem != NULL)
    {
        aflexsit = 1;
        if(0 == tagitem->len)
        {
            return PAYPASS_PARSEERR;
        }
    }

    tagitem = emvbase_avl_gettagitempointer(EMVTAG_AIP);
    if(tagitem != NULL)
    {
        aipexsit = 1;
        if(0 == tagitem->len)
        {
            return PAYPASS_PARSEERR;
        }
    }

    if( ( ( 1 == aipexsit ) && ( 0 == aflexsit ) ) || ( ( 0 == aipexsit ) && ( 1 == aflexsit ) ) )
    {
        return PAYPASS_CARDDATAMISSING;
    }

    if((0 == aipexsit ) && ( 0 == aflexsit))
    {
        return PAYPASS_PARSEERR;
    }

    #else
    if(emvbase_avl_checkiftagexist(EMVTAG_AFL) == 0 || emvbase_avl_checkiftagexist(EMVTAG_AIP) == 0)
    {
        //return PAYPASS_PARSEERR;//20160714_lhd for case 3C05-9150(02)
        return RLT_ERR_EMV_IccDataMissing;
    }
    #endif


    return RLT_EMV_OK;
}

unsigned char Paypass_AfterGPO_MS(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
	#ifdef PAYPASS_DATAEXCHANGE

    FOReveryTinTagsToReadYet(0);

	if(emvbase_avl_checkiftagexist(EMVTAG_DataNeeded) ||
		(emvbase_avl_checkiftagexist(EMVTAG_DataToSend) && !emvbase_avl_checkiftagexist(EMVTAG_TagsToReadYet)))
	{
        Paypass_SendDEKSignalAndInitialize(0x03, tempApp_UnionStruct);
        Paypass_ResvDEKSignal(0x03, 0x01, tempApp_UnionStruct);
    }

	#endif

    return RLT_EMV_OK;
}

unsigned char Paypass_InitialApp(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char retCode;
    unsigned char AIP[2], KernelConfigure;
    EMVBASE_APDU_SEND *apdu_s=NULL;
    EMVBASE_APDU_RESP *apdu_r=NULL;


	EMVBase_Trace("paypass-info: ---GPO---\r\n");

	apdu_s = (EMVBASE_APDU_SEND *)emvbase_malloc(sizeof(EMVBASE_APDU_SEND));
	apdu_r = (EMVBASE_APDU_RESP *)emvbase_malloc(sizeof(EMVBASE_APDU_RESP));
	memset(apdu_s, 0, sizeof(EMVBASE_APDU_SEND));
	memset(apdu_r, 0, sizeof(EMVBASE_APDU_RESP));

	Paypass_InitEMVDataObjects(tempApp_UnionStruct);

    retCode = Paypass_PDOLProcess(tempApp_UnionStruct, apdu_s);
    if(retCode != RLT_EMV_OK)
    {
    	emvbase_free(apdu_r);
		emvbase_free(apdu_s);
        return RLT_EMV_TERMINATE_TRANSERR;
    }

    tempApp_UnionStruct->IsoCommand(apdu_s, apdu_r);

    if(apdu_r->ReadCardDataOk != RLT_EMV_OK)
    {
    	EMVBase_Trace("paypass-error: apdu timeout or other error\r\n");

        if(tempApp_UnionStruct->EmvErrID->L3 != EMVB_ERRID_L3_STOP)
        {
            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S3_5_0);
            dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S3_5_0);
			retCode = RLT_ERR_EMV_TransRestart;
        }
		else
        {
            dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S52_13);
			retCode = RLT_EMV_TERMINATE_TRANSERR;
        }
		emvbase_free(apdu_r);
		emvbase_free(apdu_s);

		return retCode;
    }

    retCode = Paypass_GPORetData(tempApp_UnionStruct, apdu_r);

	emvbase_free(apdu_r);
	emvbase_free(apdu_s);

    if(retCode == RLT_EMV_TRANSNOTACCEPT)
    {
        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S3_9_1);
        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S3_9_2);
		retCode = RLT_EMV_APPSELECTTRYAGAIN;
    }
    else if(retCode != RLT_EMV_OK)
    {
        if(retCode == RLT_ERR_EMV_IccDataMissing)
        {
            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S3_14_0);
        }
        else
        {
            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S3_12_0);
        }
        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S3_90_1);
        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S3_90_2);
		retCode = RLT_ERR_EMV_OTHERAPCARD;
    }

    if(retCode == RLT_EMV_OK)
    {
        memset(AIP, 0, sizeof(AIP));
        emvbase_avl_gettagvalue_spec(EMVTAG_AIP, AIP, 0, 2);
        KernelConfigure = emvbase_avl_gettagvalue(EMVTAG_KernelConfiguration);
        EMVBase_Trace("paypass-info: Kernel Configuration(TagDF811B): %02x\r\n", KernelConfigure);
        EMVBase_Trace("paypass-info: AIP(Tag82): %02x %02x\r\n", AIP[0], AIP[1]);

		if (KernelConfigure & 0x80)
		{
			EMVBase_Trace("\r\n paypass-error: MSD mode not supported \r\n");
		}
		else
		{
			EMVBase_Trace("\r\n paypass-error: MSD mode supported \r\n");
		}
		if (KernelConfigure & 0x40)
		{
			EMVBase_Trace("\r\n paypass-error: EMV mode not supported \r\n");
		}
		else
		{
			EMVBase_Trace("\r\n paypass-error: EMV mode supported \r\n");
		}
		EMVBase_Trace("\r\n paypass-error:card config: \r\n");
		if (AIP[1] & 0x80)
		{
			EMVBase_Trace("\r\n paypass-error: EMV mode supported \r\n");
		}
		else
		{
			EMVBase_Trace("\r\n paypass-error: EMV mode not supported \r\n");
		}


        if(KernelConfigure & 0x40)
        {
        	EMVBase_Trace("paypass-info: EMV mode not supported by terminal(Kernel Configuration bit7 is set)\r\n");

            if(KernelConfigure & 0x80)
            {
				EMVBase_Trace("paypass-info: Mag-stripe mode not supported by terminal(Kernel Configuration bit8 is set)\r\n");

                dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S3_E18_0);
                dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S3_90_1);
                dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S3_90_2);
                retCode = RLT_ERR_EMV_OTHERAPCARD;
            }
            else
            {
                tempApp_UnionStruct->PaypassTradeParam->qPBOCOrMSD = TRANSFLOW_MSDMODE;

				EMVBase_Trace("paypass-info: Mag-stripe mode is supported by terminal(Kernel Configuration bit8 is 0)\r\n");
				EMVBase_Trace("paypass-info: Mag-stripe mode selected\r\n");

				Paypass_AfterGPO_MS(tempApp_UnionStruct);
            }
        }
        else
        {
        	EMVBase_Trace("paypass-info: EMV mode is supported by terminal(Kernel Configuration bit7 is 0)\r\n");

            if(AIP[1] & 0x80)
            {
                tempApp_UnionStruct->PaypassTradeParam->qPBOCOrMSD = TRANSFLOW_EMVMODE;

				EMVBase_Trace("paypass-info: EMV mode is supported by card(AIP Byte2 bit8 is set)\r\n");
				EMVBase_Trace("paypass-info: EMV mode selected\r\n");
            }
            else
            {
            	EMVBase_Trace("paypass-info: EMV mode not supported by card(AIP Byte2 bit8 is 0)\r\n");

                if(KernelConfigure & 0x80) //terminal only sup EMV
                {
					EMVBase_Trace("paypass-info: Mag-stripe mode not supported by terminal(Kernel Configuration bit8 is set)\r\n");

                    dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S3_E18_0);
                    dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S3_90_1);
                    dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S3_90_2);
                    retCode = RLT_ERR_EMV_OTHERAPCARD;
                }
                else
                {
                    tempApp_UnionStruct->PaypassTradeParam->qPBOCOrMSD = TRANSFLOW_MSDMODE;

					EMVBase_Trace("paypass-info: Mag-stripe mode is supported by terminal(Kernel Configuration bit8 is 0)\r\n");
					EMVBase_Trace("paypass-info: Mag-stripe mode selected\r\n");

					Paypass_AfterGPO_MS(tempApp_UnionStruct);
                }
            }
        }
    }

    return retCode;
}

void Paypass_GetTagValueForShort(unsigned char *tag, unsigned short *value)
{
    unsigned char readtemp[4];

    *value = 0;
    memset(readtemp, 0, sizeof(readtemp));

    emvbase_avl_gettagvalue_spec(tag, (unsigned char *)&readtemp, 0, 2);

    *value = readtemp[0];
    *value <<= 8;
    *value |= readtemp[1];
}

unsigned char Paypass_CommandExchangeRelayResistanceData(unsigned char *TRRE, unsigned char TRRELen, EMVBASE_APDU_SEND *apdu_s)
{
    memset(apdu_s, 0, sizeof(EMVBASE_APDU_SEND));
    memcpy(&apdu_s->Command[0], "\x80\xEA\x00\x00", 4);
    memcpy(&apdu_s->DataIn[0], TRRE, TRRELen);
    apdu_s->Lc = TRRELen;
    apdu_s->Le = 256;
    apdu_s->EnableCancel = 1;

    return RLT_EMV_OK;
}

unsigned char Paypass_RRPRetData(PAYPASSTradeUnionStruct *tempApp_UnionStruct, EMVBASE_APDU_RESP *apdu_r)
{
	unsigned short len = 0;
    unsigned short index = 0;
	unsigned char *P = NULL;
	unsigned char offer = 0;


	if((apdu_r->SW1 != 0x90) || (apdu_r->SW2 != 0x00))
    {
		EMVBase_Trace("paypass-error: SW != 9000\r\n");
        return RLT_ERR_EMV_IccReturn;
    }

    index = 0;
    len = 0;

	if(apdu_r->DataOut[index] != 0x80 )
    {
    	EMVBase_Trace("paypass-error: r-apdu Byte1 != 80\r\n");
        return RLT_ERR_EMV_IccDataFormat;
    }

    index++;
    if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &len))
    {
		EMVBase_Trace("paypass-error: Tag80 parsing error\r\n");
        return RLT_ERR_EMV_IccDataFormat;
    }

    if(10 != len)
    {
		EMVBase_Trace("paypass-error: Tag80's len != 10\r\n");
        return RLT_ERR_EMV_IccDataFormat;
    }

    P = &apdu_r->DataOut[index];
    offer = 0;

	emvbase_avl_createsettagvalue(EMVTAG_DeviceRREntropy, P + offer, 4);
    offer += 4;

    emvbase_avl_createsettagvalue(EMVTAG_MinTimeForProcessingRR_APDU, P + offer, 2);
    offer += 2;

    emvbase_avl_createsettagvalue(EMVTAG_MaxTimeForProcessingRR_APDU, P + offer, 2);
    offer += 2;

    emvbase_avl_createsettagvalue(EMVTAG_DeviceEstimatedTransmissionTimeForRR_RAPDU, P + offer, 2);
    offer += 2;

	return RLT_EMV_OK;
}

unsigned char Paypass_RelayResistanceProtocol(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
	unsigned char retCode = RLT_EMV_OK;
	unsigned char KernelConfiguration = 0;
	unsigned char AIP[2] = {0};
	unsigned char RRP_counter = 0;
	unsigned char UnpredictNum[4] = {0};
	EMVBASE_APDU_SEND *apdu_s=NULL;
    EMVBASE_APDU_RESP *apdu_r=NULL;
	unsigned int TimeStart = 0, TimeStop = 0, TimeTaken = 0;
	unsigned short MinTimeForProcessingRR_APDU = 0;
    unsigned short MaxTimeForProcessingRR_APDU = 0;
	unsigned short DeviceEstimatedTTimeForRR_R_APDU = 0;
	unsigned short TermExpectedTTimeForRR_C_APDU = 0;
	unsigned short TermExpectedTTimeForRR_R_APDU = 0;
	unsigned short MinRRGracePeriod = 0;
    unsigned short MaxRRGracePeriod = 0;
    unsigned short RRAccuracyThreshold = 0;
    unsigned char RRTTimeMismatchThreshold = 0;

	int timetemp = 0;
	unsigned short MinRR_R_APDU = 0;
    unsigned short MeasuredRRProcessingTime = 0;
	unsigned short MRRPTTEMP = 0;


    if(tempApp_UnionStruct->PaypassTradeParam->qPBOCOrMSD != TRANSFLOW_EMVMODE)
    {
		return RLT_EMV_OK;
	}

	#ifdef PAYPASS_RRP

	EMVBase_Trace("paypass-info: ---Relay Resistance Protocol---\r\n");

	KernelConfiguration = emvbase_avl_gettagvalue(EMVTAG_KernelConfiguration);
	memset(AIP, 0, sizeof(AIP));
    emvbase_avl_gettagvalue_spec(EMVTAG_AIP, AIP, 0, 2);
    EMVBase_Trace("paypass-info: Kernel Configuration(TagDF811B): %02x\r\n", KernelConfiguration);
    EMVBase_Trace("paypass-info: AIP(Tag82): %02x %02x\r\n", AIP[0], AIP[1]);

	if(0 == (0x10 & KernelConfiguration))
	{
    	EMVBase_Trace("paypass-info: Relay resistance protocol not supported by terminal(Kernel Configuration bit4 is 0)\r\n");
		emvbase_avl_setvalue_or(EMVTAG_TVR, 4, 0x01);
		return RLT_EMV_OK;
	}
	if(0 == (0x01 & AIP[1]))
	{
    	EMVBase_Trace("paypass-info: Relay resistance protocol not supported by card(AIP Byte2 bit1 is set)\r\n");
		emvbase_avl_setvalue_or(EMVTAG_TVR, 4, 0x01);
		return RLT_EMV_OK;
	}


	EMVBase_Trace("paypass-info: Relay resistance protocol supported by card(AIP Byte2 bit1 is set)\r\n");
	EMVBase_Trace("and terminal(Kernel Configuration bit4 is set)\r\n");

    memset(UnpredictNum, 0, 4);
    tempApp_UnionStruct->EMVB_RandomNum(UnpredictNum, 4);
    emvbase_avl_createsettagvalue(EMVTAG_TermRREntropy, UnpredictNum, 4);
    emvbase_avl_createsettagvalue(EMVTAG_UnpredictNum, UnpredictNum, 4);

	apdu_s = (EMVBASE_APDU_SEND *)emvbase_malloc(sizeof(EMVBASE_APDU_SEND));
	apdu_r = (EMVBASE_APDU_RESP *)emvbase_malloc(sizeof(EMVBASE_APDU_RESP));

	while(1)
	{
        memset(apdu_s, 0, sizeof(EMVBASE_APDU_SEND));
        memset(apdu_r, 0, sizeof(EMVBASE_APDU_RESP));

        Paypass_CommandExchangeRelayResistanceData(UnpredictNum, 4, apdu_s);

        emvbase_avl_setvalue_and(EMVTAG_TVR, 4, 0xFC);
        emvbase_avl_setvalue_or(EMVTAG_TVR, 4, 0x01);

		*(tempApp_UnionStruct->ppaypassbIsDoNotSaveCurRecvCMDLog) = 1;

		TimeStart = tempApp_UnionStruct->getCurMsTimer() * 1000;
        tempApp_UnionStruct->IsoCommand(apdu_s, apdu_r);
		TimeStop = tempApp_UnionStruct->getCurMsTimer() * 1000;

		if(NULL != tempApp_UnionStruct->paypassSaveDataExchangeCmd)
	    {
	        tempApp_UnionStruct->paypassSaveDataExchangeCmd(3);
	    }

		if(TimeStop > TimeStart)
	    {
	        TimeTaken = TimeStop - TimeStart;
	    }
		else if(TimeStop == TimeStart)
		{
			TimeTaken = 1;
		}
	    else
	    {
	        TimeTaken = TimeStop + (0xffffffff - TimeStart);
	    }
        EMVBase_Trace("paypass-info: Time Taken: %d\r\n", TimeTaken);
//		DebugForKernel("paypass-info: Time Taken: %d\r\n", TimeTaken);

        if(apdu_r->ReadCardDataOk != RLT_EMV_OK)
        {
			EMVBase_Trace("paypass-error: apdu timeout or other error\r\n");

        	if(tempApp_UnionStruct->EmvErrID->L3 != EMVB_ERRID_L3_STOP)
	        {
	            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S4_E4_0); 	 //S7.4
	            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S4_E5_0); 	 //S7.5
	            dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S4_E6_0);
				retCode = RLT_ERR_EMV_TransRestart;
	        }
			else
			{
				dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S16_E7);
				retCode = RLT_EMV_TERMINATE_TRANSERR;
			}

			break;
        }

		retCode = Paypass_RRPRetData(tempApp_UnionStruct, apdu_r);
		if(RLT_EMV_OK != retCode)
		{
			if(RLT_ERR_EMV_IccReturn == retCode)
			{
				dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S4_E10_1);
        		dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S4_E10_2);
			}
			else
			{
				dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S4_E27_1);
        		dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S4_E27_2);
			}

			retCode = RLT_ERR_EMV_OTHERAPCARD;
            break;
		}

		Paypass_GetTagValueForShort(EMVTAG_MinTimeForProcessingRR_APDU, &MinTimeForProcessingRR_APDU);
		Paypass_GetTagValueForShort(EMVTAG_MaxTimeForProcessingRR_APDU, &MaxTimeForProcessingRR_APDU);
		Paypass_GetTagValueForShort(EMVTAG_DeviceEstimatedTransmissionTimeForRR_RAPDU, &DeviceEstimatedTTimeForRR_R_APDU);
		EMVBase_Trace("paypass-info: Min Time For Processing Relay Resistance APDU: %d\r\n", MinTimeForProcessingRR_APDU);
        EMVBase_Trace("paypass-info: Max Time For Processing Relay Resistance APDU: %d\r\n", MaxTimeForProcessingRR_APDU);
		EMVBase_Trace("paypass-info: Device Estimated Transmission Time For Relay Resistance R-APDU: %d\r\n", DeviceEstimatedTTimeForRR_R_APDU);

        Paypass_GetTagValueForShort(EMVTAG_TermExpectedTransmissionTimeForRR_CAPDU, &TermExpectedTTimeForRR_C_APDU);
        Paypass_GetTagValueForShort(EMVTAG_TermExpectedTransmissionTimeForRR_RAPDU, &TermExpectedTTimeForRR_R_APDU);
        Paypass_GetTagValueForShort(EMVTAG_MinRRGracePeriod, &MinRRGracePeriod);
        Paypass_GetTagValueForShort(EMVTAG_MaxRRGracePeriod, &MaxRRGracePeriod);
        Paypass_GetTagValueForShort(EMVTAG_RRAccuracyThreshold, &RRAccuracyThreshold);
        emvbase_avl_gettagvalue_spec(EMVTAG_RRTransmissionTimeMismatchThreshold, &RRTTimeMismatchThreshold, 0, 1);
		EMVBase_Trace("paypass-info: Terminal Expected Transmission Time For Relay Resistance C-APDU: %d\r\n", TermExpectedTTimeForRR_C_APDU);
        EMVBase_Trace("paypass-info: Terminal Expected Transmission Time For Relay Resistance R-APDU: %d\r\n", TermExpectedTTimeForRR_R_APDU);
		EMVBase_Trace("paypass-info: Minimum Relay Resistance Grace Period: %d\r\n", MinRRGracePeriod);
        EMVBase_Trace("paypass-info: Maximum Relay Resistance Grace Period: %d\r\n", MaxRRGracePeriod);
        EMVBase_Trace("paypass-info: Relay Resistance Accuracy Threshold: %d\r\n", RRAccuracyThreshold);
		EMVBase_Trace("paypass-info: Relay Resistance Transmission Time Mismatch Threshold: %d\r\n", RRTTimeMismatchThreshold);

        MinRR_R_APDU = ((DeviceEstimatedTTimeForRR_R_APDU > TermExpectedTTimeForRR_R_APDU) ? (TermExpectedTTimeForRR_R_APDU) : (DeviceEstimatedTTimeForRR_R_APDU));

		MeasuredRRProcessingTime = 0x00;
        timetemp = (TimeTaken / 100) - TermExpectedTTimeForRR_C_APDU - MinRR_R_APDU;
        if(timetemp > 0)
        {
            MeasuredRRProcessingTime = (unsigned short)timetemp;
        }

		memset(UnpredictNum, 0x00, 4);
		UnpredictNum[0] = (unsigned char)(MeasuredRRProcessingTime >> 8);
		UnpredictNum[1] = (unsigned char)MeasuredRRProcessingTime ;
		emvbase_avl_createsettagvalue(EMVTAG_MeasureRRProcessingTime, UnpredictNum, 2);

        EMVBase_Trace("paypass-info: MinRR_R_APDU: %d\r\n", MinRR_R_APDU);
        EMVBase_Trace("paypass-info: Measured Relay Resistance Processing Time: %d\r\n", MeasuredRRProcessingTime);

        MRRPTTEMP = 0x00;
        timetemp = MinTimeForProcessingRR_APDU - MinRRGracePeriod;
        if(timetemp > 0)
        {
            MRRPTTEMP = (unsigned short)timetemp;
        }
        EMVBase_Trace("paypass-info: MAX(0, (ProcessingRR_APDU - MinRRGracePeriod)): %d\r\n", MRRPTTEMP);

        if(MeasuredRRProcessingTime < MRRPTTEMP)
        {
		    EMVBase_Trace("paypass-error: Measured Relay Resistance Processing Time < MAX\r\n");
			dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S5_E27_1);
            dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S5_E27_2);
            retCode = RLT_ERR_EMV_OTHERAPCARD;
            break;
        }

        EMVBase_Trace("paypass-info: RRP Counter: %d\r\n", RRP_counter);
        if((RRP_counter < 2) && (MeasuredRRProcessingTime > MaxTimeForProcessingRR_APDU + MaxRRGracePeriod) )
        {
            memset(UnpredictNum, 0, 4);
            tempApp_UnionStruct->EMVB_RandomNum(UnpredictNum, 4);
            emvbase_avl_createsettagvalue(EMVTAG_TermRREntropy, UnpredictNum, 4);
            emvbase_avl_createsettagvalue(EMVTAG_UnpredictNum, UnpredictNum, 4); //20160822_lhd  add
            RRP_counter += 1;
        }
        else
        {
            if(MeasuredRRProcessingTime > (MaxTimeForProcessingRR_APDU + MaxRRGracePeriod))
            {
                emvbase_avl_setvalue_or(EMVTAG_TVR, 4, 0x04);
            }

            if (DeviceEstimatedTTimeForRR_R_APDU  &&  TermExpectedTTimeForRR_R_APDU)
            {
            	MRRPTTEMP = 0x00;
	            timetemp = (MeasuredRRProcessingTime - MinTimeForProcessingRR_APDU);
	            if(timetemp > 0)
	            {
	                MRRPTTEMP = (unsigned short)timetemp;
	            }
				EMVBase_Trace("paypass-info: MAX(0, (ProcessingRR_APDU - MinTimeForProcessingRR_APDU)): %d\r\n", MRRPTTEMP);

				if(((DeviceEstimatedTTimeForRR_R_APDU * 100) / TermExpectedTTimeForRR_R_APDU) < RRTTimeMismatchThreshold)
                {
					EMVBase_Trace("paypass-info: ((Device Estimated TTime For RR RAPDU * 100) / Term Expected TTime For RR R-APDU) < RR TTime Mismatch Threshold\r\n");
					emvbase_avl_setvalue_or(EMVTAG_TVR, 4, 0x08);
                }
				if(((TermExpectedTTimeForRR_R_APDU * 100) / DeviceEstimatedTTimeForRR_R_APDU) < RRTTimeMismatchThreshold)
                {
					EMVBase_Trace("paypass-info: ((Term Expected TTime For RR RAPDU * 100) / Device Estimated TTime For RR R-APDU) < RR TTime Mismatch Threshold\r\n");
					emvbase_avl_setvalue_or(EMVTAG_TVR, 4, 0x08);
                }
				if(MRRPTTEMP > RRAccuracyThreshold)
                {
					EMVBase_Trace("paypass-info: MAX > Relay Resistance Accuracy Threshold\r\n");
					emvbase_avl_setvalue_or(EMVTAG_TVR, 4, 0x08);
                }
            }
            else
            {
				if(!DeviceEstimatedTTimeForRR_R_APDU)
				{
					EMVBase_Trace("paypass-info: Device Estimated Transmission Time For Relay Resistance R-APDU == 0\r\n");
				}
				if(!TermExpectedTTimeForRR_R_APDU)
				{
					EMVBase_Trace("paypass-info: Terminal Expected Transmission Time For Relay Resistance R-APDU == 0\r\n");
				}
                emvbase_avl_setvalue_or(EMVTAG_TVR, 4, 0x08);
            }

            emvbase_avl_setvalue_and(EMVTAG_TVR, 4, 0xFC);
            emvbase_avl_setvalue_or(EMVTAG_TVR, 4, 0x02);

            retCode = RLT_EMV_OK;
            break;
        }
    }

    emvbase_avl_createsettagvalue(EMVTAG_RRPCounter, (unsigned char *)&RRP_counter, 1);

	emvbase_free(apdu_r);
	emvbase_free(apdu_s);

    #endif

	return retCode;
}

void Paypass_CheckSensitiveData(PAYPASSTradeUnionStruct *tempApp_UnionStruct, unsigned short startpos, unsigned short endpos, unsigned char authdataflag)
{
    EMVBASETAGCVLITEM *item = NULL;


    tempApp_UnionStruct->PaypassTradeParam->bTrack2DataMasked = 0;
    item = emvbase_avl_gettagitempointer(EMVTAG_Track2Equ);

    if(item != NULL)
    {
        if((item->len < 20) && (item->data != NULL))
        {
            if(item->len > 7)
			{
				if(!memcmp(item->data, "XGD-PCI", 7))
				{
					tempApp_UnionStruct->PaypassTradeParam->bTrack2DataMasked = 1;
				}
			}
			else
			{
				if(!memcmp(item->data, "XGD-PCI", item->len))
				{
					tempApp_UnionStruct->PaypassTradeParam->bTrack2DataMasked = 1;
				}
			}
        }
    }
	EMVBase_Trace("Paypass_CheckSensitiveData--startpos: %d\r\n", startpos);
	EMVBase_Trace("Paypass_CheckSensitiveData--endpos: %d\r\n", endpos);
	EMVBase_Trace("Paypass_CheckSensitiveData--item->len: %d\r\n", item->len);

    if(tempApp_UnionStruct->PaypassTradeParam->bTrack2DataMasked && authdataflag)
    {
        if(endpos > startpos+item->len)
        {
            tempApp_UnionStruct->PaypassTradeParam->Track2OffsetInAuthData += (endpos - item->len);
            tempApp_UnionStruct->PaypassTradeParam->Track2DataLen = item->len; // endpos-startpos;
        }
    }
}

unsigned char Paypass_ReadAFLRetData(PAYPASSTradeUnionStruct *tempApp_UnionStruct, EMVBASE_APDU_RESP *apdu_r, unsigned char tempt, unsigned char authdataflag)
{
    unsigned char t;
    unsigned char bInTable;
    unsigned short index, indexAEF, lenAEF;
    unsigned char ret;
	unsigned short tempindex = 0;


	EMVBase_Trace("paypass-info: ---apdu response parsing---\r\n");

    if(!(apdu_r->SW1 == 0x90 && apdu_r->SW2 == 0x00))
    {
    	EMVBase_Trace("paypass-error: SW != 9000\r\n");
        return RLT_ERR_EMV_IccReturn;   //sxl
    }

    index = 0;

    t = tempt;
    t >>= 3;
    if((t >= 11) && (t <= 30)) //mofified according to SU Bullitin No.12
    {
        if(tempApp_UnionStruct->PaypassTradeParam->ODAstaus)
        {
            if(authdataflag)
            {
                if(apdu_r->DataOut[index] == 0x70)
                {
                    if((tempApp_UnionStruct->PaypassTradeParam->AuthDataLen + apdu_r->LenOut) > 2048)
                    {
                        //return RLT_ERR_EMV_IccDataFormat;
                        tempApp_UnionStruct->PaypassTradeParam->bErrAuthData = 1;
                        return RLT_EMV_OK;
                    }
                    if(tempApp_UnionStruct->PaypassTradeParam->AuthData == NULL)
                    {
                        tempApp_UnionStruct->PaypassTradeParam->AuthDataMaxLen = 0;
                        tempApp_UnionStruct->PaypassTradeParam->AuthData = (unsigned char *)emvbase_malloc(512);

                        if(tempApp_UnionStruct->PaypassTradeParam->AuthData == NULL)
                        {
							EMVBase_Trace("paypass-error: Memory error\r\n");
                            return RLT_EMV_ERR;
                        }

                        tempApp_UnionStruct->PaypassTradeParam->AuthDataMaxLen = 512;
                        memset(tempApp_UnionStruct->PaypassTradeParam->AuthData, 0, 512);
                    }

                    if((tempApp_UnionStruct->PaypassTradeParam->AuthDataLen + apdu_r->LenOut) > tempApp_UnionStruct->PaypassTradeParam->AuthDataMaxLen)
                    {
                        if(tempApp_UnionStruct->PaypassTradeParam->AuthDataMaxLen < 1024)
                        {
                            unsigned char *p = (unsigned char *)emvbase_malloc(1024);
                            if(p == NULL)
                            {
								EMVBase_Trace("paypass-error: Memory error\r\n");
                                return RLT_EMV_ERR;
                            }
                            memcpy(p, tempApp_UnionStruct->PaypassTradeParam->AuthData, tempApp_UnionStruct->PaypassTradeParam->AuthDataLen);
                            emvbase_free(tempApp_UnionStruct->PaypassTradeParam->AuthData);
                            tempApp_UnionStruct->PaypassTradeParam->AuthData = p;
                            tempApp_UnionStruct->PaypassTradeParam->AuthDataMaxLen = 1024;
                        }
                        else if(tempApp_UnionStruct->PaypassTradeParam->AuthDataMaxLen < 2048)
                        {
                            unsigned char *p = (unsigned char *)emvbase_malloc(2048);
                            if(p == NULL)
                            {
								EMVBase_Trace("paypass-error: Memory error\r\n");
                                return RLT_EMV_ERR;
                            }
                            memcpy(p, tempApp_UnionStruct->PaypassTradeParam->AuthData, tempApp_UnionStruct->PaypassTradeParam->AuthDataLen);
                            emvbase_free(tempApp_UnionStruct->PaypassTradeParam->AuthData);
                            tempApp_UnionStruct->PaypassTradeParam->AuthData = p;
                            tempApp_UnionStruct->PaypassTradeParam->AuthDataMaxLen = 2048;
                        }
                        else
                        {
							EMVBase_Trace("paypass-error: data len of ODA error\r\n");
                            return RLT_ERR_EMV_IccDataFormat;
                        }
                    }

                    memcpy((unsigned char *)&tempApp_UnionStruct->PaypassTradeParam->AuthData[tempApp_UnionStruct->PaypassTradeParam->AuthDataLen], apdu_r->DataOut, apdu_r->LenOut);
                    tempApp_UnionStruct->PaypassTradeParam->AuthDataLen += apdu_r->LenOut;
                }
                else
                {
                    tempApp_UnionStruct->PaypassTradeParam->bErrAuthData = 1;
                }
            }
        }
        return RLT_EMV_OK;
    }


    while(apdu_r->DataOut[index] == 0xFF || apdu_r->DataOut[index] == 0x00)
    {
        index++;
    }

    if(apdu_r->DataOut[index] != 0x70)
    {
    	EMVBase_Trace("paypass-error: r-apdu template error (Byte1 != 0x70)\r\n");
        return RLT_ERR_EMV_IccDataFormat;
    }
    index++;

    if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &lenAEF))
    {
    	EMVBase_Trace("paypass-error: Tag70 parsing error\r\n");
        return RLT_ERR_EMV_IccDataFormat;
    }

    if(tempApp_UnionStruct->PaypassTradeParam->ODAstaus)
    {
        if(authdataflag) //add record to authentication data string if it is required.
        {
            if((tempApp_UnionStruct->PaypassTradeParam->AuthDataLen + lenAEF) > 2048)
            {
                //return RLT_ERR_EMV_IccDataFormat;
                tempApp_UnionStruct->PaypassTradeParam->bErrAuthData = 1;
            }
            else
            {
                if(tempApp_UnionStruct->PaypassTradeParam->AuthData == NULL)
                {
                    tempApp_UnionStruct->PaypassTradeParam->AuthDataMaxLen = 0;
                    tempApp_UnionStruct->PaypassTradeParam->AuthData = (unsigned char *)emvbase_malloc(512);

                    if(tempApp_UnionStruct->PaypassTradeParam->AuthData == NULL)
                    {
						EMVBase_Trace("paypass-error: Memory error\r\n");
                        return RLT_EMV_ERR;
                    }

                    tempApp_UnionStruct->PaypassTradeParam->AuthDataMaxLen = 512;
                    memset(tempApp_UnionStruct->PaypassTradeParam->AuthData, 0, 512);
                }


                if((tempApp_UnionStruct->PaypassTradeParam->AuthDataLen + apdu_r->LenOut) > tempApp_UnionStruct->PaypassTradeParam->AuthDataMaxLen)
                {
                    if(tempApp_UnionStruct->PaypassTradeParam->AuthDataMaxLen < 1024)
                    {
                        unsigned char *p = (unsigned char *)emvbase_malloc(1024);
                        if(p == NULL)
                        {
							EMVBase_Trace("paypass-error: Memory error\r\n");
                            return RLT_EMV_ERR;
                        }
                        memcpy(p, tempApp_UnionStruct->PaypassTradeParam->AuthData, tempApp_UnionStruct->PaypassTradeParam->AuthDataLen);
                        emvbase_free(tempApp_UnionStruct->PaypassTradeParam->AuthData);
                        tempApp_UnionStruct->PaypassTradeParam->AuthData = p;
                        tempApp_UnionStruct->PaypassTradeParam->AuthDataMaxLen = 1024;
                    }
                    else if(tempApp_UnionStruct->PaypassTradeParam->AuthDataMaxLen < 2048)
                    {
                        unsigned char *p = (unsigned char *)emvbase_malloc(2048);
                        if(p == NULL)
                        {
							EMVBase_Trace("paypass-error: Memory error\r\n");
                            return RLT_EMV_ERR;
                        }
                        memcpy(p, tempApp_UnionStruct->PaypassTradeParam->AuthData, tempApp_UnionStruct->PaypassTradeParam->AuthDataLen);
                        emvbase_free(tempApp_UnionStruct->PaypassTradeParam->AuthData);
                        tempApp_UnionStruct->PaypassTradeParam->AuthData = p;
                        tempApp_UnionStruct->PaypassTradeParam->AuthDataMaxLen = 2048;
                    }
                    else
                    {
						EMVBase_Trace("paypass-error: data len of ODA error\r\n");
                        return RLT_ERR_EMV_IccDataFormat;
                    }
                }

                memcpy((unsigned char *)&tempApp_UnionStruct->PaypassTradeParam->AuthData[tempApp_UnionStruct->PaypassTradeParam->AuthDataLen], (unsigned char *)&apdu_r->DataOut[index], lenAEF);
                tempApp_UnionStruct->PaypassTradeParam->AuthDataLen += lenAEF;
            }
        }
    }

    indexAEF = index;

    while(index < indexAEF + lenAEF)
    {
        if(index >= apdu_r->LenOut)
        {
			EMVBase_Trace("paypass-error: r-apdu parsing error\r\n");
			EMVBase_Trace("paypass-error: sub Tags' len error\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }

        if(apdu_r->DataOut[index] == 0xFF || apdu_r->DataOut[index] == 0x00)
        {
            index++;
            continue;
        }

		tempindex = index;
        ret = Paypass_ParseAndStoreCardResponse(tempApp_UnionStruct, apdu_r->DataOut, &index, &bInTable, 0x70);
        if(ret != RLT_EMV_OK)
        {
        	EMVBase_Trace("paypass-error: sub Tags in Tag70 parsing error\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }

		if(apdu_r->DataOut[tempindex] == 0x57) // for mask tag57
        {
        	tempApp_UnionStruct->PaypassTradeParam->Track2OffsetInAuthData = tempApp_UnionStruct->PaypassTradeParam->AuthDataLen - lenAEF;
            Paypass_CheckSensitiveData(tempApp_UnionStruct, tempindex-indexAEF, index-indexAEF, authdataflag);
        }
    }
    if(index != indexAEF + lenAEF)
    {
		EMVBase_Trace("paypass-error: Tag70's value parsing error\r\n");
		EMVBase_Trace("paypass-error: sub tags' len error\r\n");
        return RLT_ERR_EMV_IccDataFormat;
    }

    //sxl20110325
    if(index < apdu_r->LenOut)
    {
		EMVBase_Trace("paypass-error: r-apdu parsing error\r\n");
		EMVBase_Trace("paypass-error: sub Tags' len error\r\n");
        return RLT_ERR_EMV_IccDataFormat;
    }

    return RLT_EMV_OK;
}

unsigned char Paypass_DealAFLData_MS(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char i, j, tmpvalue;
    unsigned char tmp[160], track2pan[19], track2panlen, track2expirydate[2];
    unsigned char track1pan[19], track1panlen, track1expirydate[2], separator;
    unsigned int tmplen;

	unsigned char PUNATC1_BitNum = 0;
	unsigned char NATC1 = 0;
    unsigned char PUNATC2_BitNum = 0;
	unsigned char NATC2 = 0;
	unsigned char nUN = 0;
	unsigned char qtrack2, qtrack1;

    unsigned char PAYPASSTRACK2DATAbExist = 0, PAYPASSPUNATCTRACK2bExist = 0;
    unsigned char PAYPASSPCVC3TRACK2bExist = 0, PAYPASSNATCTRACK2bExist = 0;
    EMVBASETAGCVLITEM *PAYPASSTRACK2DATAitem = NULL, *PAYPASSPUNATCTRACK2item = NULL, *PAYPASSPCVC3TRACK2item = NULL;
    EMVBASETAGCVLITEM *PAYPASSNATCTRACK2item = NULL;

    unsigned char PAYPASSTRACK1DATAbExist = 0, PAYPASSNATCTRACK1bExist = 0;
    unsigned char PAYPASSPUNATCTRACK1bExist = 0, PAYPASSPCVC3TRACK1bExist = 0;
    EMVBASETAGCVLITEM *PAYPASSTRACK1DATAitem = NULL, *PAYPASSNATCTRACK1item = NULL, *PAYPASSPUNATCTRACK1item = NULL, *PAYPASSPCVC3TRACK1item = NULL;


    PAYPASSTRACK2DATAitem = emvbase_avl_gettagitemandstatus(EMVTAG_PaypassMagTrack2, &PAYPASSTRACK2DATAbExist);
    PAYPASSPUNATCTRACK2item = emvbase_avl_gettagitemandstatus(EMVTAG_PaypassPunatcTrack2, &PAYPASSPUNATCTRACK2bExist);
    PAYPASSPCVC3TRACK2item = emvbase_avl_gettagitemandstatus(EMVTAG_PaypassPcvc3Track2, &PAYPASSPCVC3TRACK2bExist);
    PAYPASSNATCTRACK2item = emvbase_avl_gettagitemandstatus(EMVTAG_PaypassNatcTrack2, &PAYPASSNATCTRACK2bExist);

    if(PAYPASSTRACK2DATAbExist == 0)
    {
        EMVBase_Trace("paypass-error: Track 2 Data(Tag9F6B) Missing\r\n");
        return RLT_ERR_EMV_IccDataMissing;
    }
	if(PAYPASSPUNATCTRACK2bExist == 0)
    {
        EMVBase_Trace("paypass-error: PUNATC(Track2)(Tag9F66) Missing\r\n");
        return RLT_ERR_EMV_IccDataMissing;
    }
	if(PAYPASSPCVC3TRACK2bExist == 0)
    {
        EMVBase_Trace("paypass-error: PCVC3(Track2)(Tag9F65) Missing\r\n");
        return RLT_ERR_EMV_IccDataMissing;
    }
	if(PAYPASSNATCTRACK2bExist == 0)
    {
        EMVBase_Trace("paypass-error: NATC(Track2)(Tag9F67) Missing\r\n");
        return RLT_ERR_EMV_IccDataMissing;
    }


    PUNATC2_BitNum = 0;
    for(i = 0; i < 2; i++)
    {
        tmpvalue = PAYPASSPUNATCTRACK2item->data[i];
        for(j = 0; j < 8; j++)
        {
            if(tmpvalue & 0x80)
            {
                PUNATC2_BitNum++;
            }
            tmpvalue = tmpvalue << 1;
        }
    }

    NATC2 = PAYPASSNATCTRACK2item->data[0];


    if(PUNATC2_BitNum < NATC2)
    {
        EMVBase_Trace("paypass-error: nUN < 0\r\n");
        return RLT_ERR_EMV_IccDataFormat;
    }

    nUN = PUNATC2_BitNum - NATC2;
    if(nUN > 8)
    {
		EMVBase_Trace("paypass-error: nUN > 8\r\n");
        return RLT_ERR_EMV_IccDataFormat;
    }
    tempApp_UnionStruct->PaypassTradeParam->PaypassnUN = nUN;

	#if 1 // ���������࣬׼��ɾ��
    qtrack2 = 0;
    for(i = 0; i < 2; i++)
    {
        tmpvalue = PAYPASSPCVC3TRACK2item->data[i];
        for(j = 0; j < 8; j++)
        {
            if(tmpvalue & 0x80)
            {
                qtrack2++;
            }
            tmpvalue = tmpvalue << 1;
        }
    }
    if(qtrack2 < 3)
    {
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error:DealPaypassAFLData3333\r\n");
        #endif
        return RLT_ERR_EMV_IccDataFormat;
    }


    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-info:DealPaypassAFLDataafaafdd2\r\n");
    EMVBase_Trace("\r\n paypass-info:%d\r\n", PAYPASSTRACK2DATAitem->len);
    #endif
    //check PAN and expiry date
    memset(tmp, 0, sizeof(tmp));
    EMVBaseBcdToAsc(tmp, PAYPASSTRACK2DATAitem->data, PAYPASSTRACK2DATAitem->len);
    tmplen = (PAYPASSTRACK2DATAitem->len) << 1;

    track2panlen = 0;
    memset(track2pan, 0, sizeof(track2pan));
    memset(track2expirydate, 0, sizeof(track2expirydate));
    separator = 0;
    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-info:tmplen = %d\r\n", tmplen);
    #endif
    for(i = 0; i < tmplen; i++)
    {
        if(tmp[i] == 'D')
        {
            separator = 1;
            if(i > 19)
            {
                #ifdef EMVB_DEBUG
                EMVBase_Trace("\r\n paypass-error:DealPaypassAFLData8888\r\n");
                #endif
                return RLT_ERR_EMV_IccDataFormat;
            }
            memcpy(track2pan, tmp, i);
            track2panlen = i;
            EMVBaseAscToBcd(track2expirydate, &tmp[i + 1], 4);
            break;
        }
    }

    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-info:tmplenafaf = %d\r\n", separator);
    #endif

    //Track 2 sent by the card does not include the needed separator so that the discretionary data
    //field cannot be localized.
    if(separator == 0)
    {
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error:DealPaypassAFLData888kkk\r\n");
        #endif
        return RLT_ERR_EMV_IccDataFormat;
    }
	#endif

    PAYPASSTRACK1DATAitem = emvbase_avl_gettagitemandstatus(EMVTAG_PayPassTrack1Data, &PAYPASSTRACK1DATAbExist);
    PAYPASSNATCTRACK1item = emvbase_avl_gettagitemandstatus(EMVTAG_Paypassnatctrack1, &PAYPASSNATCTRACK1bExist);
    PAYPASSPUNATCTRACK1item = emvbase_avl_gettagitemandstatus(EMVTAG_PaypassPunatcTrack1, &PAYPASSPUNATCTRACK1bExist);
    PAYPASSPCVC3TRACK1item = emvbase_avl_gettagitemandstatus(EMVTAG_PaypassPCVC3Track1, &PAYPASSPCVC3TRACK1bExist);

    //track1 data exist
    if(PAYPASSTRACK1DATAbExist)
    {
        EMVBase_Trace("paypass-info: Track 1 Data(Tag56) Is Not Empty\r\n");

        if(PAYPASSNATCTRACK1bExist == 0)
        {
			EMVBase_Trace("paypass-error: NATC(Track1)(Tag9F64) Missing\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }
		if(PAYPASSPUNATCTRACK1bExist == 0)
        {
        	EMVBase_Trace("paypass-error: PUNATC(Track1)(Tag9F63) Missing\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }
		if(PAYPASSPCVC3TRACK1bExist == 0)
        {
        	EMVBase_Trace("paypass-error: PCVC3(Track1)(Tag9F62) Missing\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }

        PUNATC1_BitNum = 0;
        for(i = 0; i < 6; i++)
        {
            tmpvalue = PAYPASSPUNATCTRACK1item->data[i];
            for(j = 0; j < 8; j++)
            {
                if(tmpvalue & 0x80)
                {
                    PUNATC1_BitNum++;
                }
                tmpvalue = tmpvalue << 1;
            }
        }
        NATC1 = PAYPASSNATCTRACK1item->data[0];

        if(PUNATC1_BitNum < NATC1)
        {
            EMVBase_Trace("paypass-error: Number of non-zero bits in PUNATC(Track1) - NATC(Track1) != nUN\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }

        if(nUN != (PUNATC1_BitNum - NATC1))
        {
            EMVBase_Trace("paypass-error: Number of non-zero bits in PUNATC(Track1) - NATC(Track1) != nUN\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }

		#if 1 // ���������࣬׼��ɾ��
        qtrack1 = 0;
        for(i = 0; i < 6; i++)
        {
            tmpvalue = PAYPASSPCVC3TRACK1item->data[i];
            for(j = 0; j < 8; j++)
            {
                if(tmpvalue & 0x80)
                {
                    qtrack1++;
                }
                tmpvalue = tmpvalue << 1;
            }
        }
        if(qtrack1 < 3)
        {
            #ifdef EMVB_DEBUG
            EMVBase_Trace("\r\n paypass-error:DealPaypassAFLData7777\r\n");
            #endif
            return RLT_ERR_EMV_IccDataFormat;
        }



        memset(tmp, 0, sizeof(tmp));
        memcpy(tmp, PAYPASSTRACK1DATAitem->data, PAYPASSTRACK1DATAitem->len);
        tmplen = PAYPASSTRACK1DATAitem->len;

        track1panlen = 0;
        memset(track1pan, 0, sizeof(track1pan));
        memset(track1expirydate, 0, sizeof(track1expirydate));
        if(tmp[0] != 'B')
        {
            #ifdef EMVB_DEBUG
            EMVBase_Trace("\r\n paypass-error:DealPaypassAFLData9999\r\n");
            #endif
            return RLT_ERR_EMV_IccDataFormat;
        }

        separator = 0;
        for(i = 1; i < tmplen; i++)
        {
            if(tmp[i] == 0x5E)
            {
                if(separator == 0)
                {
                    if(i > 20)
                    {
                        #ifdef EMVB_DEBUG
                        EMVBase_Trace("\r\n paypass-error:DealPaypassAFLDataaaaa\r\n");
                        #endif
                        return RLT_ERR_EMV_IccDataFormat;
                    }
                    separator = 1;
                    memcpy(track1pan, &tmp[1], i - 1);
                    track1panlen = i - 1;
                }
                else
                {
                    separator = 2;
                    EMVBaseAscToBcd(track1expirydate, &tmp[i + 1], 4);
                    break;
                }
            }
        }

        if(separator < 2)
        {
            #ifdef EMVB_DEBUG
            EMVBase_Trace("\r\n paypass-error:DealPaypassAFLData888kkkjjj\r\n");
            #endif
            return RLT_ERR_EMV_IccDataFormat;
        }
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-info:check track1 and track2 pan and expiry %d %d\r\n", track2panlen, track1panlen);

        EMVBase_TraceHex("\r\n paypass-info: track2pan", track2pan, track2panlen);

        EMVBase_TraceHex("\r\n paypass-info: track1pan", track1pan, track1panlen);
        EMVBase_Trace("\r\n paypass-info:");
        EMVBase_Trace("\r\n paypass-info:%02x %02x %02x %02x ", track2expirydate[0], track2expirydate[1], track1expirydate[0], track1expirydate[1]);
        #endif


        if(memcmp(track2expirydate, track1expirydate, 2))
        {
            #ifdef EMVB_DEBUG
            EMVBase_Trace("\r\n paypass-error:DealPaypassAFLDatadddd\r\n");
            #endif
            return RLT_ERR_EMV_IccDataFormat;
        }

        if((track1panlen == 0) || (track2panlen == 0) || (track1panlen != track2panlen))
        {
            #ifdef EMVB_DEBUG
            EMVBase_Trace("\r\n paypass-error:DealPaypassAFLDataeeee\r\n");
            #endif
            return RLT_ERR_EMV_IccDataFormat;
        }

        if(memcmp(track1pan, track2pan, track1panlen))
        {
            #ifdef EMVB_DEBUG
            EMVBase_Trace("\r\n paypass-error:DealPaypassAFLDataffff\r\n");
            #endif
            return RLT_ERR_EMV_IccDataFormat;
        }
		#endif
    }

    return RLT_EMV_OK;
}

unsigned char Paypass_ReadAppData_MS(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
	unsigned char retCode = 0;
	EMVBASETAGCVLITEM *item;
	unsigned char AFL_Num = 0;
	unsigned char Read_AFL_Num = 0;
	unsigned char AFL_RecordNum = 0;
	EMVBASE_APDU_SEND apdu_s = {0};
    EMVBASE_APDU_RESP apdu_r = {0};
	unsigned char bFirstWrite = 0;
	unsigned char AIP[2] = {0};
	unsigned char Kernelconfig = 0;
	unsigned char AmtAuthNum[6] = {0};
	unsigned char CLTransLimit[6] = {0};


	EMVBase_Trace("paypass-info: ---Mag-stripe Read Application Data---\r\n");

    item = emvbase_avl_gettagitempointer(EMVTAG_AFL);
    if((item == NULL) || ((item->len >> 2) == 0))
    {
    	dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S3_E41_0);
        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S3_90_1);
        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S3_90_2);
        return RLT_ERR_EMV_OTHERAPCARD;
    }

    retCode = EMVBase_CheckReadAFL(item);
    if(retCode != RLT_EMV_OK)
    {
    	dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S3_E41_0);
        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S3_90_1);
        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S3_90_2);
        return RLT_ERR_EMV_OTHERAPCARD;
    }

	AFL_Num = (item->len >> 2);
	if(memcmp(item->data, "\x08\x01\x01\x00", 4) == 0)
    {
        AFL_Num = 1;
    }


	for(Read_AFL_Num = 0; Read_AFL_Num < AFL_Num; Read_AFL_Num++)
    {
        AFL_RecordNum = item->data[(Read_AFL_Num << 2) + 1];

        while(AFL_RecordNum <= item->data[(Read_AFL_Num << 2) + 2])
        {
        	memset(&apdu_s, 0, sizeof(EMVBASE_APDU_SEND));
            memset(&apdu_r, 0, sizeof(EMVBASE_APDU_RESP));

            EMVBase_FormReadAflData(&apdu_s, AFL_RecordNum, item->data[(Read_AFL_Num << 2) + 0]);

			//*(tempApp_UnionStruct->ppaypassbIsDoNotSaveCurRecvCMDLog) = 1;

            tempApp_UnionStruct->IsoCommand(&apdu_s, &apdu_r);

            if(apdu_r.ReadCardDataOk != APDUCOMMANDOK)
            {
            	EMVBase_Trace("paypass-error: apdu timeout or other error\r\n");

				if(tempApp_UnionStruct->EmvErrID->L3 == EMVB_ERRID_L3_STOP)
		        {
		            dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S7_8);
					return RLT_EMV_TERMINATE_TRANSERR;
		        }
				else
				{
		            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S4_E4_0);
		            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S4_E5_0);
		            dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S4_E6_0);
					return RLT_ERR_EMV_TransRestart;
		        }
            }

			retCode = Paypass_ReadAFLRetData(tempApp_UnionStruct, &apdu_r, item->data[(Read_AFL_Num << 2) + 0], 0);
            if(retCode != RLT_EMV_OK)
            {
            	if(RLT_ERR_EMV_IccReturn == retCode)
            	{
					dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S4_E10_1);
        			dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S4_E10_2);
				}
				else
				{
					dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S4_E27_1);
        			dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S4_E27_2);
				}

                return RLT_ERR_EMV_OTHERAPCARD;
            }

            if(emvbase_avl_checkiftagexist(EMVTAG_UDOL))
            {
                ForEveryTLinTagList(EMVTAG_UDOL);
            }

			AFL_RecordNum++;
        }
	}

	// deal afl data
	retCode = Paypass_DealAFLData_MS(tempApp_UnionStruct);
	if(RLT_EMV_OK != retCode)
	{
		if(RLT_ERR_EMV_IccDataMissing == retCode)
		{
			dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S5_E17_1);
	        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S5_E17_2);
		}
		else if(RLT_ERR_EMV_IccDataFormat == retCode)
		{
			dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S5_E27_1);
            dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S5_E27_2);
		}

		return RLT_ERR_EMV_OTHERAPCARD;
	}

	while(1)
	{
		bFirstWrite = 0;

		item = emvbase_avl_gettagitempointer(EMVTAG_ProceedToFirstWriteFlag);
		if(item != NULL)
		{
		    if(item->len == 0)
		    {
		        AddToList(0x01, EMVTAG_ProceedToFirstWriteFlag, 0, NULL, EMVTAG_DataNeeded);
				bFirstWrite = 1;
		    }
		    else if((1 == item->len) && (0x00 == item->data[0]))
		    {
		        bFirstWrite = 1;
		    }
		}

		if(bFirstWrite)
		{
	        FOReveryTinTagsToReadYet(0);

	        if(emvbase_avl_checkiftagexist(EMVTAG_DataNeeded) ||
				(emvbase_avl_checkiftagexist(EMVTAG_DataToSend) && !emvbase_avl_checkiftagexist(EMVTAG_TagsToReadYet)))
	        {
	            Paypass_SendDEKSignalAndInitialize(0x03, tempApp_UnionStruct);
	        }

			// State 8 �C Waiting for Mag-stripe First Write Flag
			retCode = Paypass_ResvDEKSignal(0x03, 0, tempApp_UnionStruct);

			if(RLT_EMV_OK != retCode)
	        {
	            if(RLT_ERR_EMV_APDUTIMEOUT == retCode)
	            {
	                dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S13_42_0);
	        		dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S6_3);
	            }
	            else if(RLT_ERR_EMV_CancelTrans == retCode)
	            {
	                dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S7_8);
	            }

	            return RLT_EMV_TERMINATE_TRANSERR;
	        }

	        if(NULL != tempApp_UnionStruct->paypassSaveDataExchangeCmd)
	        {
	            tempApp_UnionStruct->paypassSaveDataExchangeCmd(8);
	        }

			continue;
	    }

		break;
	}

	if(0 == emvbase_avl_checkiftagexist(EMVTAG_AmtAuthNum))
	{
		dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S4_E13_0);
		return RLT_EMV_TERMINATE_TRANSERR;
	}

	memset(AIP, 0x00, sizeof(AIP));
	emvbase_avl_gettagvalue_spec(EMVTAG_AIP, AIP, 0, 2);
	Kernelconfig = emvbase_avl_gettagvalue(EMVTAG_KernelConfiguration);

	memset(AmtAuthNum, 0x00, sizeof(AmtAuthNum));
	emvbase_avl_gettagvalue_spec(EMVTAG_AmtAuthNum, AmtAuthNum, 0, 6);

	memset(CLTransLimit, 0x00, sizeof(CLTransLimit));
	if((AIP[0] & 0x02) != 0 && (Kernelconfig & 0x20))
	{
		EMVBase_Trace("paypass-info: On device cardholder verification supported\r\n");
		emvbase_avl_gettagvalue_spec(EMVTAG_RTCL_OndeviceCVM, CLTransLimit, 0, 6);
	}
	else
	{
		emvbase_avl_gettagvalue_spec(EMVTAG_clessofflineamt, CLTransLimit, 0, 6);
	}

	if(memcmp(AmtAuthNum, CLTransLimit, 6) > 0 )
	{
		dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S4_E15_0);
		return RLT_EMV_APPSELECTTRYAGAIN;
	}

	FOReveryTinTagsToReadYet(1);

    item = emvbase_avl_gettagitempointer(EMVTAG_DataToSend);
    if((item != NULL) && item->len)
    {
        Paypass_SendDEKSignalAndInitialize(0x01, tempApp_UnionStruct);
        Paypass_ResvDEKSignal(0x01, 0x01, tempApp_UnionStruct);
    }

	#if 0
	if(NULL != tempApp_UnionStruct->paypassSaveDataExchangeCmd)
    {
        tempApp_UnionStruct->paypassSaveDataExchangeCmd(10);
    }
	#endif

	if((AIP[0] & 0x02) && (Kernelconfig & 0x20))
    {
        if(tempApp_UnionStruct->PaypassTradeParam->PaypassCVMRLEF == 1)	 //   plaintext  PIN  Verification Performanced by ICC
        {
            emvbase_avl_setvalue_or(EMVTAG_MobileSupportIndicator, 0, 0x02);

            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S78_20_0);
        }
    }

	return RLT_EMV_OK;
}

unsigned char Paypass_IDS(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    EMVBASETAGCVLITEM *item;
    unsigned char AIP[2] = {0};
	unsigned char TermCapab[3] = {0};


	#if SpecificationBulletinNo239//20200701_lhd
    item = emvbase_avl_gettagitempointer(EMVTAG_IDSStatus);
    if((item != NULL) && item->len)
    {
        EMVBase_Trace("paypass-info: IDS Status %02x\r\n", item->data[0]);

        if(0x80 & item->data[0])
        {
			EMVBase_Trace("paypass-info: 'Read' in IDS Status is set(TagDF8128 bit8 is set)\r\n");

		    item = emvbase_avl_gettagitempointer(EMVTAG_DSSlotAvailability);
		    if((item != NULL) && item->len)
		    {
		        AddToList(0x07, EMVTAG_DSSlotAvailability, item->len, item->data, EMVTAG_DataToSend);
		    }

			item = emvbase_avl_gettagitempointer(EMVTAG_DSSummary1);
		    if((item != NULL) && item->len)
		    {
		        AddToList(0x07, EMVTAG_DSSummary1, item->len, item->data, EMVTAG_DataToSend);
		    }

		    item = emvbase_avl_gettagitempointer(EMVTAG_DSUnpredictableNumber);
		    if((item != NULL) && item->len)
		    {
		        AddToList(0x07, EMVTAG_DSUnpredictableNumber, item->len, item->data, EMVTAG_DataToSend);
		    }

			item = emvbase_avl_gettagitempointer(EMVTAG_DSSlotManagementControl);
		    if((item != NULL) && item->len)
		    {
		        AddToList(0x07, EMVTAG_DSSlotManagementControl, item->len, item->data, EMVTAG_DataToSend);
		    }

			item = emvbase_avl_gettagitempointer(EMVTAG_DSODSCard);
		    if(item != NULL)
		    {
				if(item->len > 0)
				{
					AddToList(0x07, EMVTAG_DSODSCard, item->len, item->data, EMVTAG_DataToSend);
				}else
				{
					AddToList(0x03, EMVTAG_DSODSCard, 0, NULL, EMVTAG_DataToSend);
				}
	        }

			AddToList(0x07, EMVTAG_UnpredictNum, 4, "\xFF\xFF\xFF\xFF", EMVTAG_DataToSend);

			EMVBase_Trace("paypass-info: EMVTAG_DSSlotAvailability=%d \r\n", emvbase_avl_checkiftagexist(EMVTAG_DSSlotAvailability));
			EMVBase_Trace("paypass-info: EMVTAG_DSSummary1=%d \r\n", emvbase_avl_checkiftagexist(EMVTAG_DSSummary1));
			EMVBase_Trace("paypass-info: EMVTAG_DSUnpredictableNumber=%d \r\n", emvbase_avl_checkiftagexist(EMVTAG_DSUnpredictableNumber));
			EMVBase_Trace("paypass-info: EMVTAG_DSODSCard=%d \r\n", emvbase_avl_checkiftagnotpresent(EMVTAG_DSODSCard));
			EMVBase_Trace("paypass-info: EMVTAG_DSSummary1=%d \r\n", emvbase_avl_checkiftagexist(EMVTAG_DSSummary1));
			EMVBase_Trace("paypass-info: EMVTAG_DSODSCard=%d \r\n", emvbase_avl_checkiftagnotpresent(EMVTAG_DSODSCard));
            if((emvbase_avl_checkiftagexist(EMVTAG_DSSlotAvailability) && emvbase_avl_checkiftagexist(EMVTAG_DSSummary1) &&
                  emvbase_avl_checkiftagexist(EMVTAG_DSUnpredictableNumber) && (emvbase_avl_checkiftagnotpresent(EMVTAG_DSODSCard)))
                    || ( emvbase_avl_checkiftagexist(EMVTAG_DSSummary1) && emvbase_avl_checkiftagpresent(EMVTAG_DSODSCard)))
            {
				EMVBase_Trace("paypass-info: Continue IDS\r\n");
            }
            else
            {
            	EMVBase_Trace("paypass-info: CLEAR 'Read' in IDS Status\r\n");
                emvbase_avl_setvalue_and(EMVTAG_IDSStatus, 0, 0x7F);
            }
        }
    }
	#endif

    FOReveryTinTagsToReadYet(0); // S3R1.14

	if(emvbase_avl_checkiftagexist(EMVTAG_DataNeeded) ||
		(emvbase_avl_checkiftagexist(EMVTAG_DataToSend) && !emvbase_avl_checkiftagexist(EMVTAG_TagsToReadYet)))
    {
        Paypass_SendDEKSignalAndInitialize(0x03, tempApp_UnionStruct);
        Paypass_ResvDEKSignal(0x03, 0x01, tempApp_UnionStruct);
    }


    #if 1//20160728_lhd
    tempApp_UnionStruct->PaypassTradeParam->ODAstaus = 0;
    memset(AIP, 0, sizeof(AIP));
    memset(TermCapab, 0, sizeof(TermCapab));
    emvbase_avl_gettagvalue_spec(EMVTAG_AIP, AIP, 0, 2);
    emvbase_avl_gettagvalue_spec(EMVTAG_TermCapab, TermCapab, 0, 3);
    if((AIP[0] & 0x01) && (TermCapab[2] & 0x08))
    {
		EMVBase_Trace("paypass-info: CDA supported by card and terminal-->Set CDA Flag\r\n");
        tempApp_UnionStruct->PaypassTradeParam->ODAstaus = 1;
    }
    else
    {
    	#if SpecificationBulletinNo239//20200701_lhd
        item = emvbase_avl_gettagitempointer(EMVTAG_IDSStatus);
        if((item != NULL) && (0x80 & item->data[0]))
        {
			EMVBase_Trace("paypass-info: CDA not supported by card or/and terminal\r\n");
			EMVBase_Trace("but 'Read' in IDS Status is set(TagDF8128 bit8 is set)-->Set CDA Flag\r\n");
        	tempApp_UnionStruct->PaypassTradeParam->ODAstaus = 1;
        }
        else
		#endif
        {
            emvbase_avl_setvalue_or(EMVTAG_TVR, 0, 0x80);
        }
    }
    #endif

    return RLT_EMV_OK;
}

unsigned char Paypass_GetNextGetDataTagFromList(unsigned char *InputList, unsigned char *OutPutTag, unsigned char *OutPutTagLen)
{
    EMVBASETAGCVLITEM *item = NULL;
    unsigned char *List = NULL;
    unsigned short ListLen = 0;
	unsigned short index = 0;
    unsigned char tag[4] = {0};
    unsigned char taglen = 0;
    int i = 0;


	EMVBase_TraceHex("paypass-info: GetNextGetDataTagFromList: ", InputList, 3);

    item = emvbase_avl_gettagitempointer(InputList);
    if((NULL == item) || (0 == item->len))
    {
    	EMVBase_Trace("paypass-info: List Empty\r\n");
		return RLT_EMV_ERR;
    }

    List = (unsigned char *)emvbase_malloc((PAYPASS_DE_LIST_BUF_MAX_LEN / 4) + 1);
    if(List == NULL)
    {
        EMVBase_Trace("paypass-info: Memory error\r\n");
        return RLT_EMV_ERR;
    }
    memset(List, 0, (PAYPASS_DE_LIST_BUF_MAX_LEN / 4));

    if(item->len > (PAYPASS_DE_LIST_BUF_MAX_LEN / 4))
    {
        ListLen = (PAYPASS_DE_LIST_BUF_MAX_LEN / 4);
    }
    else
    {
        ListLen = item->len;
    }
    memcpy(List, item->data, ListLen);


	index = 0;
    while(index < ListLen)
    {
        if(List[index] == 0x00)
        {
            index++;
            continue;
        }

        memset(tag, 0, sizeof(tag));
        taglen = 0;
        tag[taglen++] = List[index];
        if((tag[0] & 0x1f) == 0x1f)
        {
            tag[taglen++] = List[index + 1];
            if(tag[1] & 0x80)
            {
                tag[taglen++] = List[index + 2];
            }
        }
        index += taglen;

		EMVBase_TraceHex("paypass-info: Tag in List: ", tag, taglen);

		i = 0;
		while(1)
	    {
	        if (PaypassSupportedValuesForGetDataCmd[i][0] == 0x00)
            {
                break;
            }

	        if(0 == memcmp(&PaypassSupportedValuesForGetDataCmd[i][0], tag, 2))
            {
                *OutPutTagLen = taglen;
                memcpy(OutPutTag, tag, *OutPutTagLen);
                RemoveFromList(0, tag, InputList);

				emvbase_free(List);
                return RLT_EMV_OK;
            }

	        i++;
	    }
    }

    emvbase_free(List);
    return RLT_EMV_ERR;
}

unsigned char Paypass_CommandGetData(unsigned char *tag, unsigned char tagLen, EMVBASE_APDU_SEND *apdu_s)
{
    memset(apdu_s, 0, sizeof(EMVBASE_APDU_SEND));
    memcpy(&apdu_s->Command[0], "\x80\xCA", 2);
    if(tagLen == 1)
    {
        apdu_s->Command[2] = 0x00;
        apdu_s->Command[3] = tag[0];
    }
    else// if(tagLen==2)
    {
        memcpy(&apdu_s->Command[2], tag, 2);
    }
    apdu_s->Lc = 0;
    apdu_s->Le = 256;
    apdu_s->EnableCancel = 1;

    return RLT_EMV_OK;
}

unsigned char Paypass_GetDataCheck(unsigned char *ActiveTag, unsigned char *ActiveTagLen, EMVBASE_APDU_SEND *apdu_s)
{
	unsigned char retCode = 0;
	int tagisupdate = 0;


	memset(ActiveTag, 0, sizeof(ActiveTag));
    *ActiveTagLen = 0;

    retCode = Paypass_GetNextGetDataTagFromList(EMVTAG_TagsToReadYet, ActiveTag, ActiveTagLen);
	if(RLT_EMV_OK != retCode)
	{
		return RLT_EMV_ERR;
	}

	tagisupdate = emvbase_TagBaseLib_read_condition(ActiveTag, *ActiveTagLen, EMVTAGTYPE_PAYPASS, 0, 0, NULL);
    if(0 != tagisupdate)
    {
    	return RLT_EMV_ERR;
    }

    Paypass_CommandGetData(ActiveTag, ActiveTagLen, apdu_s);

	EMVBase_Trace("paypass-info: Next Cmd: GET DATA\r\n");
	return RLT_EMV_OK;
}

unsigned char Paypass_GetDataRetData(unsigned char *CurrentTag, unsigned char CurrentTagLen, EMVBASE_APDU_RESP *apdu_r)
{
	unsigned char retCode = 0;
	unsigned char bInTable = 0;
	unsigned short index = 0;
	unsigned short len = 0;


	EMVBase_Trace("paypass-info: ---Get Data response parsing---\r\n");

	if((apdu_r->SW1 == 0x90) && (apdu_r->SW2 == 0x00))
    {
        index = 0;
        bInTable = 0;
        retCode = Paypass_ParseAndStoreCardResponse(NULL, apdu_r->DataOut, &index, &bInTable, 0);
        if(RLT_EMV_OK != retCode)
        {
        	AddToList(0x05, CurrentTag, 1, "\x00", EMVTAG_DataToSend);
            return RLT_EMV_OK;
        }

        if(!memcmp(apdu_r->DataOut, CurrentTag, CurrentTagLen))
        {
            index = 0;
            index += CurrentTagLen;
            len = 0;
            if(EMVBase_ParseExtLenWithMaxLen(&apdu_r->DataOut[0], &index, &len, apdu_r->LenOut))
            {
                ;
            }
            AddToList(0x07, CurrentTag, len, &apdu_r->DataOut[index], EMVTAG_DataToSend);
        }
        else
        {
        	AddToList(0x05, CurrentTag, 1, "\x00", EMVTAG_DataToSend);
        }
    }
    else
    {
    	AddToList(0x05, CurrentTag, 1, "\x00", EMVTAG_DataToSend);
    }

	return RLT_EMV_OK;
}

unsigned char Paypass_DealAFLData_EMV(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char TagbExist;
	unsigned char TermCap[3] = {0};
    unsigned char AIP[2] = {0};
	unsigned char Kernelconfig = 0;
	unsigned char AmtAuthNum[6] = {0};
	unsigned char CLTransLimit[6] = {0};
	unsigned char IDSStatus = 0;
	EMVBASETAGCVLITEM *item = NULL;
	EMVBASETAGCVLITEM *itemDSID = NULL;
	unsigned char *tmp = NULL;
	unsigned char *tmp1 = NULL;
	unsigned short tmplen = 0;
	unsigned short icnt = 0;
	unsigned short padzerocnt = 0;
	unsigned char DSIDANDSEQ[20] = {0}, CMPDSIDANDSEQ[20] = {0};
	unsigned char CMPDSIDANDSEQLEN = 0;
    unsigned short DSIDANDSEQLEN = 0;

	EMVBase_Trace("paypass-info: goto Paypass_DealAFLData_EMV\r\n");

	if(0 == emvbase_avl_checkiftagexist(EMVTAG_AmtAuthNum))
	{
		dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S4_E13_0);
		return RLT_EMV_TERMINATE_TRANSERR;
	}

	memset(AIP, 0x00, sizeof(AIP));
	emvbase_avl_gettagvalue_spec(EMVTAG_AIP, AIP, 0, 2);
	Kernelconfig = emvbase_avl_gettagvalue(EMVTAG_KernelConfiguration);

	memset(AmtAuthNum, 0x00, sizeof(AmtAuthNum));
	emvbase_avl_gettagvalue_spec(EMVTAG_AmtAuthNum, AmtAuthNum, 0, 6);

	memset(CLTransLimit, 0x00, sizeof(CLTransLimit));
	if((AIP[0] & 0x02) != 0 && (Kernelconfig & 0x20))
	{
		EMVBase_Trace("paypass-info: On device cardholder verification supported\r\n");
		emvbase_avl_gettagvalue_spec(EMVTAG_RTCL_OndeviceCVM, CLTransLimit, 0, 6);
	}
	else
	{
		emvbase_avl_gettagvalue_spec(EMVTAG_clessofflineamt, CLTransLimit, 0, 6);
	}

	if(memcmp(AmtAuthNum, CLTransLimit, 6) > 0 )
	{
		dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S4_E15_0);
		return RLT_EMV_APPSELECTTRYAGAIN;
	}

	if(emvbase_avl_checkiftagexist(EMVTAG_AppExpireDate) == 0)
    {
        EMVBase_Trace("paypass-error: App Expiration Date(Tag5F24) Missing\r\n");
		dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S5_E17_1);
        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S5_E17_2);
        return RLT_ERR_EMV_OTHERAPCARD;
    }
	if(emvbase_avl_checkiftagexist(EMVTAG_PAN) == 0)
    {
        EMVBase_Trace("paypass-error: PAN(Tag5A) Missing\r\n");
		dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S5_E17_1);
        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S5_E17_2);
        return RLT_ERR_EMV_OTHERAPCARD;
    }
	if(emvbase_avl_checkiftagexist(EMVTAG_CDOL1) == 0)
    {
        EMVBase_Trace("paypass-error: CDOL1(Tag8C) Missing\r\n");
		dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S5_E17_1);
        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S5_E17_2);
        return RLT_ERR_EMV_OTHERAPCARD;
    }

	IDSStatus = emvbase_avl_gettagvalue(EMVTAG_IDSStatus);
	if(0x80 & IDSStatus)
	{
	    itemDSID = emvbase_avl_gettagitempointer(EMVTAG_DSID);
	    if((itemDSID != NULL) && itemDSID->len)
	    {
	        #ifdef EMVB1_DEBUG
	        EMVBase_Trace("\r\n paypass-info: TAG_DSID data len=%d \r\n", itemDSID->len);
	        EMVBase_TraceHex("\r\n paypass-info: DSID data", itemDSID->data, itemDSID->len);
	        #endif

	        DSIDANDSEQLEN = 0;
	        memset(DSIDANDSEQ, 0, sizeof(DSIDANDSEQ));
	        item = emvbase_avl_gettagitempointer(EMVTAG_PAN);
	        if((item != NULL) && item->len)
	        {
	            tmp = (unsigned char *)emvbase_malloc(64 + 1);
	            memset(tmp, 0, 64);

	            EMVBaseBcdToAsc(&tmp[0], item->data, item->len);
	            tmplen = (item->len) << 1;

	            icnt = 0;
	            padzerocnt = 0;
	            for(icnt = tmplen - 1; icnt > 0; icnt--)
	            {
	                if( 0x46 == tmp[icnt])
	                {
	                    padzerocnt++;
	                }
	                else
	                {
	                    break;
	                }
	            }
	            icnt += 1;

	            #ifdef EMVB_DEBUG
	            EMVBase_Trace("\r\n paypass-info: icnt=%d tmplen=%d\r\n", icnt, tmplen);
	            #endif

	            if(padzerocnt)
	            {
	                tmp1 = (unsigned char *)emvbase_malloc(64 + 1);
	                memset(tmp1, 0, 64);
	                memcpy(tmp1 + padzerocnt, tmp, icnt);
	                icnt += padzerocnt;
	                memset(tmp, 0, 64);
	                memcpy(tmp, tmp1, icnt);
	                emvbase_free(tmp1);


	                EMVBaseAscToBcd(DSIDANDSEQ, tmp, icnt);
	                DSIDANDSEQLEN =	icnt >> 1;
	            }
	            else
	            {
	                DSIDANDSEQLEN =	((item->len > 10) ? 10 : (item->len) );
	                memcpy(DSIDANDSEQ, item->data, DSIDANDSEQLEN);
	            }
	            emvbase_free(tmp);
	        }

	        item = emvbase_avl_gettagitempointer(EMVTAG_PANSeq);
	        if((item != NULL) && item->len)
	        {
	            DSIDANDSEQ[DSIDANDSEQLEN++] = item->data[0];
	        }
	        else
	        {
	            DSIDANDSEQ[DSIDANDSEQLEN++] = 0x00;
	        }

	        #ifdef EMVB_DEBUG
//	        EMVBase_Trace("\r\n paypass-info: DSIDANDSEQLEN :%d\r\n", DSIDANDSEQLEN);
//	        EMVBase_TraceHex("\r\n paypass-info: printFormat", DSIDANDSEQ, DSIDANDSEQLEN);

//	        EMVBase_Trace("\r\n paypass-info: TagsToReaditem->len %02x \r\n", itemDSID->len);
//	        EMVBase_TraceHex("\r\n paypass-info: printFormat", itemDSID->data, itemDSID->len);
	        #endif

	        memset(CMPDSIDANDSEQ, 0, sizeof(CMPDSIDANDSEQ));
	        if(DSIDANDSEQLEN < 8)
	        {
	            memcpy(&CMPDSIDANDSEQ[8 - DSIDANDSEQLEN], DSIDANDSEQ, DSIDANDSEQLEN);
	            CMPDSIDANDSEQLEN = 8;
	        }
	        else
	        {
	            memcpy(&CMPDSIDANDSEQ[0], DSIDANDSEQ, DSIDANDSEQLEN);
	            CMPDSIDANDSEQLEN = DSIDANDSEQLEN;
	        }

	        #ifdef EMVB_DEBUG
//	        EMVBase_Trace("\r\n paypass-info: CMPDSIDANDSEQLEN :%d\r\n", CMPDSIDANDSEQLEN);
//	        EMVBase_TraceHex("\r\n paypass-info: printFormat", CMPDSIDANDSEQ, CMPDSIDANDSEQLEN);
	        #endif

	        if( 0 != memcmp(CMPDSIDANDSEQ, itemDSID->data, ((itemDSID->len > 19 ) ? 19 : (itemDSID->len))) ) //s456.19
	        {
	        	EMVBase_Trace("paypass-error: DS ID != PAN || PAN Seq Nr\r\n");
				dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S5_E27_1);
            	dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S5_E27_2);
	            return RLT_ERR_EMV_OTHERAPCARD;
	        }
	    }
	}

	FOReveryTinTagsToReadYet(1);

    item = emvbase_avl_gettagitempointer(EMVTAG_DataToSend);
    if((item != NULL) && item->len)
    {
        Paypass_SendDEKSignalAndInitialize(0x01, tempApp_UnionStruct);
    }

	if(tempApp_UnionStruct->PaypassTradeParam->ODAstaus)
	{
	    item = emvbase_avl_gettagitemandstatus(EMVTAG_SDATagList, &TagbExist);

	    if(TagbExist) //if SDA_TL exist
	    {
	        if((item->len == 1) && (item->data[0] == 0x82))
	        {
	            if(tempApp_UnionStruct->PaypassTradeParam->AuthData == NULL)
	            {
	                tempApp_UnionStruct->PaypassTradeParam->AuthDataMaxLen = 0;
	                tempApp_UnionStruct->PaypassTradeParam->AuthData = (unsigned char *)emvbase_malloc(512);

	                if(tempApp_UnionStruct->PaypassTradeParam->AuthData == NULL)
	                {
	                    return RLT_EMV_ERR;
	                }

	                tempApp_UnionStruct->PaypassTradeParam->AuthDataMaxLen = 512;
	                memset(tempApp_UnionStruct->PaypassTradeParam->AuthData, 0, 512);
	            }

	            if((tempApp_UnionStruct->PaypassTradeParam->AuthDataLen + 2) > tempApp_UnionStruct->PaypassTradeParam->AuthDataMaxLen)
	            {
	                if(tempApp_UnionStruct->PaypassTradeParam->AuthDataMaxLen < 1024)
	                {
	                    unsigned char *p = (unsigned char *)emvbase_malloc(1024);
	                    if(p == NULL)
	                    {
	                        return RLT_EMV_ERR;
	                    }
	                    memcpy(p, tempApp_UnionStruct->PaypassTradeParam->AuthData, tempApp_UnionStruct->PaypassTradeParam->AuthDataLen);
	                    emvbase_free(tempApp_UnionStruct->PaypassTradeParam->AuthData);
	                    tempApp_UnionStruct->PaypassTradeParam->AuthData = p;
	                    tempApp_UnionStruct->PaypassTradeParam->AuthDataMaxLen = 1024;
	                }
	                else if(tempApp_UnionStruct->PaypassTradeParam->AuthDataMaxLen < 2048)
	                {
	                    unsigned char *p = (unsigned char *)emvbase_malloc(2048);
	                    if(p == NULL)
	                    {
	                        return RLT_EMV_ERR;
	                    }
	                    memcpy(p, tempApp_UnionStruct->PaypassTradeParam->AuthData, tempApp_UnionStruct->PaypassTradeParam->AuthDataLen);
	                    emvbase_free(tempApp_UnionStruct->PaypassTradeParam->AuthData);
	                    tempApp_UnionStruct->PaypassTradeParam->AuthData = p;
	                    tempApp_UnionStruct->PaypassTradeParam->AuthDataMaxLen = 2048;
	                }
	                else
	                {
	                    return RLT_ERR_EMV_IccDataFormat;
	                }
	            }

	            memcpy((unsigned char *)&tempApp_UnionStruct->PaypassTradeParam->AuthData[tempApp_UnionStruct->PaypassTradeParam->AuthDataLen], AIP, 2);
	            tempApp_UnionStruct->PaypassTradeParam->AuthDataLen += 2;
	        }
	        else
	        {
	            EMVBase_Trace("paypass-error: Static Data Authentication Tag List != '82'\r\n");
				dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S5_E27_1);
            	dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S5_E27_2);
	            return RLT_ERR_EMV_OTHERAPCARD;
	        }
	    }
	    else
	    {
	    	EMVBase_Trace("paypass-error: Static Data Authentication Tag List(Tag9F4A) Missing\r\n");
	    	dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S5_E27_1);
            dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S5_E27_2);
	        return RLT_ERR_EMV_OTHERAPCARD;
	    }
	}

	memset(TermCap, 0, sizeof(TermCap));
    emvbase_avl_gettagvalue_spec(EMVTAG_TermCapab, TermCap, 0, 3);

    if(tempApp_UnionStruct->PaypassTradeParam->PaypassCVMRLEF)
    {
        EMVBase_Trace("paypass-info: cvm required\r\n");
        TermCap[1] = emvbase_avl_gettagvalue(EMVTAG_CVMCapabCVMRequired);
        EMVBase_Trace("paypass-info: CVM-Req DF8118: %02x\r\n", TermCap[1]);
    }
    else
    {
        EMVBase_Trace("paypass-info: no cvm required\r\n");
        TermCap[1] = emvbase_avl_gettagvalue(EMVTAG_CVMCapabNOCVMRequired);
    }
    emvbase_avl_createsettagvalue(EMVTAG_TermCapab, TermCap, 3);
	EMVBase_TraceHex("paypass-info: after set 9F33", TermCap, 3);

	item = emvbase_avl_gettagitempointer(EMVTAG_CVMList);
    if(NULL != item) //if CVM_LIST exist//sxl 080704
    {
        if((item->len) % 2 != 0)
        {
            EMVBase_Trace("paypass-error: len of CVM List(Tag8E) error-(len should be 2*x)\r\n");
			dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S5_E27_1);
            dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S5_E27_2);
            return RLT_ERR_EMV_OTHERAPCARD;
        }
    }

    return RLT_EMV_OK;
}

unsigned char Paypass_ReadAppData_EMV(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char retCode = 0;
    EMVBASETAGCVLITEM *item = NULL;
	EMVBASETAGCVLITEM *itemAFL = NULL;
    unsigned char Read_AFL_Num = 0;
    unsigned char AFL_RecordNum = 0;
    unsigned char AFL_Num = 0;
    EMVBASE_APDU_SEND *apdu_s = NULL;
    EMVBASE_APDU_RESP *apdu_r = NULL;
    unsigned char authdataflag = 0;
	unsigned char ActiveAFL = 1;
	unsigned char bFirstWrite = 0;

    #ifdef PAYPASS_DATAEXCHANGE
    unsigned char IDSStatus = 0;
	unsigned char tagexistflag = 0;
    unsigned char ActiveTag[4] = {0};
	unsigned char ActiveTagLen = 0;
	unsigned char CurrentTag[4] = {0};
	unsigned char CurrentTagLen = 0;
    unsigned char NextCmd = 0;
	#else
	unsigned char AIP[2] = {0};
	unsigned char TermCapab[3] = {0};
    #endif


	EMVBase_Trace("paypass-info: ---EMV Read Application Data---\r\n");

    tempApp_UnionStruct->PaypassTradeParam->AuthDataLen = 0;
    tempApp_UnionStruct->PaypassTradeParam->bErrAuthData = 0;
    tempApp_UnionStruct->PaypassTradeParam->bErrSDATL = 0;

    itemAFL = emvbase_avl_gettagitempointer(EMVTAG_AFL);
    if((itemAFL == NULL) || ((itemAFL->len >> 2) == 0))
    {
    	EMVBase_Trace("paypass-error: AFL(Tag94) missing or error\r\n");

        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S3_E41_0);
        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S3_90_1);
        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S3_90_2);
		return RLT_ERR_EMV_OTHERAPCARD;
    }

    retCode = EMVBase_CheckReadAFL(itemAFL);
    if(retCode != RLT_EMV_OK)
    {
    	EMVBase_Trace("paypass-error: data of AFL(Tag94) error\r\n");

    	dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S3_E41_0);
        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S3_90_1);
        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S3_90_2);
		return RLT_ERR_EMV_OTHERAPCARD;
    }

    AFL_Num = itemAFL->len >> 2;

    if(memcmp(itemAFL->data, "\x08\x01\x01\x00", 4) == 0)
    {
        if((emvbase_avl_gettagvalue(EMVTAG_KernelConfiguration) & 0x80) == 0)
        {
            Read_AFL_Num = 1;
        }
    }

    if(Read_AFL_Num == AFL_Num)
    {
    	EMVBase_Trace("paypass-error: data of AFL(Tag94) error\r\n");

    	dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S3_E41_0);
        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S3_90_1);
        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S3_90_2);
		return RLT_ERR_EMV_OTHERAPCARD;
    }


    #ifndef PAYPASS_DATAEXCHANGE	//20160728_lhd
    tempApp_UnionStruct->PaypassTradeParam->ODAstaus = 0;

    memset(AIP, 0, sizeof(AIP));
    memset(TermCapab, 0, sizeof(TermCapab));
    emvbase_avl_gettagvalue_spec(EMVTAG_AIP, AIP, 0, 2);
    emvbase_avl_gettagvalue_spec(EMVTAG_TermCapab, TermCapab, 0, 3);

	EMVBase_TraceHex("paypass-info: before CDA Check 82", AIP, 2);
	EMVBase_TraceHex("paypass-info: before CDA Check 9F33", TermCapab, 3);

	if((AIP[0] & 0x01) && (TermCapab[2] & 0x08))
    {
		EMVBase_Trace("paypass-info: CDA supported by card and terminal-->Set CDA Flag\r\n");
        tempApp_UnionStruct->PaypassTradeParam->ODAstaus = 1;
    }
    else
    {
        emvbase_avl_setvalue_or(EMVTAG_TVR, 0, 0x80);
    }
    #endif


    apdu_s = (EMVBASE_APDU_SEND *)emvbase_malloc(sizeof(EMVBASE_APDU_SEND));
    apdu_r = (EMVBASE_APDU_RESP *)emvbase_malloc(sizeof(EMVBASE_APDU_RESP));


	NextCmd = CMD_NONE;

	ActiveTagLen = 0;
	memset(ActiveTag, 0, sizeof(ActiveTag));
	memset(apdu_s, 0x00, sizeof(EMVBASE_APDU_SEND));
	retCode = Paypass_GetDataCheck(ActiveTag, &ActiveTagLen, apdu_s);
	if(RLT_EMV_OK == retCode)
	{
		NextCmd = CMD_GET_DATA;
	}
	else
	{
		AFL_RecordNum = itemAFL->data[(Read_AFL_Num << 2) + 1];
		EMVBase_FormReadAflData(apdu_s, AFL_RecordNum, itemAFL->data[(Read_AFL_Num << 2) + 0]);

		EMVBase_Trace("paypass-info: Next Cmd: READ RECORD\r\n");
        NextCmd = CMD_READRECORD;
	}

	*(tempApp_UnionStruct->ppaypassbIsDoNotSaveCurRecvCMDLog) = 1;

	memset(apdu_r, 0x00, sizeof(EMVBASE_APDU_RESP));
    tempApp_UnionStruct->IsoCommand(apdu_s, apdu_r);

	Paypass_IDS(tempApp_UnionStruct);

	while(1)
	{
		EMVBase_Trace("paypass-info: Next Cmd:%d, apdu_r->ReadCardDataOk=%d\r\n",NextCmd,apdu_r->ReadCardDataOk);

		if((CMD_NONE != NextCmd) && (apdu_r->ReadCardDataOk != APDUCOMMANDOK))
	    {
			EMVBase_Trace("paypass-error: apdu timeout or other error\r\n");

			if(tempApp_UnionStruct->EmvErrID->L3 == EMVB_ERRID_L3_STOP)
	        {
	            dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S16_E7);
				retCode = RLT_EMV_TERMINATE_TRANSERR;
	        }
			else
			{
	            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S4_E4_0);
	            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S4_E5_0);
	            dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S4_E6_0);
				retCode = RLT_ERR_EMV_TransRestart;
	        }

			break;
	    }

		if(CMD_GET_DATA == NextCmd)
		{
			if(NULL != tempApp_UnionStruct->paypassSaveDataExchangeCmd)
		    {
		        tempApp_UnionStruct->paypassSaveDataExchangeCmd(5);
		    }

			memcpy(CurrentTag, ActiveTag, 4);
	    	CurrentTagLen = ActiveTagLen;

			ActiveTagLen = 0;
			memset(ActiveTag, 0, sizeof(ActiveTag));
			memset(apdu_s, 0x00, sizeof(EMVBASE_APDU_SEND));
			retCode = Paypass_GetDataCheck(ActiveTag, &ActiveTagLen, apdu_s);
			if(RLT_EMV_OK == retCode)
			{
				NextCmd = CMD_GET_DATA;
			}
			else
			{
				NextCmd = CMD_READRECORD;
                AFL_RecordNum++;
                if(AFL_RecordNum > itemAFL->data[(Read_AFL_Num << 2) + 2])
                {
                    Read_AFL_Num++;
                    for(; Read_AFL_Num < AFL_Num; Read_AFL_Num++)
                    {
                        AFL_RecordNum = itemAFL->data[(Read_AFL_Num << 2) + 1];
                        if(AFL_RecordNum <= itemAFL->data[(Read_AFL_Num << 2) + 2])
                        {
                            break;
                        }
                    }
                    if((Read_AFL_Num >= AFL_Num))
                    {
                    	EMVBase_Trace("paypass-info: Next Cmd: NONE\r\n");
                        NextCmd = CMD_NONE;
                    }
                }

				if(CMD_READRECORD == NextCmd)
				{
					EMVBase_Trace("paypass-info: Next Cmd: READ RECORD\r\n");

					EMVBase_FormReadAflData(apdu_s, AFL_RecordNum, itemAFL->data[(Read_AFL_Num << 2) + 0]);
				}
			}

			retCode = Paypass_GetDataRetData(CurrentTag, CurrentTagLen, apdu_r);
		}
		else if(CMD_READRECORD == NextCmd)
		{
			if(NULL != tempApp_UnionStruct->paypassSaveDataExchangeCmd)
            {
                tempApp_UnionStruct->paypassSaveDataExchangeCmd(4);
            }

			#if 1 //20200603_lhd for 3.1.3 change
			if(AFL_RecordNum < itemAFL->data[(Read_AFL_Num << 2) + 1] + itemAFL->data[(Read_AFL_Num << 2) + 3])
            {
                authdataflag = 1;
            }
            else
            {
                authdataflag = 0;
            }
            retCode = Paypass_ReadAFLRetData(tempApp_UnionStruct, apdu_r, itemAFL->data[(Read_AFL_Num << 2) + 0], authdataflag);
            if(retCode != RLT_EMV_OK)
            {
            	if(retCode == RLT_ERR_EMV_IccReturn)
	            {
	                dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S4_E10_1);
	       			dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S4_E10_2);
	            }
				else
				{
	            	dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S4_E27_1);
	        		dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S4_E27_2);
				}

				retCode = RLT_ERR_EMV_OTHERAPCARD;
				break;
            }

			if( emvbase_avl_checkiftagexist(EMVTAG_CDOL1))
            {
                ForEveryTLinTagList(EMVTAG_CDOL1);
            }

			#if SpecificationBulletinNo239//20200702_lhd
            if(emvbase_avl_checkiftagexist(EMVTAG_DSDOL))
            {
                emvbase_avl_gettagvalue_spec(EMVTAG_IDSStatus, &IDSStatus, 0, 1);
                if(0x80 & IDSStatus)
                {
                    item = emvbase_avl_gettagitempointer(EMVTAG_DSSlotManagementControl);
                    if((item != NULL) && item->len && (0x10 & item->data[0]))
                    {
                        ;
                    }
                    else
                    {
                        ForEveryTLinTagList(EMVTAG_DSDOL);
                    }
                }
            }
			#endif

			//new  S4.36-S4.38
			if(tempApp_UnionStruct->PaypassTradeParam->ODAstaus || (0x04 & emvbase_avl_gettagvalue(EMVTAG_KernelConfiguration)))
			{
				//goto s4.15
				if(0 == tempApp_UnionStruct->PaypassTradeParam->ODAstaus)
				{
					EMVBase_Trace("paypass-info: Read all records even when no CDA\r\n");
				}
			}
			else
			{
				//s4.37
				if(emvbase_avl_checkiftagexist(EMVTAG_AppExpireDate) && emvbase_avl_checkiftagexist(EMVTAG_PAN) &&
					emvbase_avl_checkiftagexist(EMVTAG_PANSeq) && emvbase_avl_checkiftagexist(EMVTAG_AUC) &&
					emvbase_avl_checkiftagexist(EMVTAG_CVMList) && emvbase_avl_checkiftagexist(EMVTAG_IACDefault) &&
					emvbase_avl_checkiftagexist(EMVTAG_IACDenial) && emvbase_avl_checkiftagexist(EMVTAG_IACOnline) &&
					emvbase_avl_checkiftagexist(EMVTAG_IssuCountryCode) && emvbase_avl_checkiftagexist(EMVTAG_Track2Equ) &&
					emvbase_avl_checkiftagexist(EMVTAG_CDOL1) )
					{
						//S4.38
                		EMVBase_Trace("paypass-info: All data present to continue without CDA\r\n");
						ActiveAFL=0;
					}
			}
			//end
			#endif

			//s4.15
			ActiveTagLen = 0;
			memset(ActiveTag, 0, sizeof(ActiveTag));
			memset(apdu_s, 0x00, sizeof(EMVBASE_APDU_SEND));
			retCode = Paypass_GetDataCheck(ActiveTag, &ActiveTagLen, apdu_s);
			if(RLT_EMV_OK == retCode)
			{
				NextCmd = CMD_GET_DATA;
			}
			else
			{
				if(ActiveAFL == 0)
				{
					NextCmd = CMD_NONE;
				}
				else
				{
					NextCmd = CMD_READRECORD;
	                AFL_RecordNum++;
	                if(AFL_RecordNum > itemAFL->data[(Read_AFL_Num << 2) + 2])
	                {
	                    Read_AFL_Num++;
	                    for(; Read_AFL_Num < AFL_Num; Read_AFL_Num++)
	                    {
	                        AFL_RecordNum = itemAFL->data[(Read_AFL_Num << 2) + 1];
	                        if(AFL_RecordNum <= itemAFL->data[(Read_AFL_Num << 2) + 2])
	                        {
	                            break;
	                        }
	                    }
	                    if((Read_AFL_Num >= AFL_Num))
	                    {
	                    	EMVBase_Trace("paypass-info: Next Cmd: NONE\r\n");
	                        NextCmd = CMD_NONE;
	                    }
	                }
				}

				if(CMD_READRECORD == NextCmd)
				{
					EMVBase_Trace("paypass-info: Next Cmd: READ RECORD\r\n");

					EMVBase_FormReadAflData(apdu_s, AFL_RecordNum, itemAFL->data[(Read_AFL_Num << 2) + 0]);
				}
			}
		}

		if(CMD_NONE == NextCmd) // S456-NONE
		{
			bFirstWrite = 0;
            item = emvbase_avl_gettagitempointer(EMVTAG_ProceedToFirstWriteFlag);
            if(item != NULL)
            {
				EMVBase_Trace("paypass-info: EMVTAG_ProceedToFirstWriteFlag->len = %d\r\n", item->len);
                if(item->len == 0)
                {
					AddToList(0x01, EMVTAG_ProceedToFirstWriteFlag, 0, NULL, EMVTAG_DataNeeded);
					bFirstWrite = 1;
                }
                else if((1 == item->len) && (0x00 == item->data[0]))
                {
					bFirstWrite = 1;
                }
            }
			EMVBase_Trace("paypass-info: bFirstWrite = %d\r\n", bFirstWrite);

			if(bFirstWrite)
            {
                FOReveryTinTagsToReadYet(0);

				if(emvbase_avl_checkiftagexist(EMVTAG_DataNeeded) ||
					(emvbase_avl_checkiftagexist(EMVTAG_DataToSend) && !emvbase_avl_checkiftagexist(EMVTAG_TagsToReadYet)))
		        {
		            Paypass_SendDEKSignalAndInitialize(0x03, tempApp_UnionStruct);
		        }

				retCode = Paypass_ResvDEKSignal(0x03, 0, tempApp_UnionStruct);
				EMVBase_Trace("paypass-info: Paypass_ResvDEKSignal retCode = %d\r\n", retCode);

				if(RLT_EMV_OK != retCode)
		        {
		            if(RLT_ERR_EMV_APDUTIMEOUT == retCode)
		            {
		                dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S13_42_0);
		        		dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S6_3);
		                break;
		            }
		            else if(RLT_ERR_EMV_CancelTrans == retCode)
		            {
		                dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S7_8);
		                break;
		            }

		            retCode = RLT_EMV_TERMINATE_TRANSERR;
					break;
		        }

                if(NULL != tempApp_UnionStruct->paypassSaveDataExchangeCmd)
                {
                    tempApp_UnionStruct->paypassSaveDataExchangeCmd(6);
                }

				ActiveTagLen = 0;
				memset(ActiveTag, 0, sizeof(ActiveTag));
				memset(apdu_s, 0x00, sizeof(EMVBASE_APDU_SEND));
				retCode = Paypass_GetDataCheck(ActiveTag, &ActiveTagLen, apdu_s); // S6.8
				if(RLT_EMV_OK == retCode)
				{
					NextCmd = CMD_GET_DATA;
				}
				else
				{
					NextCmd = CMD_NONE;
					//retCode = RLT_EMV_OK;
					//break;
				}
            }
			else
			{
				retCode = RLT_EMV_OK;
				break;
			}
		}

		if(CMD_NONE != NextCmd)
		{
			*(tempApp_UnionStruct->ppaypassbIsDoNotSaveCurRecvCMDLog) = 1;

			memset(apdu_r, 0x00, sizeof(EMVBASE_APDU_RESP));
		    tempApp_UnionStruct->IsoCommand(apdu_s, apdu_r);
		}

		if(CMD_READRECORD == NextCmd)
		{
			EMVBase_Trace("paypass-info: S456.2\r\n");

			FOReveryTinTagsToReadYet(0); // S456.2

            if(emvbase_avl_checkiftagexist(EMVTAG_DataToSend) && (0 == emvbase_avl_checkiftagexist(EMVTAG_TagsToReadYet)))
            {
                Paypass_SendDEKSignalAndInitialize(0x03, tempApp_UnionStruct);
                Paypass_ResvDEKSignal(0x03, 0x01, tempApp_UnionStruct);
            }
		}
	}

    if(NULL != tempApp_UnionStruct->paypassSaveDataExchangeCmd)
    {
        tempApp_UnionStruct->paypassSaveDataExchangeCmd(10);
    }

    emvbase_free(apdu_r);
    emvbase_free(apdu_s);

	if(RLT_EMV_OK != retCode)
	{
		return retCode;
	}

    return Paypass_DealAFLData_EMV(tempApp_UnionStruct);
}

unsigned char Paypass_ReadAppData(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char retCode = 0;


    if(tempApp_UnionStruct->PaypassTradeParam->qPBOCOrMSD == TRANSFLOW_EMVMODE)
    {
    	retCode = Paypass_ReadAppData_EMV(tempApp_UnionStruct);
    }
	else
	{
    	retCode = Paypass_ReadAppData_MS(tempApp_UnionStruct);
    }

    return retCode;
}


void dllpaypass_checkavn(PAYPASSTradeUnionStruct *tempApp_UnionStruct) // ����������
{
    unsigned char AppVerbEixst;
    EMVBASETAGCVLITEM *AppVeritem = NULL;
    unsigned char TermAppVer[2];


    if(tempApp_UnionStruct->PaypassTradeParam->qPBOCOrMSD == TRANSFLOW_EMVMODE)
    {
        AppVeritem = emvbase_avl_gettagitemandstatus(EMVTAG_AppVer, &AppVerbEixst);

        if(AppVerbEixst == 0) //App version num is not exist,don't check it and continue following	process
        { return; }
        emvbase_avl_gettagvalue_spec(EMVTAG_AppVerNum, TermAppVer, 0, 2);

        #ifdef EMVB_DEBUG
        	EMVBase_Trace("\r\n paypass-info:tempcardinfo->AppVer= %02x %02x %02x %02x \r\n", AppVeritem->data[0], AppVeritem->data[1], TermAppVer[0], TermAppVer[1]);
        #endif

        if(memcmp(AppVeritem->data, TermAppVer, 2))
        {
            EMVBase_Trace("\r\n paypass-error: application version is different !!! \r\n");
			EMVBase_TraceHex("\r\n paypass-error: Card App Ver(9F08)",AppVeritem->data,2);
			EMVBase_TraceHex("\r\n paypass-error: Terminal App Ver(9F09)",TermAppVer,2);

            emvbase_avl_setvalue_or(EMVTAG_TVR, 1, 0x80);
        }
    }

}

unsigned char Paypass_DOLProcess(unsigned char type, unsigned char *DOL, unsigned short DOLLen, unsigned char *DOLData, unsigned short *DOLDataLen, unsigned short DOLDataMaxLen, PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
	EMVBASETAGCVLITEM *DOLdataitem;
	unsigned char *buf = NULL;
	unsigned short templen, indexOut = 0;
	unsigned short inPutBufMaxLen = 0, LastDsDolTagLen = 0;
	unsigned short k, m;
	unsigned short index = 0;
	unsigned char bInTable;
	unsigned char tag[4], taglen;
	unsigned char bHasUnpredictNum = 0;
	unsigned char AIP[2], TermCapab[3];
	unsigned char errflag = 0;


	if ( (NULL == DOL) || (NULL == DOLData) || (NULL == tempApp_UnionStruct) || ( 0 >= DOLDataMaxLen) || (DOLDataLen == NULL))
	{
		return RLT_EMV_ERR;
	}

	inPutBufMaxLen = DOLDataMaxLen;
	*DOLDataLen = 0;

	memset(AIP, 0, sizeof(AIP));
	emvbase_avl_gettagvalue_spec(EMVTAG_AIP, AIP, 0, 2);
	memset(TermCapab, 0, sizeof(TermCapab));
	emvbase_avl_gettagvalue_spec(EMVTAG_TermCapab, TermCapab, 0, 3);

	buf = (unsigned char *)emvbase_malloc(256);
	if (NULL == buf)
	{
		*DOLDataLen = 0;
		EMVBase_Trace("paypass-error: Memory error\r\n");
		return RLT_EMV_ERR;
	}
	errflag = 0;
	while(index < DOLLen) //Process PDOL
	{
		if(DOL[index] == 0xFF || DOL[index] == 0x00)
		{
			index++;
			continue;
		}
		memset(buf, 0, 255);
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

		DOLdataitem = emvbase_avl_gettagitempointer(tag);

		if(DOLdataitem != NULL)
		{
			index += taglen;

			if(index + 1 > DOLLen)
			{
				emvbase_free(buf);
				EMVBase_Trace("paypass-error: DOL error\r\n");
				return RLT_ERR_EMV_IccDataFormat;
			}
			templen = DOL[index++];

			k = templen;   // 1
			m = DOLdataitem->len; // 4

			if(tempApp_UnionStruct->PaypassTradeParam->qPBOCOrMSD == TRANSFLOW_MSDMODE)
			{
				if(memcmp(tag, "\x9F\x6A", 2) == 0 && k)
				{
					if(m)
					{
						bHasUnpredictNum = 1;
					}
				}
			}
			else
			{
				if(memcmp(tag, "\x9F\x37", 2) == 0 && k == 4)
				{
					if(m)
					{
						bHasUnpredictNum = 1;
					}
				}
			}

			if(DOLdataitem->datafomat & EMVTAGFORMAT_N) //numeric
			{
				if(k >= m)
				{
					if(m)
					{
						memcpy(&buf[k - m], DOLdataitem->data, m);
					}

					if ((indexOut + k) > inPutBufMaxLen)
					{
						errflag = 1;
						break;
					}
					else
					{
						memcpy(&DOLData[indexOut], buf, k);
					}
				}
				else
				{
					if(m)
					{
						memcpy(buf, DOLdataitem->data, m);
					}

					if ((indexOut + k) > inPutBufMaxLen)
					{
						errflag = 1;
						break;
					}
					else
					{
						memcpy(&DOLData[indexOut], &buf[m - k], k);
					}
				}

			}
			else if(DOLdataitem->datafomat & EMVTAGFORMAT_CN) //compact numeric
			{
				if(m)
				{
					memset(buf, 0xFF, 255);
					memcpy(buf, DOLdataitem->data, m);
				}
				else
				{
					memset(buf, 0x00, 255);
				}
				if ((indexOut + k) > inPutBufMaxLen)
				{
					errflag = 1;
					break;
				}
				else
				{
					memcpy(&DOLData[indexOut], buf, k);
				}
			}
			else//other formats
			{
				if(m)
				{
					memcpy(buf, DOLdataitem->data, m);
				}

				if ((type == typeDSDOL) && (index >= DOLLen))	//last tag in DSDOL
				{
					if( k > m )
					{
						LastDsDolTagLen = m;
					}
					else
					{
						LastDsDolTagLen = k;
					}
					if ((indexOut + LastDsDolTagLen) > inPutBufMaxLen)
					{
						errflag = 1;
						break;
					}
					else
					{
						memcpy(&DOLData[indexOut], buf, LastDsDolTagLen);
						indexOut += LastDsDolTagLen;
						bInTable = 1;
						break;
					}
				}
				else
				{
					if ((indexOut + k) > inPutBufMaxLen)
					{
						errflag = 1;
						break;
					}
					else
					{
						memcpy(&DOLData[indexOut], buf, k);
					}
				}
			}

			if(0x57 == tag[0])
			{
				/*if(type == typeDRDOL)
				{
					EMVBase_Trace("EMV-info: Tag57 in DRDOL\r\n");
					tempEmvContact_UnionStruct->EmvTradeParam->Track2OffsetInDDOLData = indexOut;
					tempEmvContact_UnionStruct->EmvTradeParam->bTrack2InDDOLData = 1;
				}
				else */if(type == typeCDOL1)
				{
					EMVBase_Trace("paypass-info: Tag57 in CDOL1\r\n");
					tempApp_UnionStruct->PaypassTradeParam->Track2OffsetInCDOL1Data = indexOut;
					tempApp_UnionStruct->PaypassTradeParam->bTrack2InCDOL1Data = 1;
				}
			}

			indexOut += k;
			bInTable = 1;
		}

		if(!bInTable)
		{
			index += taglen;
			if(index + 1 > DOLLen)
			{
				emvbase_free(buf);
				EMVBase_Trace("paypass-error: DOL error\r\n");
				return RLT_ERR_EMV_IccDataFormat;
			}
			templen = DOL[index++];
			k = templen;

			if ((indexOut + k) > inPutBufMaxLen)
			{
				errflag = 1;
				break;
			}
			else
			{
				memcpy(&DOLData[indexOut], buf, k);
				indexOut += k;
			}
		}
	}

	emvbase_free(buf);

	if (errflag)
	{
		*DOLDataLen = indexOut;
		EMVBase_Trace("paypass-error: DOL related data len over\r\n");
		return RLT_EMV_ERR;
	}

	if(bHasUnpredictNum == 0)
	{
		if(type == typeUDOL)
		{
			EMVBase_Trace("paypass-error: Tag9F6A missing in UDOL(Tag9F69)\r\n");
			return RLT_EMV_ERR;
		}
		else if(type == typeCDOL1)
		{
			EMVBase_Trace("paypass-error: Tag9F37 missing in CDOL1(Tag8C)\r\n");
			tempApp_UnionStruct->PaypassTradeParam->bCDOL1HasNoUnpredictNum = 1;
			return RLT_EMV_ERR;
		}
	}

	*DOLDataLen = indexOut;
	return RLT_EMV_OK;

}

unsigned char Paypass_CCCRetData(PAYPASSTradeUnionStruct *tempApp_UnionStruct, EMVBASE_APDU_RESP *apdu_r)
{
    unsigned short index = 0, indexTemp = 0, lenTemp = 0;
    unsigned char retCode = 0;
    unsigned char bIntable = 0;


    EMVBase_Trace("paypass-info: ---response message of CCC parsing---\r\n");

    index = 0;
    if(apdu_r->SW1 == 0x90 && apdu_r->SW2 == 0x00)
    {
        if(apdu_r->DataOut[index] == 0x77) //TLV coded data
        {
        	EMVBase_Trace("paypass-info: CCC template 77\r\n");

            index++;
            if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &lenTemp))
            {
            	EMVBase_Trace("paypass-error: Tag77 parsing error\r\n");
                return RLT_ERR_EMV_IccDataFormat;
            }

            indexTemp = index;
            while(index < indexTemp + lenTemp)
            {
                if(index >= apdu_r->LenOut)
                {
                	EMVBase_Trace("paypass-error: r-apdu parsing error\r\n");
					EMVBase_Trace("paypass-error: sub Tags' len error\r\n");
                    return RLT_ERR_EMV_IccDataFormat;
                }
                if(apdu_r->DataOut[index] == 0xFF || apdu_r->DataOut[index] == 0x00)
                {
                    index++;
                    continue;
                }

                bIntable = 0;
                retCode = Paypass_ParseAndStoreCardResponse(tempApp_UnionStruct, apdu_r->DataOut, &index, &bIntable, 2); //20160713_lhd
                if(retCode != RLT_EMV_OK)
                {
                	EMVBase_Trace("paypass-error: sub Tags in Tag77 parsing error\r\n");
                    return retCode;
                }
            }
            if(index != indexTemp + lenTemp)
            {
            	EMVBase_Trace("paypass-error: Tag77's value parsing error\r\n");
				EMVBase_Trace("paypass-error: sub Tags' len error\r\n");
                return RLT_ERR_EMV_IccDataFormat;
            }

        }
        else
        {
			EMVBase_Trace("paypass-error: CCC template error (Byte1 != 77)\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }

        if(index != apdu_r->LenOut)
        {
			EMVBase_Trace("paypass-error: r-apdu data error\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }

    }
    else
    {
		EMVBase_Trace("paypass-error: SW != 9000\r\n");
        return RLT_ERR_EMV_IccReturn;
    }

    return RLT_EMV_OK;
}

unsigned char Paypass_DealCCCRetData(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char i;    //,k
    unsigned char *tmp, *tmpcarddata, tmppos, tmplen, startpos, cnt, startcount; //[200]	[200]
    unsigned char bcdcvc3track2[24], bcdcvc3track1[24], *tmpbuf, bcdatc[24]; //[200]
    unsigned int  intcvc3track2, intcvc3track1, intatc;
    unsigned char qtrack2, j, tmpvalue, mtrack2, ktrack2, mtrack1, qtrack1, ktrack1;
    unsigned char qtrack2dexist, qtrack1dexist, separatorcnt;
    unsigned char *posbuf, posbuflen;  //[200]
    unsigned char *ddcarddata, ddcarddatalen;  //[200]
    unsigned char tmpbuftrack[24];

    unsigned char PaypassCCCRespType;
    unsigned char AIP[2], Kernelconfig;
    unsigned char PaypassnUN1;
    unsigned char POSCII[3];
    EMVBASETAGCVLITEM *item;
    unsigned char PaypassCVC3Track2[2];
    unsigned char PaypassPcvc3Track2[2];
    unsigned char PaypassPunatcTrack2[2];
    unsigned char paypassun[4];
    unsigned char ATC[2];

    unsigned char PaypassCVC3Track1[2];
    unsigned char PaypassPCVC3Track1[6];
    unsigned char PaypassPunatcTrack1[6];
    unsigned char *mallocbuf = NULL;


    PaypassCCCRespType = 0;
    memset(AIP, 0, sizeof(AIP));
    emvbase_avl_gettagvalue_spec(EMVTAG_AIP, AIP, 0, 2);
    Kernelconfig = emvbase_avl_gettagvalue(EMVTAG_KernelConfiguration);
    if((AIP[0] & 0x02) != 0 && (Kernelconfig & 0x20))
    {
        PaypassCCCRespType = 1;
    }

    if(PaypassCCCRespType == 0)	//CCC response-1
    {
		EMVBase_Trace("paypass-info: --CCC response 1--\r\n");

        if(emvbase_avl_checkiftagexist(EMVTAG_ATC) == 0)
        {
        	EMVBase_Trace("paypass-error: ATC(Tag9F36) Missing\r\n");
            return RLT_ERR_EMV_IccDataMissing;
        }

        if(emvbase_avl_checkiftagexist(EMVTAG_POSCII) == 0 && emvbase_avl_checkiftagexist(EMVTAG_PaypassCVC3Track2) == 0)
        {
        	EMVBase_Trace("paypass-error: POS Cardholder Interaction Information(TagDF4B) and CVC3-Track2(Tag9F61) Missing\r\n");
            return RLT_ERR_EMV_IccDataMissing;
        }

        if(emvbase_avl_checkiftagexist(EMVTAG_POSCII) && emvbase_avl_checkiftagexist(EMVTAG_PaypassCVC3Track2) == 0)
        {
            return PAYPASS_PHONECCCPCIIONLYE;
        }

        if(emvbase_avl_checkiftagexist(EMVTAG_POSCII) == 0 && emvbase_avl_checkiftagexist(EMVTAG_PaypassCVC3Track2))
        {
            PaypassnUN1 = tempApp_UnionStruct->PaypassTradeParam->PaypassnUN;
        }
        else
        {
            //S14.6
            PaypassnUN1 = tempApp_UnionStruct->PaypassTradeParam->PaypassnUN;
            memset(POSCII, 0, sizeof(POSCII));
            emvbase_avl_gettagvalue_spec(EMVTAG_POSCII, POSCII, 0, 3);
            if(POSCII[1] & 0x10) //Offline PIN success
            {
                //S14.8
                PaypassnUN1 = (PaypassnUN1 + 5) % 10;
            }
        }
    }
    else   //CCC response-2
    {
    	EMVBase_Trace("paypass-info: --CCC response 2--\r\n");

        if(emvbase_avl_checkiftagexist(EMVTAG_ATC) == 0)
        {
        	EMVBase_Trace("paypass-error: ATC(Tag9F36) Missing\r\n");
            return RLT_ERR_EMV_IccDataMissing;
        }
		if(emvbase_avl_checkiftagexist(EMVTAG_POSCII) == 0)
        {
        	EMVBase_Trace("paypass-error: POS Cardholder Interaction Information(TagDF4B) Missing\r\n");
            return RLT_ERR_EMV_IccDataMissing;
        }

        if(emvbase_avl_checkiftagexist(EMVTAG_PaypassCVC3Track2) == 0)
        {
            return PAYPASS_PHONECCCPCIIONLYE;
        }

        PaypassnUN1 = tempApp_UnionStruct->PaypassTradeParam->PaypassnUN;
        memset(POSCII, 0, sizeof(POSCII));
        emvbase_avl_gettagvalue_spec(EMVTAG_POSCII, POSCII, 0, 3);
        if(POSCII[1] & 0x10) //Offline PIN success
        {
            //S14.24
            PaypassnUN1 = (PaypassnUN1 + 5) % 10;
        }
        else
        {
        	//S14.21
            if(tempApp_UnionStruct->PaypassTradeParam->PaypassCVMRLEF)
            {
				EMVBase_Trace("paypass-error: CVM Required but OD-CVM verification fails\r\n");
                return RLT_ERR_EMV_IccDataFormat;
            }
            *(tempApp_UnionStruct->paypassccctimecnt) = 0;
        }
    }

    if(emvbase_avl_checkiftagexist(EMVTAG_PayPassTrack1Data))
    {
        if(emvbase_avl_checkiftagexist(EMVTAG_PaypassCVC3Track1) == 0)
        {
			EMVBase_Trace("paypass-error: Track1(Tag56) present and CVC3-Track1(Tag9F60) not present\r\n");
            return RLT_ERR_EMV_IccDataMissing;
        }
    }


	mallocbuf = (unsigned char *)emvbase_malloc(1024);
    tmp = &mallocbuf[0];
    tmpcarddata = &mallocbuf[200];
    tmpbuf = &mallocbuf[400];
    posbuf = &mallocbuf[600];
    ddcarddata = &mallocbuf[800];

	item = emvbase_avl_gettagitempointer(EMVTAG_PaypassMagTrack2);
	memset(tmp, 0, 200);
    EMVBaseBcdToAsc(tmp, item->data, item->len);
    tmplen = (item->len) << 1;


    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-info: transfer data\r\n");
    #endif
    qtrack2dexist = 0;
    for(i = 0; i < tmplen; i++)
    {
        if(tmp[i] == 'D')
        {
            qtrack2dexist = 1;
            if(i > 19)
            {
                #ifdef EMVB_DEBUG
                EMVBase_Trace("\r\n paypass-error:PayPass_CryptochecksumRetData1111\r\n");
                #endif
                emvbase_free(mallocbuf);
                return RLT_ERR_EMV_IccDataFormat;
            }

            if((tmp[tmplen - 1] == 'F') || (tmp[tmplen - 1] == 'f'))
            {
                if((i + 8 + 1) > tmplen)
                {
                    #ifdef EMVB_DEBUG
                    EMVBase_Trace("\r\n paypass-error:PayPass_CryptochecksumRetDat4444\r\n");
                    #endif
                    emvbase_free(mallocbuf);
                    return RLT_ERR_EMV_IccDataFormat;
                }
            }
            else
            {
                if((i + 8) > tmplen)
                {
                    #ifdef EMVB_DEBUG
                    EMVBase_Trace("\r\n paypass-error:PayPass_CryptochecksumRetDat5555\r\n");
                    #endif
                    emvbase_free(mallocbuf);
                    return RLT_ERR_EMV_IccDataFormat;
                }
            }
            memcpy(tmpcarddata, tmp, tmplen);
            tmppos = i + 8;
            break;
        }
    }

    if(qtrack2dexist == 0)
    {
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error:PayPass_CryptochecksumRetData3333\r\n");
        #endif
        emvbase_free(mallocbuf);
        return RLT_ERR_EMV_IccDataFormat;
    }

    if((tmp[tmplen - 1] == 'F') || (tmp[tmplen - 1] == 'f'))
    {
        mtrack2 = tmplen - i - 8 - 1;
    }
    else
    {
        mtrack2 = tmplen - i - 8;
    }

    memcpy(ddcarddata, &tmp[tmppos], tmplen - tmppos);
    ddcarddatalen = tmplen - tmppos;

    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-info:mtrack2 = %d \r\n", mtrack2);
    #endif

    //binary encoded CVC3TRACK2 to the BCD encoding
    memset(PaypassCVC3Track2, 0, sizeof(PaypassCVC3Track2));
    emvbase_avl_gettagvalue_spec(EMVTAG_PaypassCVC3Track2, PaypassCVC3Track2, 0, 2);
    intcvc3track2 = PaypassCVC3Track2[0];
    intcvc3track2 = (intcvc3track2 << 8) + PaypassCVC3Track2[1];
    EMVBaseU32ToBcd(bcdcvc3track2, intcvc3track2, 8);
    memset(tmpbuf, 0, 200);
    EMVBaseBcdToAsc(tmpbuf, bcdcvc3track2, 8);

    #ifdef EMVB_DEBUG
    EMVBase_TraceHex("\r\n paypass-info: bcdcvc3track2", tmpbuf, 16);
    #endif

    qtrack2 = 0;
    cnt = 0;
    startcount = 0;
    posbuflen = 0;
    memset(posbuf, 0, 200);

    memset(PaypassPcvc3Track2, 0, sizeof(PaypassPcvc3Track2));
    emvbase_avl_gettagvalue_spec(EMVTAG_PaypassPcvc3Track2, PaypassPcvc3Track2, 0, 2);
    for(i = 0; i < 2; i++)
    {
        tmpvalue = PaypassPcvc3Track2[i];
        for(j = 0; j < 8; j++)
        {
            if(startcount == 0)
            {
                if((16 - cnt) == mtrack2)
                {
                    startpos = cnt;
                    startcount = 1;
                }
            }
            if(tmpvalue & 0x80)
            {
                if(startcount)
                {
                    posbuf[posbuflen++] = tmppos + cnt - startpos;
                    qtrack2++;
                    #ifdef EMVB_DEBUG
                    EMVBase_Trace("\r\n paypass-error:posbuf[posbuflen++]2 %d %02x", posbuflen - 1, posbuf[posbuflen - 1]);
                    #endif
                }
            }
            tmpvalue = tmpvalue << 1;
            cnt++;
        }
    }

    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-info:qtrack2 = %d\r\n", qtrack2);
    #endif
    if(qtrack2 < 3)
    {
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error:PayPass_CryptochecksumRetData333a\r\n");
        #endif
        emvbase_free(mallocbuf);
        return RLT_ERR_EMV_IccDataFormat;
    }

    //deal CVC3TRACK2
    //unusedpos = tmppos;
    for(i = 0; i < qtrack2; i++)
    {
        tmpcarddata[posbuf[i]] = tmpbuf[16 + i - qtrack2];
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-info:tmpcarddata[posbuf[i]] = %d %02x", posbuf[i], tmpcarddata[posbuf[i]]);
        #endif
        //unusedpos = posbuf[i];
    }

    //deal ATC and UN
    ktrack2 = 0;
    cnt = 0;
    startcount = 0;
    posbuflen = 0;
    memset(posbuf, 0, 200);
    memset(PaypassPunatcTrack2, 0, sizeof(PaypassPunatcTrack2));
    emvbase_avl_gettagvalue_spec(EMVTAG_PaypassPunatcTrack2, PaypassPunatcTrack2, 0, 2);
    for(i = 0; i < 2; i++)
    {
        tmpvalue = PaypassPunatcTrack2[i];
        for(j = 0; j < 8; j++)
        {
            if(startcount == 0)
            {
                if((16 - cnt) == mtrack2)
                {
                    startpos = cnt;
                    startcount = 1;
                }
            }
            if(tmpvalue & 0x80)
            {
                if(startcount)
                {
                    posbuf[posbuflen++] = tmppos + cnt - startpos;
                    ktrack2++;
                    #ifdef EMVB_DEBUG
                    EMVBase_Trace("\r\n paypass-error:posbuf[posbuflen++]1 %d %02x", posbuflen - 1, posbuf[posbuflen - 1]);
                    #endif
                }
            }
            tmpvalue = tmpvalue << 1;
            cnt++;
        }
    }

    memset(tmpbuf, 0, 200);
    memset(tmpbuftrack, 0, sizeof(tmpbuftrack));
    memset(paypassun, 0, sizeof(paypassun));
    emvbase_avl_gettagvalue_spec(EMVTAG_paypassun, paypassun, 0, 4);
    memcpy(&tmpbuftrack[4], paypassun, 4);
    EMVBaseBcdToAsc(tmpbuf, tmpbuftrack, 8);
    for(i = 0; i < (tempApp_UnionStruct->PaypassTradeParam->PaypassnUN); i++)
    {
        tmpcarddata[posbuf[i + ktrack2 - tempApp_UnionStruct->PaypassTradeParam->PaypassnUN]] = tmpbuf[16 + i - tempApp_UnionStruct->PaypassTradeParam->PaypassnUN];
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-info:111 %d %02x \r\n", posbuf[i + ktrack2 - tempApp_UnionStruct->PaypassTradeParam->PaypassnUN], tmpbuf[8 + i - tempApp_UnionStruct->PaypassTradeParam->PaypassnUN]);
        #endif
    }

    memset(ATC, 0, sizeof(ATC));
    emvbase_avl_gettagvalue_spec(EMVTAG_ATC, ATC, 0, 2);
    intatc = ATC[0];
    intatc = (intatc << 8) + ATC[1];
    EMVBaseU32ToBcd(bcdatc, intatc, 8);
    memset(tmpbuf, 0, 200);
    EMVBaseBcdToAsc(tmpbuf, bcdatc, 8);



    for(i = 0; i < (ktrack2 - tempApp_UnionStruct->PaypassTradeParam->PaypassnUN); i++)
    {
        tmpcarddata[posbuf[i]] = tmpbuf[16 + i - ktrack2 + tempApp_UnionStruct->PaypassTradeParam->PaypassnUN];
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-info:112 %d %02x \r\n", posbuf[i], tmpbuf[16 + i - ktrack2 + tempApp_UnionStruct->PaypassTradeParam->PaypassnUN]);
        #endif
    }



    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-info:tempterminfo->PaypassnUN = %d %d\r\n", tempApp_UnionStruct->PaypassTradeParam->PaypassnUN, tmppos + mtrack2 - 1);
    #endif

    tmpcarddata[tmppos + mtrack2 - 1] = PaypassnUN1 + 0x30;	//20130109

    //9f6B
    if(tmplen > 38)
    {
        tmplen = 38;
    }

    EMVBaseAscToBcd(tmp, tmpcarddata, tmplen);
    emvbase_avl_createsettagvalue(EMVTAG_PaypassMagTrack2, tmp, tmplen >> 1);

    #if 0 /*Modify by luohuidong at 2017.02.09  11:28 是否要添加待确定 */
    if((ddcarddata[ddcarddatalen - 1] == 'F') || (ddcarddata[ddcarddatalen - 1] == 'f'))
    {
        ddcarddatalen = ddcarddatalen - 1;
    }
    #endif /* if 1 */

    emvbase_avl_createsettagvalue(EMVTAG_Paypassconverttrack2, ddcarddata, ddcarddatalen);


    //如果一磁道数据存在，转化一磁道数据
    if(emvbase_avl_checkiftagexist(EMVTAG_PayPassTrack1Data))
    {
        item = emvbase_avl_gettagitempointer(EMVTAG_PayPassTrack1Data);

		memset(tmp, 0, 200);
        memcpy(tmp, item->data, item->len);
        tmplen = item->len;


        qtrack1dexist = 0;
        separatorcnt = 0;
        cnt = 0;
        for(i = 1; i < tmplen; i++)
        {
            if(tmp[i] == 0x5E)
            {
                //qtrack2dexist = 1;
                if(separatorcnt == 0)//PAN
                {
                    if(cnt > 19)
                    {
                        #ifdef EMVB_DEBUG
                        EMVBase_Trace("\r\n paypass-error:PayPass_CryptochecksumRetData1111\r\n");
                        #endif
                        emvbase_free(mallocbuf);
                        return RLT_ERR_EMV_IccDataFormat;
                    }
                    separatorcnt = 1;
                }
                else if(separatorcnt == 1)//name
                {
                    if((cnt > 26 || cnt < 2) && cnt != 0)
                    {
                        #ifdef EMVB_DEBUG
                        EMVBase_Trace("\r\n paypass-error:PayPass_CryptochecksumRetData5555\r\n");
                        #endif
                        emvbase_free(mallocbuf);
                        return RLT_ERR_EMV_IccDataFormat;
                    }

                    if((i + 8) > tmplen)
                    {
                        #ifdef EMVB_DEBUG
                        EMVBase_Trace("\r\n paypass-error:PayPass_CryptochecksumRetData6666\r\n");
                        #endif
                        emvbase_free(mallocbuf);
                        return RLT_ERR_EMV_IccDataFormat;
                    }
                    qtrack1dexist = 1;
                    memcpy(tmpcarddata, tmp, tmplen);
                    tmppos = i + 8;
                    break;
                }
                cnt = 0;
            }
            else
            {
                cnt++;
            }
        }

        if(qtrack1dexist == 0)
        {
            #ifdef EMVB_DEBUG
            EMVBase_Trace("\r\n paypass-error:PayPass_CryptochecksumRetData333b\r\n");
            #endif
            emvbase_free(mallocbuf);
            return RLT_ERR_EMV_IccDataFormat;
        }

        mtrack1 = tmplen - i - 8;

        memcpy(ddcarddata, &tmp[tmppos], tmplen - tmppos);
        ddcarddatalen = tmplen - tmppos;

        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-info:mtrack1 = %d \r\n", mtrack1);
        #endif

        //binary encoded CVC3TRACK2 to the BCD encoding
        memset(PaypassCVC3Track1, 0, sizeof(PaypassCVC3Track1));
        emvbase_avl_gettagvalue_spec(EMVTAG_PaypassCVC3Track1, PaypassCVC3Track1, 0, 2);
        intcvc3track1 = PaypassCVC3Track1[0];
        intcvc3track1 = (intcvc3track1 << 8) + PaypassCVC3Track1[1];
        EMVBaseU32ToBcd(bcdcvc3track1, intcvc3track1, 24);
        memset(tmpbuf, 0, 200);
        EMVBaseBcdToAsc(tmpbuf, bcdcvc3track1, 24);

        qtrack1 = 0;
        cnt = 0;
        startcount = 0;
        posbuflen = 0;
        memset(posbuf, 0, 200);

        memset(PaypassPCVC3Track1, 0, sizeof(PaypassPCVC3Track1));
        emvbase_avl_gettagvalue_spec(EMVTAG_PaypassPCVC3Track1, PaypassPCVC3Track1, 0, 6);
        for(i = 0; i < 6; i++)
        {
            tmpvalue = PaypassPCVC3Track1[i];
            for(j = 0; j < 8; j++)
            {
                if(startcount == 0)
                {
                    if((48 - cnt) == mtrack1)
                    {
                        startpos = cnt;
                        startcount = 1;
                    }
                }
                if(tmpvalue & 0x80)
                {
                    if(startcount)
                    {
                        posbuf[posbuflen++] = tmppos + cnt - startpos;
                        qtrack1++;
                    }
                }
                tmpvalue = tmpvalue << 1;
                cnt++;
            }
        }
        if(qtrack1 < 3)
        {
            #ifdef EMVB_DEBUG
            EMVBase_Trace("\r\n paypass-error:PayPass_CryptochecksumRetData333f\r\n");
            #endif
            emvbase_free(mallocbuf);
            return RLT_ERR_EMV_IccDataFormat;
        }

        //deal CVC3TRACK2
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-info:qtrack1 %d\r\n", qtrack1);
        #endif
        for(i = 0; i < qtrack1; i++)
        {
            tmpcarddata[posbuf[i]] = tmpbuf[48 + i - qtrack1];
            #ifdef EMVB_DEBUG
            EMVBase_Trace("\r\n paypass-info:posbuf[i] %d %02x \r\n", posbuf[i], tmpcarddata[posbuf[i]]);
            #endif
        }

        //deal ATC and UN
        ktrack1 = 0;
        cnt = 0;
        startcount = 0;
        posbuflen = 0;
        memset(posbuf, 0, 200);
        memset(PaypassPunatcTrack1, 0, sizeof(PaypassPunatcTrack1));
        emvbase_avl_gettagvalue_spec(EMVTAG_PaypassPunatcTrack1, PaypassPunatcTrack1, 0, 6);
        for(i = 0; i < 6; i++)
        {
            tmpvalue = PaypassPunatcTrack1[i];
            for(j = 0; j < 8; j++)
            {
                if(startcount == 0)
                {
                    if((48 - cnt) == mtrack1)
                    {
                        startpos = cnt;
                        startcount = 1;
                    }
                }
                if(tmpvalue & 0x80)
                {
                    if(startcount)
                    {
                        posbuf[posbuflen++] = tmppos + cnt - startpos;
                        ktrack1++;
                    }
                }
                tmpvalue = tmpvalue << 1;
                cnt++;
            }
        }

        memset(tmpbuf, 0, 200);
        memset(tmpbuftrack, 0, sizeof(tmpbuftrack));
        memcpy(&tmpbuftrack[20], paypassun, 4);
        EMVBaseBcdToAsc(tmpbuf, tmpbuftrack, 24);
        for(i = 0; i < (tempApp_UnionStruct->PaypassTradeParam->PaypassnUN); i++)
        {
            tmpcarddata[posbuf[i + ktrack1 - tempApp_UnionStruct->PaypassTradeParam->PaypassnUN]] = tmpbuf[48 + i - tempApp_UnionStruct->PaypassTradeParam->PaypassnUN];
            #ifdef EMVB_DEBUG
            EMVBase_Trace("\r\n paypass-info:113 %d %02x \r\n", posbuf[i + ktrack1 - tempApp_UnionStruct->PaypassTradeParam->PaypassnUN], tmpbuf[8 + i - tempApp_UnionStruct->PaypassTradeParam->PaypassnUN]);
            #endif
        }

        intatc = ATC[0];
        intatc = (intatc << 8) + ATC[1];
        EMVBaseU32ToBcd(bcdatc, intatc, 24);
        memset(tmpbuf, 0, 200);
        EMVBaseBcdToAsc(tmpbuf, bcdatc, 24);
        for(i = 0; i < (ktrack1 - tempApp_UnionStruct->PaypassTradeParam->PaypassnUN); i++)
        {
            tmpcarddata[posbuf[i]] = tmpbuf[48 - ktrack1 + tempApp_UnionStruct->PaypassTradeParam->PaypassnUN + i];
            #ifdef EMVB_DEBUG
            EMVBase_Trace("\r\n paypass-info:113 %d %02x \r\n", posbuf[i], tmpbuf[48 - ktrack1 + tempApp_UnionStruct->PaypassTradeParam->PaypassnUN + i]);
            #endif
        }

        tmpcarddata[tmppos + mtrack1 - 1] = PaypassnUN1 + 0x30;	//sxl20130109

        emvbase_avl_createsettagvalue(EMVTAG_PayPassTrack1Data, tmpcarddata, tmplen);
        emvbase_avl_createsettagvalue(EMVTAG_Paypassconverttrack1, ddcarddata, ddcarddatalen);
    }

    emvbase_free(mallocbuf);
    return RLT_EMV_OK;
}

//20140222
unsigned char Paypass_phonemessagetrackdata(PAYPASSTradeUnionStruct *tempApp_UnionStruct) // ����������
{
    unsigned char i;    //,k
    unsigned char tmp[200], tmppos, tmplen, cnt;
    unsigned char qtrack2dexist, qtrack1dexist, separatorcnt;
    EMVBASETAGCVLITEM *item;


    item = emvbase_avl_gettagitempointer(EMVTAG_PaypassMagTrack2);
    if(item == NULL)
    {
        return PAYPASS_CARDDATAMISSING;
    }
    if(item->len == 0)
    {
        return PAYPASS_CARDDATAMISSING;
    }

	memset(tmp, 0, sizeof(tmp));
    EMVBaseBcdToAsc(tmp, item->data, item->len);
    tmplen = (item->len) << 1;


    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-info: transfer data\r\n");
    #endif
    qtrack2dexist = 0;
    for(i = 0; i < tmplen; i++)
    {
        if(tmp[i] == 'D')
        {
            qtrack2dexist = 1;
            if(i > 19)
            {
                #ifdef EMVB_DEBUG
                EMVBase_Trace("\r\n paypass-error:PayPass_CryptochecksumRetData1111\r\n");
                #endif
                return PAYPASS_CARDDATAERR;
            }

            if((tmp[tmplen - 1] == 'F') || (tmp[tmplen - 1] == 'f'))
            {
                if((i + 8 + 1) > tmplen)
                {
                    #ifdef EMVB_DEBUG
                    EMVBase_Trace("\r\n paypass-error:PayPass_CryptochecksumRetDat4444\r\n");
                    #endif
                    return PAYPASS_CARDDATAERR;
                }
            }
            else
            {
                if((i + 8) > tmplen)
                {
                    #ifdef EMVB_DEBUG
                    EMVBase_Trace("\r\n paypass-error:PayPass_CryptochecksumRetDat5555\r\n");
                    #endif
                    return PAYPASS_CARDDATAERR;
                }
            }

            tmppos = i + 8;
            break;
        }
    }


    if(qtrack2dexist == 0)
    {
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error:PayPass_CryptochecksumRetData3333\r\n");
        #endif
        return PAYPASS_CARDDATAERR;
    }



    #if 0 /*Modify by luohuidong at 2017.02.09  11:28 */
    unsigned char mtrack2 = tmplen - tmppos;

    if((tmp[mtrack2 - 1] == 'F') || (tmp[mtrack2 - 1] == 'f'))
    {
        mtrack2 = mtrack2 - 1;
    }
    emvbase_avl_createsettagvalue(EMVTAG_Paypassconverttrack2, &tmp[tmppos], mtrack2);
    #else
    emvbase_avl_createsettagvalue(EMVTAG_Paypassconverttrack2, &tmp[tmppos], tmplen - tmppos);
    #endif /* if 1 */






    //如果一磁道数据存在，转化一磁道数据
    if(emvbase_avl_checkiftagexist(EMVTAG_PayPassTrack1Data))
    {
        memset(tmp, 0, sizeof(tmp));
        item = emvbase_avl_gettagitempointer(EMVTAG_PayPassTrack1Data);
        if(item == NULL)
        {
            return PAYPASS_CARDDATAMISSING;
        }
        if(item->len == 0)
        {
            return PAYPASS_CARDDATAMISSING;
        }

        memcpy(tmp, item->data, item->len);
        tmplen = item->len;


        qtrack1dexist = 0;
        separatorcnt = 0;
        cnt = 0;
        for(i = 1; i < tmplen; i++)
        {
            if(tmp[i] == 0x5E)
            {
                //qtrack2dexist = 1;
                if(separatorcnt == 0)//PAN
                {
                    if(cnt > 19)
                    {
                        #ifdef EMVB_DEBUG
                        EMVBase_Trace("\r\n paypass-error:PayPass_CryptochecksumRetData1111\r\n");
                        #endif
                        return PAYPASS_CARDDATAERR;
                    }
                    separatorcnt = 1;
                }
                else if(separatorcnt == 1)//name
                {
                    if((cnt > 26 || cnt < 2) && cnt != 0)
                    {
                        #ifdef EMVB_DEBUG
                        EMVBase_Trace("\r\n paypass-error:PayPass_CryptochecksumRetData5555\r\n");
                        #endif
                        return PAYPASS_CARDDATAERR;
                    }

                    if((i + 8) > tmplen)
                    {
                        #ifdef EMVB_DEBUG
                        EMVBase_Trace("\r\n paypass-error:PayPass_CryptochecksumRetData6666\r\n");
                        #endif
                        return PAYPASS_CARDDATAERR;
                    }
                    qtrack1dexist = 1;
                    //memcpy(tmpcarddata,tmp,tmplen);
                    tmppos = i + 8;
                    break;
                }
                cnt = 0;
            }
            else
            {
                cnt++;
            }
        }

        if(qtrack1dexist == 0)
        {
            #ifdef EMVB_DEBUG
            EMVBase_Trace("\r\n paypass-error:PayPass_CryptochecksumRetData333b\r\n");
            #endif
            return PAYPASS_CARDDATAERR;
        }

        //mtrack1 = tmplen-i-8;

        //memcpy(ddcarddata,&tmp[tmppos],tmplen-tmppos);
        //ddcarddatalen = tmplen-tmppos;


        emvbase_avl_createsettagvalue(EMVTAG_Paypassconverttrack1, &tmp[tmppos], tmplen - tmppos);


    }

    return RLT_EMV_OK;
}
unsigned char Paypass_SecondTapCheck(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
	unsigned char POSCII[3] = {0};
	PAYPASS_PHONEMESSAGETABLEINFO *tempphonemsgtbl;
	unsigned char j = 0;


	EMVBase_Trace("paypass-info: Second tap needed?\r\n");

	tempphonemsgtbl = tempApp_UnionStruct->pphonemsgtable;
	memset(POSCII, 0, sizeof(POSCII));
    emvbase_avl_gettagvalue_spec(EMVTAG_POSCII, POSCII, 0, 3);
    EMVBase_Trace("paypass-info: POS Cardholder Interaction Information(TagDF4B): %02x %02x %02x\r\n", POSCII[0], POSCII[1], POSCII[2]);

	if((POSCII[1] & 0x03) || (POSCII[2] & 0x0F)) //S13.41   S14.19.1-YES
    {
        //S13.44
        for(j = 0; j < tempphonemsgtbl->phonemessagetablenum; j++)
        {
            if((POSCII[0]&tempphonemsgtbl->phonemessagetable[j].PCIIMASK[0]) == tempphonemsgtbl->phonemessagetable[j].PCIIVALUE[0] &&
                    (POSCII[1]&tempphonemsgtbl->phonemessagetable[j].PCIIMASK[1]) == tempphonemsgtbl->phonemessagetable[j].PCIIVALUE[1] &&
                    (POSCII[2]&tempphonemsgtbl->phonemessagetable[j].PCIIMASK[2]) == tempphonemsgtbl->phonemessagetable[j].PCIIVALUE[2])  //messge id messege status
            {
                tempApp_UnionStruct->PaypassTradeParam->PaypassSelectedPOSCIIID = j;
                dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S13_44_0);
                break;
            }
        }

        if (tempApp_UnionStruct->rf_card_poweroff)
        {
            tempApp_UnionStruct->rf_card_poweroff();
        }
        if (tempApp_UnionStruct->paypassCcctimerGetStartId)
        {
            tempApp_UnionStruct->paypassCcctimerGetStartId();
        }

        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S13_45_0);

        #ifdef PAYPASSV303
        tempApp_UnionStruct->paypassdelay();

        (*(tempApp_UnionStruct->paypassccctimecnt))++; 	  //sxl20150612 paypass modify
        if((*(tempApp_UnionStruct->paypassccctimecnt)) > MAXPAYPASSCCCTIMES)
        {
            *(tempApp_UnionStruct->paypassccctimecnt) = MAXPAYPASSCCCTIMES;
        }
        #endif

		EMVBase_Trace("paypass-info: Second tap\r\n");
		return RLT_ERR_EMV_SEEPHONE; //RLT_ERR_EMV_TransRestart;
    }
    else//S13.42    //S14.22
	{
	    if (tempApp_UnionStruct->rf_card_poweroff)
	    {
	        tempApp_UnionStruct->rf_card_poweroff();
	    }
	    if (tempApp_UnionStruct->paypassCcctimerGetStartId)
	    {
	        tempApp_UnionStruct->paypassCcctimerGetStartId();
	    }

	    dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S13_42_0);

	    #ifdef PAYPASSV303
	    tempApp_UnionStruct->paypassdelay();

	    (*(tempApp_UnionStruct->paypassccctimecnt))++; 	  //sxl20150612 paypass modify
	    if((*(tempApp_UnionStruct->paypassccctimecnt)) > MAXPAYPASSCCCTIMES)
	    {
	        *(tempApp_UnionStruct->paypassccctimecnt) = MAXPAYPASSCCCTIMES;
	    }
	    #endif

	    dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S13_43_0);

	    return RLT_EMV_OFFLINE_DECLINE;
	}

	return RLT_EMV_OFFLINE_DECLINE;
}

unsigned char Paypass_ComputeCryptographicChecksum(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char paypassun[4];
    unsigned char tmp[8];
    unsigned char retCode;
    unsigned char *doldata;	//[255]
    unsigned short doldatalen;  //,len
    unsigned char *dol;	 //[300]
    unsigned short dollen;
    EMVBASETAGCVLITEM *item;
    EMVBASE_APDU_SEND *apdu_s = NULL;
    EMVBASE_APDU_RESP *apdu_r = NULL;
    unsigned char needcccdelaytime = 0;
    unsigned char POSCII[3];
    PAYPASS_PHONEMESSAGETABLEINFO *tempphonemsgtbl;
    unsigned char PaypassCCCRespType, AIP[2], Kernelconfig;
    unsigned char TermCapab[3];
    unsigned char j;


    EMVBase_Trace("paypass-info: --COMPUTE CRYPTOGRAPHIC CHECKSUM--\r\n");

    tempphonemsgtbl = tempApp_UnionStruct->pphonemsgtable;
    emvbase_avl_createsettagvalue(EMVTAG_CVMList, NULL, 0);


    memset(paypassun, 0, sizeof(paypassun));
    emvbase_avl_gettagvalue_spec(EMVTAG_paypassun, paypassun, 0, 4);
    EMVBaseBcdToAsc(tmp, paypassun, 4);
    memset(tmp, 0x30, 8 - tempApp_UnionStruct->PaypassTradeParam->PaypassnUN);
    EMVBaseAscToBcd(paypassun, tmp, 8);
    emvbase_avl_createsettagvalue(EMVTAG_paypassun, paypassun, 4);


    dol = (unsigned char *)emvbase_malloc(300);
    memset(dol, 0, 300);
    dollen = 0;

    item = emvbase_avl_gettagitempointer(EMVTAG_UDOL);
    if(item != NULL)
    {
        if(item->len)
        {
            dollen = item->len;
        }
    }

    if(dollen == 0)
    {
		EMVBase_Trace("paypass-info: UDOL(Tag9F69) Missing-->uses the Default UDOL\r\n");
        item = emvbase_avl_gettagitempointer(EMVTAG_TermUDOL);
        if(item != NULL)
        {
            if(item->len)
            {
                dollen = item->len;
            }
        }
    }

    if(dollen)
    {
        if(dollen > 255)
        {
            dollen = 255;
        }
        memcpy(dol, item->data, dollen);
    }
    else
    {
        dollen = 3;
        memcpy(dol, "\x9F\x6A\x04", 3);
    }

    doldata = (unsigned char *)emvbase_malloc(255);
    memset(doldata, 0, 255);


    retCode = Paypass_DOLProcess(typeUDOL, dol, dollen, doldata, &doldatalen, 255, tempApp_UnionStruct);
    if(retCode != RLT_EMV_OK)
    {
        emvbase_free(doldata);
        emvbase_free(dol);
        return retCode;
    }
    emvbase_free(dol);

    apdu_s = (EMVBASE_APDU_SEND *)emvbase_malloc(sizeof(EMVBASE_APDU_SEND));
    memset((unsigned char *)apdu_s, 0, (sizeof(EMVBASE_APDU_SEND)));

    memcpy(apdu_s->Command, "\x80\x2A\x8E\x80", 4);
    apdu_s->Lc = doldatalen;
    memcpy((unsigned char *)&apdu_s->DataIn[0], doldata, doldatalen);
    apdu_s->Le = 256;
    apdu_s->EnableCancel = 0;

    emvbase_free(doldata);

    apdu_r = (EMVBASE_APDU_RESP *)emvbase_malloc(sizeof(EMVBASE_APDU_RESP));
    memset((unsigned char *)apdu_r, 0, (sizeof(EMVBASE_APDU_RESP)));

    tempApp_UnionStruct->IsoCommand(apdu_s, apdu_r);

    if(apdu_r->ReadCardDataOk != APDUCOMMANDOK)
    {
        emvbase_free(apdu_r);
        emvbase_free(apdu_s);

        if (tempApp_UnionStruct->rf_card_poweroff)
        {
            tempApp_UnionStruct->rf_card_poweroff();
        }
        if (tempApp_UnionStruct->paypassCcctimerGetStartId)
        {
            tempApp_UnionStruct->paypassCcctimerGetStartId();
        }

        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S13_4_0);
        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S13_5_0);

        tempApp_UnionStruct->paypassdelay();
        (*(tempApp_UnionStruct->paypassccctimecnt))++; 	  //sxl20150612 paypass modify
        if((*(tempApp_UnionStruct->paypassccctimecnt)) > MAXPAYPASSCCCTIMES)
        {
            *(tempApp_UnionStruct->paypassccctimecnt) = MAXPAYPASSCCCTIMES;
        }

        return RLT_ERR_EMV_TransRestart;
    }


    retCode = Paypass_CCCRetData(tempApp_UnionStruct, apdu_r);

    emvbase_free(apdu_r);
    emvbase_free(apdu_s);

	if(retCode != RLT_EMV_OK)
    {
        if (tempApp_UnionStruct->rf_card_poweroff)
        {
            tempApp_UnionStruct->rf_card_poweroff();
        }
        if (tempApp_UnionStruct->paypassCcctimerGetStartId)
        {
            tempApp_UnionStruct->paypassCcctimerGetStartId();
        }

        if(retCode == RLT_ERR_EMV_IccReturn)
        {
            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S13_10_0);
        }
        else
        {
            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S13_13_0);
        }

        tempApp_UnionStruct->paypassdelay();

        (*(tempApp_UnionStruct->paypassccctimecnt))++; 	  //sxl20150612 paypass modify
        if((*(tempApp_UnionStruct->paypassccctimecnt)) > MAXPAYPASSCCCTIMES)
        {
            *(tempApp_UnionStruct->paypassccctimecnt) = MAXPAYPASSCCCTIMES;
        }

		dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S13_32_0);
        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S13_33_0);

        return RLT_EMV_TERMINATE_TRANSERR;
    }


	retCode = Paypass_DealCCCRetData(tempApp_UnionStruct);
    if(retCode != RLT_EMV_OK)
    {
        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S13_12_1);
    }

    if(retCode == PAYPASS_PHONECCCPCIIONLYE)
    {
    	#if 1 // �淶û��Ҫ�������������Ƕ����
        retCode = Paypass_phonemessagetrackdata(tempApp_UnionStruct);
        if(retCode != RLT_EMV_OK)
        {
            if (tempApp_UnionStruct->rf_card_poweroff)
            {
                tempApp_UnionStruct->rf_card_poweroff();
            }
            if (tempApp_UnionStruct->paypassCcctimerGetStartId)
            {
                tempApp_UnionStruct->paypassCcctimerGetStartId();
            }

            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S14_21_1);

            tempApp_UnionStruct->paypassdelay();

            (*(tempApp_UnionStruct->paypassccctimecnt))++; 	  //sxl20150612 paypass modify
            if((*(tempApp_UnionStruct->paypassccctimecnt)) > MAXPAYPASSCCCTIMES)
            {
                *(tempApp_UnionStruct->paypassccctimecnt) = MAXPAYPASSCCCTIMES;
            }
            retCode = PAYPASS_CCCWAITERR;
            needcccdelaytime = 1;

        }
        else
		#endif
		{
			return Paypass_SecondTapCheck(tempApp_UnionStruct);
		}
    }
    if(retCode != RLT_EMV_OK)
    {
        if (tempApp_UnionStruct->rf_card_poweroff)
        {
            tempApp_UnionStruct->rf_card_poweroff();
        }
        if (tempApp_UnionStruct->paypassCcctimerGetStartId)
        {
            tempApp_UnionStruct->paypassCcctimerGetStartId();
        }

        if(retCode == RLT_ERR_EMV_IccReturn)
        {
            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S13_10_0);
        }
        else if(retCode == RLT_ERR_EMV_IccDataMissing)
        {
            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S13_14_4);
        }
        else if(retCode == RLT_ERR_EMV_IccDataFormat)
        {
            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S14_21_1);
        }
        else
        {
            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S13_13_0);
        }

        tempApp_UnionStruct->paypassdelay();

        (*(tempApp_UnionStruct->paypassccctimecnt))++; 	  //sxl20150612 paypass modify
        if((*(tempApp_UnionStruct->paypassccctimecnt)) > MAXPAYPASSCCCTIMES)
        {
            *(tempApp_UnionStruct->paypassccctimecnt) = MAXPAYPASSCCCTIMES;
        }

		dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S13_32_0);
        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S13_33_0);

        return RLT_EMV_TERMINATE_TRANSERR;
    }


    *(tempApp_UnionStruct->paypassccctimecnt) = 0;

    if (tempApp_UnionStruct->displayRemoveCard)
    {
        tempApp_UnionStruct->displayRemoveCard();
    }

    dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S13_12_1);

    PaypassCCCRespType = 0;
    memset(AIP, 0, sizeof(AIP));
    emvbase_avl_gettagvalue_spec(EMVTAG_AIP, AIP, 0, 2);
    Kernelconfig = emvbase_avl_gettagvalue(EMVTAG_KernelConfiguration);
    if((AIP[0] & 0x02) != 0 && (Kernelconfig & 0x20))
    {
        PaypassCCCRespType = 1;
    }

    if(PaypassCCCRespType == 0)	//CCC response-1
    {
        memset(TermCapab, 0, sizeof(TermCapab));
        emvbase_avl_gettagvalue_spec(EMVTAG_TermCapab, TermCapab, 0, 3);
        if(tempApp_UnionStruct->PaypassTradeParam->PaypassCVMRLEF)  //CVM REQ
        {
            TermCapab[1] = emvbase_avl_gettagvalue(EMVTAG_magcvmrequirer); //CVM required  jiguang modify 20121220
            emvbase_avl_createsettagvalue(EMVTAG_TermCapab, TermCapab, 3);
            dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S13_26_0);
        }
        else   //No CVM req
        {
            TermCapab[1] = emvbase_avl_gettagvalue(EMVTAG_magnocvmrequirer);
            emvbase_avl_createsettagvalue(EMVTAG_TermCapab, TermCapab, 3);
            dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S13_25_0);

        }
    }
    else
    {
        memset(POSCII, 0, sizeof(POSCII));
        emvbase_avl_gettagvalue_spec(EMVTAG_POSCII, POSCII, 0, 3);
        if(POSCII[1] & 0x10)
        {
            dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S14_34_0);
        }
        else
        {
            dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S14_33_0);
        }
    }

    return RLT_EMV_ONLINE_WAIT;
}

unsigned char Paypass_CheckDataMissCDA(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char i;


    i = 0;
    while(1)
    {
        if(memcmp(PaypassCDAmandatoryTag[i].Tag, "\x00\x00\x00\x00", NEWTAGNAMELEN) == 0)
        {
            break;
        }

        if(emvbase_avl_checkiftagexist((unsigned char *)PaypassCDAmandatoryTag[i].Tag) == 0)
        {
        	EMVBase_Trace("paypass-info: Tag%02x%02x missing\r\n", PaypassCDAmandatoryTag[i].Tag[0], PaypassCDAmandatoryTag[i].Tag[1]);
            return RLT_ERR_EMV_IccDataMissing;
        }

        i++;
    }

    return RLT_EMV_OK;
}

unsigned char Paypass_DataAuth(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char retCode = RLT_EMV_OK;
    EMVBASE_CAPK_STRUCT *tempcapk;


    if(tempApp_UnionStruct->PaypassTradeParam->ODAstaus)  //need do CDA
    {
        retCode = Paypass_CheckDataMissCDA(tempApp_UnionStruct);
        if(retCode != RLT_EMV_OK)
        {
            emvbase_avl_setvalue_or(EMVTAG_TVR, 0, 0x24);
            return RLT_EMV_OK;
        }

        tempcapk = tempApp_UnionStruct->PaypassTradeParam->CAPK;
        if(tempcapk->ModulLen == 0 || tempApp_UnionStruct->PaypassTradeParam->bErrAuthData)
        {
            emvbase_avl_setvalue_or(EMVTAG_TVR, 0, 0x04);
        }
    }

    return RLT_EMV_OK;
}

unsigned char  Paypass_ReadBalanceRetData(EMVBASE_APDU_RESP *apdu_r, unsigned char type)
{
    unsigned char index = 0, len = 0;


    if(apdu_r->SW1 == 0x90 && apdu_r->SW2 == 0x00) //Get data OK
    {
        if(apdu_r->LenOut != 9)
        {
            return RLT_ERR_EMV_IccDataFormat;
        }

        if(memcmp(apdu_r->DataOut, "\x9F\x50", 2) == 0)
        {
            index += 2;

            len = apdu_r->DataOut[index++];

            if(len != 0 && len != 6)
            {
                return RLT_ERR_EMV_IccDataFormat;
            }

            if(len == 0)
            {
                return RLT_EMV_OK;
            }
            if((index + len) != apdu_r->LenOut)
            {
                return RLT_ERR_EMV_IccDataFormat;
            }

            if(type == 0)
            {
                emvbase_avl_createsettagvalue(EMVTAG_BalanceBeforeGAC, &apdu_r->DataOut[index], 6);
            }
            else
            {
                emvbase_avl_createsettagvalue(EMVTAG_BalanceAfterGAC, &apdu_r->DataOut[index], 6);
            }

            return RLT_EMV_OK;
        }
        else
        {
            return RLT_ERR_EMV_IccDataFormat;
        }
    }

    return RLT_ERR_EMV_IccReturn;
}

unsigned char Paypass_PreGenACBalanceRead(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char retCode = RLT_EMV_OK;
    EMVBASE_APDU_SEND *apdu_s = NULL;
    EMVBASE_APDU_RESP *apdu_r = NULL;
    unsigned char AppCapabilitiesInfor[3] = {0};
    EMVBASETAGCVLITEM *item = NULL;


    EMVBase_Trace("paypass-info: --Pre-GAC balance reading--\r\n");

    memset(AppCapabilitiesInfor, 0, sizeof(AppCapabilitiesInfor));
    emvbase_avl_gettagvalue_spec(EMVTAG_AppCapabilitiesInfor, AppCapabilitiesInfor, 0, 3);
    EMVBase_TraceHex("paypass-info: pplication Capabilities Information(Tag9F5D): ", AppCapabilitiesInfor, 3);

	if(0 == (AppCapabilitiesInfor[1] & 0x02))
	{
		EMVBase_Trace("paypass-info: Card does not support balance reading(Tag9F5D Byte2 bit2 is 0)\r\n");
		return RLT_EMV_OK;
	}

    item = emvbase_avl_gettagitempointer(EMVTAG_BalanceBeforeGAC);
	if(item == NULL)
	{
		EMVBase_Trace("paypass-info: Balance Read Before GAC is not present\r\n");
		return RLT_EMV_OK;
	}

	apdu_s = (EMVBASE_APDU_SEND *)emvbase_malloc(sizeof(EMVBASE_APDU_SEND));
	apdu_r = (EMVBASE_APDU_RESP *)emvbase_malloc(sizeof(EMVBASE_APDU_RESP));

    memset(apdu_s, 0, sizeof(EMVBASE_APDU_SEND));
    memset(apdu_r, 0, sizeof(EMVBASE_APDU_RESP));

    memcpy(apdu_s->Command, "\x80\xCA\x9F\x50", 4);
    apdu_s->Le = 0;
    apdu_s->EnableCancel = 1;

    tempApp_UnionStruct->IsoCommand(apdu_s, apdu_r);
    if(apdu_r->ReadCardDataOk != RLT_EMV_OK)
    {
        if(tempApp_UnionStruct->EmvErrID->L3 != EMVB_ERRID_L3_STOP)
        {
            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S16_E1_0);
            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S16_E2_0);
            dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S16_E3_0);
            //retCode = RLT_EMV_APPSELECTTRYAGAIN;
            retCode = RLT_ERR_EMV_TransRestart;
        }
		else
		{
			dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S16_E7);
			retCode = RLT_EMV_TERMINATE_TRANSERR;
		}
    }
    else
    {
        Paypass_ReadBalanceRetData(apdu_r, 0);
    }

	emvbase_free(apdu_r);
	emvbase_free(apdu_s);
    return retCode;
}

unsigned char Paypass_PostGenACBalanceRead(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    EMVBASE_APDU_SEND *apdu_s = NULL;
    EMVBASE_APDU_RESP *apdu_r = NULL;
    unsigned char AppCapabilitiesInfor[3] = {0};
    EMVBASETAGCVLITEM *item = NULL;


    EMVBase_Trace("paypass-info: --Post-GAC balance reading--\r\n");

    memset(AppCapabilitiesInfor, 0, sizeof(AppCapabilitiesInfor));
    emvbase_avl_gettagvalue_spec(EMVTAG_AppCapabilitiesInfor, AppCapabilitiesInfor, 0, 3);
	if(0 == (AppCapabilitiesInfor[1] & 0x02))
	{
		EMVBase_Trace("paypass-info: Card does not support balance reading(Tag9F5D Byte2 bit2 is 0)\r\n");
		return RLT_EMV_OK;
	}

    item = emvbase_avl_gettagitempointer(EMVTAG_BalanceAfterGAC);
	if(item == NULL)
	{
		EMVBase_Trace("paypass-info: Balance Read After GAC is not present\r\n");
		return RLT_EMV_OK;
	}


	apdu_s = (EMVBASE_APDU_SEND *)emvbase_malloc(sizeof(EMVBASE_APDU_SEND));
	apdu_r = (EMVBASE_APDU_RESP *)emvbase_malloc(sizeof(EMVBASE_APDU_RESP));
    memset(apdu_s, 0, sizeof(EMVBASE_APDU_SEND));
    memset(apdu_r, 0, sizeof(EMVBASE_APDU_RESP));

    memcpy(apdu_s->Command, "\x80\xCA\x9F\x50", 4);
    apdu_s->Le = 0;
    apdu_s->EnableCancel = 0;

    tempApp_UnionStruct->IsoCommand(apdu_s, apdu_r);
    if(apdu_r->ReadCardDataOk == RLT_EMV_OK)
    {
        Paypass_ReadBalanceRetData(apdu_r, 1);
    }

	emvbase_free(apdu_r);
	emvbase_free(apdu_s);

	#if 0
    #ifdef PAYPASS_DATAEXCHANGE
    //S10.ED20
    if(!emvbase_avl_checkiftagexist(EMVTAG_TagsToWriteAfterGAC))
    {
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error: S10.ED20 no \r\n");
        EMVBase_Trace("\r\n paypass-error: S11.ED23 NO \r\n");
        #endif

        if (tempApp_UnionStruct->displayRemoveCard)
        {
            tempApp_UnionStruct->displayRemoveCard();
        }

        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S9_E27_0);
    }
    #else

    if (tempApp_UnionStruct->displayRemoveCard)
    {
        tempApp_UnionStruct->displayRemoveCard();
    }

    dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S9_E27_0);

    #endif
	#endif

    return RLT_EMV_OK;
}

unsigned char Paypass_CheckFloorLimit(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    if(tempApp_UnionStruct->PaypassTradeParam->PaypassTermEFLEF)
    {
    	EMVBase_Trace("paypass-info: Floor Limit exceeded\r\n");
        emvbase_avl_setvalue_or(EMVTAG_TVR, 3, 0x80); //set 'transaction exceeds floor limit' bit 1.
    }

    if(tempApp_UnionStruct->PaypassTradeParam->PaypassCVMRLEF)
    {
    	EMVBase_Trace("paypass-info: CVM Limit exceeded\r\n");
        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S5_E31_0);
    }

    return RLT_EMV_OK;
}

unsigned char Paypass_InitCardHolderVerify(PAYPASSTradeUnionStruct *tempApp_UnionStruct, CVMSTRCUT *CVMdata)
{
    unsigned char i;
    unsigned char AIP[2], KernelConfigure;
    unsigned char CVMResult[3];
    unsigned char CVMListbExist;
    EMVBASETAGCVLITEM *CVMListitem = NULL;


    memset(AIP, 0, sizeof(AIP));
    emvbase_avl_gettagvalue_spec(EMVTAG_AIP, AIP, 0, 2);
    KernelConfigure = emvbase_avl_gettagvalue(EMVTAG_KernelConfiguration);
	EMVBase_TraceHex("paypass-info: AIP(Tag82): ", AIP, 2);
	EMVBase_Trace("paypass-info: Kernel Configuration(TagDF811B): %02x\r\n", KernelConfigure);


    if((AIP[0] & 0x02) && (KernelConfigure & 0x20))
    {
    	EMVBase_Trace("paypass-info: On device cardholder verification supported by card and terminal\r\n");
		EMVBase_Trace("AIP(Tag82) Byte1 bit2 is 1 and Kernel Configuration(TagDF811B) bit6 is 1\r\n");

        if(tempApp_UnionStruct->PaypassTradeParam->PaypassCVMRLEF == 1)	 //   plaintext  PIN  Verification Performanced by ICC
        {
        	EMVBase_Trace("paypass-info: CVM Required Limit exceeded-->CDCVM\r\n");

            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_CVM_E4_0);
            CVMResult[0] = 0x01;
            CVMResult[1] = 0;
            CVMResult[2] = CVR_SUCCESS;
            emvbase_avl_createsettagvalue(EMVTAG_CVMResult, CVMResult, 3);
        }
        else
        {
        	EMVBase_Trace("paypass-info: CVM Required Limit not exceeded-->No CVM\r\n");

            CVMResult[0] = 0x3f;
            CVMResult[1] = 0;
            CVMResult[2] = CVR_SUCCESS;
            emvbase_avl_createsettagvalue(EMVTAG_CVMResult, CVMResult, 3);
            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_CVM_E8_0);
        }

        return RLT_EMV_OK;
    }

    if(!(AIP[0] & 0x10))
    {
		EMVBase_Trace("paypass-info: Cardholder verification is not supported(AIP Byte1 bit5 is 0)\r\n");

        CVMResult[0] = 0x3f;
        CVMResult[1] = 0;
        CVMResult[2] = CVR_UNKNOWN;
        emvbase_avl_createsettagvalue(EMVTAG_CVMResult, CVMResult, 3);
        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_CVM_E8_0);

        return RLT_EMV_OK;
    }

    CVMListitem = emvbase_avl_gettagitemandstatus(EMVTAG_CVMList, &CVMListbExist);
    if(CVMListbExist == 0x00)
    {
    	EMVBase_Trace("EMV-info: CVM List(Tag8E) missing\r\n");

        emvbase_avl_setvalue_or(EMVTAG_TVR, 0, 0x20);
        CVMResult[0] = 0x3f;
        CVMResult[1] = 0;
        CVMResult[2] = CVR_UNKNOWN;
        emvbase_avl_createsettagvalue(EMVTAG_CVMResult, CVMResult, 3);
        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_CVM_E8_0);

        return RLT_EMV_OK;
    }

    if(CVMListitem->len < 10 || (CVMListitem->len % 2))
    {
    	if(CVMListitem->len < 10)
    	{
    		EMVBase_Trace("paypass-info: CVM List(Tag8E)'s len is %d < 10-->error\r\n", CVMListitem->len);
    	}
		if(CVMListitem->len % 2)
    	{
    		EMVBase_Trace("paypass-info: CVM List(Tag8E)'s len is %d(should be 2*x)\r\n", CVMListitem->len);
    	}

        emvbase_avl_setvalue_or(EMVTAG_TVR, 0, 0x20);
        CVMResult[0] = 0x3f;
        CVMResult[1] = 0;
        CVMResult[2] = CVR_UNKNOWN;
        emvbase_avl_createsettagvalue(EMVTAG_CVMResult, CVMResult, 3);
        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_CVM_E8_0);

        return RLT_EMV_OK;
    }

	EMVBase_TraceHex("paypass-info: CVM List(Tag8E): ", CVMListitem->data, CVMListitem->len);

    memcpy(&CVMdata->CVM_X[2], CVMListitem->data, 4);
    memcpy(&CVMdata->CVM_Y[2], &CVMListitem->data[4], 4);

    CVMdata->CVRListLen = (CVMListitem->len - 8) / 2;
    for(i = 0; i < CVMdata->CVRListLen; i++)
    {
        CVMdata->CVRList[i].method = CVMListitem->data[i * 2 + 8];
        CVMdata->CVRList[i].condition = CVMListitem->data[i * 2 + 9];
    }

    CVMResult[0] = 0x3f;
    CVMResult[1] = 0;
    CVMResult[2] = CVR_FAIL;
    emvbase_avl_createsettagvalue(EMVTAG_CVMResult, CVMResult, 3);
    dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_CVM_E8_0);

    return RLT_EMV_ERR;
}

unsigned char Paypass_CheckIfSupportCVM(unsigned char method)
{
    unsigned char temp;
    unsigned char TermCapab[3];


    temp = method & 0x3F;

    memset(TermCapab, 0, sizeof(TermCapab));
    emvbase_avl_gettagvalue_spec(EMVTAG_TermCapab, TermCapab, 0, 3);
	EMVBase_TraceHex("paypass-info: Terminal Capabilities(Tag9F33): ", TermCapab, 3);
	EMVBase_Trace("paypass-info: CVM code: %02x\r\n", temp);

    switch(temp)
    {
        case 0x00:
            return 1;
            break;

        case 0x02://enciphered PIN verification online
            if((TermCapab[1] & 0x40) == 0)
            {
            	EMVBase_Trace("paypass-info: Enciphered PIN for online is not supported(Terminal Capabilities Byte2 bit7 is 0)\r\n");
                return 0;
            }
            else
            {
                return 1;
            }
            break;

        case 0x1E://signature (paper)
            if((TermCapab[1] & 0x20) == 0)
            {
            	EMVBase_Trace("paypass-info: Signature is not supported(Terminal Capabilities Byte2 bit6 is 0)\r\n");
                return 0;
            }
            else
            {
                return 1;
            }
            break;

        case 0x1F://no CVM required
            if((TermCapab[1] & 0x08) == 0)
            {
            	EMVBase_Trace("paypass-info: No CVM is not supported(Terminal Capabilities Byte2 bit4 is 0)\r\n");
                return 0;
            }
            else
            {
                return 1;
            }
            break;

        default:
            return 0;
            break;
    }

    //return 0;
}

unsigned char Paypass_AnalyCvmlist_cmpamount(unsigned char *CVMamount, unsigned char type)
{
    unsigned char TransCurcyCodebExist, TransCurcyCode[2];
    unsigned char TermAmtAuth[6], TermAmtCVM[6];
    unsigned char AppCurcyCodebExist, AppCurcyCode[2];
    unsigned int TermAmt;

    memset(TransCurcyCode, 0, sizeof(TransCurcyCode));
    TransCurcyCodebExist = !(emvbase_avl_gettagvalue_spec(EMVTAG_TransCurcyCode, TransCurcyCode, 0, 2));

    memset(AppCurcyCode, 0, sizeof(AppCurcyCode));
    AppCurcyCodebExist = !(emvbase_avl_gettagvalue_spec(EMVTAG_AppCurcyCode, AppCurcyCode, 0, 2));

    memset(TermAmtAuth, 0, sizeof(TermAmtAuth));
    emvbase_avl_gettagvalue_spec(EMVTAG_AmtAuthNum, TermAmtAuth, 0, 6);

    if(TransCurcyCodebExist == 0x00 || AppCurcyCodebExist == 0)	 //Application Currency Code is not present
    {
        return 0;
    }

    if( !memcmp(TransCurcyCode, AppCurcyCode, 2)) //under x shouldn't include case of equal x.(EMV2000 2CJ.077.02)
    {

        TermAmt = EMVBaseByteArrayToInt(&CVMamount[2], 4);
        memset(TermAmtCVM, 0, sizeof(TermAmtCVM));
        EMVBaseU32ToBcd(TermAmtCVM, TermAmt, 6);

        if(type == 0)
        {
            if(memcmp(TermAmtAuth, TermAmtCVM, 6) < 0)
            {
                return 1;
            }
        }
        else
        {
            if(memcmp(TermAmtAuth, TermAmtCVM, 6) > 0)
            {
                return 1;
            }
        }
    }

    return 0;
}

unsigned short Paypass_AnalyCvmlist(unsigned char method, unsigned char condition, unsigned char TransType, unsigned char TermType, unsigned char *CVM_X, unsigned char *CVM_Y)
{

    unsigned char needcvm = 0, nextcvm = 0;
    unsigned short returnvalue;
    unsigned char TermAmtOther[6];
    unsigned char TermAmtOthernotzero = 0;
    unsigned char ret;

    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-info:%02x %02x %02x %02x \r\n", TermType, condition, method, TransType);
    #endif


    switch(condition)
    {
        case 0x00://always
            needcvm = 1;
            break;

        case 0x01://if cash or cashback(EMV2000)�? if unattended cash(modified in EMV4.1,SU16)
            if(TransType == EMVBASE_TRANS_CASH && (TermType & 0x0F) > 3 )
            {
                needcvm = 1;
            }
            else { nextcvm = 1; }
            break;

        case 0x02://if not cash or cashback
            memset(TermAmtOther, 0, sizeof(TermAmtOther));
            emvbase_avl_gettagvalue_spec(EMVTAG_AmtOtherNum, TermAmtOther, 0, 6);
            if(memcmp(TermAmtOther, "\x00\x00\x00\x00\x00\x00", 6))
            {
                TermAmtOthernotzero = 1;
            }
            if(TransType != EMVBASE_TRANS_CASH  && TransType != EMVBASE_TRANS_CASHBACK && TermAmtOthernotzero == 0)
            {
                needcvm = 1;
            }
            else { nextcvm = 1; }
            break;

        case 0x03://if terminal support CVM	 //sxl arm_linux
            //tempterminfo->needcvm = 1;
            if(Paypass_CheckIfSupportCVM(method))
            {
                needcvm = 1;
            }
            else
            {
                nextcvm = 1;
            }
            break;

        case 0x04://if manual cash (added in EMV4.1,SU16)  //cash参考EMV文档
            #if 0//20151211_lhd
            if(TransType == EMVBASE_TRANS_CASH && (TermType & 0x0F) <= 3 )
            #else
            if((TransType == EMVBASE_TRANS_CASH || EMVBASE_TRANS_CASHDISBURSEMENT == TransType ) && ((TermType & 0x0F) <= 3 ))
            #endif
            {
                needcvm = 1;
            }
            else { nextcvm = 1; }
            break;

        case 0x05://if purchase with cashback (added in EMV4.1,SU16)
            memset(TermAmtOther, 0, sizeof(TermAmtOther));
            emvbase_avl_gettagvalue_spec(EMVTAG_AmtOtherNum, TermAmtOther, 0, 6);
            if(memcmp(TermAmtOther, "\x00\x00\x00\x00\x00\x00", 6))
            {
                TermAmtOthernotzero = 1;
            }
            if(TransType == EMVBASE_TRANS_CASHBACK || TermAmtOthernotzero )
            {
                needcvm = 1;
            }
            else { nextcvm = 1; }
            break;

        case 0x06://if trans is in App currency and under X value
            ret	= Paypass_AnalyCvmlist_cmpamount(CVM_X, 0);
            if(ret == 0)
            {
                nextcvm = 1;
            }
            else
            {
                needcvm = 1;
            }
            break;

        case 0x07://if trans is in App currency and over X value
            ret	= Paypass_AnalyCvmlist_cmpamount(CVM_X, 1);
            if(ret == 0)
            {
                nextcvm = 1;
            }
            else
            {
                needcvm = 1;
            }
            break;

        case 0x08://if trans is in App currency and under Y value
            ret	= Paypass_AnalyCvmlist_cmpamount(CVM_Y, 0);
            if(ret == 0)
            {
                nextcvm = 1;
            }
            else
            {
                needcvm = 1;
            }
            break;

        case 0x09://if trans is in App currency and over Y value
            ret	= Paypass_AnalyCvmlist_cmpamount(CVM_Y, 1);
            if(ret == 0)
            {
                nextcvm = 1;
            }
            else
            {
                needcvm = 1;
            }
            break;

        default:
            nextcvm = 1;
            break;
    }

    returnvalue = nextcvm;
    returnvalue = (returnvalue << 8) | needcvm;

    return returnvalue;
}

unsigned char Paypass_PerformCVM(PAYPASSTradeUnionStruct *tempApp_UnionStruct, unsigned char method, unsigned char condition, unsigned char *TermCapab)
{
    unsigned char retCode = 0;
    unsigned char temp;
    unsigned char CVMResult[3];


    temp = method & 0x3F;
	EMVBase_Trace("paypass-info: CVM code: %02x\r\n", temp);

    switch(temp)
    {
        case 0x00:
			EMVBase_Trace("paypass-info: code == 00-->Fail CVM processing\r\n");
            if((method & 0x40) == 0)
            {
                CVMResult[2] = CVR_FAIL;
                retCode = PAYPASS_ERR_CVMFailALWAYS;
            }
            else
            {
                retCode = PAYPASS_ERR_CVMFail;
            }
            break;

        case 0x01:	 //PLAINTEXT PIN VERIFICATION PERFORMED BY ICC
            return PAYPASS_ERR_CVMNoSupport;
            break;

        case 0x02:	//enciphered PIN verification online
            if((TermCapab[1] & 0x40) == 0) //sxl
            {
            	EMVBase_Trace("paypass-info: Enciphered PIN for online is not supported(Terminal Capabilities Byte2 bit7 is 0)\r\n");
                return PAYPASS_ERR_CVMNoSupport;
            }

			EMVBase_Trace("EMV-info: Enciphered PIN for online selected\r\n");

            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_CVM_E18_1);
            CVMResult[2] = CVR_UNKNOWN;
            tempApp_UnionStruct->PaypassTradeParam->paypassonlinepinreq = 1;
            emvbase_avl_setvalue_or(EMVTAG_TVR, 2, 0x04);	//密码在最后输入，一定要先置起来标志
            retCode = RLT_EMV_OK;
            break;

        case 0x03:  //Plaintext PIN verification performed by ICC and signature(paper)
            return PAYPASS_ERR_CVMNoSupport;
            break;
        case 0x04:  //enciphered PIN verification performed by ICC
            return PAYPASS_ERR_CVMNoSupport;
            break;
        case 0x05: //enciphered PIN verification performed by ICC and signature(paper)
            return PAYPASS_ERR_CVMNoSupport;
            break;

        case 0x1E://signature (paper)
            if((TermCapab[1] & 0x20) == 0) //sxl EMV4.1e
            {
            	EMVBase_Trace("paypass-info: Signature is not supported(Terminal Capabilities Byte2 bit6 is 0)\r\n");
                return PAYPASS_ERR_CVMNoSupport;
            }

			EMVBase_Trace("paypass-info: Signature selected\r\n");

            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_CVM_E18_0);
            tempApp_UnionStruct->PaypassTradeParam->bPrintReceipt = 1;
            CVMResult[2] = CVR_UNKNOWN;
            retCode = RLT_EMV_OK;
            break;

        case 0x1F://no CVM required
            if((TermCapab[1] & 0x08) == 0)	//sxl EMV4.1E
            {
            	EMVBase_Trace("paypass-info: No CVM is not supported(Terminal Capabilities Byte2 bit4 is 0)\r\n");
                return PAYPASS_ERR_CVMNoSupport;
            }

			EMVBase_Trace("paypass-info: No CVM selected\r\n");

            emvbase_avl_setvalue_and(EMVTAG_TVR, 2, 0x7F);
            CVMResult[2] = CVR_SUCCESS;
            retCode = RLT_EMV_OK;
            break;

        default:
            emvbase_avl_setvalue_or(EMVTAG_TVR, 2, 0x40);
            CVMResult[2] = CVR_FAIL; //sxl
            return PAYPASS_ERR_CVMFail;
            break;

    }
    CVMResult[0] = method;
    CVMResult[1] = condition;

    emvbase_avl_createsettagvalue(EMVTAG_CVMResult, CVMResult, 3);

    return retCode;
}

unsigned char Paypass_CardHolderVerf(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char retCode;
    CVMSTRCUT CVMdata;
    unsigned char NowCVRListLen = 0;
    unsigned char needcvm, nextcvm;
    unsigned short needcvmnextcvm;
    unsigned char TransType, TermType;
    unsigned char TermCapab[3];
    unsigned char CVMResult[3];


	EMVBase_Trace("paypass-info: Cardholder Verification\r\n");

    tempApp_UnionStruct->PaypassTradeParam->paypassonlinepinreq = 0;

    memset(&CVMdata, 0, sizeof(CVMSTRCUT));
    retCode = Paypass_InitCardHolderVerify(tempApp_UnionStruct, &CVMdata);
    if(retCode == RLT_EMV_OK)
    {
        return RLT_EMV_OK;
    }

    TransType = emvbase_avl_gettagvalue(EMVTAG_TransTypeValue);
    TermType = emvbase_avl_gettagvalue(EMVTAG_TermType);
    memset(TermCapab, 0, sizeof(TermCapab));
    emvbase_avl_gettagvalue_spec(EMVTAG_TermCapab, TermCapab, 0, 3);
	EMVBase_TraceHex("paypass-info: Terminal Capabilities(Tag9F33): ", TermCapab, 3);
	EMVBase_Trace("paypass-info: Transaction Type(Tag9C): %02x\r\n", TransType);
	EMVBase_Trace("paypass-info: Terminal Type(Tag9F35): %02x\r\n", TermType);

    while(NowCVRListLen < CVMdata.CVRListLen)
    {

        needcvmnextcvm = 0;
        needcvmnextcvm = Paypass_AnalyCvmlist(CVMdata.CVRList[NowCVRListLen].method, CVMdata.CVRList[NowCVRListLen].condition, TransType, TermType, CVMdata.CVM_X, CVMdata.CVM_Y);

        needcvm = needcvmnextcvm & 0xff;
        nextcvm = (needcvmnextcvm >> 8) & 0xff;

        if(needcvm == 1)
        {
            retCode = Paypass_PerformCVM(tempApp_UnionStruct, CVMdata.CVRList[NowCVRListLen].method, CVMdata.CVRList[NowCVRListLen].condition, TermCapab);
            if(retCode == RLT_EMV_OK)
            {
                return retCode;
            }

            if(retCode == PAYPASS_ERR_CVMFailALWAYS)
            {
                emvbase_avl_setvalue_or(EMVTAG_TVR, 2, 0x80);

                if((CVMdata.CVRList[NowCVRListLen].method & 0x3f) == 0)
                {
                    CVMResult[0] = CVMdata.CVRList[NowCVRListLen].method;
                    CVMResult[1] = CVMdata.CVRList[NowCVRListLen].condition;
                    CVMResult[2] = 0x01;

                    emvbase_avl_createsettagvalue(EMVTAG_CVMResult, CVMResult, 3);
                }
                return RLT_EMV_OK;
            }

            if(((CVMdata.CVRList[NowCVRListLen].method & 0x40) == 0x00) || (NowCVRListLen == CVMdata.CVRListLen - 1))
            {
                emvbase_avl_setvalue_or(EMVTAG_TVR, 2, 0x80);
                if((CVMdata.CVRList[NowCVRListLen].method & 0x3f) == 0)
                {
                    CVMResult[0] = CVMdata.CVRList[NowCVRListLen].method;
                    CVMResult[1] = CVMdata.CVRList[NowCVRListLen].condition;
                    CVMResult[2] = 0x01;

                    emvbase_avl_createsettagvalue(EMVTAG_CVMResult, CVMResult, 3);
                }
                return RLT_EMV_OK;
            }
            else { nextcvm = 1; }

        }
        if(nextcvm == 1)
        {
            (NowCVRListLen)++;
        }
    }
    emvbase_avl_setvalue_or(EMVTAG_TVR, 2, 0x80);		 //set "Cardholder verification was not successful"
    return RLT_EMV_OK;
}

unsigned char Paypass_GetNextPutDataTagFromList(unsigned char *InputList, unsigned char *PutTag, unsigned char *PutTagLen, unsigned char *Value, unsigned short *ValueLen)
{
    EMVBASETAGCVLITEM *item = NULL;
    unsigned char *List = NULL;
	unsigned short ListLen = 0;
    unsigned short index = 0;
	unsigned short len = 0;
    unsigned char  tag[4];
    unsigned short taglen = 0;


	EMVBase_TraceHex("paypass-info: GetNextPutDataTagFromList: ", InputList, 3);

    item = emvbase_avl_gettagitempointer(InputList);
	if((NULL == item) || (0 == item->len))
    {
    	EMVBase_Trace("paypass-info: List Empty\r\n");
		return RLT_EMV_ERR;
    }

    EMVBase_TraceHex("paypass-info: List data:", item->data, item->len);

    List = item->data;
    ListLen = item->len;

    index = 0;
    while(index < ListLen)
    {
        if(/*PDOL[index]==0xFF ||*/ List[index] == 0x00)
        {
            index++;
            continue;
        }

        memset(tag, 0, sizeof(tag));
        taglen = 0;
        tag[taglen++] = List[index];
        if((tag[0] & 0x1f) == 0x1f)
        {
            tag[taglen++] = List[index + 1];
            if(tag[1] & 0x80)
            {
                tag[taglen++] = List[index + 2];
            }
        }
        index += taglen;

		EMVBase_TraceHex("paypass-info: Tag in List: ", tag, taglen);

        len = 0;
        EMVBase_ParseExtLenWithMaxLen(List, &index, &len, ListLen);

		*PutTagLen = taglen;
		memcpy(PutTag, tag, taglen);
		*ValueLen = len;
        memcpy(Value, &List[index], len);

		index += len;

		RemoveFromList(1, tag, InputList);

		return RLT_EMV_OK;
    }

	return RLT_EMV_ERR;
}

unsigned char Paypass_CommandPutData(unsigned char *tag, unsigned char tagLen, unsigned short L, unsigned char *V, EMVBASE_APDU_SEND *apdu_s)
{
    memset(apdu_s, 0, sizeof(EMVBASE_APDU_SEND));

    memcpy(&apdu_s->Command[0], "\x80\xDA", 2);
    if(tagLen == 1)
    {
        apdu_s->Command[2] = 0x00;
        apdu_s->Command[3] = tag[0];
    }
    else// if(tagLen==2)
    {
        memcpy(&apdu_s->Command[2], tag, 2);
    }
    if(L > 0)
    {
        if(L > sizeof(apdu_s->DataIn))
        {
            apdu_s->Lc = sizeof(apdu_s->DataIn);
        }
        else
        {
            apdu_s->Lc = L;
        }
        memcpy(&apdu_s->DataIn[0], V, apdu_s->Lc);
    }
    else
    {
        apdu_s->Lc = 0;
    }
    apdu_s->Le = 0x00;
    apdu_s->EnableCancel = 1;

    return RLT_EMV_OK;
}

unsigned char Paypass_PutData(unsigned char *InputList, PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    EMVBASE_APDU_SEND *apdu_s = NULL;
    EMVBASE_APDU_RESP *apdu_r = NULL;
    unsigned char retCode = 0;
    unsigned char Tag[4];
	unsigned char TagLen = 0;
	unsigned char *Value = NULL; //[512];
	unsigned short Len = 0;


    Value = (unsigned char *)emvbase_malloc(XML_FILE_DEK_LEN + 1);
    if(NULL == Value)
    {
        EMVBase_Trace("paypass-error: Memory error\r\n");
        return RLT_EMV_ERR;
    }

	apdu_s = (EMVBASE_APDU_SEND *)emvbase_malloc(sizeof(EMVBASE_APDU_SEND));
	apdu_r = (EMVBASE_APDU_RESP *)emvbase_malloc(sizeof(EMVBASE_APDU_RESP));


    while(1)
    {
        memset(Tag, 0x00, sizeof(Tag));
        TagLen = 0x00;
        Len = 0x00;
        memset(Value, 0x00, XML_FILE_DEK_LEN);

        retCode = Paypass_GetNextPutDataTagFromList(InputList, Tag, &TagLen, Value, &Len);

        if(RLT_EMV_ERR == retCode)
        {
        	EMVBase_Trace("paypass-info: Put Data Completed\r\n");
            retCode = RLT_EMV_OK;
            break;
        }
        else
        {
            memset(apdu_s, 0, sizeof(EMVBASE_APDU_SEND));
            memset(apdu_r, 0, sizeof(EMVBASE_APDU_RESP));

            Paypass_CommandPutData(Tag, TagLen, Len, Value, apdu_s);

            tempApp_UnionStruct->IsoCommand(apdu_s, apdu_r);

            if(apdu_r->ReadCardDataOk != RLT_EMV_OK)
            {
				EMVBase_Trace("paypass-error: apdu timeout or other error\r\n");
                retCode = RLT_ERR_EMV_APDUTIMEOUT;
				break;
            }

            if((apdu_r->SW1 != 0x90) || (apdu_r->SW2 != 0x00))
            {
                EMVBase_Trace("paypass-error: SW != 9000\r\n");
                retCode = RLT_ERR_EMV_IccReturn;
                break;
            }
        }
    }

	emvbase_free(apdu_r);
	emvbase_free(apdu_s);
    emvbase_free(Value);

    return retCode;
}

unsigned char Paypass_PreGenACPutData(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char tagisexsit = 0;
    EMVBASETAGCVLITEM *item = NULL;
    tagisexsit = 0;

	#ifdef PAYPASS_DATAEXCHANGE
    item = emvbase_avl_gettagitempointer(EMVTAG_TagsToWriteBeforeGAC);
    if((item != NULL) && item->len)
    {
		EMVBase_Trace("paypass-info: Pre GenAC Put Data\r\n");
        if(RLT_EMV_OK == Paypass_PutData(EMVTAG_TagsToWriteYetBeforeGenAC, tempApp_UnionStruct))
        {
            emvbase_avl_setvalue_or(EMVTAG_PreGACPutDataStatus, 0, 0x80);
        }
    }
	#endif

    return RLT_EMV_OK;
}

unsigned char Paypass_GenerateAC(PAYPASSTradeUnionStruct *tempApp_UnionStruct, EMVBASE_APDU_RESP *apdu_r, unsigned char type)
{
    unsigned char ret;
    unsigned char k, indexRet = 0; //j,t,
    unsigned short index, indexTemp, lenTemp, orgindex;
    unsigned char countpadding = 0;
    unsigned char bIntable;
    unsigned char GenerateACRetDataflag;
    unsigned char CryptInfo, reqCrpt;
    unsigned char CVMResult[3];


	EMVBase_Trace("paypass-info: ---response message of GAC parsing---\r\n");

    tempApp_UnionStruct->PaypassTradeParam->GenerateACRetDataLen = 0;
    memset(tempApp_UnionStruct->PaypassTradeParam->GenerateACRetData, 0, 255);


    if(!(apdu_r->SW1 == 0x90 && apdu_r->SW2 == 0x00))
    {
    	EMVBase_Trace("paypass-error: SW != 9000\r\n");
        return RLT_ERR_EMV_IccReturn;
    }

    if(type == 0) // removed for SB261 but reserved in production
    {
        ret = tempApp_UnionStruct->deletetornlog();
        if(ret != RLT_EMV_OK)
        {
            return ret;
        }

        ret = tempApp_UnionStruct->copytornrecorddata();
        if(ret != RLT_EMV_OK)
        {
            return ret;
        }

        memset(CVMResult, 0, sizeof(CVMResult));
        emvbase_avl_gettagvalue_spec(EMVTAG_CVMResult, CVMResult, 0, 3);
        if((CVMResult[0] & 0x3F) == 0x02 && CVMResult[2] == CVR_UNKNOWN)
        {
            tempApp_UnionStruct->PaypassTradeParam->paypassonlinepinreq = 1;
        }
        else
        {
            tempApp_UnionStruct->PaypassTradeParam->paypassonlinepinreq = 0;
            if((CVMResult[0] & 0x3F) == 0x1E && CVMResult[2] == CVR_UNKNOWN)
            {
                tempApp_UnionStruct->PaypassTradeParam->bPrintReceipt = 1;
            }
            else
            {
                tempApp_UnionStruct->PaypassTradeParam->bPrintReceipt = 0;
            }
        }
    }


    index = 0;
    if(apdu_r->DataOut[index] == 0x80) //primitive data
    {
		EMVBase_Trace("paypass-info: GPO template 80\r\n");

        index++;
        if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &lenTemp))
        {
			EMVBase_Trace("paypass-error: Tag80 parsing error\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }

        if((lenTemp + index) != apdu_r->LenOut)
        {
			EMVBase_Trace("paypass-error: Tag80's len error\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }

        if(lenTemp < 11)
		{
			EMVBase_Trace("paypass-error: Tag80's len < 11\r\n");
			return RLT_ERR_EMV_IccDataFormat;
		}

        while(apdu_r->DataOut[index + countpadding] == 0)
        {
            countpadding++;
            if(countpadding > 2)
            {
            	return RLT_ERR_EMV_IccDataFormat;
			}
        }

        if(emvbase_avl_checkiftagexist(EMVTAG_CryptInfo))
        {
			EMVBase_Trace("paypass-error: CID(Tag9F27) already exist\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }
		if(emvbase_avl_checkiftagexist(EMVTAG_ATC))
        {
			EMVBase_Trace("paypass-error: ATC(Tag9F36) already exist\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }
		if(emvbase_avl_checkiftagexist(EMVTAG_AppCrypt))
        {
			EMVBase_Trace("paypass-error: Application Cryptogram(Tag9F26) already exist\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }

        emvbase_avl_settag(EMVTAG_CryptInfo, apdu_r->DataOut[index]);
        index++;
        emvbase_avl_createsettagvalue(EMVTAG_ATC, &apdu_r->DataOut[index], 2);
        index += 2;
        emvbase_avl_createsettagvalue(EMVTAG_AppCrypt, &apdu_r->DataOut[index], 8);
        index += 8;

        if(apdu_r->LenOut > index)
        {
            k = apdu_r->LenOut - index;
            if(k > 32)
			{
				EMVBase_Trace("paypass-error: Tag80's len > 43\r\n");
				return RLT_ERR_EMV_IccDataFormat;
			}
            if(emvbase_avl_checkiftagexist(EMVTAG_IssuAppData))
            {
				EMVBase_Trace("paypass-error: Issuer App Data(Tag9F10) already exist\r\n");
                return RLT_ERR_EMV_IccDataFormat;
            }

            emvbase_avl_createsettagvalue(EMVTAG_IssuAppData, (unsigned char *)&apdu_r->DataOut[index], k);
        }
    }
    else if(apdu_r->DataOut[index] == 0x77) //TLV coded data
    {
		EMVBase_Trace("paypass-info: GPO template 77\r\n");

        index++;
        if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &lenTemp))
        {
			EMVBase_Trace("paypass-error: Tag77 parsing error\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }
        indexTemp = index;
        indexRet = 0;
        while(index < indexTemp + lenTemp)
        {
            if(index >= apdu_r->LenOut)
            {
                EMVBase_Trace("paypass-error: r-apdu parsing error\r\n");
				EMVBase_Trace("paypass-error: sub Tags' len error\r\n");
                return RLT_ERR_EMV_IccDataFormat;
            }
            if(apdu_r->DataOut[index] == 0xFF || apdu_r->DataOut[index] == 0x00)
            {
                index++;
                continue;
            }

            orgindex = index;

            bIntable = 0;
            GenerateACRetDataflag = 1;

            if(!memcmp((unsigned char *)&apdu_r->DataOut[index], "\x9F\x4B", 2))
            {
                GenerateACRetDataflag = 0;
            }

            //ret = Paypass_ParseAndStoreCardResponse(tempApp_UnionStruct,apdu_r->DataOut,&index,&bIntable,0);
            ret = Paypass_ParseAndStoreCardResponse(tempApp_UnionStruct, apdu_r->DataOut, &index, &bIntable, 2); //20160713_lhd
            if(ret != RLT_EMV_OK)
            {
                return ret;
            }

            //sxl20110406
            if(GenerateACRetDataflag && (indexRet + index - orgindex <= 255))
            {
                memcpy(tempApp_UnionStruct->PaypassTradeParam->GenerateACRetData + indexRet, (unsigned char *)&apdu_r->DataOut[orgindex], index - orgindex);
                indexRet += index - orgindex;
            }

        }   //endwhile
        if(index != indexTemp + lenTemp)
        {
        	EMVBase_Trace("paypass-error: Tag77's value parsing error\r\n");
			EMVBase_Trace("paypass-error: sub Tags' len error\r\n");
            return RLT_ERR_EMV_IccDataFormat;
        }
    }
    else
    {
		EMVBase_Trace("paypass-error: GAC template error (Byte1 != 77 or 80)\r\n");
        return RLT_ERR_EMV_IccDataFormat;
    }

    tempApp_UnionStruct->PaypassTradeParam->GenerateACRetDataLen = indexRet;

	if (emvbase_avl_checkiftagexist(EMVTAG_CryptInfo) == 0)
	{
		EMVBase_Trace("paypass-error: CID(Tag9F27) Missing\r\n");
		return RLT_ERR_EMV_IccDataMissing;
	}
	if (emvbase_avl_checkiftagexist(EMVTAG_ATC) == 0)
	{
		EMVBase_Trace("paypass-error: ATC(Tag9F36) Missing\r\n");
		return RLT_ERR_EMV_IccDataMissing;
	}


    CryptInfo = emvbase_avl_gettagvalue(EMVTAG_CryptInfo);
    reqCrpt = emvbase_avl_gettagvalue(EMVTAG_ReferenceControlParameter);

    if((CryptInfo & 0xc0) == 0x40 && (reqCrpt & TAA_OFFLINE))
    {
		tempApp_UnionStruct->PaypassTradeParam->TransResult = RLT_EMV_OFFLINE_APPROVE;
    }
    else if((CryptInfo & 0xc0) == 0x80 && ((reqCrpt & TAA_OFFLINE) || (reqCrpt & TAA_ONLINE)))
    {
		tempApp_UnionStruct->PaypassTradeParam->TransResult = RLT_EMV_ONLINE_WAIT;
    }
    else if((CryptInfo & 0xc0) == 0x00)
    {
		tempApp_UnionStruct->PaypassTradeParam->TransResult = RLT_EMV_OFFLINE_DECLINE;
    }
    else
    {
        return PAYPASS_CARDDATAERR;
    }

    //NO CDA
    if(emvbase_avl_checkiftagexist(EMVTAG_SignDynAppData) == 0)
    {
		EMVBase_Trace("paypass-info: Signed Dynamic Application Data(Tag9F4B) Missing-->No CDA\r\n");

        //S910.E30
        if(emvbase_avl_checkiftagexist(EMVTAG_AppCrypt) == 0)
        {
			EMVBase_Trace("paypass-error: Application Cryptogram(Tag9F26) Missing\r\n");
            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S9_E27_0);
            return RLT_ERR_EMV_IccDataMissing;
        }
        //S910.E32
        if((CryptInfo & 0xc0) == 0x00) //AAC
        {
        	#if SpecificationBulletinNo239//20200702_lhd del s910 33
            #ifdef PAYPASS_DATAEXCHANGE
            //S910.ED33
            unsigned char IDSStatus = 0;
            emvbase_avl_gettagvalue_spec(EMVTAG_IDSStatus, &IDSStatus, 0, 1);
            if(0x80 & IDSStatus)
            {
                dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S9_E27_0);
                return PAYPASS_CARDDATAERR;
            }
            #endif
			#endif

            if((reqCrpt & 0xC0) == 0x00)
            {
                if(reqCrpt & 0x10)
                {
					EMVBase_Trace("paypass-error: CDA requested\r\n");
                    dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S9_E27_0);
                    return PAYPASS_CARDDATAERR;
                }
            }
        }
        else
        {
            //S910.E34
            if(reqCrpt & 0x10)
            {
            	EMVBase_Trace("paypass-error: CDA requested\r\n");
                dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S9_E27_0);
                return PAYPASS_CARDDATAERR;
            }
            #ifdef PAYPASS_DATAEXCHANGE
            else
            {
                //S910.E70 待确�?加在哪里
                //S910.E70 - S910.E75 先不�?直接写S910.ED76 - S910.ED78
            }
            #endif
        }
    }

    return RLT_EMV_OK;
}

unsigned char Paypass_Recovery(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
	#if SpecificationBulletinNo239 //20200702_lhd for s456 del s456.43 s456.44

	unsigned char retCode = 0;
	unsigned char bDRDOLExist = 0;
	unsigned char bMaxNumTornLog = 0;
    EMVBASE_APDU_SEND *apdu_s = NULL;
    EMVBASE_APDU_RESP *apdu_r = NULL;


	apdu_s = (EMVBASE_APDU_SEND *)emvbase_malloc(sizeof(EMVBASE_APDU_SEND));
	apdu_r = (EMVBASE_APDU_RESP *)emvbase_malloc(sizeof(EMVBASE_APDU_RESP));
	memset(apdu_s, 0, sizeof(EMVBASE_APDU_SEND));
	memset(apdu_r, 0, sizeof(EMVBASE_APDU_RESP));

	tempApp_UnionStruct->ucPaypassintornlog = 0;

	bDRDOLExist = emvbase_avl_checkiftagexist(EMVTAG_DRDOL);
	bMaxNumTornLog = emvbase_avl_checkiftagexist(EMVTAG_MaxNumTornLog);

	if(bDRDOLExist && bMaxNumTornLog) // s456.43, s12.13
	{
		EMVBase_Trace("paypass-info: Recovery supported\r\n");

        retCode = tempApp_UnionStruct->checkifcardintornlog();
        if(retCode)
        {
			EMVBase_Trace("paypass-info: Torn Transaction\r\n");
            tempApp_UnionStruct->ucPaypassintornlog = 1;

            retCode = tempApp_UnionStruct->RECOVERAC(apdu_s);
            if(retCode != RLT_EMV_OK)
            {
            	emvbase_free(apdu_r);
            	emvbase_free(apdu_s);

                return RLT_EMV_TERMINATE_TRANSERR;
            }

            tempApp_UnionStruct->IsoCommand(apdu_s, apdu_r);
            if(apdu_r->ReadCardDataOk != RLT_EMV_OK)
            {
            	emvbase_free(apdu_r);
            	emvbase_free(apdu_s);

                tempApp_UnionStruct->freetornlogtempdata();

                dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S10_E5_0);
                dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S10_E6_0);

                return RLT_ERR_EMV_TransRestart;
            }

            retCode = Paypass_GenerateAC(tempApp_UnionStruct, apdu_r, 0);
            if(retCode != RLT_ERR_EMV_IccReturn)
            {
            	emvbase_free(apdu_r);
            	emvbase_free(apdu_s);

                //20160905_lhd add for case 3MX6-9000  //tempApp_UnionStruct->freetornlogtempdata();//20160824_lhd add
                if(retCode != RLT_EMV_OK)
                {
                    tempApp_UnionStruct->freetornlogtempdata();//20160905_lhd  add for case 3MX6-9000

                    if(retCode == PAYPASS_CARDDATAERR)
                    {
                        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S10_E18_0);
                    }
                    else if(retCode == RLT_ERR_EMV_IccDataMissing)
                    {
                        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S10_E16_0);
                    }
                    else
                    {
                        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S10_E14_0);
                    }
                    dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S10_E50_0);

                    #ifdef PAYPASS_DATAEXCHANGE
                    unsigned char IDSStatus = 0;

                    emvbase_avl_gettagvalue_spec(EMVTAG_IDSStatus, &IDSStatus, 0, 1);
                    if(IDSStatus & 0x40)
                    {
                        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S9_ED8_0);
                    }
                    else
                    {
                        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S10_E53_0);
                    }
                    #else
                    dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S10_E53_0);
                    #endif

                    return RLT_ERR_EMV_OTHERAPCARD;
                }

                return RLT_EMV_OTHER;
            }
        }
    }

	#endif

	return RLT_EMV_OK;
}

unsigned char Paypass_TermActAnaly(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char i, k; //TermAnaResult,CardAnaResult;//0-Denial,1-Online,2-Offline
    unsigned char TVR[5];
    unsigned char TACDenial[5];
    unsigned char IACDenial[5], IACDenialbExist;
    unsigned char TermType;
    unsigned char TACOnline[5], IACOnline[5], IACOnlinebExist;
    unsigned char TACDefault[5], IACDefault[5], IACDefaultbExist;


	EMVBase_Trace("paypass-info: Terminal Action Analysis\r\n");

    memset(TVR, 0, sizeof(TVR));
    emvbase_avl_gettagvalue_spec(EMVTAG_TVR, TVR, 0, 5);
	EMVBase_TraceHex("paypass-info: TVR(Tag95): ", TVR, 5);

    memset(TACDenial, 0, sizeof(TACDenial));
    emvbase_avl_gettagvalue_spec(EMVTAG_TACDenial, TACDenial, 0, 5);
	EMVBase_TraceHex("paypass-info: TAC Denial: ", TACDenial, 5);

    memset(IACDenial, 0, sizeof(IACDenial));
    IACDenialbExist = !(emvbase_avl_gettagvalue_spec(EMVTAG_IACDenial, IACDenial, 0, 5));
	EMVBase_TraceHex("paypass-info: IAC Denial(Tag9F0E): ", IACDenial, 5);

    if(IACDenialbExist == 0)
    {
        for(i = 0; i < 5; i++)
        {
            if((TVR[i]&TACDenial[i]) != 0)
            {
            	EMVBase_Trace("paypass-info: AAC\r\n");
                return TAA_DENIAL;
            }
        }
    }
    else
    {
        for(i = 0; i < 5; i++)
        {
            k = TVR[i];
            if((k & IACDenial[i]) != 0)
            {
            	EMVBase_Trace("paypass-info: AAC\r\n");
                return TAA_DENIAL;
            }
            if((k & TACDenial[i]) != 0)
            {
            	EMVBase_Trace("paypass-info: AAC\r\n");
                return TAA_DENIAL;
            }

        }

    }

    TermType = emvbase_avl_gettagvalue(EMVTAG_TermType);
	EMVBase_Trace("paypass-info: Terminal Type(Tag9F35): %02x\r\n", TermType);

    //online only
    #ifdef PAYPASSV303
    if(TermType == 0x11 || TermType == 0x21 || TermType == 0x14 || TermType == 0x24 || TermType == 0x34)
    {
    	EMVBase_Trace("paypass-info: ARQC\r\n");
        return TAA_ONLINE;
    }
    #endif

    //not offline only
    if(TermType != 0x23 && TermType != 0x26 && TermType != 0x36 && TermType != 0x13 && TermType != 0x16) //Terminal has Online capability
    {
        memset(TACOnline, 0, sizeof(TACOnline));
        emvbase_avl_gettagvalue_spec(EMVTAG_TACOnline, TACOnline, 0, 5);
		EMVBase_TraceHex("paypass-info: TAC Online: ", TACOnline, 5);

        memset(IACOnline, 0, sizeof(IACOnline));
        IACOnlinebExist = !(emvbase_avl_gettagvalue_spec(EMVTAG_IACOnline, IACOnline, 0, 5));
		EMVBase_TraceHex("paypass-info: IAC Online(Tag9F0F): ", IACOnline, 5);

        if(IACOnlinebExist == 0)
        {
            for(i = 0; i < 5; i++)
            {
            	if(4 == i)
            	{
					if(TVR[i] & (TACOnline[i] | 0xFC))
		            {
		            	EMVBase_Trace("paypass-info: ARQC\r\n");
		                return TAA_ONLINE;
		            }
				}
				else
				{
	                if(TVR[i])
	                {
	                	EMVBase_Trace("paypass-info: ARQC\r\n");
	                    return TAA_ONLINE;
	                }
				}
            }

			EMVBase_Trace("paypass-info: TC\r\n");
			return TAA_OFFLINE;
        }
        else
        {
            for(i = 0; i < 5; i++)
            {
                k = TVR[i];
                if((k & IACOnline[i]) != 0)
                {
                	EMVBase_Trace("paypass-info: ARQC\r\n");
                    return TAA_ONLINE;
                }
                if((k & TACOnline[i]) != 0)
                {
                	EMVBase_Trace("paypass-info: ARQC\r\n");
                    return TAA_ONLINE;
                }
            }

			EMVBase_Trace("paypass-info: TC\r\n");
            return TAA_OFFLINE;
        }

    }

    memset(TACDefault, 0, sizeof(TACDefault));
    emvbase_avl_gettagvalue_spec(EMVTAG_TACDefault, TACDefault, 0, 5);
	EMVBase_TraceHex("paypass-info: TAC Default: ", TACDefault, 5);

    memset(IACDefault, 0, sizeof(IACDefault));
    IACDefaultbExist = !(emvbase_avl_gettagvalue_spec(EMVTAG_IACDefault, IACDefault, 0, 5));
	EMVBase_TraceHex("paypass-info: IAC Default(Tag9F0D): ", IACDefault, 5);

    if(IACDefaultbExist == 0)
    {
        for(i = 0; i < 5; i++)
        {
			if(4 == i)
			{
				if(TVR[i] & (TACDefault[i] | 0xFC))
	            {
	            	EMVBase_Trace("paypass-info: AAC\r\n");
	                return TAA_DENIAL;
	            }
			}
			else
			{
				if(TVR[i])
	            {
	            	EMVBase_Trace("paypass-info: AAC\r\n");
	                return TAA_DENIAL;
	            }
			}
        }

		EMVBase_Trace("paypass-info: TC\r\n");
        return TAA_OFFLINE;
    }
    else
    {
        for(i = 0; i < 5; i++)
        {
            k = TVR[i];
            if((k & IACDefault[i]) != 0)
			{
				EMVBase_Trace("paypass-info: AAC\r\n");
				return TAA_DENIAL;
			}
            if((k & TACDefault[i]) != 0)
			{
				EMVBase_Trace("paypass-info: AAC\r\n");
				return TAA_DENIAL;
			}
        }

		EMVBase_Trace("paypass-info: TC\r\n");
        return TAA_OFFLINE;
    }
}

unsigned char Paypass_CommandGAC1(PAYPASSTradeUnionStruct *tempApp_UnionStruct, unsigned char TermAnaResult, EMVBASE_APDU_SEND *apdu_s)
{
    unsigned char retCode = 0;
    unsigned short len= 0;
	unsigned short RelatedDataLen = 0;
    unsigned char TVR[5] = {0}, AIP[2] = {0}, KernelConfig = 0;
    unsigned char *DOL = NULL;   //[255]
    unsigned short DOLLen = 0;
    unsigned char AppCapabilitiesInfor[3] = {0};

    #ifdef PAYPASS_DATAEXCHANGE
    unsigned char bDSACTypeExist = 0;
	unsigned char bDSODSInfoExist = 0;
    unsigned char DSACType = 0;
	unsigned char DSIDSinfoForRead = 0;
	unsigned char DataStorageVersionNumber = 0;
    EMVBASETAGCVLITEM *item = NULL;
    unsigned char DSInputTerm[8] = {0};
	unsigned char DSDigestH[8] = {0};
    unsigned char bDSDOLNeeded = 0;
    #endif


    memset(TVR, 0, sizeof(TVR));
    emvbase_avl_gettagvalue_spec(EMVTAG_TVR, TVR, 0, 5);
    memset(AIP, 0, sizeof(AIP));
    emvbase_avl_gettagvalue_spec(EMVTAG_AIP, AIP, 0, 2);
    KernelConfig = emvbase_avl_gettagvalue(EMVTAG_KernelConfiguration);

    memset(DSInputTerm, 0x00, sizeof(DSInputTerm));
    memset(DSDigestH, 0x00, sizeof(DSDigestH));

	#if SpecificationBulletinNo239 //20200702_lhd
    item = emvbase_avl_gettagitempointer(EMVTAG_IDSStatus);
    if((item != NULL) && (0x80 & item->data[0]))
    {
		EMVBase_Trace("paypass-info: IDS Read Flag set\r\n");

        // GAC.2
        if(TVR[0] & 0x04) // CDA fail
        {
            // GAC.22
            if((AIP[0] & 0x02) && (KernelConfig & 0x20))
            {
				EMVBase_Trace("paypass-info: On device cardholder verification supported by card and terminal-->AAC\r\n");
                TermAnaResult = TAA_DENIAL;
            }

            // GAC.26
            emvbase_avl_settag(EMVTAG_ReferenceControlParameter, TermAnaResult);
        }
        else
        {
			if(emvbase_avl_checkiftagexist(EMVTAG_DSODSInfo) && emvbase_avl_checkiftagexist(EMVTAG_DSDOL)) // GAC.3  GAC.4
            {
                //GAC.ED5
                item = emvbase_avl_gettagitempointer(EMVTAG_DSACType);
                if(item != NULL)
                {
                    if(item->len > 0)
                    {
                        DSACType = item->data[0];
                        bDSACTypeExist = 1;
						EMVBase_Trace("paypass-info: DS AC Type: %02x\r\n", DSACType);
                    }
                }

                item = emvbase_avl_gettagitempointer(EMVTAG_DSODSInfoReader);
                if(item != NULL)
                {
                    if(item->len > 0)
                    {
                        DSIDSinfoForRead = item->data[0];
                        bDSODSInfoExist = 1;
						EMVBase_Trace("paypass-info: DS ODS Info for Reader: %02x\r\n", DSIDSinfoForRead);
                    }
                }

                if(bDSACTypeExist && bDSODSInfoExist)
                {
                	EMVBase_Trace("paypass-info: AC Type: %02x\r\n", TermAnaResult);

                    //GAC.ED7
                    if((0x00 == (0xC0 & DSACType)) || (TermAnaResult == DSACType) ||
                            ((0x80 == (0xC0 & DSACType)) && (0x40 == (0xC0 & TermAnaResult))))
                    {
                        //gac.ed8
                        TermAnaResult = DSACType;

                        //gac.ed40
                        if(FindaTagInTagList(EMVTAG_DSDOL, EMVTAG_DSDigestH, 2))
                        {
                            item = emvbase_avl_gettagitempointer(EMVTAG_DSInputTerm);
                            if(item != NULL)
                            {
                                memcpy(DSInputTerm, item->data, item->len);
                                EMVBase_TraceHex("paypass-info: DS Input Term(TagDF8109): ", DSInputTerm, item->len);

                                //gac.ed42
                                item = emvbase_avl_gettagitempointer(EMVTAG_AppCapabilitiesInfor);
                                if(item != NULL)
                                {
                                    if(item->len)
                                    {
                                        DataStorageVersionNumber = item->data[0];
                                    }
                                }
								EMVBase_Trace("paypass-info: Data Storage Version Number: %02x\r\n", DataStorageVersionNumber);

                                if(0x01 == (DataStorageVersionNumber & 0x0F))
                                {
                                    //gac.ed43
									EMVBase_Trace("paypass-info: DSVN == Version 1\r\n");
                                    OWHF2(DSInputTerm, DSDigestH);
                                }
                                else
                                {
                                    //gac.ed44
                                    EMVBase_Trace("paypass-info: DSVN != Version 1\r\n");
                                    OWHF2AES(DSInputTerm, DSDigestH);
                                }
								emvbase_avl_createsettagvalue(EMVTAG_DSDigestH, DSDigestH, 8);
                            }
                        }

                        //gac.ed45
                        TermAnaResult |= 0x10;
                        emvbase_avl_settag(EMVTAG_ReferenceControlParameter, TermAnaResult);

                        //gac.ed47
                        bDSDOLNeeded = 1;
                    }
                    else//GAC.ED9
                    {
                        if(((0x00 == (0xC0 & TermAnaResult)) && (0x20 == (0x20 & DSIDSinfoForRead))) ||
                                ((0x80 == (0xC0 & TermAnaResult)) && (0x40 == (0x40 & DSIDSinfoForRead))))
                        {
                            //gac.ed40
                            if(FindaTagInTagList(EMVTAG_DSDOL, EMVTAG_DSDigestH, 2))
                            {
                                item = emvbase_avl_gettagitempointer(EMVTAG_DSInputTerm);
                                if(item != NULL)
                                {
                                    memcpy(DSInputTerm, item->data, item->len);
                                    EMVBase_TraceHex("paypass-info: DS Input Term(TagDF8109): ", DSInputTerm, 8);

                                    //gac.ed42
                                    item = emvbase_avl_gettagitempointer(EMVTAG_AppCapabilitiesInfor);
                                    if(item != NULL)
                                    {
                                        if(item->len)
                                        {
                                            DataStorageVersionNumber = item->data[0];
                                        }
                                    }
									EMVBase_Trace("paypass-info: Data Storage Version Number: %02x\r\n", DataStorageVersionNumber);

                                    if(0x01 == (DataStorageVersionNumber & 0x0F))
                                    {
                                        //gac.ed43
										EMVBase_Trace("paypass-info: DSVN == Version 1\r\n");
                                        OWHF2(DSInputTerm, DSDigestH);
                                    }
                                    else
                                    {
                                        //gac.ed44
										EMVBase_Trace("paypass-info: DSVN != Version 1\r\n");
                                        OWHF2AES(DSInputTerm, DSDigestH);
                                    }
									emvbase_avl_createsettagvalue(EMVTAG_DSDigestH, DSDigestH, 8);
                                }
                            }

                            //gac.ed45
                            TermAnaResult |= 0x10;
                            emvbase_avl_settag(EMVTAG_ReferenceControlParameter, TermAnaResult);

                            //gac.ed47
                            bDSDOLNeeded = 1;
                        }
                        else
                        {
                            //gac.ed10
                            if(( 0x04 == (0x04 & DSIDSinfoForRead) ))
                            {
                                //gac.ed11
                                dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S3_90_1);
                                dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_SGAC_ED11);
                                return RLT_ERR_EMV_OTHERAPCARD;
                            }
                            else
                            {
                                //gac.ed27
                                TermAnaResult |= 0x10;
                                emvbase_avl_settag(EMVTAG_ReferenceControlParameter, TermAnaResult);
                            }
                        }
                    }
                }
                else
                {
                	// GAC.6
                    dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S3_90_1);
                    dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_SGAC_ED6);
                    return RLT_ERR_EMV_OTHERAPCARD;
                }
            }
            else
            {
            	// GAC.27
                TermAnaResult |= 0x10;
                emvbase_avl_settag(EMVTAG_ReferenceControlParameter, TermAnaResult);
            }
        }
    }
    else
	#endif
    {
        //no ids
        //gac.20
        if(tempApp_UnionStruct->PaypassTradeParam->ODAstaus)
        {
            //gac.21
            if(TVR[0] & 0x04) // CDA fail
            {
                //gac.22
                if((AIP[0] & 0x02) && (KernelConfig & 0x20))
                {
					EMVBase_Trace("paypass-info: On device cardholder verification supported by card and terminal-->AAC\r\n");
                    TermAnaResult = TAA_DENIAL;
                }

                //gac.26
                emvbase_avl_settag(EMVTAG_ReferenceControlParameter, TermAnaResult);
            }
            else//gac.24
            {
                if(TermAnaResult == TAA_DENIAL)
                {
                    //gac.25
                    memset(AppCapabilitiesInfor, 0, sizeof(AppCapabilitiesInfor));
                    emvbase_avl_gettagvalue_spec(EMVTAG_AppCapabilitiesInfor, AppCapabilitiesInfor, 0, 3);
					EMVBase_TraceHex("paypass-info: Application Capabilities Information(Tag9F5D): ", AppCapabilitiesInfor, 3);

                    if(AppCapabilitiesInfor[1] & 0x01)
                    {
                        //gac.27
						EMVBase_Trace("paypass-info: CDA SUPPORTED OVER TC, ARQC AND AAC(Tag9F5D Byte2 bit1 is 1)\r\n");
                        TermAnaResult |= 0x10;
                        emvbase_avl_settag(EMVTAG_ReferenceControlParameter, TermAnaResult);
                    }
                    else
                    {
                        //gac.26
                        emvbase_avl_settag(EMVTAG_ReferenceControlParameter, TermAnaResult);
                    }
                }
                else
                {
                    //gac.27
                    TermAnaResult |= 0x10;
                    emvbase_avl_settag(EMVTAG_ReferenceControlParameter, TermAnaResult);
                }
            }
        }
        else
        {
            //gac.26
            emvbase_avl_settag(EMVTAG_ReferenceControlParameter, TermAnaResult);
        }
    }

    // CDOL1 AND DSDOL
    memcpy(apdu_s->Command, "\x80\xAE\x00\x00", 4);

    apdu_s->Command[2] = TermAnaResult;

    DOL = (unsigned char *)emvbase_malloc(256);	//20201210 lhd  255
    memset(DOL, 0, 256);//255
    len = 0;
	RelatedDataLen = 0;

    emvbase_avl_gettagvalue_all(EMVTAG_CDOL1, DOL, &DOLLen);
    EMVBase_TraceHex("paypass-info: CDOL1(Tag8C): ", DOL, DOLLen);

    retCode = Paypass_DOLProcess(typeCDOL1, DOL, DOLLen, &apdu_s->DataIn[0], &RelatedDataLen, 255, tempApp_UnionStruct);
    if(retCode == RLT_EMV_OK)
    {
        emvbase_avl_createsettagvalue(EMVTAG_CDOL1RelatedData, apdu_s->DataIn, RelatedDataLen);
		len += RelatedDataLen;
    }

    if(bDSDOLNeeded && (255 > len))
    {
        memset(DOL, 0, 255);
        DOLLen = 0;
        emvbase_avl_gettagvalue_all(EMVTAG_DSDOL, DOL, &DOLLen);
        RelatedDataLen = 0;

        EMVBase_TraceHex("paypass-info: DSDOL(Tag9F5B): ", DOL, DOLLen);

        retCode = Paypass_DOLProcess(typeDSDOL, DOL, DOLLen, &apdu_s->DataIn[len], &RelatedDataLen, (255 - len), tempApp_UnionStruct);

        len += RelatedDataLen;

        emvbase_avl_setvalue_or(EMVTAG_IDSStatus, 0, 0x40);
    }

    apdu_s->Lc = len;
    apdu_s->Le = 256;
    apdu_s->EnableCancel = 0;

	emvbase_free(DOL);
    return retCode;

}

unsigned char Paypass_TermiAnalys(PAYPASSTradeUnionStruct *tempApp_UnionStruct, unsigned char TermAnaResult)
{
    unsigned char retCode;
    EMVBASE_APDU_SEND *apdu_s;
    EMVBASE_APDU_RESP *apdu_r;
    unsigned char TronFuncRet = 0;
    unsigned char temptransTypeValue;
    unsigned char IDSStatus = 0;


	EMVBase_Trace("paypass-info: --Gen AC--\r\n");

	apdu_s = (EMVBASE_APDU_SEND *)emvbase_malloc(sizeof(EMVBASE_APDU_SEND));
	apdu_r = (EMVBASE_APDU_RESP *)emvbase_malloc(sizeof(EMVBASE_APDU_RESP));
	memset(apdu_s, 0x00, sizeof(EMVBASE_APDU_SEND));
	memset(apdu_r, 0x00, sizeof(EMVBASE_APDU_RESP));

    retCode = Paypass_CommandGAC1(tempApp_UnionStruct, TermAnaResult, apdu_s);
    if(retCode != RLT_EMV_OK)
    {
    	emvbase_free(apdu_r);
        emvbase_free(apdu_s);
        return retCode;
    }

    tempApp_UnionStruct->IsoCommand(apdu_s, apdu_r);

    emvbase_free(apdu_s);
    apdu_s = NULL;

	if(apdu_r->ReadCardDataOk != RLT_EMV_OK)
    {
		EMVBase_Trace("paypass-error: apdu timeout or other error\r\n");

        emvbase_free(apdu_r);
        apdu_r = NULL;

        if(tempApp_UnionStruct->ucPaypassintornlog) //s11.7  removed for SB261 but reserved in production
        {
            TornTransactionLogIDSPartRecord ptornlogrecord;

            memset(&ptornlogrecord, 0x00, sizeof(ptornlogrecord));

            if(NULL != tempApp_UnionStruct->gettornlogtempdata)
            {
                TronFuncRet = tempApp_UnionStruct->gettornlogtempdata(&ptornlogrecord);
            }

            if(0 == TronFuncRet)
            {
                EMVBase_Trace("\r\n paypass-error: Torn Transaction Log exist-IDSStatus: \r\n", ptornlogrecord.IDSStatus);

                if(ptornlogrecord.IDSStatus & 0x40)
                {
                    tempApp_UnionStruct->freetornlogtempdata();//20160824_lhd add
                }
                else
                {
                    tempApp_UnionStruct->freetornlogtempdata();//20160824_lhd add
                    tempApp_UnionStruct->deletetornlog();
                }
            }

            //S11.13
            tempApp_UnionStruct->addtornlog((unsigned int)tempApp_UnionStruct);
            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S9_E14_0);
            dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S9_E15_0);

			return RLT_EMV_TORN;
        }
        else//s9
        {
        	#if (!SpecificationBulletinNo239)//2020072_lhd s456 del s9.5 s9.6 s9.7 s9.8

			dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S9_E9_0);
            dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S9_E10_0);
            return RLT_ERR_EMV_TransRestart;

			#else

            //S9.5
            //if( (emvbase_avl_gettagvalue(EMVTAG_MaxNumTornLog) > 0 ) && emvbase_avl_checkiftagexist(EMVTAG_DRDOL) )
            if((emvbase_avl_gettagvalue(EMVTAG_MaxNumTornLog) > 0) && emvbase_avl_checkiftagexist(EMVTAG_DRDOL)
				&& (0 == tempApp_UnionStruct->PaypassTradeParam->bretforminit)) // removed for SB261 but retained in production
            {
                tempApp_UnionStruct->addtornlog((unsigned int)tempApp_UnionStruct);

                dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S9_E14_0);
                dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S9_E15_0);

				return RLT_EMV_TORN;
            }
            else//s9.6
            {
                emvbase_avl_gettagvalue_spec(EMVTAG_IDSStatus, &IDSStatus, 0, 1);

                if(IDSStatus & 0x40)
                {
                    dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S9_ED7_0);
                    dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S9_8);
                    return RLT_ERR_EMV_OTHERAPCARD;
                }
                else
                {
                    dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S9_E9_0);
                    dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S9_E10_0);
                    return RLT_ERR_EMV_TransRestart;
                }
            }
			#endif
        }
    }


    if(tempApp_UnionStruct->ucPaypassintornlog)//s11.5  // removed for SB261 but reserved in production
    {
        tempApp_UnionStruct->deletetornlog();
    }

    retCode = Paypass_GenerateAC(tempApp_UnionStruct, apdu_r, 1);

    emvbase_free(apdu_r);
    apdu_r = NULL;

    if(tempApp_UnionStruct->ucPaypassintornlog)
    {
        unsigned char ReferenceControlParameter = emvbase_avl_gettagvalue(EMVTAG_ReferenceControlParameter);

        if(emvbase_avl_checkiftagexist(EMVTAG_SignDynAppData) && (ReferenceControlParameter & 0x10)) //check if do CDA faile
        {

        }
        else
        {
            tempApp_UnionStruct->freetornlogtempdata();//no cda 后面用不到先删除 以免错误的时�?输出 tron for case 3MX6 7099
            //In Discretionary data, Torn Record shall not be present
        }
    }

    if(retCode != RLT_EMV_OK)
    {
    	if(retCode == RLT_ERR_EMV_IccReturn)
    	{
			dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S9_E17_0);
		}
        else if(retCode == RLT_ERR_EMV_IccDataMissing)
        {
            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S10_E22_0);
        }
        else if(retCode == PAYPASS_CARDDATAERR)
        {
            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S10_E24_0);
        }
        else
        {
            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S10_E20_0);
        }

        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S10_E50_0);

        #ifdef PAYPASS_DATAEXCHANGE
        unsigned char IDSStatus = 0;

        emvbase_avl_gettagvalue_spec(EMVTAG_IDSStatus, &IDSStatus, 0, 1);
        if(IDSStatus & 0x40)
        {
            dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S11_E77_0); //20160825
        }
        else
        {
            dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S10_E53_0);
        }
        #else
        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S10_E53_0);
        #endif

        return RLT_ERR_EMV_OTHERAPCARD;
    }

    return RLT_EMV_OK;
}

unsigned char Paypass_GetIPK(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char IPKData[266], IPKHash[20];
    unsigned char buf[12], bytePAN[8], byteIssuID[8];
    EMVBASE_IPK_RECOVER recovIPK;
    EMVBASETAGCVLITEM *item;
    unsigned char IPKCertexist;
    unsigned char IPKRemexist;
    unsigned char IPKExpexist;
    unsigned char PAN[10];
    unsigned short PANLen;
    unsigned char IPKRem[36];
    int i, index, ret;
    EMVBASE_CAPK_STRUCT *tempcapk;
	static unsigned char callbackFlag = 0;


    tempcapk = tempApp_UnionStruct->PaypassTradeParam->CAPK;
    if(NULL == tempcapk)
    {
        EMVBase_Trace("\r\n paypass-error: tempApp_UnionStruct->PaypassTradeParam->CAPK==null err! \r\n");
    }

    IPKCertexist = 0;
    item = emvbase_avl_gettagitempointer(EMVTAG_IPKCert);

    if(item != NULL)
    {
        if(item->len)
        {
            IPKCertexist = 1;
        }
    }

    if(IPKCertexist == 0)
    {
    	EMVBase_Trace("\r\n paypass-error: IPKCert(90) not exist !!! \r\n");
        return RLT_ERR_EMV_IccDataFormat;
    }

    if(item->len != tempcapk->ModulLen) 		//检查发卡行公钥证书和认证中心公钥模长度是否相同
    {
		EMVBase_Trace("\r\n paypass-error: IPKCert len=%d,ModulLen=%d,Unequal length err! \r\n",item->len,tempcapk->ModulLen);
        return RLT_ERR_EMV_IccDataFormat;
    }
    memset(IPKData, 0, 266);

	EMVBase_RSARecover(tempcapk->Modul, tempcapk->ModulLen, tempcapk->Exponent, tempcapk->ExponentLen, item->data, IPKData);
    EMVBase_FillIPK(IPKData, &recovIPK, tempcapk);					//数据格式转换

    if(recovIPK.DataTrail != 0xBC)				//数据格式判断
    {
    	EMVBase_Trace("\r\n paypass-error:IPK certifate is invalid,DataTrail=0x%x, \r\n",recovIPK.DataTrail);
        return RLT_ERR_EMV_IccDataFormat;
    }
    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-error:getipk %02x %02x \r\n", recovIPK.DataHead, recovIPK.CertFormat);
    #endif


    if(recovIPK.DataHead != 0x6A)
    {
    	EMVBase_Trace("\r\n paypass-error:IPK certifate is invalid,DataHead=0x%x, \r\n",recovIPK.DataHead);
        return RLT_ERR_EMV_IccDataFormat;
    }

    if(recovIPK.CertFormat != 0x02)
    {
		EMVBase_Trace("\r\n paypass-error:IPK certifate is invalid,CertFormat=0x%x, \r\n",recovIPK.CertFormat);
        return RLT_ERR_EMV_IccDataFormat;
    }
    //sign all the correspoding variables to the IPK
    index = 0;

    for(i = 0; i < tempcapk->ModulLen - 22; i++)
    {
        IPKData[i] = IPKData[i + 1];
    }

    index += tempcapk->ModulLen - 22;

    memset(IPKRem, 0, sizeof(IPKRem));

    if(recovIPK.IPKLen > tempcapk->ModulLen - 36)
    {
        IPKRemexist = 0;
        item = emvbase_avl_gettagitempointer(EMVTAG_IPKRem);

        if(item != NULL)
        {
            if(item->len)
            {
                IPKRemexist = 1;
            }
        }

        if(IPKRemexist == 0)
        {
			EMVBase_Trace("\r\n paypass-error:IPKRem(92) not exist !!!  \r\n");

            emvbase_avl_setvalue_or(EMVTAG_TVR, 0, 0x20); //ICC data missing
            return RLT_ERR_EMV_IccDataMissing;
        }
        memcpy(IPKRem, item->data, item->len);
        memcpy((unsigned char *)&IPKData[index], (unsigned char *)item->data, item->len);
        index += item->len;
    }
    IPKExpexist = 0;
    item = emvbase_avl_gettagitempointer(EMVTAG_IPKExp);

    if(item != NULL)
    {
        if(item->len)
        {
            IPKExpexist = 1;
        }
    }

    if(IPKExpexist)
    {
        memcpy((unsigned char *)&IPKData[index], (unsigned char *)item->data, item->len);
        index += item->len;
    }
    memset(IPKHash, 0, 20);

    EMVBase_Hash(IPKData, index, IPKHash);		//讲连接的数据进行HASH

    if(recovIPK.HashInd == 0x01)  //SHA-1 algorithm //HASH算法类型�?
    {
        if(memcmp(recovIPK.HashResult, IPKHash, 20))
        {
            EMVBase_Trace("\r\n paypass-error:IPK Hash result is not equal !!!  \r\n");
			EMVBase_TraceHex("\r\n paypass-error: HashResult",recovIPK.HashResult,20);
			EMVBase_TraceHex("\r\n paypass-error: IPKHash",IPKHash,20);
            return RLT_ERR_EMV_IccDataFormat;
        }
    }
    else
    {
		EMVBase_Trace("\r\n paypass-error:IPK Hash result is err !!! HashInd=0x%x  \r\n",recovIPK.HashInd);
        return RLT_ERR_EMV_IccDataFormat;
    }
    memset(PAN, 0, sizeof(PAN));
    emvbase_avl_gettagvalue_all(EMVTAG_PAN, PAN, &PANLen);

    //比较帐号
    //verify if leftmost 3-8 digits of PAN matches IssuID
    for(i = 0; i < 4; i++)
    {
        bytePAN[2 * i] = (PAN[i] & 0xF0) >> 4;
        bytePAN[2 * i + 1] = PAN[i] & 0x0F;
        byteIssuID[2 * i] = (recovIPK.IssuID[i] & 0xF0) >> 4;
        byteIssuID[2 * i + 1] = recovIPK.IssuID[i] & 0x0F;
    }

    #ifdef EMVB_DEBUG
//    EMVBase_TraceHex("\r\n paypass-info: \r\nSDA PAN:", bytePAN, 8);
//    EMVBase_TraceHex("\r\n paypass-info: \r\nbyteIssuID:", byteIssuID, 8);
    #endif

    for(i = 7; i >= 2; i--)
    {
        if(byteIssuID[i] != 0x0F)
        {
            if(memcmp(byteIssuID, bytePAN, i + 1))
            {
                EMVBase_Trace("\r\n paypass-error:byteIssuID and bytePAN unequal !!! len=%d,  \r\n",(i+1));
//				EMVBase_TraceHex("\r\n paypass-error: byteIssuID",byteIssuID,(i+1));
//				EMVBase_TraceHex("\r\n paypass-error: bytePAN",bytePAN,(i+1));
                return RLT_ERR_EMV_IccDataFormat;
            }
            else
            {
                break;				  //
            }
        }
    }

    if(i < 2)
    {
		EMVBase_Trace("\r\n paypass-error:PAN len err! i=%d, \r\n",i);
        return RLT_ERR_EMV_IccDataFormat;
    }

    //比较是否过期
    //verify if expiredate is later than current date
    if(EMVBase_ExpireDateVerify(recovIPK.ExpireDate) != RLT_EMV_OK)
    {
		EMVBase_Trace("\r\n paypass-error:expiredate is later than current date, \r\n");
        return RLT_EMV_ERR;
    }
    memset(buf, 0, sizeof(buf));
    //check if IPK is revoked.
    memcpy(buf, tempcapk->RID, 5);
    #ifdef EMVB_DEBUG
    EMVBase_TraceHex("\r\n paypass-info: \r\ntempcapk->RID:", tempcapk->RID, 5);
    #endif
    buf[5] = tempcapk->CAPKI;
    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-info:CAPKI %02x\r\n", tempcapk->CAPKI);
    #endif
    memcpy((unsigned char *)&buf[6], recovIPK.CertSerial, 3);
    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-info:certserial:%02x %02x %02x\r\n", recovIPK.CertSerial[0], recovIPK.CertSerial[1], recovIPK.CertSerial[2]);
    #endif

	if(callbackFlag == 0)
	{
		callbackFlag++;
		return PAYPASS_REQ_SETCALLBACK_REVOKEY;
	}
	else
	{
		ret = tempApp_UnionStruct->GetVerifyCAPKRes();
		callbackFlag = 0;
		if(ret == -1)
		{
			EMVBase_Trace("\r\n paypass-error:RID and CAPK index is in the ipkrevoke check list!!! ,\r\n");
			EMVBase_TraceHex("\r\n paypass-error: RID+CAPKI+CertSerial=",buf,sizeof(buf));
			return RLT_EMV_ERR;
		}
	}

    //比较是否RID,认证中心公钥索引,证书序列号是否匹�?
//    if (NULL == tempApp_UnionStruct->termipkrevokecheck)
//    {
//        EMVBase_Trace("\r\n paypass-error:tempApp_UnionStruct->termipkrevokecheck == null err ! \r\n");
//    }
//    else
//    {
//    	if(1 != tempApp_UnionStruct->termipkrevokecheck(buf))//����SDK_OK��ʾ���ǻ��չ�Կ
//	    {
//
//			EMVBase_Trace("\r\n paypass-error:RID and CAPK index is in the ipkrevoke check list!!! ,\r\n");
//			EMVBase_TraceHex("\r\n paypass-error: RID+CAPKI+CertSerial=",buf,sizeof(buf));
//	        return RLT_EMV_ERR;
//	    }
//    }

    //算法识别
    if(recovIPK.IPKAlgoInd != 0x01)  //other than '01' is not recognised.
    {
		EMVBase_Trace("\r\n paypass-error:IPKAlgoInd=%d, other than '01' is not recognised, \r\n",recovIPK.IPKAlgoInd);
        return RLT_ERR_EMV_IccDataFormat;
    }
    tempApp_UnionStruct->PaypassTradeParam->IPKModulLen = recovIPK.IPKLen;
    if(tempApp_UnionStruct->PaypassTradeParam->IPKModul == NULL)
    {
        tempApp_UnionStruct->PaypassTradeParam->IPKModul = (unsigned char *)emvbase_malloc(recovIPK.IPKLen);
        if(tempApp_UnionStruct->PaypassTradeParam->IPKModul == NULL)
        {
	        EMVBase_Trace("\r\n paypass-error:\n (tempApp_UnionStruct->PaypassTradeParam->IPKModul == NULL) , \r\n");
            return RLT_EMV_ERR;
        }
        memset(tempApp_UnionStruct->PaypassTradeParam->IPKModul, 0, recovIPK.IPKLen);
    }

    if(recovIPK.IPKLen <= tempcapk->ModulLen - 36)
    {
        memcpy(tempApp_UnionStruct->PaypassTradeParam->IPKModul, recovIPK.IPKLeft, recovIPK.IPKLen);
    }
    else
    {
        memcpy(tempApp_UnionStruct->PaypassTradeParam->IPKModul, recovIPK.IPKLeft, tempcapk->ModulLen - 36);
        memcpy((unsigned char *)&tempApp_UnionStruct->PaypassTradeParam->IPKModul[tempcapk->ModulLen - 36], IPKRem, recovIPK.IPKLen - tempcapk->ModulLen + 36);
    }
    return RLT_EMV_OK;
}

unsigned char Paypass_GetICCPK(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char ICCPKData[248], ICCPKHash[20];	 //ICCPKToSign[2048+256],
    unsigned short index;
    EMVBASE_ICCPK_RECOVER recovICCPK;
    EMVBASETAGCVLITEM *item, *item1;
    unsigned char *ICCPKToSign=NULL;
	unsigned int fixlen = 0;
	unsigned short offset = 0;


    unsigned char testlen;
    unsigned char tmppan1[50], tmppan1len, tmppan2[50], tmppan2len;
    unsigned char IPKExpexist, ICCPKCertexist, ICCPKRemexist, PANexist;
    unsigned char ICCPKRem[42];

    #ifdef EMVB_DEBUG
	    int k;

	    EMVBase_Trace("\r\n paypass-error:dllpaypass_GetICCPK\r\n");
    #endif

	if(tempApp_UnionStruct->PaypassTradeParam->AuthDataLen)
	{
		if(tempApp_UnionStruct->PaypassTradeParam->AuthData == NULL)
		{
			EMVBase_Trace("\r\n paypass-error:EmvTradeParam->AuthData is freed \r\n");
			return RLT_EMV_ERR;
		}
	}

    ICCPKCertexist = 0;
    item = emvbase_avl_gettagitempointer(EMVTAG_ICCPKCert);

    if(item != NULL)
    {
        if(item->len)
        {
            ICCPKCertexist = 1;
        }
    }

    if(ICCPKCertexist == 0)
    {
    	EMVBase_Trace("\r\n paypass-error: ICCPKCert(9F46) not exist !!! \r\n");
        return RLT_ERR_EMV_IccDataFormat;
    }

    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-error:tempApp_UnionStruct->PaypassTradeParam->IPKModulLen = %d %d\r\n", item->len, tempApp_UnionStruct->PaypassTradeParam->IPKModulLen);
    #endif

    if(item->len != tempApp_UnionStruct->PaypassTradeParam->IPKModulLen)
    {
		EMVBase_Trace("\r\n paypass-error: ICCPKCertlen=%d,IPKModulLen=%d,Unequal length err! \r\n",item->len,tempApp_UnionStruct->PaypassTradeParam->IPKModulLen);
        return RLT_EMV_ERR;
    }

    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-error:dllpaypass_GetICCPK111\r\n");
    #endif


    IPKExpexist = 0;
    item = emvbase_avl_gettagitempointer(EMVTAG_IPKExp);

    if(item != NULL)
    {
        if(item->len)
        {
            IPKExpexist = 1;
        }
    }

    if(IPKExpexist == 0)
    {
		 EMVBase_Trace("\r\n paypass-error: IPKExp(9F32) not exist !!! \r\n");
        return RLT_ERR_EMV_IccDataFormat;
    }
    ICCPKCertexist = 0;
    item1 = emvbase_avl_gettagitempointer(EMVTAG_ICCPKCert);

    if(item1 != NULL)
    {
        if(item1->len)
        {
            ICCPKCertexist = 1;
        }
    }

    if(ICCPKCertexist == 0)
    {
		  EMVBase_Trace("\r\n paypass-error: ICCPKCert(9F46) not exist !!! \r\n");
        return RLT_ERR_EMV_IccDataFormat;
    }
    EMVBase_RSARecover(tempApp_UnionStruct->PaypassTradeParam->IPKModul, tempApp_UnionStruct->PaypassTradeParam->IPKModulLen, item->data, item->len, item1->data, ICCPKData);

    EMVBase_FillICCPK(ICCPKData, tempApp_UnionStruct->PaypassTradeParam->IPKModulLen, &recovICCPK);

    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-error:dllpaypass_GetICCPK %02x %02x %02x\r\n", recovICCPK.DataTrail, recovICCPK.DataHead, recovICCPK.CertFormat);

    #endif

    if(recovICCPK.DataTrail != 0xBC)			//If it is not ‘BC�?ICCPK certifate is invalid
    {
	  EMVBase_Trace("\r\n paypass-error:ICCPK certifate is invalid,DataTrail=0x%x, \r\n",recovICCPK.DataTrail);
        return RLT_EMV_ERR;
    }

    if(recovICCPK.DataHead != 0x6A) 			//If it is not �?A�?ICCPK certifate is invalid
    {
	  EMVBase_Trace("\r\n paypass-error:ICCPK certifate is invalid,DataHead=0x%x, \r\n",recovICCPK.DataHead);
        return RLT_EMV_ERR;
    }

    if(recovICCPK.CertFormat != 0x04)	//If it is not �?4�? ICCPK certifate is invalid
    {
	  EMVBase_Trace("\r\n paypass-error:ICCPK certifate is invalid,CertFormat=0x%x, \r\n",recovICCPK.CertFormat);
        return RLT_EMV_ERR;
    }
    index = 0;

    ICCPKToSign = (unsigned char *)emvbase_malloc(2048 + 256);
	if (NULL == ICCPKToSign)
	{
		EMVBase_Trace("\r\n paypass-error: emvbase_malloc(2048 + 256) err !!!!!! \r\n");
		return RLT_EMV_ERR;
	}

    memcpy(ICCPKToSign, (unsigned char *)&ICCPKData[1], tempApp_UnionStruct->PaypassTradeParam->IPKModulLen - 22);
    index += tempApp_UnionStruct->PaypassTradeParam->IPKModulLen - 22;

    memset(ICCPKRem, 0, sizeof(ICCPKRem));

    if(recovICCPK.ICCPKLen > (tempApp_UnionStruct->PaypassTradeParam->IPKModulLen - 42))
    {
        ICCPKRemexist = 0;
        item1 = emvbase_avl_gettagitempointer(EMVTAG_ICCPKRem);

        if(item1 != NULL)
        {
            if(item1->len)
            {
                ICCPKRemexist = 1;
            }
        }

        if(ICCPKRemexist == 0)		//if ICCPK Rem exist
        {
			EMVBase_Trace("\r\n paypass-error:ICCPKRem(9F48) not exist !!!  \r\n");
            emvbase_avl_setvalue_or(EMVTAG_TVR, 0, 0x20);			//ICC data missing

            emvbase_free(ICCPKToSign);
            return RLT_ERR_EMV_IccDataMissing;
        }
        memcpy(ICCPKRem, item1->data, item1->len);
        memcpy((unsigned char *)&ICCPKToSign[index], (unsigned char *)item1->data, item1->len);
        index += item1->len;
    }
    ICCPKCertexist = 0;
    item = emvbase_avl_gettagitempointer(EMVTAG_ICCPKExp);

    if(item != NULL)
    {
        if(item->len)
        {
            ICCPKCertexist = 1;
        }
    }

    if(ICCPKCertexist == 0)
    {
        emvbase_free(ICCPKToSign);

		EMVBase_Trace("\r\n paypass-error:ICCPKExp(9F47) not exist !!!  \r\n");
        return RLT_ERR_EMV_IccDataFormat;
    }
    memcpy((unsigned char *)&ICCPKToSign[index], (unsigned char *)item->data, item->len);
    index += item->len;
	fixlen = index;

    memcpy((unsigned char *)&ICCPKToSign[index], tempApp_UnionStruct->PaypassTradeParam->AuthData, tempApp_UnionStruct->PaypassTradeParam->AuthDataLen);
    index += tempApp_UnionStruct->PaypassTradeParam->AuthDataLen;

    EMVBase_Hash(ICCPKToSign, index, ICCPKHash);

    if(recovICCPK.HashInd == 0x01)	//SHA-1 algorithm
    {
        if(memcmp(recovICCPK.HashResult, ICCPKHash, 20))
        {
            emvbase_free(ICCPKToSign);

			EMVBase_Trace("\r\n paypass-error:Hash result is not equal !!!  \r\n");
			EMVBase_TraceHex("\r\n paypass-error: HashResult",recovICCPK.HashResult,20);
			EMVBase_TraceHex("\r\n paypass-error: ICCPKHash",ICCPKHash,20);

            return RLT_EMV_ERR;
        }
    }
    else
    {
		EMVBase_Trace("\r\n paypass-error:Hash result is err !!! HashInd=0x%x  \r\n",recovICCPK.HashInd);

        emvbase_free(ICCPKToSign);
        return RLT_EMV_ERR;
    }
    PANexist = 0;
    item1 = emvbase_avl_gettagitempointer(EMVTAG_PAN);

    if(item1 != NULL)
    {
        if(item1->len)
        {
            PANexist = 1;
        }
    }

    if(PANexist == 0)
    {
        emvbase_free(ICCPKToSign);
		EMVBase_Trace("\r\n paypass-error:PAN(5A) not exist !!!  \r\n");
        return RLT_ERR_EMV_IccDataFormat;
    }

    memset(tmppan1, 0, sizeof(tmppan1));
    EMVBaseBcdToAsc(tmppan1, recovICCPK.AppPAN, 10);
    tmppan1len = 20;

    for(testlen = 0; testlen < 20; testlen++)
    {
        if((tmppan1[testlen] == 'F') || (tmppan1[testlen] == 'f'))
        {
            tmppan1len = testlen;
            break;
        }
    }

    memset(tmppan2, 0, sizeof(tmppan2));
    EMVBaseBcdToAsc(tmppan2, item1->data, item1->len);
    tmppan2len = (item1->len) << 1;

    for(testlen = 0; testlen < tmppan2len; testlen++)
    {
        if((tmppan1[testlen] == 'F') || (tmppan2[testlen] == 'f'))
        {
            tmppan2len = testlen;
            break;
        }
    }

    if(tmppan1len != tmppan2len)
    {
        EMVBase_Trace("\r\n paypass-error:PAN len unequal !!!  \r\n");
		EMVBase_Trace("\r\n paypass-error:tmppan1len=%d,tmppan2len=%d\r\n",tmppan1len,tmppan2len);

        emvbase_free(ICCPKToSign);
        return RLT_EMV_ERR;
    }

    if(memcmp(tmppan1, tmppan2, tmppan1len))
    {
        EMVBase_Trace("\r\n paypass-error:PAN unequal !!!  \r\n");
//		EMVBase_TraceHex("\r\n paypass-error: tmppan1",tmppan1,tmppan1len);
//		EMVBase_TraceHex("\r\n paypass-error: tmppan2",tmppan2,tmppan1len);

        emvbase_free(ICCPKToSign);
        return RLT_EMV_ERR;
    }

    //verify expiredate is later than current date
    if(EMVBase_ExpireDateVerify(recovICCPK.ExpireDate) == RLT_EMV_ERR)
    {
		EMVBase_Trace("\r\n paypass-error:expiredate is later than current date, \r\n");

        emvbase_free(ICCPKToSign);
        return RLT_EMV_ERR;
    }

    if(recovICCPK.ICCPKAlgoInd != 0x01)  //other than '01' is not recognised.
    {
		EMVBase_Trace("\r\n paypass-error:ICCPKAlgoInd other than '01' is not recognised, \r\n");

        emvbase_free(ICCPKToSign);
        return RLT_EMV_ERR;
    }
    tempApp_UnionStruct->PaypassTradeParam->ICCPKModulLen = recovICCPK.ICCPKLen;

    if(tempApp_UnionStruct->PaypassTradeParam->ICCPKModul == NULL)
    {
        tempApp_UnionStruct->PaypassTradeParam->ICCPKModul = (unsigned char *)emvbase_malloc(recovICCPK.ICCPKLen);
    }

    if(tempApp_UnionStruct->PaypassTradeParam->ICCPKModul == NULL)
    {
		EMVBase_Trace("\r\n paypass-error:ICCPKModul malloc err!!, \r\n");
        return RLT_EMV_ERR;
    }

    if(recovICCPK.ICCPKLen <= (tempApp_UnionStruct->PaypassTradeParam->IPKModulLen - 42))
    {
        memcpy(tempApp_UnionStruct->PaypassTradeParam->ICCPKModul, recovICCPK.ICCPKLeft, recovICCPK.ICCPKLen);
    }
    else
    {
        memcpy(tempApp_UnionStruct->PaypassTradeParam->ICCPKModul, recovICCPK.ICCPKLeft, tempApp_UnionStruct->PaypassTradeParam->IPKModulLen - 42);
        memcpy((unsigned char *)&tempApp_UnionStruct->PaypassTradeParam->ICCPKModul[tempApp_UnionStruct->PaypassTradeParam->IPKModulLen - 42], ICCPKRem, recovICCPK.ICCPKLen - tempApp_UnionStruct->PaypassTradeParam->IPKModulLen + 42);
    }

    emvbase_free(ICCPKToSign);
    return RLT_EMV_OK;
}

unsigned char Paypass_InitDynamicAuth(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
	unsigned char ret;
    if(tempApp_UnionStruct->PaypassTradeParam->IPKModulLen == 0)
    {
		ret = Paypass_GetIPK(tempApp_UnionStruct);
		EMVBase_Trace("paypass-info: Paypass_GetIPK ret = %d\r\n", ret);
        if(ret != RLT_EMV_OK) 	 //retrieval of the issuer public key modulus
        {
			if(ret == PAYPASS_REQ_SETCALLBACK_REVOKEY)
			{
				return ret;
			}
            EMVBase_Trace("paypass-info: Retrieve the Issuer PK fails\r\n");
            return RLT_EMV_ERR;
        }
    }

    if(tempApp_UnionStruct->PaypassTradeParam->ICCPKModulLen == 0)
    {
        if(Paypass_GetICCPK(tempApp_UnionStruct) != RLT_EMV_OK)		 //retrieval of the ICC public key
        {
			EMVBase_Trace("paypass-info: Retrieve the IC Card PK fails\r\n");
            return RLT_EMV_ERR;
        }
    }

    return RLT_EMV_OK;
}

unsigned char Paypass_FillICCDynData(unsigned char *DynData, unsigned char DynDataLen, ICC_DYN_DATA *ICCDynData, PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char Ldn;


    Ldn = DynData[0];
    ICCDynData->ICCDynNumLen = Ldn;
    memcpy(ICCDynData->ICCDynNum, DynData + 1, Ldn);
    ICCDynData->CryptInfo = DynData[Ldn + 1];
    memcpy(ICCDynData->AppCrypt, DynData + Ldn + 2, 8);
    memcpy(ICCDynData->HashResult, DynData + Ldn + 10, 20);


    #ifdef PAYPASS_DATAEXCHANGE
    int bufLen = 0;
    unsigned char DataStorageVersionNumber = 0;
	unsigned char Offset = 0;
	unsigned char readinidsstatus = 0;
	unsigned char TVR[5] = {0};
	unsigned char RRPwasPerformed = 0;
	unsigned char UnpredictNum[5];
    EMVBASETAGCVLITEM *item = NULL;


    bufLen = DynDataLen - 1 - Ldn - 1 - 8 - 20;
    #ifdef EMVB_DEBUG
    EMVBase_Trace("\r\n paypass-error: dllpaypass_FillICCDynData DynDataLen=%d bufLen=%d \r\n", DynDataLen, bufLen);
    EMVBase_TraceHex("\r\n paypass-error: printFormat", DynData, DynDataLen);
    #endif
    ICCDynData->DSSummary2Len = 0;
    ICCDynData->DSSummary3Len = 0;
    memset(ICCDynData->DSSummary2, 0, sizeof(ICCDynData->DSSummary2));
    memset(ICCDynData->DSSummary3, 0, sizeof(ICCDynData->DSSummary3));
    memset(ICCDynData->TerminalRelayResistanceEntropy, 0, sizeof(ICCDynData->TerminalRelayResistanceEntropy));
    memset(ICCDynData->DeviceRelayResistanceEntropy, 0, sizeof(ICCDynData->DeviceRelayResistanceEntropy));
    memset(ICCDynData->MinTimeForPRRAPDU, 0, sizeof(ICCDynData->MinTimeForPRRAPDU));
    memset(ICCDynData->MaxTimeForPRRAPDU, 0, sizeof(ICCDynData->MaxTimeForPRRAPDU));
    memset(ICCDynData->DeviceEstimatedTransTFRRRAPDU, 0, sizeof(ICCDynData->DeviceEstimatedTransTFRRRAPDU));

    Offset = Ldn + 10 + 20;


#if SpecificationBulletinNo239//20200702_lhd del for del s910 2 s910 2.2  s910 3.1 s910 3

    if(0x80 & emvbase_avl_gettagvalue(EMVTAG_IDSStatus))//S910.2
    {
		EMVBase_Trace("paypass-info: IDS Read Flag set\r\n");
        readinidsstatus = 1;
    }
#endif

    memset(TVR, 0, sizeof(TVR));
    emvbase_avl_gettagvalue_spec(EMVTAG_TVR, TVR, 0, 5);
    if(TVR[4] & 0x02)
    {
    	EMVBase_Trace("paypass-info: RRP performed\r\n");
        RRPwasPerformed = 1;
    }

#if SpecificationBulletinNo239//20200702_lhd del for del s910 2 s910 2.2  s910 3.1 s910 3
    if(readinidsstatus)
    {
		item = emvbase_avl_gettagitempointer(EMVTAG_AppCapabilitiesInfor);
        if((item != NULL) && item->len)
        {
            DataStorageVersionNumber = item->data[0];
            EMVBase_Trace("paypass-info: Data Storage VersionNumber: %02x\r\n", DataStorageVersionNumber);
        }
        if(RRPwasPerformed)
        {
            //s910.3.1
            if(0x01 == (DataStorageVersionNumber & 0x0F))
            {
            	EMVBase_Trace("paypass-info: VERSION 1\r\n");

                if(DynDataLen < (60 + DynData[0]))
                {
					EMVBase_Trace("paypass-info: len of ICC Dynamic Data < 60 + Len of ICC Dynamic Number bytes\r\n");
                    return RLT_EMV_ERR;
                }
                if(bufLen >= 8)
                {
                    ICCDynData->DSSummary2Len = 8;
                    memcpy(ICCDynData->DSSummary2, DynData + Offset, ICCDynData->DSSummary2Len);
                    Offset += ICCDynData->DSSummary2Len;
                    emvbase_avl_createsettagvalue(EMVTAG_DSSummary2, ICCDynData->DSSummary2, ICCDynData->DSSummary2Len);
                }
                if(bufLen >= 16)
                {
                    ICCDynData->DSSummary3Len = 8;
                    memcpy(ICCDynData->DSSummary3, DynData + Offset, ICCDynData->DSSummary3Len);
                    Offset += ICCDynData->DSSummary3Len;
                    emvbase_avl_createsettagvalue(EMVTAG_DSSummary3, ICCDynData->DSSummary3, ICCDynData->DSSummary3Len);
                }
            }
            else if(0x02 ==	(DataStorageVersionNumber & 0x0F))
            {
            	EMVBase_Trace("paypass-info: VERSION 2\r\n");

                if(DynDataLen < (76 + DynData[0]))
                {
					EMVBase_Trace("paypass-info: len of ICC Dynamic Data < 76 + Len of ICC Dynamic Number bytes\r\n");
                    return RLT_EMV_ERR;
                }
                if(bufLen >= 16)
                {
                    ICCDynData->DSSummary2Len = 16;
                    memcpy(ICCDynData->DSSummary2, DynData + Offset, ICCDynData->DSSummary2Len);
                    Offset += ICCDynData->DSSummary2Len;
                    emvbase_avl_createsettagvalue(EMVTAG_DSSummary2, ICCDynData->DSSummary2, ICCDynData->DSSummary2Len);
                }
                if(bufLen >= 32)
                {
                    ICCDynData->DSSummary3Len = 16;
                    memcpy(ICCDynData->DSSummary3, DynData + Offset, ICCDynData->DSSummary3Len);
                    Offset += ICCDynData->DSSummary3Len;
                    emvbase_avl_createsettagvalue(EMVTAG_DSSummary3, ICCDynData->DSSummary3, ICCDynData->DSSummary3Len);
                }
            }

            if((bufLen - ICCDynData->DSSummary2Len - ICCDynData->DSSummary3Len) >= 14)
            {
                memcpy(ICCDynData->TerminalRelayResistanceEntropy, DynData + Offset, 4);
                Offset += 4;
                memcpy(ICCDynData->DeviceRelayResistanceEntropy, DynData + Offset, 4);
                Offset += 4;
                memcpy(ICCDynData->MinTimeForPRRAPDU, DynData + Offset, 2);
                Offset += 2;
                memcpy(ICCDynData->MaxTimeForPRRAPDU, DynData + Offset, 2);
                Offset += 2;
                memcpy(ICCDynData->DeviceEstimatedTransTFRRRAPDU, DynData + Offset, 2);
                Offset += 2;
            }

            memset(UnpredictNum, 0, sizeof(UnpredictNum));
            emvbase_avl_gettagvalue_spec(EMVTAG_TermRREntropy, UnpredictNum, 0, 4);
            if(0 != memcmp(UnpredictNum, ICCDynData->TerminalRelayResistanceEntropy, 4))
            {
				EMVBase_Trace("paypass-info: now in SpecificationBulletinNo239\r\n");
				EMVBase_TraceHex("paypass-info: Terminal Relay Resistance Entropy(RRP): ", UnpredictNum, 4);
				EMVBase_TraceHex("paypass-info: Terminal Relay Resistance Entropy(CDA): ", ICCDynData->TerminalRelayResistanceEntropy, 4);
				EMVBase_Trace("paypass-info: different->error\r\n");
                return RLT_EMV_ERR;
            }

            memset(UnpredictNum, 0, sizeof(UnpredictNum));
            emvbase_avl_gettagvalue_spec(EMVTAG_DeviceRREntropy, UnpredictNum, 0, 4);
            if(0 != memcmp(UnpredictNum, ICCDynData->DeviceRelayResistanceEntropy, 4))
            {
            	EMVBase_TraceHex("paypass-info: Device Relay Resistance Entropy(RRP): ", UnpredictNum, 4);
				EMVBase_TraceHex("paypass-info: Device Relay Resistance Entropy(CDA): ", ICCDynData->DeviceRelayResistanceEntropy, 4);
				EMVBase_Trace("paypass-info: different->error\r\n");
                return RLT_EMV_ERR;
            }

            memset(UnpredictNum, 0, sizeof(UnpredictNum));
            emvbase_avl_gettagvalue_spec(EMVTAG_MinTimeForProcessingRR_APDU, UnpredictNum, 0, 2);
            if(0 != memcmp(UnpredictNum, ICCDynData->MinTimeForPRRAPDU, 2))
            {
            	//EMVBase_TraceHex("paypass-info: Min Time For Processing Relay Resistance APDU(RRP): ", UnpredictNum, 2);
				//EMVBase_TraceHex("paypass-info: Min Time For Processing Relay Resistance APDU(CDA): ", ICCDynData->MinTimeForPRRAPDU, 2);
				//EMVBase_Trace("paypass-info: different->error\r\n");
                return RLT_EMV_ERR;
            }

            memset(UnpredictNum, 0, sizeof(UnpredictNum));
            emvbase_avl_gettagvalue_spec(EMVTAG_MaxTimeForProcessingRR_APDU, UnpredictNum, 0, 2);
            if(0 != memcmp(UnpredictNum, ICCDynData->MaxTimeForPRRAPDU, 2))
            {
            	EMVBase_TraceHex("paypass-info: Max Time For Processing Relay Resistance APDU(RRP): ", UnpredictNum, 2);
				EMVBase_TraceHex("paypass-info: Max Time For Processing Relay Resistance APDU(CDA): ", ICCDynData->MaxTimeForPRRAPDU, 2);
				EMVBase_Trace("paypass-info: different->error\r\n");
                return RLT_EMV_ERR;
            }

            memset(UnpredictNum, 0, sizeof(UnpredictNum));
            emvbase_avl_gettagvalue_spec(EMVTAG_DeviceEstimatedTransmissionTimeForRR_RAPDU, UnpredictNum, 0, 2);
            if(0 != memcmp(UnpredictNum, ICCDynData->DeviceEstimatedTransTFRRRAPDU, 2))
            {
            	EMVBase_TraceHex("paypass-info: Device Estimated Transmission Time For Relay Resistance RAPDU(RRP): ", UnpredictNum, 2);
				EMVBase_TraceHex("paypass-info: Device Estimated Transmission Time For Relay Resistance RAPDU(CDA): ", ICCDynData->DeviceEstimatedTransTFRRRAPDU, 2);
				EMVBase_Trace("paypass-info: different->error\r\n");
                return RLT_EMV_ERR;
            }
        }
        else
        {
            //s910.3
            if(DynDataLen < (30 + DynData[0]))
            {
				EMVBase_Trace("paypass-info: len of ICC Dynamic Data < 30 + Len of ICC Dynamic Number bytes\r\n");
                return RLT_EMV_ERR;
            }

            if(0x01 == (DataStorageVersionNumber & 0x0F))
            {
            	EMVBase_Trace("paypass-info: VERSION 1\r\n");

                if(bufLen >= 8)
                {
                    ICCDynData->DSSummary2Len = 8;
                    memcpy(ICCDynData->DSSummary2, DynData + Offset, ICCDynData->DSSummary2Len);
                    Offset += ICCDynData->DSSummary2Len;
                    emvbase_avl_createsettagvalue(EMVTAG_DSSummary2, ICCDynData->DSSummary2, ICCDynData->DSSummary2Len);
                }
                if(bufLen >= 16)
                {
                    ICCDynData->DSSummary3Len = 8;
                    memcpy(ICCDynData->DSSummary3, DynData + Offset, ICCDynData->DSSummary3Len);
                    Offset += ICCDynData->DSSummary3Len;
                    emvbase_avl_createsettagvalue(EMVTAG_DSSummary3, ICCDynData->DSSummary3, ICCDynData->DSSummary3Len);
                }
            }
            else if(0x02 == (DataStorageVersionNumber & 0x0F))
            {
            	EMVBase_Trace("paypass-info: VERSION 2\r\n");

                if(bufLen >= 16)
                {
                    ICCDynData->DSSummary2Len = 16;
                    memcpy(ICCDynData->DSSummary2, DynData + Offset, ICCDynData->DSSummary2Len);
                    Offset += ICCDynData->DSSummary2Len;
                    emvbase_avl_createsettagvalue(EMVTAG_DSSummary2, ICCDynData->DSSummary2, ICCDynData->DSSummary2Len);
                }
                if(bufLen >= 32)
                {
                    ICCDynData->DSSummary3Len = 16;
                    memcpy(ICCDynData->DSSummary3, DynData + Offset, ICCDynData->DSSummary3Len);
                    Offset += ICCDynData->DSSummary3Len;
                    emvbase_avl_createsettagvalue(EMVTAG_DSSummary3, ICCDynData->DSSummary3, ICCDynData->DSSummary3Len);
                }
            }
        }
    }
    else
#endif
    {
        if(RRPwasPerformed)
        {
            //s910.4.1
            if(DynDataLen < (44 + DynData[0]))
            {
				EMVBase_Trace("paypass-info: len of ICC Dynamic Data < 44 + Len of ICC Dynamic Number bytes\r\n");
                return RLT_EMV_ERR;
            }

            memcpy(ICCDynData->TerminalRelayResistanceEntropy, DynData + Offset, 4);
            Offset += 4;
            memcpy(ICCDynData->DeviceRelayResistanceEntropy, DynData + Offset, 4);
            Offset += 4;
            memcpy(ICCDynData->MinTimeForPRRAPDU, DynData + Offset, 2);
            Offset += 2;
            memcpy(ICCDynData->MaxTimeForPRRAPDU, DynData + Offset, 2);
            Offset += 2;
            memcpy(ICCDynData->DeviceEstimatedTransTFRRRAPDU, DynData + Offset, 2);
            Offset += 2;

            #if 1
            memset(UnpredictNum, 0, sizeof(UnpredictNum));
            emvbase_avl_gettagvalue_spec(EMVTAG_TermRREntropy, UnpredictNum, 0, 4);
            if(0 != memcmp(UnpredictNum, ICCDynData->TerminalRelayResistanceEntropy, 4))
            {
            	EMVBase_TraceHex("paypass-info: Terminal Relay Resistance Entropy(RRP): ", UnpredictNum, 4);
				EMVBase_TraceHex("paypass-info: Terminal Relay Resistance Entropy(CDA): ", ICCDynData->TerminalRelayResistanceEntropy, 4);
				EMVBase_Trace("paypass-info: different->error\r\n");
                return RLT_EMV_ERR;
            }

            memset(UnpredictNum, 0, sizeof(UnpredictNum));
            emvbase_avl_gettagvalue_spec(EMVTAG_DeviceRREntropy, UnpredictNum, 0, 4);
            if(0 != memcmp(UnpredictNum, ICCDynData->DeviceRelayResistanceEntropy, 4))
            {
            	EMVBase_TraceHex("paypass-info: Device Relay Resistance Entropy(RRP): ", UnpredictNum, 4);
				EMVBase_TraceHex("paypass-info: Device Relay Resistance Entropy(CDA): ", ICCDynData->DeviceRelayResistanceEntropy, 4);
				EMVBase_Trace("paypass-info: different->error\r\n");
                return RLT_EMV_ERR;
            }

            memset(UnpredictNum, 0, sizeof(UnpredictNum));
            emvbase_avl_gettagvalue_spec(EMVTAG_MinTimeForProcessingRR_APDU, UnpredictNum, 0, 2);
            if(0 != memcmp(UnpredictNum, ICCDynData->MinTimeForPRRAPDU, 2))
            {
            	EMVBase_TraceHex("paypass-info: Min Time For Processing Relay Resistance APDU(RRP): ", UnpredictNum, 2);
				//EMVBase_TraceHex("paypass-info: Min Time For Processing Relay Resistance APDU(CDA): ", ICCDynData->MinTimeForPRRAPDU, 2);
				//EMVBase_Trace("paypass-info: different->error\r\n");
                return RLT_EMV_ERR;
            }

            memset(UnpredictNum, 0, sizeof(UnpredictNum));
            emvbase_avl_gettagvalue_spec(EMVTAG_MaxTimeForProcessingRR_APDU, UnpredictNum, 0, 2);
            if(0 != memcmp(UnpredictNum, ICCDynData->MaxTimeForPRRAPDU, 2))
            {
            	EMVBase_TraceHex("paypass-info: Max Time For Processing Relay Resistance APDU(RRP): ", UnpredictNum, 2);
				EMVBase_TraceHex("paypass-info: Max Time For Processing Relay Resistance APDU(CDA): ", ICCDynData->MaxTimeForPRRAPDU, 2);
				EMVBase_Trace("paypass-info: different->error\r\n");
                return RLT_EMV_ERR;
            }

            memset(UnpredictNum, 0, sizeof(UnpredictNum));
            emvbase_avl_gettagvalue_spec(EMVTAG_DeviceEstimatedTransmissionTimeForRR_RAPDU, UnpredictNum, 0, 2);
            if(0 != memcmp(UnpredictNum, ICCDynData->DeviceEstimatedTransTFRRRAPDU, 2))
            {
            	EMVBase_TraceHex("paypass-info: Device Estimated Transmission Time For Relay Resistance RAPDU(RRP): ", UnpredictNum, 2);
				EMVBase_TraceHex("paypass-info: Device Estimated Transmission Time For Relay Resistance RAPDU(CDA): ", ICCDynData->DeviceEstimatedTransTFRRRAPDU, 2);
				EMVBase_Trace("paypass-info: different->error\r\n");
                return RLT_EMV_ERR;
            }
            #endif
        }
        else
        {
            //s910.4  old transflow
            if(DynDataLen < (30 + DynData[0]))
            {
				EMVBase_Trace("paypass-info: len of ICC Dynamic Data < 30 + Len of ICC Dynamic Number bytes\r\n");
                return RLT_EMV_ERR;
            }
        }

        //S910.6 GOTO e

    }
    #endif

    return RLT_EMV_OK;
}

unsigned char Paypass_CombineDDASignVerify(unsigned char type, PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char *SDAToSign, SDAHash[20], *SDAData; //[512]    [248]
    unsigned int index;
    EMVBASE_SIGN_DYNDATA_RECOVER *recovSDA;
    ICC_DYN_DATA ICCDynDataSign;
    EMVBASETAGCVLITEM *SignDynAppDataitem, *ICCPKExpitem, *DOLDataitem;
    unsigned char CryptInfo, ICCDynDataResult = 0;
    unsigned char UnpredictNum[4];
	unsigned short offset = 0;


	EMVBase_Trace("paypass-info: Signed Dynamic Application Data(Tag9F4B) verification\r\n");

	#if 0 // ����������
    if((emvbase_avl_checkiftagexist(EMVTAG_CryptInfo) == 0) || (emvbase_avl_checkiftagexist(EMVTAG_SignDynAppData) == 0)
            || (emvbase_avl_checkiftagexist(EMVTAG_ATC) == 0))
    {
        if(emvbase_avl_checkiftagexist(EMVTAG_CryptInfo) == 0)
		{
			EMVBase_Trace("\r\n paypass-error:CryptInfo(9F27) does not exist!\r\n");
		}
		if(emvbase_avl_checkiftagexist(EMVTAG_SignDynAppData) == 0)
		{
			EMVBase_Trace("\r\n paypass-error:SignDynAppData(9F4B) does not exist!\r\n");
		}
		if(emvbase_avl_checkiftagexist(EMVTAG_ATC) == 0)
		{
			EMVBase_Trace("\r\n paypass-error:ATC(9F36) does not exist!\r\n");
		}
		EMVBase_Trace("\r\n paypass-error: data authentication has failed\r\n");
        return PAYPASS_ERR_EMV_CDAFORMAT;
    }
	#endif

    memset((unsigned char *)&ICCDynDataSign, 0, sizeof(ICC_DYN_DATA));
    memset(SDAHash, 0, 20);

    SignDynAppDataitem = emvbase_avl_gettagitempointer(EMVTAG_SignDynAppData);
    if(SignDynAppDataitem->len != tempApp_UnionStruct->PaypassTradeParam->ICCPKModulLen)
    {
    	EMVBase_Trace("paypass-info: ICC public key len: %d\r\n", tempApp_UnionStruct->PaypassTradeParam->ICCPKModulLen);
		EMVBase_Trace("paypass-info: Signed Dynamic Application Data(Tag9F4B) value len: %d\r\n", SignDynAppDataitem->len);
		EMVBase_Trace("paypass-info: different len->error\r\n");
        return RLT_EMV_ERR;
    }

    ICCPKExpitem = emvbase_avl_gettagitempointer(EMVTAG_ICCPKExp);

    SDAData = (unsigned char *)emvbase_malloc(248);
    EMVBase_RSARecover(tempApp_UnionStruct->PaypassTradeParam->ICCPKModul, tempApp_UnionStruct->PaypassTradeParam->ICCPKModulLen, ICCPKExpitem->data, ICCPKExpitem->len, SignDynAppDataitem->data, SDAData);

    recovSDA = (EMVBASE_SIGN_DYNDATA_RECOVER *)emvbase_malloc(sizeof(EMVBASE_SIGN_DYNDATA_RECOVER));

    if(EMVBase_FillSDA(SDAData, recovSDA, tempApp_UnionStruct->PaypassTradeParam->ICCPKModulLen) != RLT_EMV_OK)
    {
    	EMVBase_Trace("paypass-info: Format of Data Recovered error\r\n");
        emvbase_free(recovSDA);
        emvbase_free(SDAData);
        return RLT_EMV_ERR;
    }

    if(recovSDA->DataTrail != 0xBC)
    {
		EMVBase_Trace("paypass-info: Data Trailer != 0xBC\r\n");
        emvbase_free(recovSDA);
        emvbase_free(SDAData);
        return RLT_EMV_ERR;
    }

    if(recovSDA->DataHead != 0x6A)
    {
    	EMVBase_Trace("paypass-info: Data Header != 0x6A\r\n");

        emvbase_free(recovSDA);
        emvbase_free(SDAData);
        return RLT_EMV_ERR;
    }

    if(recovSDA->DataFormat != 0x05)
    {
		EMVBase_Trace("paypass-info: Certificate Format != 0x05\r\n");
        emvbase_free(recovSDA);
        emvbase_free(SDAData);
        return RLT_EMV_ERR;
    }

    if(RLT_EMV_OK != Paypass_FillICCDynData(recovSDA->ICCDynData, recovSDA->ICCDynDataLen, &ICCDynDataSign, tempApp_UnionStruct))
    {
        emvbase_free(recovSDA);
        emvbase_free(SDAData);
        return RLT_EMV_ERR;
    }

    CryptInfo = emvbase_avl_gettagvalue(EMVTAG_CryptInfo);

    if(ICCDynDataSign.CryptInfo != CryptInfo)	   //sxl v2cc127000 这里需要看最新的文档
    {
    	EMVBase_Trace("paypass-info: CID of ICC Dynamic Data != CID(Tag9F27)\r\n");

        if((ICCDynDataSign.CryptInfo & 0xC0) == 0x80)
        {
            emvbase_free(recovSDA);
            emvbase_free(SDAData);
            return PAYPASS_ERR_EMV_CDADifferCID;
        }
        else if((ICCDynDataSign.CryptInfo & 0xC0) == 0x40)
        {
            emvbase_free(recovSDA);
            emvbase_free(SDAData);
            return PAYPASS_ERR_EMV_CDADifferCIDTC;//
        }
        else
        {
            emvbase_free(recovSDA);
            emvbase_free(SDAData);
            return RLT_EMV_ERR;
        }
    }

    SDAToSign = (unsigned char *)emvbase_malloc(1024);
    index = 0;
    memset(SDAToSign, 0, 1024);

    if ( ( tempApp_UnionStruct->PaypassTradeParam->ICCPKModulLen - 22 ) > 0 )
    {
        memcpy(SDAToSign, (unsigned char *)&SDAData[1], tempApp_UnionStruct->PaypassTradeParam->ICCPKModulLen - 22);
        index += (tempApp_UnionStruct->PaypassTradeParam->ICCPKModulLen - 22);
    }

    emvbase_free(SDAData);


    if(0 == tempApp_UnionStruct->PaypassTradeParam->bCDOL1HasNoUnpredictNum)
    {
        emvbase_avl_gettagvalue_spec(EMVTAG_UnpredictNum, UnpredictNum, 0, 4);
        memcpy((unsigned char *)&SDAToSign[index], UnpredictNum, 4);
        index += 4;
    }

    if(recovSDA->HashInd != 0x01)  //SHA-1 algorithm
    {
    	EMVBase_Trace("paypass-info: Hash Algorithm Indicator != 0x01\r\n");
        emvbase_free(recovSDA);
        emvbase_free(SDAToSign);

        return RLT_EMV_ERR;
    }

    EMVBase_Hash(SDAToSign, index, SDAHash);
    if(memcmp(recovSDA->HashResult, SDAHash, 20))
    {
		EMVBase_TraceHex("paypass-info: Hash Result of Recover SIGN DYN DATA: " ,recovSDA->HashResult ,20);
		EMVBase_TraceHex("paypass-info: Hash Result Computed: ", SDAHash, 20);
		EMVBase_Trace("paypass-info: Hash Result verification fails\r\n");

        emvbase_free(recovSDA);
        emvbase_free(SDAToSign);

        return RLT_EMV_ERR;
    }
    emvbase_free(recovSDA);

    memset(SDAToSign, 0, 1024);
    memset(SDAHash, 0, 20);
    index = 0;

    DOLDataitem = emvbase_avl_gettagitempointer(EMVTAG_PDOLData);
    if(DOLDataitem != NULL)
    {
        if(DOLDataitem->len)
        {
            memcpy(SDAToSign, DOLDataitem->data, DOLDataitem->len);
            index += DOLDataitem->len;
        }
    }

    DOLDataitem = emvbase_avl_gettagitempointer(EMVTAG_CDOL1RelatedData);
    if(DOLDataitem != NULL)
    {
        if(DOLDataitem->len)
        {
        	if(tempApp_UnionStruct->PaypassTradeParam->bTrack2InCDOL1Data)
			{
				offset = index + tempApp_UnionStruct->PaypassTradeParam->Track2OffsetInCDOL1Data;
			}

            memcpy(SDAToSign + index, DOLDataitem->data, DOLDataitem->len);
            index += DOLDataitem->len;
        }
    }


    memcpy(SDAToSign + index, tempApp_UnionStruct->PaypassTradeParam->GenerateACRetData, tempApp_UnionStruct->PaypassTradeParam->GenerateACRetDataLen);
    index += tempApp_UnionStruct->PaypassTradeParam->GenerateACRetDataLen;

    EMVBase_Hash(SDAToSign, index, SDAHash);

    emvbase_free(SDAToSign);

    if(memcmp(ICCDynDataSign.HashResult, SDAHash, 20))
    {
		EMVBase_TraceHex("paypass-info: Hash Result of ICC Dynamic Data", ICCDynDataSign.HashResult, 20);
		EMVBase_TraceHex("paypass-info: Hash Result Computed",SDAHash,20);
		EMVBase_Trace("paypass-info: ICC Dynamic Data Hash Result verification fails\r\n");

        return RLT_EMV_ERR;
    }

    emvbase_avl_createsettagvalue(EMVTAG_ICCDynNum, ICCDynDataSign.ICCDynNum, ICCDynDataSign.ICCDynNumLen);
    emvbase_avl_createsettagvalue(EMVTAG_AppCrypt, ICCDynDataSign.AppCrypt, 8);

    #if 1 /*Modify by luohuidong at 2016.10.01  12:17 */  //FOR CASE 1.	3MX6-7049 2.	3MX6-7079
    if(0x80 & emvbase_avl_gettagvalue(EMVTAG_IDSStatus))//S910.2
    {
		unsigned char DSSummary1[16] = {0};
		unsigned char DSSummary1Len = 0;
		unsigned char bDSSummary2Exsit = 0, bDSSummary3Exsit = 0;
        EMVBASETAGCVLITEM *item = NULL, *item2 = NULL, *item3 = NULL;


		DSSummary1Len = 0;
        memset(DSSummary1, 0, sizeof(DSSummary1));
        item = emvbase_avl_gettagitempointer(EMVTAG_DSSummary1);
        if(item != NULL)
        {
            if(item->len)
            {
                if(item->len > 16)
                {
                    DSSummary1Len = 16;
                }
                else
                {
                    DSSummary1Len = item->len;
                }
                memcpy(DSSummary1, item->data, DSSummary1Len);

                EMVBase_TraceHex("paypass-info: DS Summary1(Tag9F7D): ", item->data, DSSummary1Len);
            }
        }

        //s11.45 yes
        if(tempApp_UnionStruct->ucPaypassintornlog)
        {
            TornTransactionLogIDSPartRecord ptornlogrecord;
            unsigned char TronFuncRet = 0;

            memset(&ptornlogrecord, 0, sizeof(ptornlogrecord));
            if(NULL != tempApp_UnionStruct->gettornlogtempdata)
            {
                TronFuncRet = tempApp_UnionStruct->gettornlogtempdata(&ptornlogrecord);
            }

            if(ptornlogrecord.IDSStatus & 0x40)
            {
                if((ptornlogrecord.DSSummary1Len == DSSummary1Len) &&
                        (0 == memcmp(ptornlogrecord.DSSummary1, DSSummary1, DSSummary1Len)))
                {
                    ;
                }
                else
                {
                    EMVBase_TraceHex("paypass-info: DS Summary1 in Torn: ", ptornlogrecord.DSSummary1, ptornlogrecord.DSSummary1Len);
					EMVBase_Trace("paypass-error: different->error\r\n");
                    return PAYPASS_L2_IDS_READ_ERROR;
                }
            }
        }

        //s910.5 yes
        if(0 == ICCDynDataSign.DSSummary2Len)
        {
            EMVBase_Trace("paypass-error: DS Summary2 Missing\r\n");
            //s910.9
            return RLT_ERR_EMV_IccDataMissing;
        }


        if(DSSummary1Len)
        {
            if((DSSummary1Len == ICCDynDataSign.DSSummary2Len) && (0 == memcmp(DSSummary1, ICCDynDataSign.DSSummary2, ICCDynDataSign.DSSummary2Len)))
            {
                //s910.12
			//DEK support IDS, modified by	sjz20220815
//                if(0 == tempApp_UnionStruct->PaypassTradeParam->bretforminit) // removed for SB261 but retained in production
				{
                	emvbase_avl_setvalue_or(EMVTAG_DSSummaryStatus, 0, 0x80);
                }
                EMVBase_Trace("paypass-info: DS Summary1(Tag9F7D) == DS Summary2\r\n");
            }
            else
            {
                EMVBase_Trace("paypass-info: DS Summary1(Tag9F7D) != DS Summary2\r\n");
				return PAYPASS_L2_IDS_READ_ERROR;
            }
        }
        else
        {
			EMVBase_Trace("paypass-info: DS Summary1(Tag9F7D) Missing\r\n");
			return PAYPASS_L2_IDS_READ_ERROR;
        }

        //s910.ed13
        if(0x40 & emvbase_avl_gettagvalue(EMVTAG_IDSStatus)) //s910.ed14
        {
			EMVBase_Trace("paypass-info: IDS Write Flag set\r\n");

            bDSSummary3Exsit = 0;
            item3 = emvbase_avl_gettagitempointer(EMVTAG_DSSummary3);
            if(item3 != NULL)
            {
                if(item3->len)
                {
                    bDSSummary3Exsit = 1;
                    EMVBase_TraceHex("paypass-info: DS Summary3: ", item3->data, item3->len);
                }
            }

            bDSSummary2Exsit = 0;
            item2 = emvbase_avl_gettagitempointer(EMVTAG_DSSummary2);
            if(item2 != NULL)
            {
                if(item2->len)
                {
                    bDSSummary2Exsit = 1;
                    EMVBase_TraceHex("paypass-info: DS Summary2: ", item2->data, item2->len);
                }
            }

            if((bDSSummary3Exsit) && (bDSSummary2Exsit))
            {
                //s910.ed16
                if(0 == memcmp(item3->data, item2->data, item3->len))
                {
					EMVBase_Trace("paypass-info: DS Summary3 == DS Summary2\r\n");

                    //s910.ed18
                    if(0x02 & emvbase_avl_gettagvalue(EMVTAG_DSODSInfoReader))
                    {
                        //s910.ed19
                        EMVBase_Trace("paypass-error: 'Stop if write failed'(bit2) in DS Info for Reader is set\r\n");
                        return PAYPASS_L2_IDS_WRITE_ERROR;
                    }
                }
				else
                {
                    //S910.ED17
                    emvbase_avl_setvalue_or(EMVTAG_DSSummaryStatus, 0, 0x40);
                    EMVBase_Trace("\r\n paypass-info: S910.ED12  SET 'Successful Write'' in DS Summary Status \r\n");
                    //goto E
                    //return PAYPASS_CDA_ERROR_RET_FOR_E;
                }
            }
            else
            {
                //s910.ed15
				EMVBase_Trace("paypass-info: DS Summary3 Missing\r\n");
                return RLT_ERR_EMV_IccDataMissing;
            }
        }
    }
    #endif /* if 0 */

    return RLT_EMV_OK;
}

unsigned char Paypass_CombineDDA(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char retCode = 0;

    //S910.E1
    EMVBase_Trace("paypass-info: CDA\r\n");

	retCode = Paypass_InitDynamicAuth(tempApp_UnionStruct);
    if(retCode != RLT_EMV_OK)
    {
		if(retCode == PAYPASS_REQ_SETCALLBACK_REVOKEY)
		{
			return retCode;
		}
        emvbase_avl_setvalue_or(EMVTAG_TVR, 0, 0x04);
        return RLT_EMV_OK;
    }

    retCode = Paypass_CombineDDASignVerify(GenerateAC1, tempApp_UnionStruct);
    if(retCode != RLT_EMV_OK)
    {
        if( (RLT_ERR_EMV_IccDataMissing == retCode) ||
                (PAYPASS_L2_IDS_WRITE_ERROR == retCode) ||
                (PAYPASS_L2_IDS_READ_ERROR == retCode) )
        {
            return retCode;
        }

        emvbase_avl_setvalue_or(EMVTAG_TVR, 0, 0x04);
        return RLT_EMV_OK;
    }

    return RLT_EMV_OK;
}

unsigned char Paypass_itoa(int n, char ascii[])
{
    int i, j, k;
    unsigned char as[64];
    memset(as, 0, sizeof(as));
    i = 0;
    do
    {
        as[i++] = n % 10 + '0';
    }
    while ((n /= 10) > 0);

    k = 0;
    for(j = i - 1; j >= 0; j--)
    {
        ascii[k++] = as[j];
    }

    return k;
}

unsigned char Paypass_TransProcess1(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char retCode = RLT_EMV_OK;
    unsigned char CryptInfo;
    unsigned char TVR[5];
    unsigned char ReferenceControlParameter;
	unsigned char POSCII[3] = {0};
	PAYPASS_PHONEMESSAGETABLEINFO *tempphonemsgtbl = NULL;
	unsigned short FormFactIndicatorlen = 0;
    unsigned char TermCapab[3], FormFactIndicator[32];

    unsigned char i = 0, tempbuflen = 0, *tempbuf = NULL, *tempbuf1 = NULL, RRP_counter = 0, UnpredictNum[5], asciibuf[5], ascii[12], asciilen = 0, ifFexist = 0;
    EMVBASETAGCVLITEM *item;
    int drre = 0;
    int asciioffer = 0, k, applicationPANLen = 0;
    unsigned short MeasuredRelayResistanceProcessingTime = 0;



	#if 0 //�������Ƕ���ģ�׼��ɾ��
    CryptInfo = emvbase_avl_gettagvalue(EMVTAG_CryptInfo);
    EMVBase_Trace("\r\n paypass-info:CryptInfo(9F27)=%02x,\r\n", CryptInfo);
    if((CryptInfo & 0xC0) == 0x00) //ICC return AAC,refuse the transaction.
    {
        tempApp_UnionStruct->PaypassTradeParam->TransResult = RLT_EMV_OFFLINE_DECLINE;
    }
    else if( (CryptInfo & 0xC0) == 0x40) //ICC return TC,approve the trans executed offline //lechan 090714
    {
        tempApp_UnionStruct->PaypassTradeParam->TransResult = RLT_EMV_OFFLINE_APPROVE;
    }
    else if( (CryptInfo & 0xC0) == 0x80) //ICC return TC,approve the trans executed offline //lechan 090714
    {
        tempApp_UnionStruct->PaypassTradeParam->TransResult = RLT_EMV_ONLINE_WAIT;
    }
    else
    {

        #if 1//20160905_lhd add for case 3MX6-9000
        if(tempApp_UnionStruct->ucPaypassintornlog)
        {
            if (tempApp_UnionStruct->freetornlogtempdata)
            {
                tempApp_UnionStruct->freetornlogtempdata();//no cda 后面用不到先删除 以免错误的时�?输出 tron for case 3MX6 7099
            }
            else
            {
                EMVBase_Trace("\r\n paypass-error: b tempApp_UnionStruct->freetornlogtempdata == null ! err ! ");
            }
        }
        #endif
        return RLT_EMV_ERR;
    }
	#endif


	#ifdef PAYPASS_DATAEXCHANGE	//20160824_lhd add for case 3MX4-5900
    if(emvbase_avl_checkiftagexist(EMVTAG_TagsToWriteAfterGAC))
    {
        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S9_E27_0);
    }
	else
	{
        #ifdef EMVB_DEBUG
        EMVBase_Trace("\r\n paypass-error: S10.ED20 no \r\n");
        EMVBase_Trace("\r\n paypass-error: S11.ED23 NO \r\n");
        #endif

        if (tempApp_UnionStruct->displayRemoveCard)
        {
            tempApp_UnionStruct->displayRemoveCard();
        }

        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S9_E27_0);
    }
	#else
    if (tempApp_UnionStruct->displayRemoveCard)
    {
        tempApp_UnionStruct->displayRemoveCard();
    }

    dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S9_E27_0);
    #endif

    ReferenceControlParameter = emvbase_avl_gettagvalue(EMVTAG_ReferenceControlParameter);

    if(emvbase_avl_checkiftagexist(EMVTAG_SignDynAppData) && (ReferenceControlParameter & 0x10)) //check if do CDA faile
    {
    	retCode = Paypass_CombineDDA(tempApp_UnionStruct);
		if(retCode == PAYPASS_REQ_SETCALLBACK_REVOKEY)
		{
			return retCode;
		}
        memset(TVR, 0, sizeof(TVR));
        emvbase_avl_gettagvalue_spec(EMVTAG_TVR, TVR, 0, 5);
		EMVBase_Trace("Paypass_CombineDDA ret = %d, tvr[0]=%02x\r\n", retCode, TVR[0]);
        if( (TVR[0] & 0x04) ||
                (RLT_ERR_EMV_IccDataMissing == retCode) ||
                (PAYPASS_L2_IDS_WRITE_ERROR == retCode) ||
                (PAYPASS_L2_IDS_READ_ERROR == retCode) )
        {
            if(tempApp_UnionStruct->ucPaypassintornlog)
            {
                TornTransactionLogIDSPartRecord ptornlogrecord;
                unsigned char TronFuncRet = 0;


                memset(&ptornlogrecord, 0, sizeof(ptornlogrecord));

                if(NULL != tempApp_UnionStruct->gettornlogtempdata)
                {
                    TronFuncRet = tempApp_UnionStruct->gettornlogtempdata(&ptornlogrecord);
                }

                if(0 == (ptornlogrecord.IDSStatus & 0x40))//s11.91
                {
                    if(NULL != tempApp_UnionStruct->freetornlogtempdata)
                    {
                        tempApp_UnionStruct->freetornlogtempdata();//no cda 后面用不到先删除 以免错误的时�?输出 tron for case 3MX6 7099
                    }
                }
            }

            //Invalid Response
            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S11_E115_2); //20160707_lhd add for 3M25 7305 01

            if(PAYPASS_L2_IDS_READ_ERROR == retCode)
            {
                dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S910_ED11_0);
            }
            else if(RLT_ERR_EMV_IccDataMissing == retCode)
            {
                dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S13_14_4);
            }
            else if(PAYPASS_L2_IDS_WRITE_ERROR == retCode)
            {
                dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S910_ED19_0);
                dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S11_E90_0);
                dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S11_E95_0);
                return RLT_ERR_EMV_OTHERAPCARD;
            }
            else
            {
                dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S11_E46_0);
            }

            //s910.e50
            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S11_E90_0);

			#if SpecificationBulletinNo239//20200702_lhd
            if(0x40 & emvbase_avl_gettagvalue(EMVTAG_IDSStatus))
            {
                //s910.52
                dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S910_ED52_0);
            }
            else//s910.53
            #endif
            {
                dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S910_ED53_0);
            }

            return RLT_ERR_EMV_OTHERAPCARD;
        }

        #if 1 /*Modify by luohuidong at 2016.10.01  12:47 ADD */
        if(tempApp_UnionStruct->ucPaypassintornlog)
        {
            if( NULL != tempApp_UnionStruct->freetornlogtempdata)
            {
                tempApp_UnionStruct->freetornlogtempdata();//no cda 后面用不到先删除 以免错误的时�?输出 tron for case 3MX6 7099
            }
        }
        #endif /* if 1 */
    }
    #ifdef PAYPASS_RRP
    else
    {
        #if 1 /*Modify by luohuidong at 2016.10.01  12:47 */
        if(tempApp_UnionStruct->ucPaypassintornlog)
        {
            if( NULL != tempApp_UnionStruct->freetornlogtempdata)
            {
                tempApp_UnionStruct->freetornlogtempdata();//no cda 后面用不到先删除 以免错误的时�?输出 tron for case 3MX6 7099
            }
        }
        #endif /* if 1 */

        //S910.E30
        if(emvbase_avl_checkiftagexist(EMVTAG_AppCrypt))
        {
            //S910.E32
            CryptInfo = emvbase_avl_gettagvalue(EMVTAG_CryptInfo);
            if((CryptInfo & 0xc0) == 0x00) //AAC
            {
            	#if (!SpecificationBulletinNo239)//20200702_lhd

				if(0 == emvbase_avl_checkiftagexist(EMVTAG_SignDynAppData))
				{
					unsigned char reqCrpt = emvbase_avl_gettagvalue(EMVTAG_ReferenceControlParameter);

                    if((reqCrpt & 0xC0) == 0x00)
                    {
                        if(reqCrpt & 0x10)
                        {
                            //dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S9_E27_0);
							dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S10_E24_0);
                            return RLT_EMV_TERMINATE_TRANSERR;
                        }
                    }
				}
				else
				{
					unsigned char IDSStatus = 0;

	                emvbase_avl_gettagvalue_spec(EMVTAG_IDSStatus, &IDSStatus, 0, 1);
	                if(0x80 & IDSStatus)
	                {
	                    dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S10_E24_0);

	                    //C
	                    dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S10_E50_0);

	                    if(IDSStatus & 0x40)
	                    {
	                        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S11_E77_0);
	                    }
	                    else
	                    {
	                        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S10_E53_0);
	                    }

	                    return RLT_ERR_EMV_OTHERAPCARD;
	                }
	                else//s910.35
	                {
	                    unsigned char reqCrpt = emvbase_avl_gettagvalue(EMVTAG_ReferenceControlParameter);

	                    if((reqCrpt & 0xC0) == 0x00)
	                    {
	                        if(reqCrpt & 0x10)
	                        {
	                            //dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S9_E27_0);
								dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S10_E24_0);
	                            return RLT_EMV_TERMINATE_TRANSERR;
	                        }
	                    }
	                }
				}

				#else

				#if 0 // ��δ�����generateAC�����Ѿ����ˣ��������Ƕ����
                #ifdef PAYPASS_DATAEXCHANGE

				unsigned char IDSStatus = 0;
                emvbase_avl_gettagvalue_spec(EMVTAG_IDSStatus, &IDSStatus, 0, 1);
                if(0x80 & IDSStatus)
                {
                    dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S10_E24_0);

                    //C
                    dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S10_E50_0);

                    if(IDSStatus & 0x40)
                    {
                        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S11_E77_0);
                        //return PAYPASS_ENDAPPLICATION;
                    }
                    else
                    {
                        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S10_E53_0);
                    }

                    return PAYPASS_ENDAPPLICATION;
                }
                else//s910.35
                {
                    unsigned char reqCrpt = emvbase_avl_gettagvalue(EMVTAG_ReferenceControlParameter);

                    if((reqCrpt & 0xC0) == 0x00)
                    {
                        if(reqCrpt & 0x10)
                        {
                            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S9_E27_0);
                            return PAYPASS_CARDDATAERR;
                        }
                    }
                }

                #endif
				#endif

				#endif
            }
            else
            {
                memset(TVR, 0, sizeof(TVR));
                emvbase_avl_gettagvalue_spec(EMVTAG_TVR, TVR, 0, 5);
                if(TVR[4] & 0x02)
                {
					EMVBase_Trace("paypass-info: RRP performed\r\n");

                    item = emvbase_avl_gettagitempointer(EMVTAG_Track2Equ);
                    if(item != NULL)
                    {
                        tempbuf = emvbase_malloc(64);
                        memset(tempbuf, 0, 64);
                        i = 0;

                        EMVBaseBcdToAsc(tempbuf, item->data, item->len);
                    	i = EMVBaseStrPos(tempbuf, 'D');
                        applicationPANLen = i;

                        EMVBase_Trace("paypass-info: applicationPANLen=%d\r\n", applicationPANLen);

                        i += (1);
                        i += (2 * 2);
                        i += (3);

                        tempbuflen = i;

                        tempbuf1 = emvbase_malloc(64);
                        memset(tempbuf1, 'F', 64);
                        memcpy(tempbuf1, tempbuf, tempbuflen);
                        #if 0
                        if(item1->len <= 8)
                        #else
                        if(applicationPANLen <= 16)
                        #endif
                        {
                            memcpy(tempbuf1 + tempbuflen, "0000000000000", 13);
                            tempbuflen += 13;
                        }
                        else
                        {
                            memcpy(tempbuf1 + tempbuflen, "0000000000", 10);
                            tempbuflen += 10;
                        }
                        ifFexist = 0;
                        if(tempbuflen % 2)
                        {
                            ifFexist = 1;
                            tempbuflen++;
                        }

                        item = emvbase_avl_gettagitempointer(EMVTAG_CAPKI);
                        if((NULL != item) && ( item->data[0] < 0x0A))
                        {
                            tempbuf1[i] = item->data[0] + 0x30;
                        }
                        i++;

                        emvbase_avl_gettagvalue_spec(EMVTAG_RRPCounter, &RRP_counter, 0, 1);
                        tempbuf1[i++] = RRP_counter + 0x30;

                        memset(UnpredictNum, 0, sizeof(UnpredictNum));
                        emvbase_avl_gettagvalue_spec(EMVTAG_DeviceRREntropy, UnpredictNum, 0, 4);
                        drre = 0;
                        drre = UnpredictNum[2];
                        drre = drre << 8;
                        drre |= UnpredictNum[3];
                        #ifdef EMVB_DEBUG
                        EMVBase_Trace("paypass-info: Device Relay Resistance Entropy-drre: %d\r\n", drre);
                        EMVBase_TraceHex("paypass-info: Device Relay Resistance Entropy: ", UnpredictNum, 5);
                        #endif

                        memset(ascii, 0, sizeof(ascii));
                        asciilen = Paypass_itoa(drre, ascii);
                        if(asciilen > 5) { asciilen = 5; }
                        memset(asciibuf, '0', sizeof(asciibuf));
                        k = 4;
                        for(asciioffer = asciilen; asciioffer > 0; asciioffer--) //(asciilen)
                        {
                            asciibuf[k] = ascii[asciioffer - 1];
                            k -= 1;
                            if(0 > k) { break; }
                        }
                        memcpy(&tempbuf1[i], asciibuf, 5);
                        i += 5;

                        #if 0
                        if(item1->len <= 8)
                        #else
                        if(applicationPANLen <= 16)
                        #endif
                        {
                            drre = 0;
                            drre = UnpredictNum[1];
                            memset(ascii, 0, sizeof(ascii));
                            asciilen = Paypass_itoa(drre, ascii);
                            if(asciilen > 3) { asciilen = 3; }
                            memset(asciibuf, '0', sizeof(asciibuf));
                            k = 2;
                            for(asciioffer = asciilen; asciioffer > 0; asciioffer--) //(asciilen)
                            {
                                asciibuf[k] = ascii[asciioffer - 1];
                                k -= 1;
                                if(0 > k) { break; }
                            }
                            memcpy(&tempbuf1[i], asciibuf, 3);
                            i += 3;
                        }

                        Paypass_GetTagValueForShort(EMVTAG_MeasureRRProcessingTime, &MeasuredRelayResistanceProcessingTime);
                        #ifdef EMVB_DEBUG
                        EMVBase_Trace("paypass-info: Measured Relay Resistance Processing Time: %d\r\n", MeasuredRelayResistanceProcessingTime);
                        #endif
                        MeasuredRelayResistanceProcessingTime = MeasuredRelayResistanceProcessingTime / 10;
                        if(MeasuredRelayResistanceProcessingTime > 999)
                        {
                            MeasuredRelayResistanceProcessingTime = 999;
                        }
                        drre = 0;
                        drre = MeasuredRelayResistanceProcessingTime;
                        memset(ascii, 0, sizeof(ascii));
                        asciilen = Paypass_itoa(drre, ascii);
                        if(asciilen > 3) { asciilen = 3; }

                        k = tempbuflen - ifFexist - 1;

                        for(asciioffer = asciilen; asciioffer > 0; asciioffer--)
                        {
                            tempbuf1[k] = ascii[asciioffer - 1];
                            k -= 1;
                            if(0 > k) { break; }
                        }

                        #ifdef EMVB_DEBUG
//                        EMVBase_Trace("\r\n paypass-error: after replace TAG_Track2Equ asciibuf : ");
//                        EMVBase_TraceHex("\r\n paypass-error: printFormat", tempbuf1, (tempbuflen));
                        #endif

                        memset(tempbuf, 0, 64);
                        EMVBaseAscToBcd(tempbuf, tempbuf1, tempbuflen);
                        #ifdef EMVB_DEBUG
//                        EMVBase_Trace("\r\n paypass-error: after replace TAG_Track2Equ: ");
//                        EMVBase_TraceHex("\r\n paypass-error: printFormat", tempbuf, (tempbuflen / 2));
                        #endif
                        emvbase_avl_createsettagvalue(EMVTAG_Track2Equ, tempbuf, (tempbuflen / 2));
                        emvbase_free(tempbuf1);
                        emvbase_free(tempbuf);
                    }
                }
            }
        }
    }
    #endif

	dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S11_E110_0); // S910.70

    memset(POSCII, 0x00, sizeof(POSCII));
    emvbase_avl_gettagvalue_spec(EMVTAG_POSCII, POSCII, 0, 3);
    EMVBase_Trace("paypass-info: POS Cardholder Interaction Information(TagDF4B): %02x%02x%02x\r\n", POSCII[0], POSCII[1], POSCII[2]);

	tempphonemsgtbl = tempApp_UnionStruct->pphonemsgtable;

	if((POSCII[1] & 0x03) || (POSCII[2] & 0x0F))
    {
        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S11_E112_0);
		tempApp_UnionStruct->PaypassTradeParam->TransResult = RLT_EMV_TERMINATE_TRANSERR;

        for(i=0; i<tempphonemsgtbl->phonemessagetablenum; i++)
        {
            if((POSCII[0]&tempphonemsgtbl->phonemessagetable[i].PCIIMASK[0]) == tempphonemsgtbl->phonemessagetable[i].PCIIVALUE[0] &&
                    (POSCII[1]&tempphonemsgtbl->phonemessagetable[i].PCIIMASK[1]) == tempphonemsgtbl->phonemessagetable[i].PCIIVALUE[1] &&
                    (POSCII[2]&tempphonemsgtbl->phonemessagetable[i].PCIIMASK[2]) == tempphonemsgtbl->phonemessagetable[i].PCIIVALUE[2])  //messge id messege status
            {
                tempApp_UnionStruct->PaypassTradeParam->PaypassSelectedPOSCIIID = i;
                dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S11_E113_0);
                break;
            }
        }
    }
    else
	{
        EMVBase_Trace("paypass-info: Transaction Result: %d\r\n", tempApp_UnionStruct->PaypassTradeParam->TransResult);

		dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S11_E115_1); // S910.75-status

        //OFFLINE approved
        if(tempApp_UnionStruct->PaypassTradeParam->TransResult == RLT_EMV_OFFLINE_APPROVE)
        {
            dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S11_E114_1);

			dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S11_E115_2);
            if(emvbase_avl_checkiftagexist(EMVTAG_BalanceAfterGAC))
            {
                dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S11_E115_3);
                if(emvbase_avl_checkiftagexist(EMVTAG_AppCurcyCode))
                {
                    dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S11_E115_4);
                }
            }
            dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S11_E115_5);
        }
        else if(tempApp_UnionStruct->PaypassTradeParam->TransResult == RLT_EMV_ONLINE_WAIT)
        {
            dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S11_E114_2);

			dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S11_E115_6);
        }
        else  //E74
        {
            if(emvbase_avl_gettagvalue(EMVTAG_TransTypeValue) != 0x20)
            {
                dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S11_E115_2);

                memset(TermCapab, 0x00, sizeof(TermCapab));
                emvbase_avl_gettagvalue_spec(EMVTAG_TermCapab, TermCapab, 0, 3);
                emvbase_avl_gettagvalue_all(EMVTAG_ThirdPartyData, FormFactIndicator, &FormFactIndicatorlen);
                #ifdef EMVB_DEBUG
                EMVBase_Trace("\r\n paypass-info:declined data:%d %02x %02x %02x\r\n", FormFactIndicatorlen, FormFactIndicator[2], FormFactIndicator[4], FormFactIndicator[5]);
                #endif
                if(((FormFactIndicatorlen >= 6) && ((FormFactIndicator[2] & 0x80) == 0x00) && (FormFactIndicator[4] != 0x30 || FormFactIndicator[5] != 0x30)) ||
                        ((TermCapab[0] & 0x20) == 0))
                {
                    dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S11_E114_3);
					tempApp_UnionStruct->PaypassTradeParam->TransResult = RLT_EMV_OFFLINE_DECLINE;

					dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S11_E115_7);
                }
                else
                {
                    dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S11_E114_4);
					tempApp_UnionStruct->PaypassTradeParam->TransResult = RLT_ERR_EMV_SWITCHINTERFACE;

					dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S11_E115_8);
                }
            }
            else   //END APPLICATION
            {
                tempApp_UnionStruct->PaypassTradeParam->TransResult = RLT_EMV_TERMINATE_TRANSERR;
                dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S11_E114_5);

                dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S11_E115_9);
            }
        }
    }

    return RLT_EMV_OK;
}

unsigned char Paypass_PostGenACPutData(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
	unsigned char retCode = RLT_EMV_OK;
	EMVBASETAGCVLITEM *item = NULL;
	unsigned char POSCII[3] = {0};


	memset(POSCII, 0, sizeof(POSCII));
    emvbase_avl_gettagvalue_spec(EMVTAG_POSCII, POSCII, 0, 3);



	#ifdef PAYPASS_DATAEXCHANGE //20160806_lhd move to down line

    item = emvbase_avl_gettagitempointer(EMVTAG_TagsToWriteYetAfterGenAC);
    if((item != NULL) && item->len)
    {
    	EMVBase_Trace("paypass-info: Post GenAC Put Data\r\n");
        retCode = Paypass_PutData(EMVTAG_TagsToWriteYetAfterGenAC, tempApp_UnionStruct);
        if(RLT_EMV_OK == retCode)
        {
            emvbase_avl_setvalue_or(EMVTAG_PostGACPutDataStatus, 0, 0x80);

			if((POSCII[1] & 0x03) || (POSCII[2] & 0x0F))
			{
            	dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S15_E10); //20160806_lhd
			}

			if (tempApp_UnionStruct->displayRemoveCard)
	        {
	            tempApp_UnionStruct->displayRemoveCard();
	        }
        }
        #if 1 /*Modify by luohuidong at 2016.10.06  0:27 */
        else if(RLT_ERR_EMV_IccReturn == retCode )
        {
            if((POSCII[1] & 0x03) || (POSCII[2] & 0x0F))
			{
            	dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S15_E10); //20160806_lhd
			}

			if (tempApp_UnionStruct->displayRemoveCard)
	        {
	            tempApp_UnionStruct->displayRemoveCard();
	        }
        }
        #endif /* if 1 */
    }

    #endif

	return RLT_EMV_OK;
}

unsigned char Paypass_TransProcess2(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
	unsigned char POSCII[3] = {0};


	memset(POSCII, 0, sizeof(POSCII));
    emvbase_avl_gettagvalue_spec(EMVTAG_POSCII, POSCII, 0, 3);
    if((POSCII[1] & 0x03) || (POSCII[2] & 0x0F))
    {
        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S11_E119_0);
        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S11_E120_1);
		return RLT_ERR_EMV_SEEPHONE;
    }
    else
    {
        dllpaypass_SetErrPar(tempApp_UnionStruct, PAYPASSSTEP_S11_E120_3);
        dllpaypass_packoutsignal(tempApp_UnionStruct, PAYPASSSTEP_S11_E120_2);
		return RLT_EMV_OK;
    }
}

unsigned char Paypass_OnlinePIN(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char retCode = RLT_EMV_OK;
    unsigned char tempCreditPwd[20];
	static unsigned char callbackFlag = 0;
	int ret;


    EMVBase_Trace("\r\n paypass-info:paypassonlinepinreq=%d\n", tempApp_UnionStruct->PaypassTradeParam->paypassonlinepinreq);
    if(tempApp_UnionStruct->PaypassTradeParam->paypassonlinepinreq == 0)
    {
        return RLT_EMV_OK;
    }

    memset(tempCreditPwd, 0, sizeof(tempCreditPwd));

	if(callbackFlag == 0)
	{
		callbackFlag++;
		return PAYPASS_REQ_SETCALLBACK_INPUTPIN;
	}
	else
	{
		ret = tempApp_UnionStruct->GetPinRes();
		callbackFlag = 0;
	}

	if(ret == -10)
	{
        emvbase_avl_setvalue_or(EMVTAG_TVR, 2, 0x08); //Online PIN not entered
        return RLT_EMV_ERR;
	}

	if(ret == RLT_EMV_OK)
	{
        emvbase_avl_createsettagvalue(EMVTAG_PIN, tempCreditPwd, 8);
	}
	else if(RLT_ERR_EMV_InputBYPASS == retCode)
	{

	}
	else
	{
        return RLT_ERR_EMV_CancelTrans;
	}

//    if (tempApp_UnionStruct->EMVB_InputCreditPwd)
//    {
//        retCode = tempApp_UnionStruct->EMVB_InputCreditPwd(EMVB_DATAAUTH_ONLINEPIN, tempCreditPwd);
//    }
//    else
//    {
//        emvbase_avl_setvalue_or(EMVTAG_TVR, 2, 0x08); //Online PIN not entered
//        return RLT_EMV_ERR;
//    }

    //关于密码的TVR已经在CVM步骤里置�?PIN后置的时候不能再修改TVR
//    if(retCode == RLT_EMV_OK)
//    {
//        emvbase_avl_createsettagvalue(EMVTAG_PIN, tempCreditPwd, 8);
//        //emvbase_avl_setvalue_or(EMVTAG_TVR, 2, 0x04); //Online PIN entered
//    }
//    else if(/*retCode == RLT_ERR_EMV_CancelTrans ||*/ RLT_ERR_EMV_InputBYPASS == retCode)
//    {
//        //emvbase_avl_setvalue_or(EMVTAG_TVR, 2, 0x08); //Online PIN not entered
//    }
//    else
//    {
//        return RLT_ERR_EMV_CancelTrans;
//    }

    return RLT_EMV_OK;

}

unsigned char Paypass_CheckExpDate(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char currentDate[4], EffectDate[4], ExpireDate[4]; //buf[10],
    unsigned char AppEffectDateexist, AppExpireDateexist;
    EMVBASETAGCVLITEM *item;


    emvbase_avl_gettagvalue_spec(EMVTAG_TransDate, &currentDate[1], 0, 3);
	EMVBase_TraceHex("paypass-info: Transaction Date(Tag9A): ", &currentDate[1], 3);

    if(currentDate[1] > 0x49) { currentDate[0] = 0x19; }
    else { currentDate[0] = 0x20; }


    AppEffectDateexist = 0;
    item = emvbase_avl_gettagitempointer(EMVTAG_AppEffectDate);
    if(item != NULL)
    {
        if(item->len >= 3)
        {
            AppEffectDateexist = 1;
        }
    }

    if(AppEffectDateexist)//App effective date exist
    {
		EMVBase_TraceHex("paypass-info: Effective Date(Tag5F25): ", item->data, 3);
        if(EMVBase_CheckDateFormat(item->data) == RLT_EMV_ERR)
		{
			EMVBase_Trace("paypass-error: data of Effective Date(Tag5F25) error\r\n");
			return RLT_EMV_ERR;
		}

        if(item->data[0] > 0x49) { EffectDate[0] = 0x19; }
        else { EffectDate[0] = 0x20; }

        memcpy((unsigned char *)&EffectDate[1], item->data, 3);
        if(memcmp(currentDate, EffectDate, 4) < 0)
        {
			EMVBase_Trace("paypass-info: Application not yet effective \r\n");
            emvbase_avl_setvalue_or(EMVTAG_TVR, 1, 0x20);
        }
    }


    AppExpireDateexist = 0;
    item = emvbase_avl_gettagitempointer(EMVTAG_AppExpireDate);
    if(item != NULL)
    {
        if(item->len >= 3)
        {
            AppExpireDateexist = 1;
        }
    }

    if(AppExpireDateexist)//App expiration date exist
    {
    	EMVBase_TraceHex("paypass-info: Expiry Date(Tag5F24):", item->data, 3);
        if(EMVBase_CheckDateFormat(item->data) == RLT_EMV_ERR)
		{
			EMVBase_Trace("paypass-error: data of Expiry Date(Tag5F24) error\r\n");
			return RLT_EMV_ERR;
		}

        if(item->data[0] > 0x49) { ExpireDate[0] = 0x19; }
        else { ExpireDate[0] = 0x20; }

        memcpy((unsigned char *)&ExpireDate[1], item->data, 3);
        if(memcmp(currentDate, ExpireDate, 4) > 0)
        {
        	EMVBase_Trace("paypass-info: Expired\r\n");
            emvbase_avl_setvalue_or(EMVTAG_TVR, 1, 0x40);
        }
    }

    return RLT_EMV_OK;
}

void Paypass_CheckVer(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char CardAppVerbExist;
    unsigned char TermAppVerbExist;
    EMVBASETAGCVLITEM *Termitem = NULL, *Carditem = NULL;


    Termitem = emvbase_avl_gettagitemandstatus(EMVTAG_AppVerNum, &CardAppVerbExist);
    Carditem = emvbase_avl_gettagitemandstatus(EMVTAG_AppVer, &TermAppVerbExist);

    if(CardAppVerbExist && TermAppVerbExist)
    {
    	EMVBase_TraceHex("paypass-info: Terminal APP version(Tag9F09): ", Termitem->data, Termitem->len);
		EMVBase_TraceHex("paypass-info: Card APP version(Tag9F08): ", Carditem->data, Carditem->len);

        if(Carditem->len != Termitem->len)
        {
        	EMVBase_Trace("paypass-info: different version\r\n");
            emvbase_avl_setvalue_or(EMVTAG_TVR, 1, 0x80);
        }
        else
        {
            if(memcmp(Carditem->data, Termitem->data, Termitem->len))
            {
            	EMVBase_Trace("paypass-info: different version\r\n");
                emvbase_avl_setvalue_or(EMVTAG_TVR, 1, 0x80);
            }
        }
    }

	return;
}

void Paypass_CheckAUC(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char bTestFail;
    unsigned char TermType, AUC[2], AUCbExist;
    unsigned char IssuCountryCode[2], CountryCode[2], IssuCountryCodebExist;
    unsigned char TermAmtOther[6];
    unsigned char TermAddCapab[5];
    unsigned char TermAmtOtherBin = 0;
    unsigned char TransTypeValue;//20160920_lhd add


    bTestFail = 0;
    memset(AUC, 0, sizeof(AUC));
    AUCbExist = !(emvbase_avl_gettagvalue_spec(EMVTAG_AUC, AUC, 0, 2));

    TermType = emvbase_avl_gettagvalue(EMVTAG_TermType);
    TransTypeValue = emvbase_avl_gettagvalue(EMVTAG_TransTypeValue);
    memset(TermAddCapab, 0, sizeof(TermAddCapab));
    emvbase_avl_gettagvalue_spec(EMVTAG_TermAddCapab, TermAddCapab, 0, 5);

    memset(IssuCountryCode, 0, sizeof(IssuCountryCode));
    IssuCountryCodebExist = !(emvbase_avl_gettagvalue_spec(EMVTAG_IssuCountryCode, IssuCountryCode, 0, 2));
    memset(CountryCode, 0, sizeof(CountryCode));
    emvbase_avl_gettagvalue_spec(EMVTAG_CountryCode, CountryCode, 0, 2);

	EMVBase_Trace("paypass-info: Terminal Type(Tag9F35): %02x\r\n", TermType);
	EMVBase_Trace("paypass-info: Transaction Type(Tag9C): %02x\r\n", TransTypeValue);
	EMVBase_TraceHex("paypass-info: AUC(Tag9F07):", AUC, 2);
	EMVBase_TraceHex("paypass-info: Issuer Country Code(Tag5F28):", IssuCountryCode, 2);
	EMVBase_TraceHex("paypass-info: Terminal Country Code(Tag9F1A):", CountryCode, 2);

    memset(TermAmtOther, 0, sizeof(TermAmtOther));
    emvbase_avl_gettagvalue_spec(EMVTAG_AmtOtherNum, TermAmtOther, 0, 6);
    if(memcmp(TermAmtOther, "\x00\x00\x00\x00\x00\x00", 6))
    {
        TermAmtOtherBin = 1;
    }

    if(AUCbExist)
    {
        if((TermType == 0x14 || TermType == 0x15 || TermType == 0x16 )
                && (TermAddCapab[0] & 0x80)) //The termianl is ATM
        {
            if(!(AUC[0] & 0x02)) // if??Valid at ATMs??bit not on.
            { bTestFail = 1; }
        }
        else
        {
            if(!(AUC[0] & 0x01))
            {
            	EMVBase_Trace("paypass-info: Unvalid at Terminals other than ATMs(AUC Byte1 bit8 is 0)\r\n");
            	bTestFail = 1;
			}
        }
        if(IssuCountryCodebExist)
        {
            if(!memcmp(IssuCountryCode, CountryCode, 2))
            {
            	EMVBase_Trace("paypass-info: Country Code same-->domestic\r\n");

                if(TransTypeValue == EMVBASE_TRANS_CASH || TransTypeValue == EMVBASE_TRANS_CASHDISBURSEMENT)
                {
                    if(!(AUC[0] & 0x80))
                    {
                    	EMVBase_Trace("paypass-info: Unvalid for Domestic Cash Transactions(AUC Byte1 bit8 is 0)\r\n");
                    	bTestFail = 1;
					}
                }

                if(TransTypeValue == EMVBASE_TRANS_GOODS || TransTypeValue == EMVBASE_TRANS_CASHBACK )
                    // if(TransTypeValue == EMVBASE_TRANS_GOODS )
                {
                    if(!(AUC[0] & 0x28))
                    {
                    	EMVBase_Trace("paypass-info: Unvalid for Domestic Goods(AUC Byte1 bit6 is 0)\r\n");
						EMVBase_Trace("paypass-info: Unvalid for Domestic Services(AUC Byte1 bit4 is 0)\r\n");
                    	bTestFail = 1;
					}
                }

                if(TermAmtOtherBin != 0 && TransTypeValue == EMVBASE_TRANS_CASHBACK)
                {
                    if(!(AUC[1] & 0x80))
                    {
                    	EMVBase_Trace("paypass-info: Unvalid for Domestic Cash Transactions(AUC Byte1 bit8 is 0)\r\n");
                    	bTestFail = 1;
					}
                }
            }
            else
            {
            	EMVBase_Trace("paypass-info: Country Code different-->international\r\n");

                if(TransTypeValue == EMVBASE_TRANS_CASH || TransTypeValue == EMVBASE_TRANS_CASHDISBURSEMENT)
                {
                    if(!(AUC[0] & 0x40))
                    {
                    	EMVBase_Trace("paypass-info: Unvalid for International Cash Transactions(AUC Byte1 bit7 is 0)\r\n");
                    	bTestFail = 1;
					}
                }

                if(TransTypeValue == EMVBASE_TRANS_GOODS || TransTypeValue == EMVBASE_TRANS_CASHBACK)
                {
                    if( !(AUC[0] & 0x14))
                    {
                    	EMVBase_Trace("paypass-info: Unvalid for International Goods(AUC Byte1 bit1 is 0)\r\n");
						EMVBase_Trace("paypass-info: Unvalid for International Services(AUC Byte1 bit3 is 0)\r\n");
                    	bTestFail = 1;
					}
                }

                if(TermAmtOtherBin != 0 && TransTypeValue == EMVBASE_TRANS_CASHBACK)
                {
                    if(!(AUC[1] & 0x40))
                    {
                    	EMVBase_Trace("paypass-info: Unvalid for International Cash Transactions(AUC Byte1 bit7 is 0)\r\n");
                    	bTestFail = 1;
					}
                }
            }
        }
    }

    if(bTestFail)
    {
        emvbase_avl_setvalue_or(EMVTAG_TVR, 1, 0x10); //set??Requested service not allowed for card product??bit 1
    }

	return;
}

unsigned char Paypass_ProcessRestrict(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    Paypass_CheckVer(tempApp_UnionStruct);
    Paypass_CheckAUC(tempApp_UnionStruct);
    if(Paypass_CheckExpDate(tempApp_UnionStruct) != RLT_EMV_OK) { return RLT_EMV_ERR; }
    return RLT_EMV_OK;
}

unsigned char Paypass_OnLineDeal(PAYPASSTradeUnionStruct *tempApp_UnionStruct)
{
    unsigned char AuthRespCode[2];


    memset(AuthRespCode, 0, sizeof(AuthRespCode));
    emvbase_avl_gettagvalue_spec(EMVTAG_AuthRespCode, AuthRespCode, 0, 2);

    if(!memcmp(AuthRespCode, "00", 2) ||
            !memcmp(AuthRespCode, "10", 2) ||
            !memcmp(AuthRespCode, "11", 2) ) //Issuer approval,according to p172 of VIS140 terminal specification.
    {
        tempApp_UnionStruct->PaypassTradeParam->TransResult = RLT_EMV_ONLINE_APPROVE;
    }
    else if(!memcmp(AuthRespCode, "01", 2) ||
            !memcmp(AuthRespCode, "02", 2)) //Issuer referral
    {
        return RLT_EMV_TRANSNOTACCEPT;
    }
    else//Other codes are looked as Issuer decline.
    {
        tempApp_UnionStruct->PaypassTradeParam->TransResult = RLT_EMV_ONLINE_DECLINE;
    }

    return RLT_EMV_OK;
}

