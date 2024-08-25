#ifndef SDKEMVBASEPRIVATE_H
#define SDKEMVBASEPRIVATE_H

#include "dllemvbase.h"
#include "sdkemvbase.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define SDKTINYQUICKPASS		// only support quickpass online ,no aid,no capk
#undef SDKTINYQUICKPASS


#define SDK_EMVBASE_MAX_PATH 64

typedef struct
{
	s32 (*InputPWD)(u8 ucIccEncryptWay, u8 ucPINTryCount, u8 *CreditPwd);//SDK_OK:正常输密;SDK_PED_NOPIN:bypass; SDK_ERR:失败
	EMVBase_termipkrevokecheck termipkrevokecheck;//SDK_OK:不是回收公钥; SDK_ERR:是回收公钥
	unsigned char CheckTag84;//bit0: tag_84 forced exist; bit1: tag_84 check length==14; bit2: tag_84 check data=="2PAY.SYS.DDF01" //sjz20200402 add
	unsigned char SupportExternSelect;//support 9f29 extern select aid; sjz20200408 add
	unsigned char PpseRespType;//0-默认; 1-JCB
	unsigned char SupportSPICommand; // 20210708  for SEND POI INFORMATION (SPI) Command (SB 230)
	//SDK_EMVBASE_TRANSTYPE tradetype;
}SDK_EMVBASE_TRADE_PARAM;

typedef struct
{
	unsigned char SelectedAppNo;
	EMVBASE_LISTAPPDATA *SelectedApp;
	EMVBASE_LISTAPPDATA *AppListCandidate;

	unsigned char AppListCandidatenum;
	unsigned char AppListCandidateMaxNum;
}SDK_EMVBASE_CONTACT_PARAM;


extern EMVBase_UnionStruct *gstEMVBase_UnionStruct;
extern EMVBase_EntryPoint *gstEMVBase_EntryPoint;
extern SDK_EMVBASE_TRADE_PARAM *gstEMVBase_TradeParam;
extern u8 gAppListCandicateMaxNum;//默认8个


extern SDK_EMVBASE_CONTACT_PARAM *gstEMVBase_ContactParam;


extern u8 gstemvbaseforL2TEST;		//for L2 test if anything not same as commercial
extern u8 gstemvbaseneedsign;		//cvm need sign
extern SDK_EMVBASE_CVM_RESULT gstemvbaseCVMresult;
extern s32 sdkEMVBaseRltToSdkRlt(u8 ucInBuf, s32 *puiOutBuf);
//extern s32 sdkEMVBaseSetForL2Test(BOOL enable);		//default is false
extern int sdkEMVBase_CheckMatchTermAID_CL(unsigned char* aid,unsigned char aidLen,EMVBASE_TERMAPP *Applist,unsigned char* kernelid,unsigned char kernelidlen);
extern int sdkEMVBase_CheckMatchTermAID_CT(unsigned char* aid,unsigned char aidLen,EMVBASE_TERMAPP *Applist,unsigned char* kernelid,unsigned char kernelidlen);
extern void sdkEMVBase_ReadTermAID_CL(EMVBASE_TERMAPP * Applist, unsigned char *TermApplistNum);
extern void sdkEMVBase_ReadTermAID_CT(EMVBASE_TERMAPP * Applist, unsigned char *TermApplistNum);
extern void sdkDev_Printf(char *fmt, ...);
extern void sdkEMVBase_ReadCAPK(u8 *RID, u8 capki, EMVBASE_CAPK_STRUCT * tmpcapk);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif

