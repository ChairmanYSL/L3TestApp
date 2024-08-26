#include "appglobal.h"
#include "sdkGlobal.h"

#include <sys/stat.h>
#include <unistd.h>
#include "devApi.h"
#include <fcntl.h>
#include <dlfcn.h>
#include "sdkmaths.h"

#include <time.h>

#include <android/log.h>

#define  LOG_TAG    "EMVPAYPASSCORE"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

extern PAYPASS_APDU_ERRORID gPaypassApduErrorID;
extern DDI_API gstddiapi;
extern bool sdkIsBcdNum(u8 const *pheSrc, s32 siSrclen);


s32 sdkemvbaseInitDDI(void)
{
	void* pHandler;

	pHandler = dlopen("libAsdkClient_8623.so", RTLD_LAZY);
	if(pHandler == NULL)
	{
//		Trace("DDI", "ddi-fatal:open libAsdkClient_8623.so fail\n");
		pHandler = dlopen("/system/lib/libAsdkClient.so", RTLD_LAZY);
		if(pHandler == NULL)
		{
			Trace("DDI", "ddi-fatal:open /system/lib/libAsdkClient.so fail\n");
			return -1;
		}
		Trace("DDI", "load libAsdkClient_8623.so success\n");
	}

	gstddiapi.ddi_apdu_exchange = dlsym(pHandler,"API_ICC_Apdu");
	gstddiapi.ddi_get_random= dlsym(pHandler,"API_Rng");
	gstddiapi.ddi_get_timerId= dlsym(pHandler,"API_TimeGet");
	gstddiapi.ddi_encry = dlsym(pHandler,"API_Crypt");
	gstddiapi.ddi_hash = dlsym(pHandler,"API_Hash");
	gstddiapi.ddi_get_systick = dlsym(pHandler, "API_GetSysTick");
	return 0;
}

u32 sdkTimerGetId()
{
	u32 time = (u32)gstddiapi.ddi_get_systick();

	Trace("extern", "get time id:%d\r\n", time);
	return time;
}

u32 sdkTimerGetIdtemp()
{
    struct timeval t0;
    gettimeofday(&t0, NULL);
	Trace("extern", "get time id:%d\r\n", (t0.tv_sec * 1000 + t0.tv_usec / 1000));
    return t0.tv_sec * 1000 + t0.tv_usec / 1000;
}

bool sdkTimerIsEnd(u32 uiId, u32 uiMs)
{
    u32 lCurid = 0;

    lCurid = sdkTimerGetId();

    if(lCurid < uiId)
    {
        return true;
    }

    if(lCurid - uiId < uiMs)
    {
        return false;
    }
    else
    {
        return true;
    }
}

int sdkGetRtc(unsigned char *pbcDest)
{
    if(NULL == pbcDest)
    {
        return SDK_PARA_ERR;
    }
    ddi_sys_get_time(pbcDest);

    if(sdkIsBcdNum(pbcDest, 6))
    {
        return SDK_OK;
    }
    else
    {
        return SDK_PARA_ERR;
    }
}

void sdkmSleep(const int siMs)
{
	usleep(siMs);
}

int sdkGetRandom(unsigned char *pheRdm, int siNum)
{
	gstddiapi.ddi_get_random(pheRdm, siNum);
	TraceHex("ddi", "random:", pheRdm, siNum);
    return SDK_OK;
}

void *sdkGetMem(unsigned int size)
{
	return ddi_k_malloc(size);
}

int sdkFreeMem(void *ap )
{
    ddi_k_free(ap);
    return 1;
}

s32 sdkSysGetCurAppDir(u8 *pasData)
{
    if(NULL == pasData)
    {
        return SDK_PARA_ERR;
    }
    strcpy(pasData, "/data/local/config/app/");
    return SDK_OK;
}

bool sdkAccessFile(const u8 *pasFile)
{
    if (NULL == pasFile)
	{
        return false;
    }

    if (DDI_OK == access(pasFile, F_OK|R_OK|W_OK))
	{
        return true;
    }

    return false;
}

s32 sdkWriteFile(const u8 *pasFile, const u8 *pheSrc, s32 siSrclen)
{
    FILE *fp;
	s32 ret;

    if (NULL == pasFile || NULL == pheSrc)
	{
        return SDK_PARA_ERR;
    }

    fp = fopen(pasFile, "rb+");
    if (fp == NULL)
	{
        return SDK_ERR;
    }

	ret = fwrite(pheSrc, sizeof(u8), siSrclen, fp);
    if (ret != siSrclen)
	{
        fclose(fp);
        return SDK_ERR;
    }
    else
	{
        fclose(fp);
        return SDK_OK;
    }

}

s32 sdkReadFile(const u8 *pasFile, u8 *pheDest, s32 siOffset, s32 *psiDestlen)
{
//	s32 fp=0;
	FILE *fp = NULL;
	u32 i;

    if (NULL == pasFile || NULL == pheDest || NULL == psiDestlen || siOffset < 0)
	{
        return SDK_PARA_ERR;
    }

	fp = fopen(pasFile, "rb");
	if(fp == NULL)
	{
		return SDK_FUN_NULL;																						//�ļ���ʧ��
	}

    if( 0 != fseek(fp, siOffset, SEEK_SET) )
    {
        fclose(fp);
        return SDK_ERR;
    }

	i = *psiDestlen;
	*psiDestlen = fread(pheDest, sizeof(u8), i, fp);
	if(*psiDestlen != i)
	{
		fclose(fp);
		return SDK_ERR;
	}
	else
	{
		fclose(fp);
		return SDK_OK;
	}
}

s32 sdkInsertFile(const u8 *pasFile, const u8 *pheSrc, s32 siStart, s32 siSrclen)
{
	s32 i,ret;
	FILE *fp;

    if (NULL == pasFile || NULL == pheSrc || siStart < 0 || siSrclen < 0) {
		Trace("ddi", "Input Param invalid\r\n");
		return SDK_PARA_ERR;
    }

    if (siStart != 0)
	{
        i = sdkGetFileSize(pasFile);
        if (siStart > i && i > 0)
        {
            return SDK_PARA_ERR;
   		}
    }
	Trace("ddi", "want open file:%s", pasFile);
	fp = fopen(pasFile, "rb+");
	if((NULL == fp) && (sdkGetFileSize(pasFile) <= 0))//file doesn't exist
	{
		fp = fopen(pasFile, "wb+");
		if(fp == NULL)
		{
			Trace("ddi", "open file error\r\n");
			return SDK_ERR;
		}
		else
		{
			fclose(fp);
			fp = fopen(pasFile, "rb+");
		}
	}
	fseek(fp, 0, SEEK_END);
	i = ftell(fp);
	if (siStart > i)
	{
		fclose(fp);
		return SDK_PARA_ERR;
	}
	fseek(fp, siStart, SEEK_SET);

	ret = fwrite(pheSrc, sizeof(u8), siSrclen, fp);
	if(ret == siSrclen)
	{
		fclose(fp);
		return SDK_OK;
	}
	else
	{
		fclose(fp);
		return SDK_ERR;
	}
}

s32 sdkGetFileSize(const u8 *pasFile)
{
	s32 fileSize;
	s32 fp;

	if (NULL == pasFile)
	{
		return SDK_PARA_ERR;
	}

	fp = fopen(pasFile, "rb+");
	if(NULL == fp)
	{
		return SDK_FUN_NULL;																						//�ļ���ʧ��
	}

	fseek(fp, 0, SEEK_END);

	fileSize = ftell(fp);

	fclose(fp);

	return fileSize;
}

s32 sdkDelFile(const u8 *pasFile)
{
	s32 ret;

	if(pasFile == NULL)
	{
		return SDK_PARA_ERR;
	}

	ret = remove(pasFile);
	if(ret == 0)
	{
		return SDK_OK;
	}
	else
	{
		return SDK_ERR;
	}
}

s32 sdkDevContactlessSendAPDU(const u8 *pheInBuf, s32 siInLen, u8 *pheOutBuf, s32 *psiOutLen, u32 *sw)
{
    s32 rslt = 0,i = 0,mask_len;
	s32 size = 300;
	s32 key = 0;
	u8 *tmp;

	#ifdef PAYPASS_DATAEXCHANGE
		unsigned short resvdateLen=0;
	#endif

	#ifdef PAYPASS_DATAEXCHANGE
		resvdateLen = (u16)siInLen;
		sdkPaypassDEsaveSendApdulog(pheInBuf,resvdateLen);
	#endif


    if((NULL == pheInBuf) || (NULL == pheOutBuf) || (NULL == psiOutLen) || siInLen < 0)
    {
        return SDK_PARA_ERR;
    }

	tmp = (u8 *)sdkGetMem(siInLen);
	memset(tmp, 0, siInLen);
	memcpy(tmp, pheInBuf, siInLen);
	while (i < siInLen)
	{
		if(0x57 == tmp[i])
		{
			mask_len = tmp[i+1];
			if(mask_len > 0 && mask_len <= 19)
			{
				memset(&(tmp[i+2]), 0xFF, mask_len);
				i++;
				i += mask_len;
			}
			else
			{
				i++;
			}
		}
		else if(0x5A == tmp[i])
		{
			mask_len = tmp[i+1];
			if(mask_len > 0 && mask_len <= 10)
			{
				memset(&(tmp[i+2]), 0xFF, mask_len);
				i++;
				i += mask_len;
			}
			else
			{
				i++;
			}
		}
		else
		{
			i++;
		}
	}

	TraceHex("apdu", "pheInBuf:", tmp, siInLen);
//    TraceHex("emv", "contactless c-apdu:", pheInBuf, siInLen);


	rslt = gstddiapi.ddi_apdu_exchange(2, pheInBuf, (u32)siInLen,pheOutBuf, size, (u32 *)psiOutLen, sw);
	Trace("extern", "ddi_apdu_exchange ret = %d\r\n", rslt );

	#ifdef PAYPASS_DATAEXCHANGE
		if( (*psiOutLen) >= 0 )
		{
			resvdateLen = (u16)(*psiOutLen);
			sdkPaypassDEsaveRecvApdulog(pheOutBuf,resvdateLen);
		}
	#endif

	gPaypassApduErrorID.L1 = EMVB_ERRID_L1_OK;
	gPaypassApduErrorID.L3 = EMVB_ERRID_L3_OK;
	if(rslt != DDI_OK)
	{
		gPaypassApduErrorID.L1 = EMVB_ERRID_L1_TIMEOUTERR;
	}
	else
	{
		if(*psiOutLen >= 2)
		{
			gPaypassApduErrorID.SW12[0] = pheOutBuf[*psiOutLen - 2];
			gPaypassApduErrorID.SW12[1] = pheOutBuf[*psiOutLen - 1];
		}
	}

    if(rslt == DDI_OK)
    {
		if(*psiOutLen == 0 && sw != 0)
		{
			TraceHex("emv", "contactless r-apdu:", pheOutBuf, 2);
		}
		else
		{
			if(*psiOutLen != 0)
			{
				tmp = (u8 *)sdkGetMem(*psiOutLen);
				memset(tmp, 0, *psiOutLen);
				memcpy(tmp, pheOutBuf, *psiOutLen);
				i = 0;
				while (i < *psiOutLen)
				{
					if(0x57 == tmp[i])
					{
						mask_len = tmp[i+1];
						if(mask_len > 0 && mask_len <= 19)
						{
							memset(&(tmp[i+2]), 0xFF, mask_len);
							i++;
							i += mask_len;
							Trace("emv", "after mask 57 index point: %d\r\n", i);
							Trace("emv", "mask 57 len: %d\r\n", mask_len);
						}
						else
						{
							i++;
						}
					}
					else if(0x5A == tmp[i])
					{
						Trace("emv", "mask 5A index: %d\r\n", i);
						mask_len = tmp[i+1];
						if(mask_len > 0 && mask_len <= 10)
						{
							memset(&(tmp[i+2]), 0xFF, mask_len);
							i++;
							i += mask_len;
						}
						else
						{
							i++;
						}
					}
					else
					{
						i++;
					}
				}

				TraceHex("emv", "contactless r-apdu:", tmp, *psiOutLen);
			}
//			TraceHex("emv", "contactless r-apdu:", pheOutBuf, *psiOutLen);
			sdkFreeMem(tmp);
			tmp = NULL;
		}
//    	TraceHex("emv", "contactless r-apdu:", pheOutBuf, *psiOutLen);
        return SDK_OK;
    }
    else if( rslt == DDI_EINVAL )
    {
        return SDK_PARA_ERR;
    }
    else if( rslt == DDI_ETIMEOUT )
    {
        return SDK_TIME_OUT;
    }
    else
    {
        return SDK_ERR;
    }

    return SDK_ERR;
}

void sdkDev_Printf(char *fmt, ...)
{
	va_list args;
	unsigned char printbuffer[256] = {0};

	va_start ( args, fmt );
	vsprintf ( printbuffer, fmt, args );
	va_end ( args );
	LOGD("%s",printbuffer);
}

int sdkReadPosSn(unsigned char *pasDest)
{
    if(pasDest == NULL)
    {
        return SDK_PARA_ERR;
    }
    return 0;
}

