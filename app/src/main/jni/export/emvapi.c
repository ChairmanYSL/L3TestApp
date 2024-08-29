#include "emvapi.h"
#include "emv_tag.h"

#define SZZT_AID_PARAM_LEN	(1024)
#define SZZT_AID_HEADER_LEN	(4)
#define SZZT_AID_DATA_LEN	(SZZT_AID_PARAM_LEN-SZZT_AID_HEADER_LEN)

#define is_constructed(c)       ((c) & 0x20)
#define is_primitive(c)         (!is_constructed((c)))
#define has_subsequent(c)       (((c) & 0x1F) == 0x1F)
#define another_byte_follow(c)  ((c) & 0x80)
#define lenlen_exceed_one(c)    ((c) & 0x80)

extern PAYPASSDLLTRADEPARAMETER *gstPaypassTradeParam;

enum EMV_ErrCodes
{
    ErOK = 0,
    ErUNKNOWN = -1,
    ErFUNCPAR = -2,
    ErFOPEN = -3,
    ErFSEEK = -4,
    ErFREAD = -5,
    ErFWRITE = -6,
    ErFREMOVE = -7,
    ILL_TERMPARAM = -8,
    ILL_TERMPARAMLEN = -9,
    ILL_AIDPARAM = -10,
    ILL_AIDPARAMLEN = -11,
    ILL_AIDLEN = -12,
    ILL_CAPKPARAM = -13,
    ILL_CAPKPARAMLEN = -14,
    ErFCCTRNULL = -15,//!<�������Ʋ���Ϊ��
    ErFCRECORDNULL = -16,//!<������¼Ϊ��
    ErFCRECORDFULL = -17,//!<������¼����
    ErFCRECORDNO = -18,   //!<������¼�Ŵ���
    ErNOTFINDAID = -19,
    ErNOTFINDCAPK = -20,
    ErWRCOMPLETE = -21,//�ļ���д��
    ErFILESIZE = -22,//�ļ���С����
    ILL_DRL = -23,
    ILL_DRLNOTFIND= -24,

};

enum _ErrCodes
{
    TLV_ERR                         = (-10100),
    TLV_NOOBJSPACE                  = (TLV_ERR -  1),
    TLV_DETAGLENERR                 = (TLV_ERR -  2),
    TLV_DELENLENERR                 = (TLV_ERR -  3),
    TLV_TAGSTARTILL                 = (TLV_ERR -  4),
    TLV_NOTSINGLEOBJ                = (TLV_ERR -  5),
    TLV_INPUTDATAERR                = (TLV_ERR -  6),

    BUF_ERR                         = (-10200),
    BUF_BUFOVER                     = (BUF_ERR -  1),
    BUF_OBJDUP                      = (BUF_ERR -  2),
};

static int recur_decode(int parent, EMV_TLVOBJ *tlvs, int tlvs_max, int flags)
{
	unsigned int tmptag;
	int tmplen, i;
	unsigned char *data = tlvs[parent].pvalue;
	int data_len = tlvs[parent].valuelen;
	int data_index;
	int tlvs_index = parent + 1;

	for (data_index = 0; data_index < data_len;)
	{
        if (data[data_index] == 0x00 || data[data_index] == 0xFF)
        {
            data_index++;
            continue;
        }

		if (tlvs_index == tlvs_max)
		{
			return TLV_NOOBJSPACE;
		}

		if (is_primitive(data[data_index]))//�ж��ǲ��Ǹ��Ͻṹ�����ǵ�һ�ṹ��tag�ĵ�һ���ֽڵĵ���λ��1�Ǹ��Ͻṹ��0�ǵ�һ�ṹ
		{
			tlvs[tlvs_index].childnum = -1;
		}
		else
		{
			tlvs[tlvs_index].childnum = 0;
		}
		////////////////////////////
		tmptag = 0;
		if (has_subsequent(data[data_index]))//tlv��ʽ˵�����������λȫ��Ϊ1����ʾtagΪ���ֽڵģ���ȫ��Ϊ1����ʾtagΪ���ֽ�
		{
			i = 0;
			do {
				tmptag = (tmptag | data[data_index++]) << 8;
				if (++i == 4 || data_index == data_len)
				{
					return TLV_DETAGLENERR;
				}
			} while (another_byte_follow(data[data_index]));
		}
		tlvs[tlvs_index].tagname = tmptag | data[data_index++];

		if (data_index == data_len)
		{
			return TLV_DETAGLENERR;
		}
		////////////////////////////
		tmplen = data[data_index] & 0x7F;
		if (lenlen_exceed_one(data[data_index++]))//�жϳ��ȵ����λ�Ƿ�Ϊ1
		{
			if (tmplen < 1 || tmplen > 3 || tmplen > (data_len - data_index))
			{
				return TLV_DELENLENERR;
			}
			i = tmplen;
			tmplen = 0;
			while (i > 0)
			{
				tmplen |= data[data_index++] << (--i * 8);
			}
		}
		if (tmplen > (data_len - data_index))
		{
			return TLV_DELENLENERR;
		}
		////////////////////////////
		tlvs[tlvs_index].valuelen = tmplen;
		tlvs[tlvs_index].parent = parent;
		tlvs[tlvs_index].pvalue = data + data_index;
		data_index += tmplen;

		if (tlvs[tlvs_index].childnum == 0 && !(flags & DECODE_LEVEL1))
		{
			if ((tmplen = recur_decode(tlvs_index, tlvs, tlvs_max, flags)))
			{
				return tmplen;
			}
			tlvs_index += tlvs[tlvs_index].childnum;
		}
		tlvs_index++;
	}
	tlvs[parent].childnum = tlvs_index - parent - 1;
	return 0;
}


static int emvtlv_decode(u8 *data, int data_len, EMV_TLVOBJ *tlvs, int tlvs_max, int flags)
{
	int ret;

	if (data == NULL || tlvs == NULL)
	{
		return TLV_INPUTDATAERR;
	}

	emv_memset(tlvs, 0x00, sizeof(EMV_TLVOBJ) * tlvs_max);

	tlvs[0].pvalue = data;
	tlvs[0].valuelen = data_len;

	if ((ret = recur_decode(0, tlvs, tlvs_max, flags)))
	{
		return ret;
	}

	if (flags & SINGLE_TLVOBJ)
	{
		if ((tlvs[1].pvalue + tlvs[1].valuelen) != (data + data_len))
		{
			return TLV_NOTSINGLEOBJ;
		}
	}

	return EMV_OK;
}



static int EMV_TLV_Decode(u8 *pData, int nDataLen, EMV_TLVOBJ *pTLVs, int nTLVs, int bFlags)
{
    int ret;

    ret = emvtlv_decode(pData, nDataLen, pTLVs, nTLVs, bFlags);
    if (ret != EMV_OK)
    {
        ret = EMV_FAIL;
    }

    return ret;
}

static int sdkU32Len(u32 value)
{
    int count = 0;

    while (value != 0)
    {
        count++;
        value >>= 8;
    }

    return count;
}

static int emv_api_conv_aid(EMV_AIDPARAM* apar, u8 *buf, int len)
{
	u8 *tlvPtr,*vPrt;
	u16 vLen;

	if(NULL == apar || NULL == buf || len < 0)
	{
		return SDK_PARA_ERR;
	}

	//init aid param
	memset(apar, 0, sizeof(EMV_AIDPARAM));
	apar->default_ddol_len = 11;
	memcpy(apar->default_ddol,"\x9F\x37\x04\x9F\x47\x01\x8F\x01\x9F\x32\x01", 11);
	apar->default_tdol_len = 3;
	memcpy(apar->default_tdol, "\x9F\x08\x02", 3);
	memcpy(apar->term_country_code, "\x09\x78", 2); //euro
	memcpy(apar->trans_currency_code, "\x09\x78", 2);
	apar->trans_currency_exponent = 2;
	memcpy(apar->trans_ref_currency_code, "\x09\x78", 2);
	apar->trans_ref_currency_exponent = 0;
	apar->online_pin_support_indicator = 1;
	apar->cl_trans_limit_exist = 0;
	apar->cl_floor_limit_exist = 0;
	apar->cvm_limit_exist = 0;
	apar->extend_sel_sup = 1;
	memcpy(apar->term_trans_qualifiers, "\x76\x00\x00\x80", 4);

	tlvPtr = TlvSeek(buf, len, TAG_9F06_AID_TERMINAL);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		if(vPrt)
		{
			vLen = TlvLen(tlvPtr);
			if(!vLen)
			{
				Trace("AID", "AID Name Len invalid\r\n");
				return SDK_ERR;
			}
			apar->aid_len = vLen;
			memcpy(apar->aid, vPrt, apar->aid_len);
		}
		else
		{
			Trace("AID", "AID Name value invalid\r\n");
			return SDK_ERR;
		}
	}
	else
	{
		Trace("AID", "AID Name miss\r\n");
		return SDK_ERR;
	}


	tlvPtr = TlvSeek(buf, len, TAG_DF01_ASI);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		if(vPrt && 1 == TlvLen(tlvPtr))
		{
			apar->app_sel_indicator = *vPrt;
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_9F09_APPLICATION_VERSION_NUMBER_TERMINAL);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		vLen = TlvLen(tlvPtr);
		if(vPrt && sizeof(apar->app_ver) >= vLen)
		{
			memcpy(apar->app_ver, vPrt, vLen);
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_DF17_TARGET_PERCENTAGE);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		if(vPrt && 1 == TlvLen(tlvPtr))
		{
			apar->target_percent = *vPrt;
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_DF16_MAX_TARGET_PERCENTAGE);
	{
		vPrt = TlvVPtr(tlvPtr);
		if(vPrt && 1 == TlvLen(tlvPtr))
		{
			apar->max_target_percent = *vPrt;
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_DF15_THRESHOLD_VALUE);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		vLen = TlvLen(tlvPtr);
		if(vPrt && sizeof(apar->threshold_value) >= vLen)
		{
			memcpy(apar->threshold_value, vPrt, vLen);
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_9F1C_TERMINAL_IDENTIFICATION);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		vLen = TlvLen(tlvPtr);
		if(vPrt && sizeof(apar->term_id) >= vLen)
		{
			memcpy(apar->term_id, vPrt, vLen);
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_9F1E_IFD_SERIAL_NUMBER);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		vLen = TlvLen(tlvPtr);
		if(vPrt && sizeof(apar->term_ifd) >= vLen)
		{
			memcpy(apar->term_ifd, vPrt, vLen);
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_9F1A_TERMINAL_COUNTRY_CODE);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		vLen = TlvLen(tlvPtr);
		if(vPrt && 2 == vLen)
		{
			memcpy(apar->term_country_code, vPrt, vLen);
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_5F2A_TRANSACTION_CURRENCY_CODE);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		vLen = TlvLen(tlvPtr);
		if(vPrt && 2 == vLen)
		{
			memcpy(apar->trans_currency_code, vPrt, vLen);
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_5F36_TRANSACTION_CURRENCY_EXPONENT);
	{
		vPrt = TlvVPtr(tlvPtr);
		if(vPrt && 1 == TlvLen(tlvPtr))
		{
			apar->trans_currency_exponent = *vPrt;
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_9F3C_TRANSACTION_REFERENCE_CURRENCY);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		vLen = TlvLen(tlvPtr);
		if(vPrt && 2 == vLen)
		{
			memcpy(apar->trans_ref_currency_code, vPrt, vLen);
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_9F3D_TRANSACTION_REFERENCE_CURRENCY_EXPONENT);
	{
		vPrt = TlvVPtr(tlvPtr);
		if(vPrt && 1 == TlvLen(tlvPtr))
		{
			apar->trans_currency_exponent = *vPrt;
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_DF11_TAC_DEFAULT);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		vLen = TlvLen(tlvPtr);
		if(vPrt && sizeof(apar->term_action_code_default) >= vLen)
		{
			apar->term_action_code_default_exist = 1;
			memcpy(apar->term_action_code_default, vPrt, vLen);
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_DF12_TAC_ONLINE);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		vLen = TlvLen(tlvPtr);
		if(vPrt && sizeof(apar->term_action_code_online) >= vLen)
		{
			apar->term_action_code_online_exist = 1;
			memcpy(apar->term_action_code_online, vPrt, vLen);
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_DF13_TAC_DENIAL);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		vLen = TlvLen(tlvPtr);
		if(vPrt && sizeof(apar->term_action_code_denial) >= vLen)
		{
			apar->term_action_code_denial_exist = 1;
			memcpy(apar->term_action_code_denial, vPrt, vLen);
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_9F1D_TERMINAL_RISK_MANGEMENT_DATA);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		vLen = TlvLen(tlvPtr);
		if(vPrt && sizeof(apar->term_risk_mana_data) >= vLen)
		{
			apar->term_risk_mana_data_len = vLen;
			memcpy(apar->term_risk_mana_data, vPrt, vLen);
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_9F01_ACQUIRER_IDENTIFIER);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		vLen = TlvLen(tlvPtr);
		if(vPrt && sizeof(apar->acquirer_id) >= vLen)
		{
			memcpy(apar->acquirer_id, vPrt, vLen);
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_9F15_MERCHANT_CATEGORY_CODE);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		vLen = TlvLen(tlvPtr);
		if(vPrt && 2 == vLen)
		{
			memcpy(apar->merchant_category_code, vPrt, vLen);
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_9F16_MERCHANT_IDENTIFIER);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		vLen = TlvLen(tlvPtr);
		if(vPrt && sizeof(apar->merchant_id) >= vLen)
		{
			memcpy(apar->merchant_id, vPrt, vLen);
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_9F4E_MERCHANT_NAME);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		vLen = TlvLen(tlvPtr);
		if(vPrt && sizeof(apar->merchant_name) >= vLen)
		{
			memcpy(apar->merchant_name, vPrt, vLen);
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_DF18_ONLINE_PIN_SUPPORT_INDICATOR);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		vLen = TlvLen(tlvPtr);
		if(vPrt && 1 == vLen)
		{
			apar->online_pin_support_indicator = *vPrt;
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_EXTENDED_SELECT_SUPPORT);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		vLen = TlvLen(tlvPtr);
		if(vPrt && 1 == vLen)
		{
			apar->extend_sel_sup = *vPrt;
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_DF19_CONTACTLESS_FLOOR_LIMIT);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		vLen = TlvLen(tlvPtr);
		if(vPrt && sizeof(apar->cl_floor_limit) >= vLen)
		{
			apar->cl_floor_limit_exist = 1;
			memcpy(apar->cl_floor_limit, vPrt, vLen);
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_DF20_CONTACTLESS_TRANSACTION_LIMIT);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		vLen = TlvLen(tlvPtr);
		if(vPrt && sizeof(apar->cl_trans_limit) >= vLen)
		{
			apar->cl_trans_limit_exist = 1;
			memcpy(apar->cl_trans_limit, vPrt, vLen);
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_DF21_CVM_REQUIRED_LIMIT);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		vLen = TlvLen(tlvPtr);
		if(vPrt && sizeof(apar->cvm_limit) >= vLen)
		{
			apar->cvm_limit_exist = 1;
			memcpy(apar->cvm_limit, vPrt, vLen);
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_DF8125_READER_CTL_TRX_FLOOR_LIMIT_CDCVM);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		vLen = TlvLen(tlvPtr);
		if(vPrt && sizeof(apar->cl_trans_limit_odcvm) >= vLen)
		{
			apar->cl_trans_limit_odcvm_exist = 1;
			memcpy(apar->cl_trans_limit_odcvm, vPrt, vLen);
		}
	}

	tlvPtr = TlvSeek(buf, len, TAG_9F66_TERMINAL_TRANSACTION_QUALIFIERS);
	if(tlvPtr)
	{
		vPrt = TlvVPtr(tlvPtr);
		vLen = TlvLen(tlvPtr);
		if(vPrt && sizeof(apar->term_trans_qualifiers) >= vLen)
		{
			memcpy(apar->term_trans_qualifiers, vPrt, vLen);
		}
	}

	return SDK_OK;
}



int emv_get_tag_data(u32 tag, u8* value, int* length)
{
	int ret, tag_len;
	u8 tag_hex[4]={0};

	if(value == NULL || length == NULL)
	{
		return EMV_FAIL;
	}

	tag_len = sdkU32Len(tag);

	sdkU32ToHex(tag_hex, tag, tag_len);

	ret = sdkEMVBaseReadTLV(tag_hex, value, length);
	if(ret == SDK_OK)
	{
		return EMV_OK;
	}
	else
	{
		return EMV_FAIL;
	}

}

int emv_set_tag_data(u32 tag, u8 *value, int length)
{
	int ret, tag_len;
	u8 tag_hex[4]={0};

	if(value == NULL || length < 0)
	{
		return EMV_FAIL;
	}

	tag_len = sdkU32Len(tag);

	sdkU32ToHex(tag_hex, tag, tag_len);

	ret = sdkEMVBaseConfigTLV(tag_hex, value, length)；
	if(ret == SDK_OK)
	{
		return EMV_OK;
	}
	else
	{
		return EMV_FAIL;
	}
}

int emv_get_version(u8* ver)
{
	if(ver == NULL)
	{
		return EMV_FAIL;
	}

	sdkPaypassGetLibVerson(ver);
	return EMV_OK;
}

int emv_get_candidate_list(void* list)
{
	SDK_EMVBASE_CL_AIDLIST *listSdk = NULL;
	SDK_EMVBASE_AID_STRUCT *termAIDList = NULL;
	EMV_CANDIDATE *listSzzt = NULL;
	int numAID=0,i,maxNum=0,j,numList;

	sdkEMVBaseGetAIDListNum(&maxNum);
	if(maxNum)
	{
		termAIDList = (SDK_EMVBASE_CL_AIDLIST *)sdkGetMem(maxNum * sizeof(SDK_EMVBASE_CL_AIDLIST));
		memset(termAIDList, 0, maxNum * sizeof(SDK_EMVBASE_CL_AIDLIST));

		listSzzt = (EMV_CANDIDATE *)sdkGetMem(maxNum * sizeof(EMV_CANDIDATE));
		memset(listSzzt, 0, maxNum * sizeof(EMV_CANDIDATE));

		sdkEMVBaseGetAnyAIDList(0, maxNum, termAIDList, &numAID);
	}

	sdkEMVBaseGetEntryPointAIDList(listSdk, sizeof(listSdk)/sizeof(SDK_EMVBASE_CL_AIDLIST), &numList);

	if(numList)
	{
		for(i = 0; i < numList; i++)
		{
			listSzzt[i].aid_len = listSdk[i].AIDLen;
			memcpy(listSzzt[i].aid, listSdk[i].AID, listSzzt[i].aid_len);
			listSzzt[i].lable_len = listSdk[i].AppLabelLen;
			memcpy(listSzzt[i].lable, listSdk[i].AppLabel, listSzzt[i].lable_len);
			listSzzt[i].preferred_name_len = listSdk[i].PreferNameLen;
			memcpy(listSzzt[i].preferred_name, listSdk[i].PreferName, listSzzt[i].preferred_name_len);
			listSzzt[i].priority = listSdk[i].Priority;
			listSzzt[i].rsv[1] = listSdk[i].KernelIdentifier[0];

			for(j = 0; j < numAID; j++)
			{
				if(listSdk[i].AIDLen >= termAIDList[j].AidLen)
				{
					if(0 == termAIDList[j].Asi)
					{
						if(!memcmp(termAIDList[j].Aid, listSdk[i].AID, termAIDList[j].AidLen))
						{
							listSzzt[i].rsv[0] = j;
							break;
						}
					}
					else
					{
						if(termAIDList[j].AidLen ==  listSdk[i].AIDLen && !memcmp(termAIDList[j].Aid, listSdk[i].AID, termAIDList[j].AidLen))
						{
							listSzzt[i].rsv[0] = j;
							break;
						}
					}
				}
			}
		}

		memcpy(list, listSzzt, numList*sizeof(EMV_CANDIDATE));
		return numList;
	}

	return EMV_OK;
}

int emv_initialize(EMV_INIT_PARAM* par)
{
	sdkEMVBasePowerStartInit();
	sdkEMVBaseTransInit();

	if(par && par->fun_cmpKernelID)
	{
		sdkEMVBaseSet_CMPKernelIDFunc(par->fun_cmpKernelID);
	}
	sdkEMVBaseSetCheckTag84(0x07);

	if(par)
	{
		if(par->bIsDispDekExchange)
		{
			sdkPayPassSupportSaveDekLogSet();
		}
		else
		{
			sdkPayPassSupportSaveDekLogClear();
		}
	}

	sdkEMVBase_SetEMVLogOutput(1);

	sdkPaypassTransInit();
	sdkPayPassSetSupportDE(1);
	sdkPaypassSetRefundrequestAAC(1);
	sdkPaypassSetCertificationStatus(1);

	if(par && par->fun_setRfCardPowerOff)
	{
		sdkPaypassSetRfCardPowerOff(par->fun_setRfCardPowerOff);
	}

	if(par && par->fun_setdisplayRemoveCard)
	{
		sdkPaypassSetDisplayRemoveCard(par->fun_setdisplayRemoveCard);
	}

	if(par && par->fun_setSendUserInterfaceRequestData)
	{
		sdkPaypassSetSendUserInterfaceRequestData(par->fun_setSendUserInterfaceRequestData);
	}

	if(par && par->fun_setSendOutParamset)
	{
		sdkPaypassSetSendOutParamset(par->fun_setSendOutParamset);
	}

	if(par && par->fun_setSendDisData)
	{
		sdkPaypassSetSendDisData(par->fun_setSendDisData);
	}

	if(par && par->fun_setSendEndApplicationDataRecord)
	{
		sdkPaypassSetSendEndApplicationDataRecord(par->fun_setSendEndApplicationDataRecord);
	}

	if(par && par->sdkExtAid)
	{
		sdkPaypassSetAppExAidParam(par->sdkExtAid);
	}

	return EMV_OK;
}

int emv_set_trans_amount(u32 amount)
{
	u8 amtBCD[6]={0};

	sdkU32ToBcd(amtBCD, amount, sizeof(amtBCD));
	sdkEMVBaseSetTwoTransAmount(amtBCD, NULL);

	return EMV_OK;
}

int emv_set_other_amount(u32 amount)
{
	u8 amtBCD[6]={0};

	sdkU32ToBcd(amtBCD, amount, sizeof(amtBCD));
	sdkEMVBaseSetTwoTransAmount(NULL, amtBCD);

	return EMV_OK;
}

int emv_set_trans_type(u8 type)
{
	if(SDK_OK == sdkEMVBaseConfigTLV("\x9C", &type, 1))
	{
		return EMV_OK;
	}
	else
	{
		return EMV_FAIL;
	}
}

int emv_get_trans_type(u8 *type)
{
	int len;

	if(type == NULL)
	{
		return EMV_FAIL;
	}

	if(SDK_OK == sdkEMVBaseReadTLV("\x9C", type, &len))
	{
		return EMV_OK;
	}
	else
	{
		return EMV_FAIL;
	}
}

int EMV_PreProcess(u8 bOnline)
{
	FILE *fp;
	u8 fn[128]={0};
	int ret,i,size,len,retCode=SDK_EMV_NoAppSel;
	u8 buf[SZZT_AID_PARAM_LEN]={0};
	u8 *clTransLmtCDCVM,*tlvPtr,*vPrt;
	u8 termTransType=0x00;
	bool flag=false;
	EMV_AIDPARAM apar={0};
	u8 transAmount[6]={0};

	sdkSysGetCurAppDir(fn);
    strcat(fn, "emvaids0.par");

	fp = fopen(fn, "rb");
	if(fp == NULL)
	{
		Trace("AID", "open %s failed\r\n", fn);
		return EMV_ERR;
	}

	size = ftell(fp);
	if(size < 0)
	{
		Trace("AID", "file size invalid\r\n", fn);
		return EMV_ERR;
	}
	if((size % SZZT_AID_PARAM_LEN) != 0)
	{
		Trace("AID", "file size invalid\r\n", fn);
		return EMV_ERR;
	}

	ret = fseek(fp, 0, SEEK_SET);

	sdkEMVBaseReadTLV("\x9C", &termTransType, &len);
	sdkEMVBaseReadTLV("\x9F\x02", transAmount, &len);

	for(i = 0; i < size; i++)
	{
		memset(buf, 0, sizeof(buf));
		ret = fread(buf, sizeof(u8), sizeof(buf), fp);
		if(SZZT_AID_PARAM_LEN == ret)
		{
			tlvPtr = TlvSeek(buf+4, ret-4, TAG_009C_TRANSACTION_TYPE);
			if(tlvPtr)
			{
				vPrt = TlvVPtr(tlvPtr);
				if(vPrt && 1 == TlvLen(tlvPtr))
				{
					if(termTransType == *vPrt)
					{
						ret = emv_api_conv_aid(&apar, buf+4, ret-4);
						if(SDK_OK == ret)
						{
							if(apar->cl_trans_limit_exist)
							{
								if(memcmp(transAmount, apar->cl_trans_limit, sizeof(apar->cl_trans_limit)) > 0)
								{
									gstPaypassTradeParam->gPaypassProceFlag[i].TermCTLEF = 1;
								}
								else
								{
									flag = true;
								}
							}
							if(apar->cl_floor_limit_exist)
							{
								if(memcmp(transAmount, apar->cl_floor_limit, sizeof(apar->cl_floor_limit)) > 0)
								{
									gstPaypassTradeParam->gPaypassProceFlag[i].TermCFLEF = 1;
								}
							}
							if(apar->cvm_limit_exist)
							{
								if(memcmp(transAmount, apar->cvm_limit, sizeof(apar->cvm_limit)) > 0)
								{
									gstPaypassTradeParam->gPaypassProceFlag[i].TermCVMRLEF = 1;
								}
							}
							if(apar->cl_trans_limit_odcvm_exist)
							{
								if(memcmp(transAmount, apar->cl_trans_limit_odcvm, sizeof(apar->cl_trans_limit_odcvm)) > 0)
								{
									gstPaypassTradeParam->gPaypassProceFlag[i].OndevCTLEF = 1;
								}
								else
								{
									flag = true;
								}
							}
						}
					}
				}
				else
				{
					Trace("AID", "current AID transType does not match,select next\r\n", fn);
					fseek(fp, SZZT_AID_PARAM_LEN, SEEK_CUR);
					continue;
				}
			}
			else
			{
				Trace("AID", "current AID doesn't have transType,select next\r\n", fn);
				ret = fseek(fp, SZZT_AID_PARAM_LEN, SEEK_CUR);
				continue;
			}
		}
		else if(0 == ret)
		{
			Trace("AID", "read end of file,quit\r\n", fn);
			break;
		}
	}

	if(flag)
	{
		retCode = SDK_OK;
	}
	else
	{
		retCode = SDK_EMV_NoAppSel;
	}

	return retCode;
}


int emv_preprocess(u8 online)
{
	int ret;

	if(online)
	{
		sdkEMVBaseConfigTLV(0xFF810E, &online, 1);
	}

	ret = EMV_PreProcess(online);
	if(EMV_ERR == ret)
	{
		ret = sdkPaypassPreTrans();
	}
	else
	{
		return EMV_ENDAPPLICATION;
	}

	if(ret == SDK_OK)
	{
		return EMV_OK;
	}
	else if(ret == SDK_EMV_NoAppSel)
	{
		return EMV_AMOUNTOVER;
	}
	else
	{
		return EMV_ENDAPPLICATION;
	}
}

int emv_set_cardtype(u8 type)
{
	return EMV_OK;
}

int emv_process(void)
{
	return sdkPaypassTransFlow();
}

int emv_set_online_pin_entered(int result, u8* pinbuf, int pinlen)
{
	return sdkPaypassSetInputPINRes(result, pinbuf, pinlen);
}

int emv_set_force_online(int result)
{
	sdkEMVBaseConfigTLV(0xFF810E, (u8 *)&result, 1);
}

int emv_set_online_result(int result, u8* resp, int resplen)
{
	int ret;
	u8 *tlvPtr,*vPtr;
	u8 respCode[2]={0x30,0x30};
	u16 vLen;

	switch (result)
	{
		case EMV_ONLINE_FAIL:
			ret = SDK_ERR;
			break;
		case EMV_ONLINE_SUCC_ACCEPT:
			ret = SDK_OK;
			break;
		case EMV_ONLINE_SUCC_DENIAL:
			ret = SDK_OK;
			break;
		case EMV_ONLINE_SUCC_ISSREF:
			ret = SDK_OK;
			break;
		default:
			ret = SDK_ERR;
			break;
	}

	if(resp && resplen > 0)
	{
		tlvPtr = TlvSeek(resp, resplen, TAG_008A_ARC);
		if(tlvPtr)
		{
			vPtr = TlvVPtr(tlvPtr);
			vLen = TlvLen(tlvPtr);
			if(vPtr && 2 == vLen)
			{
				memcpy(respCode, vPtr, vLen);
				sdkPaypassImportOnlineResult(ret, respCode);
			}
			else
			{
				sdkPaypassImportOnlineResult(ret, NULL);
			}
		}
		else
		{
			sdkPaypassImportOnlineResult(ret, NULL);
		}

		tlvPtr = TlvSeek(resp, resplen, TAG_0071_ISSUER_SCRIPT_TEMPLATE_1);
		if(tlvPtr)
		{
			vPtr = TlvVPtr(tlvPtr);
			vLen = TlvLen(tlvPtr);
			if(vPtr && 256 >= vLen)
			{
				sdkEMVBaseConfigTLV("\x71", vPtr, vLen);
			}
		}

		tlvPtr = TlvSeek(resp, resplen, TAG_0072_ISSUER_SCRIPT_TEMPLATE_2);
		if(tlvPtr)
		{
			vPtr = TlvVPtr(tlvPtr);
			vLen = TlvLen(tlvPtr);
			if(vPtr && 256 >= vLen)
			{
				sdkEMVBaseConfigTLV("\x72", vPtr, vLen);
			}
		}

	}

	return EMV_OK;
}

int emv_is_need_signature(void)
{
	SDK_EMVBASE_CVM_RESULT cvmRes = sdkEMVBaseGetCVMresult();
	bool res = sdkEMVBaseNeedSignature();

	if(cvmRes == SDKEMVBASE_CVM_OBTAINSIGNATURE || res == 1)
	{
		return 1;
	}

    return 0;
}

int emv_set_termparam(u8* par, int len)
{
    FILE *fp;
	u8 fn[64]={0};

    if (par == NULL || len <= 0 || len > SZZT_AID_DATA_LEN)
    {
        return ErFUNCPAR;
    }

	sdkSysGetCurAppDir(fn);
    strcat(fn, "emvterm0.par");

	fp = fopen(fn, "rb+");
    if(NULL == fp)
    {
		Trace("TERM", "open %s failed\r\n", fn);
        return ErFOPEN;
    }

    if (fwrite((void*)&len, sizeof(int), 1, fp) != SZZT_AID_HEADER_LEN)
    {
        fclose(fp);
        return ErFWRITE;
    }

    if (fwrite(par, sizeof(u8), len, fp) != len)
    {
        fclose(fp);
        return ErFWRITE;
    }

    TraceHex("TERM", "input term param", par, len);

    fclose(fd);
    return 0;
}

int emv_clr_aidparam(void)
{
	u8 fn[64]={0};

	sdkEMVBaseDelAllAIDLists();

	sdkSysGetCurAppDir(fn);
    strcat(fn, "emvaids0.par");

	return remove(fn);
}

int emv_add_aidparam(u8* par, int len)
{
    FILE *fp, ret;
    u8 buf[SZZT_AID_PARAM_LEN]={0};
    EMV_TLVOBJ tlvs[64]={0};
    int pos,size;
    u8 aid[16];
    u8 aidlen;
	u8 fn[64]={0};

    Trace("AID", "add aid start");
    Trace("AID", "len:%d", len);
    TraceHex("AID", "par", par, len);

    if (par == NULL || len <= 0 || len > SZZT_AID_DATA_LEN)
    {
        return ErFUNCPAR;
    }

    if (EMV_TLV_Decode(par, len, tlvs, sizeof(tlvs)/sizeof(tlvs[0]), STRING_TLVOBJ))
    {
        return ILL_AIDPARAM;
    }
    if (!(pos = EMV_TLV_Find(0, TAG_9F06_AID_TERMINAL, tlvs, SEARCH_ALL_DESC)))
    {
        return ILL_AIDPARAM;
    }

    if (tlvs[pos].valuelen > 16)
    {
        return ILL_AIDLEN;
    }

	if(TlvToSzztAIDStruct(par, len) != SDK_OK)
	{
		Trace("AID", "add to SDK arch failed\r\n");
	}

    aidlen = tlvs[pos].valuelen;
    memcpy(aid, tlvs[pos].pvalue, aidlen);

	sdkSysGetCurAppDir(fn);
    strcat(fn, "emvaids0.par");

	fp = fopen(fn, "rb+");
	if(NULL == fp)
	{
		Trace("AID", "open %s failed\r\n", fn);
		return EMV_ERR;
	}

	size = ftell(fp);
	if(size < 0)
	{
		Trace("AID", "file size invalid\r\n", fn);
		return EMV_ERR;
	}
	if((size % SZZT_AID_PARAM_LEN) != 0)
	{
		Trace("AID", "file size invalid\r\n", fn);
		remove(fn);
		return EMV_ERR;
	}

	ret = fseek(fp, 0, SEEK_SET);

    while (1)
    {
        ret = fread(buf, sizeof(u8), sizeof(buf), fp);
		Trace("AID", "fread ret = %d", ret);
        if (ret == SZZT_AID_PARAM_LEN)
        {
            memcpy(&ret, buf, SZZT_AID_HEADER_LEN);
			TraceHex("AID", "read buf", buf+4, ret-4);
			Trace("AID", "read buf head: %d", ret);
            if (ret <= 0 || ret > SZZT_AID_DATA_LEN)
            {
                Trace("AID", "error: read tlv buf len invalid");
                continue;
            }
            if (EMV_TLV_Decode(buf+SZZT_AID_HEADER_LEN, ret, tlvs, sizeof(tlvs)/sizeof(tlvs[0]), STRING_TLVOBJ))
            {
                Trace("AID", "error: parse tlv error");
                continue;
            }
            if (!(pos = EMV_TLV_Find(0, TAG_9F06_AID_TERMINAL, tlvs, SEARCH_ALL_DESC)))
            {
                Trace("AID", "error: cant find AID name");
                continue;
            }

            if (aidlen == tlvs[pos].valuelen && memcmp(aid, tlvs[pos].pvalue, aidlen) == 0)	//overwrite original data
            {
                fseek(fp, -(SZZT_AID_PARAM_LEN), SEEK_CUR);
                break;
            }
            else
            {
                continue;
            }
        }
        else if (ret == 0)
        {
            break;
        }
        else
        {
            fclose(fp);
            return ErFREAD;
        }
    }

    memset(buf, 0, sizeof(buf));
    memcpy(buf, &len, SZZT_AID_HEADER_LEN);
    memcpy(buf+SZZT_AID_HEADER_LEN, par, len);
    if (fwrite(buf, sizeof(u8), SZZT_AID_PARAM_LEN, fp) != SZZT_AID_PARAM_LEN)
    {
        fclose(fp);
        return ErFWRITE;
    }

    Trace("AID", "add aid success");

    fclose(fp);
    return 0;

}

int emv_clr_capkparam()
{
	u8 fn[64]={0};

	sdkEMVBaseDelAllCAPKLists();

	sdkSysGetCurAppDir(fn);
    strcat(fn, "emvcapk0.par");

	return remove(fn);
}

int emv_add_capkparam(u8* par, int len)
{
	int ret;
	if(par == NULL || len <= 0 || len > 1020)
	{
		Trace("Download AID", "Error Code:-2\r\n");
		Trace("Download AID", "Input Param invalid\r\n");
		return SDK_ERR;
	}

	ret = TlvToCAPKStruct(par, len);
	if(ret == SDK_OK)
	{
		return 0;
	}
	else
	{
		return SDK_ERR;
	}
}

int emv_clr_cardblack(void)
{
	u8 fn[64]={0};

	sdkSysGetCurAppDir(fn);
    strcat(fn, "emvcard0.blk");

	return remove(fn);
}


int emv_add_cardblack(void* blk)
{
	EMV_CARDBLACK temp={0};
	EMV_CARDBLACK *src = (EMV_CARDBLACK *)blk;
	int ret,len,num;
	unsigned char fn[64]={0};
	FILE *fp=NULL;

	if(blk == NULL)
	{
		Trace("Download Exception File", "Error Code:-2\r\n");
		Trace("Download Exception File", "Info:Input Param invalid\r\n");
		return -1;
	}

	sdkSysGetCurAppDir(fn);
    strcat(fn, "emvcard0.blk");

	len = sdkGetFileSize(fn);
	if((len > 0) && ((len % sizeof(EMV_CARDBLACK)) != 0))
	{
		sdkDelFile(fn);
	}

	fp = fopen(fn, "rb+");
	if(NULL == fp)
	{
        return ErFOPEN;
	}

	fseek(fp, 0, SEEK_SET);

	while (1)
	{
		memset((void *)&temp, 0, sizeof(EMV_CARDBLACK));
		ret = fread((void *)&temp, sizeof(EMV_CARDBLACK), 1, fp);
		if(ret == sizeof(EMV_CARDBLACK))
		{
			if((src->pan_len == temp.pan_len) && (!memcmp(src->pan, temp.pan, src->pan_len)) && (src->psn == temp.psn))
			{
				Trace("Download Exception File", "Exception File Already Exist\r\n");
				return 0;
			}
			else
			{
				fseek(fp, sizeof(EMV_CARDBLACK), SEEK_CUR);
			}
		}
		else if(0 == ret)
		{
			break;
		}
		else
		{
			fclose(fp);
			Trace("Download Exception File", "Info:Read File len invalid\r\n");
			return ErFREAD;
		}
	}

	fseek(fp, 0, SEEK_END);
	ret = fwrite((void *)src, sizeof(EMV_CARDBLACK), 1, fp);
	if(ret != sizeof(EMV_CARDBLACK))
	{
		fclose(fp);
        return ErFWRITE;
	}

	fclose(fp);
	return 0;
}

int emv_add_certblack(void *blk)
{
	EMV_CERTBLACK temp;
	EMV_CERTBLACK *src = (EMV_CERTBLACK *)blk;
	int ret,len;
	unsigned char fn[64];
	FILE *fp=NULL;

	if(blk == NULL)
	{
		Trace("Download Revocation Cert", "Error Code:-2\r\n");
		Trace("Download Revocation Cert", "Info:Input Param invalid\r\n");
		return -1;
	}

	sdkSysGetCurAppDir(fn);
    strcat(fn, "emvcert0.blk");

	len = sdkGetFileSize(fn);
	if((len > 0) && ((len % sizeof(EMV_CARDBLACK)) != 0))
	{
		sdkDelFile(fn);
	}

	fp = fopen(fn, "rb+");
	if(NULL == fp)
	{
        return ErFOPEN;
	}

	fseek(fp, 0, SEEK_SET);

	while (1)
	{
		memset((void *)&temp, 0, sizeof(EMV_CARDBLACK));
		ret = fread((void *)&temp, sizeof(EMV_CARDBLACK), 1, fp);
		if(ret == sizeof(EMV_CARDBLACK))
		{
			if((src->pki == temp.pki) && \
				(!memcmp(src->rid, temp.rid, sizeof(temp.rid))) && \
				(!memcmp(src->csn, temp.csn, sizeof(temp.csn))))
			{
				Trace("Download Revocation Cert", "Revocation Cert Already Exist\r\n");
				return 0;
			}
			else
			{
				fseek(fp, sizeof(EMV_CERTBLACK), SEEK_CUR);
			}
		}
		else if(0 == ret)
		{
			break;
		}
		else
		{
			fclose(fp);
			Trace("Download Revocation Cert", "Info:Read File len invalid\r\n");
			return ErFREAD;
		}
	}

	fseek(fp, 0, SEEK_END);
	ret = fwrite((void *)src, sizeof(EMV_CERTBLACK), 1, fp);
	if(ret != sizeof(EMV_CERTBLACK))
	{
		fclose(fp);
        return ErFWRITE;
	}

	fclose(fp);
	return 0;
}

int emv_clr_certblack(void)
{
	int fp,ret,len;
	unsigned char fn[64];

	sdkSysGetCurAppDir(fn);
    strcat(fn, "Revokey");

	ret = sdkDelFile(fn);
	if(ret == SDK_OK)
	{
		return 0;
	}
	else
	{
		Trace("Delete Revocation Cert", "Error Code:-7\r\n");
		Trace("Delete Revocation Cert", "Info:Remove File Error\r\n");
		return -1
	}
}
