#ifndef _SDKPAYPASS_OUTCOME_H_
#define _SDKPAYPASS_OUTCOME_H_


#include "dllpaypass.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


#define TIMER500MSVALUE   500
#define TIMERINTERVAL     1


#ifdef PAYPASS_OUTCOME_SUPPORT_FLAG
extern PAYPASS_USERINTERFACEREQDATA gPaypassUserInterfaceReqData;
#endif
extern PAYPASS_OUTPARAMETERSET gPaypassOutParameterSet;
extern PAYPASS_OUTCOME_ERRORID gstPaypassOutComeErrID;
extern PAYPASS_PHONEMESSAGETABLEINFO gPaypassPhoneMessageTableInfo;
extern PAYPASS_APDU_ERRORID gPaypassApduErrorID;


extern void EMVBaseU32ToBcd(unsigned char *Bcd, unsigned int const Src, unsigned char Len);
extern void EMVBaseBcdToAsc(unsigned char *Dest, unsigned char *Src, unsigned short Len);
extern unsigned char EMVBaseStrPos(unsigned char *Src, unsigned char chr);

u8 sdkPaypassOutComeformsendpaypasspredata(u8 *carddata, u32 *carddatalen);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */



#endif

