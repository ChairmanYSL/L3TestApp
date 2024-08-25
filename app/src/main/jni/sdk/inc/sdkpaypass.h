#ifndef SDKPAYPASS_H
#define SDKPAYPASS_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include"sdkpaypassprivate.h"

extern void sdkPayPassSetSupportDE(unsigned char mode);//0:not support ,1:support
extern unsigned char sdkPayPassGetSupportDEstatus(void);//0:not support ,1:support
extern s32 sdkPayPassManageInitdekdet(void);
extern s32 sdkPayPassManageSetdekdetParam(unsigned char *dekdetdata, unsigned int dekdetdatalen);
extern s32 sdkPayPassManageGetDE_Apdu_LOG(unsigned char *dekdetdata, int *dekdetdatalen);
extern	s32 sdkPaypassSetRfCardPowerOff(s32(*fun_setRfCardPowerOff)());
extern	s32 sdkPaypassSetDisplayRemoveCard(s32(*fun_setdisplayRemoveCard)());
extern	s32 sdkPaypassSetBeforeGPO(s32(*fun_setbeforegpo)());
extern	s32 sdkPaypassSetRevocationKey(s32(*fun_cmprevocation)(u8 *key));
extern	s32 sdkPaypassTransInit();
extern	s32 sdkPaypassPreTrans();
extern	s32 sdkPaypassImportOnlineResult(s32 ucOnlineResult, u8 *pheRspCode);
extern  s32 sdkPaypassTransFlow1();
extern  s32 sdkPaypassTransFlow2();
extern  s32 sdkPaypassTransFlow();
extern  void sdkPaypassSetBeforeGPORes(int res);
extern 	void sdkPaypassSetVerifyRevocationKeyRes(int res);
extern int sdkPaypassSetInputPINRes(int res, unsigned char *pinBuf, int bufLen);
extern s32 sdkPaypassGetLibVerson(u8 *version);

extern u8 sdkPaypassSetRefundrequestAAC(u8 enable);
extern void sdkPayPassSupportSaveDekLogSet(void);
extern void sdkPayPassSupportSaveDekLogClear(void);
extern u8 sdkPaypassSetCertificationStatus(u8 enable);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
