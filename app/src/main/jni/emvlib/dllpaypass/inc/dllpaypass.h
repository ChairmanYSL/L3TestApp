#ifndef _DLLPAYPASS_H_
#define _DLLPAYPASS_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include "dllemvbase.h"


#define PAYPASSV303

#define PAYPASS_DATAEXCHANGE	//20160426_lhd
#define PAYPASS_RRP				//20160606_LHD

#define	PAYPASS_OUTCOME_SUPPORT_FLAG	//20160606_LHD
//#undef PAYPASS_OUTCOME_SUPPORT_FLAG


#define PAYPASS_L2_CERT_USE
//#undef  PAYPASS_L2_CERT_USE


#ifdef PAYPASS_DATAEXCHANGE
#define PAYPASS_DE_LIST_BUF_MAX_LEN			(1200)
#define DEKDETSENDDATALEN					(1200)
#define XML_FILE_DEK_LEN					(1024)
#define XML_FILE_DET_LEN					(1024)
#endif


//dol
#define typeCDOL1       1
#define typeCDOL2       2
#define typeUDOL        3
#define typeTDOL        4
#define typeDRDOL       5
#define typeDSDOL       6


//9C
#define EMVBASE_TRANS_GOODS  	0x00
#define EMVBASE_TRANS_CASH   	0x01
#define EMVBASE_TRANS_CASHBACK 	0x09
#define EMVBASE_TRANS_CASHDISBURSEMENT			0x17	//20160920_lhd add
#define EMVBASE_TRANS_REFUND  	0x20


//emvcontact response start form 51
#define PAYPASS_READAIDPARAMETERS	51
#define PAYPASS_EXTRACTMATCH			52
#define PAYPASS_PARTICALMATCH		53
#define PAYPASS_NOTMATCH 			54
#define PAYPASS_ERR_CVMFailALWAYS	55
#define PAYPASS_ERR_CVMNoSupport		56
#define PAYPASS_ERR_CVMFail			57
#define PAYPASS_ERR_NEEDMAGCARD		58
#define PAYPASS_ERR_EMV_CDAFORMAT	59//CDA TC
#define PAYPASS_ERR_EMV_CDADifferCID	60
#define PAYPASS_ERR_EMV_CDADifferCIDTC	61
#define PAYPASS_ERR_EMV_CDADifferCIDAAC	62	//20190530_lhd
#define PAYPASS_ERR_EMV_ScriptFormat	63
#define PAYPASS_EMV_NEEDREVERSAL		64
#define PAYPASS_ERR_PBOCLOAD			65
#define PAYPASS_GAC_REQUEST_TAP_AGAIN		66	//FOR CDCVM

#define PAYPASS_REQ_SETCALLBACK_INPUTPIN 		67
#define PAYPASS_REQ_SETCALLBACK_EXCEPTIONFILE 	68
#define PAYPASS_REQ_SETCALLBACK_REVOKEY			69


#define PAYPASS_DLL_CCCTIMER	(600) // 500


#define PAYPASSSTEP_S51_11_0    51110
#define PAYPASSSTEP_S51_12_0    51120
#define PAYPASSSTEP_S52_5       5205
#define PAYPASSSTEP_S52_7       5207
#define PAYPASSSTEP_S52_8       5208
#define PAYPASSSTEP_S52_9       5209
#define PAYPASSSTEP_S52_11      5211
#define PAYPASSSTEP_S52_13      5213
#define PAYPASSSTEP_S52_14      5214
#define PAYPASSSTEP_S52_18      5218
#define PAYPASSSTEP_S52_19      5219
#define PAYPASSSTEP_S53_5       5305
#define PAYPASSSTEP_S53_7       5307
#define PAYPASSSTEP_S53_8       5308
#define PAYPASSSTEP_S53_13      5313
#define PAYPASSSTEP_S53_14      5314
#define PAYPASSSTEP_S53_15      5315
#define PAYPASSSTEP_S1_7_0      1070
#define PAYPASSSTEP_S1_7_1      1071
#define PAYPASSSTEP_S1_8_0      1080
#define PAYPASSSTEP_S3_5_0      3050
#define PAYPASSSTEP_S3_9_1      3091
#define PAYPASSSTEP_S3_9_2      3092
#define PAYPASSSTEP_S3_12_0     3120
#define PAYPASSSTEP_S3_14_0     3140
#define PAYPASSSTEP_S3_90_1     3901
#define PAYPASSSTEP_S3_90_2     3902
#define PAYPASSSTEP_S3_E41_0  103400
#define PAYPASSSTEP_S3_E18_0  103180
#define PAYPASSSTEP_S4_E4_0   104040
#define PAYPASSSTEP_S4_E5_0   104050
#define PAYPASSSTEP_S4_E6_0   104060
#define PAYPASSSTEP_S4_E10_1  104101
#define PAYPASSSTEP_S4_E10_2  104102
#define PAYPASSSTEP_S4_E13_0  104130
#define PAYPASSSTEP_S4_E15_0  104152
#define PAYPASSSTEP_S4_E27_1  104271
#define PAYPASSSTEP_S4_E27_2  104272
#define PAYPASSSTEP_S5_E17_1  105171
#define PAYPASSSTEP_S5_E17_2  105172
#define PAYPASSSTEP_S5_E27_1  105271
#define PAYPASSSTEP_S5_E27_2  105272
#define PAYPASSSTEP_S5_E31_0  105310
#define PAYPASSSTEP_S6_3	  106300
#define PAYPASSSTEP_S7_8        7080
#define PAYPASSSTEP_S16_E1_0  116010
#define PAYPASSSTEP_S16_E2_0  116020
#define PAYPASSSTEP_S16_E3_0  116030
#define PAYPASSSTEP_CVM_E4_0  600040
#define PAYPASSSTEP_CVM_E8_0  600080
#define PAYPASSSTEP_CVM_E18_0 600180
#define PAYPASSSTEP_CVM_E18_1 600181
#define PAYPASSSTEP_S78_20_0   78200
#define PAYPASSSTEP_S13_4_0    13040
#define PAYPASSSTEP_S13_5_0    13050
#define PAYPASSSTEP_S13_10_0   13100
#define PAYPASSSTEP_S13_13_0   13130
#define PAYPASSSTEP_S13_12_1   13121
#define PAYPASSSTEP_S13_14_4   13144
#define PAYPASSSTEP_S13_42_0   13420
#define PAYPASSSTEP_S13_44_0   13440
#define PAYPASSSTEP_S13_43_0   13430
#define PAYPASSSTEP_S13_45_0   13450
#define PAYPASSSTEP_S13_32_0   13320
#define PAYPASSSTEP_S13_33_0   13330
#define PAYPASSSTEP_S13_23_0   13230
#define PAYPASSSTEP_S13_25_0   13250
#define PAYPASSSTEP_S13_26_0   13260
#define PAYPASSSTEP_S14_21_1   14211
#define PAYPASSSTEP_S14_33_0   14330
#define PAYPASSSTEP_S14_34_0   14340
#define PAYPASSSTEP_S15_E10    11510
#define PAYPASSSTEP_S16_E7     11670
#define PAYPASSSTEP_S10_E5_0  110050
#define PAYPASSSTEP_S10_E6_0  110060
#define PAYPASSSTEP_S10_E14_0 110140
#define PAYPASSSTEP_S10_E16_0 110160
#define PAYPASSSTEP_S10_E18_0 110180
#define PAYPASSSTEP_S10_E50_0 110500
#define PAYPASSSTEP_S10_E53_0 110530
#define PAYPASSSTEP_S9_ED7_0    109070
#define PAYPASSSTEP_S9_ED8_0    109080
#define PAYPASSSTEP_S9_8	    109081	//20191123_lhd add
#define PAYPASSSTEP_S910_ED9_0  109091
#define PAYPASSSTEP_S910_ED11_0 109011
#define PAYPASSSTEP_S910_ED19_0 109019
#define PAYPASSSTEP_S910_ED52_0 109052
#define PAYPASSSTEP_S910_ED53_0 109053
#define PAYPASSSTEP_S9_E9_0   109090
#define PAYPASSSTEP_S9_E10_0  109100
#define PAYPASSSTEP_S9_E14_0  109140
#define PAYPASSSTEP_S9_E15_0  109150
#define PAYPASSSTEP_S9_E17_0  109170
#define PAYPASSSTEP_S10_E20_0 110200
#define PAYPASSSTEP_S10_E22_0 110220
#define PAYPASSSTEP_S10_E24_0 110240
#define PAYPASSSTEP_S9_E27_0  109270
#define PAYPASSSTEP_S11_E46_0 111460
#define PAYPASSSTEP_S11_E77_0 111770
#define PAYPASSSTEP_S11_E90_0 111900
#define PAYPASSSTEP_S11_E95_0 111950
#define PAYPASSSTEP_S11_E110_0 1111100
#define PAYPASSSTEP_S11_E112_0 1111120
#define PAYPASSSTEP_S11_E113_0 1111130
#define PAYPASSSTEP_S11_E114_1 1111141
#define PAYPASSSTEP_S11_E114_2 1111142
#define PAYPASSSTEP_S11_E114_3 1111143
#define PAYPASSSTEP_S11_E114_4 1111144
#define PAYPASSSTEP_S11_E114_5 1111145
#define PAYPASSSTEP_S11_E115_1 1111151
#define PAYPASSSTEP_S11_E115_2 1111152
#define PAYPASSSTEP_S11_E115_3 1111153
#define PAYPASSSTEP_S11_E115_4 1111154
#define PAYPASSSTEP_S11_E115_5 1111155
#define PAYPASSSTEP_S11_E115_6 1111156
#define PAYPASSSTEP_S11_E115_7 1111157
#define PAYPASSSTEP_S11_E115_8 1111158
#define PAYPASSSTEP_S11_E115_9 1111159
#define PAYPASSSTEP_S11_E119_0 1111190
#define PAYPASSSTEP_S11_E120_1 1111201
#define PAYPASSSTEP_S11_E120_2 1111202
#define PAYPASSSTEP_S11_E120_3 1111203
#define PAYPASSSTEP_SGAC_ED6   1111301
#define PAYPASSSTEP_SGAC_ED11  1111302
#define PAYPASSSTEP_SGAC_ED20  1111303	//for case 3MX4-5900-01


#define qPBOC_RESTARTRANDE       118
#define PAYPASS_CCCWAIT          122
#define PAYPASS_PREDEALFAILED    123
#define PAYPASS_ENDAPPLICATION   124
#define PAYPASS_ERRICCRETURN     125
#define PAYPASS_CCCERR           126
#define PAYPASS_CCCWAITERR       127
#define PAYWAVE_DETECTEDOTHERMODE 128

//#define qPBOC_RESETCOMMAND        171
#define PAYPASS_USER_CANCELL        171


#define PAYPASS_READAFTERBALANCEOK        180  //jiguang
#define PAYPASS_RESELECTAPP       181  //jiguang add
#define PAYPASS_DEALWAIT           183
#define PAYPASS_PARSEERR 		184
#define PAYPASS_CARDDATAMISSING 185
#define PAYPASS_CARDDATAERR     186
#define PAYPASS_SW1SW2ERR       187
#define PAYPASS_PHONECCCPCIIONLYE 188
#define PAYPASS_DECLINED           189

#define PAYPASS_RECVDEFWF_TIMEOUT			195
#define PAYPASS_SR1_STOP					196
#define PAYPASS_L2_IDS_READ_ERROR			202	//197	//20160704_lhd
#define PAYPASS_L2_IDS_WRITE_ERROR			203	//198	//20160704_lhd
#define PAYPASS_CDA_ERROR_RET_FOR_D			199	//20160704_lhd
#define PAYPASS_CDA_ERROR_RET_FOR_E			200

#define EMVB_ERRID_L1_OK            0
#define EMVB_ERRID_L1_TIMEOUTERR    1
#define EMVB_ERRID_L1_TRANSERR     2
#define EMVB_ERRID_L1_PROTOERR     3


#define EMVB_ERRID_L2_SUCCESS         0
#define EMVB_ERRID_L2_CARDDATAMISSING 1
#define EMVB_ERRID_L2_CAMFAILED       2
#define EMVB_ERRID_L2_STATUSBYTES     3
#define EMVB_ERRID_L2_PARSINGERROR    4
#define EMVB_ERRID_L2_MAXLIMITEXCEEDED 5
#define EMVB_ERRID_L2_CARDDATAERR     6
#define EMVB_ERRID_L2_MAGNOTSUP       7
#define EMVB_ERRID_L2_NOPPSE          8
#define EMVB_ERRID_L2_PPSEFAULT       9
#define EMVB_ERRID_L2_EMPTYCANLIST    10
#define EMVB_ERRID_L2_IDSREADERR      11
#define EMVB_ERRID_L2_IDSWRITEERR     12
#define EMVB_ERRID_L2_IDSDATAERR      13
#define EMVB_ERRID_L2_IDSNOMATCHAC    14
#define EMVB_ERRID_L2_TERMINALDATAERR 15


#define EMVB_ERRID_L3_OK               0x00
#define EMVB_ERRID_L3_TIMEOUT          0x01
#define EMVB_ERRID_L3_STOP             0x02
#define EMVB_ERRID_L3_AMOUNTNOTPRESENT 0x03
#define EMVB_ERRID_L3_CARDMAGINSERT    0x04



typedef struct _PAYPASSDLLPREPROCEFLAG
{
    unsigned char TermCTLEF : 1;      //terminal contactless transaction limit exceeded flag
    unsigned char TermCFLEF : 1;     //terminal contactless floor limit exceed flag
    unsigned char TermCVMRLEF : 1;   //terminal CVM required limit exceeded flag
    unsigned char OndevCTLEF : 1;
} PAYPASS_DLL_PREPROCEFLAG;


typedef struct _PAYPASSDLLTRADEPARAMETER
{
    EMVBASE_CAPK_STRUCT *CAPK;
    EMVBASE_LISTAPPDATA *SelectedApp;
    EMVBASE_LISTAPPDATA *AppListCandidate;

    PAYPASS_DLL_PREPROCEFLAG *gPaypassProceFlag;

    unsigned char *AuthData;
    unsigned char *IPKModul;
    unsigned char *ICCPKModul;
    unsigned char *RedundantData;
    unsigned short AuthDataMaxLen;
    unsigned short AuthDataLen;
    unsigned short RedundantDataMaxLen;
    unsigned short RedundantDataLen;
    unsigned short AuthDataFixLen;

	unsigned short Track2OffsetInAuthData;
	unsigned short Track2OffsetInCDOL1Data;
	unsigned char bTrack2InCDOL1Data;
	unsigned char bTrack2DataMasked;
	unsigned char Track2DataLen;

    unsigned char IPKModulLen;
    unsigned char ICCPKModulLen;
    unsigned char AppListCandidatenum;

    unsigned char bPrintReceipt: 1;
    unsigned char bErrSDATL: 1;
    unsigned char bErrAuthData: 1;
    unsigned char bCombineDDA: 1;
    unsigned char bCombineDDAFail: 1;
    unsigned char bDDOLHasNoUnpredictNum: 1;
    unsigned char bCDOL1HasNoUnpredictNum: 1;
    unsigned char bCDOL2HasNoUnpredictNum: 1;

    unsigned char TransResult;
    unsigned char qPBOCOrMSD;
    unsigned char SelectedAppNo;
    unsigned char onlinetradestatus;
    unsigned char PaypassTermEFLEF;
    unsigned char PaypassCVMRLEF;
    unsigned char PaypassOndevCTLEF;
    unsigned char PaypassSelectedPOSCIIID;
    unsigned char ODAstaus;
    unsigned char PaypassnUN;
    unsigned char paypassonlinepinreq;
    unsigned char GenerateACRetDataLen;
    unsigned char GenerateACRetData[255];
    unsigned char bretforminit;//L2 cert =1,
} PAYPASSDLLTRADEPARAMETER;



typedef struct _TornTransactionLogIDSPartRecord
{
    unsigned char DSSummary1Len;    //8  or 16
    unsigned char DSSummary1[16];
    unsigned char IDSStatus;
} TornTransactionLogIDSPartRecord;




typedef void (*EMVBASE_PaypassIsoCommand)(EMVBASE_APDU_SEND *ApduSend, EMVBASE_APDU_RESP *apdu_r);
typedef void (*EMVBASE_PaypassIsoCommandEx)(EMVBASE_APDU_SEND *ApduSend,EMVBASE_APDU_RESP *apdu_r, unsigned short offset);
typedef unsigned char (*EMVBCORE_InputCreditPwd)(unsigned char pintype, unsigned char *ASCCreditPwd);
typedef void (*Paypass_RandomNum)(unsigned char *RandomNum, int RandomNumLen);
typedef int (*Paypass_termipkrevokecheck)(unsigned char *buf);
typedef void (*Paypass_HashWithSensitiveData)(unsigned int mode, unsigned char cardtype,
	unsigned int inlen, unsigned char *p_datain, unsigned int *p_outlen, unsigned char *p_outdata);

//typedef int (*paypass_check_MatchTermAID)(unsigned char *aid, unsigned char aidLen, EMVBASE_TERMAPP *Applist);

typedef void (*core_paypassccctimergetstartid)(void);//luohuidong 2017.03.21 19:59
typedef void (*core_paypassdelay)(void);
typedef int (*core_paypass_dataexchange)(unsigned char cmd, unsigned char *data, unsigned int *dataLen); //20160516_lhd
typedef int (*core_paypass_get_dek_total_num)(void);
typedef int (*core_paypass_bIs_support_de)(void);
typedef unsigned char (*core_paypass_resv_stop_signal)(void);
typedef int (*core_paypass_OpenTimer)(unsigned char mode);
typedef int (*core_paypass_CloseTimer)(unsigned char mode);
typedef unsigned int (*core_paypass_GetTimeTaken)(void);
typedef unsigned char (*core_gettornlogtempdata)(TornTransactionLogIDSPartRecord *tornlogrecord);
typedef int (*core_paypass_save_datachange_cmd)(int mode);

typedef unsigned char (*core_checkifcardintornlog)(void);
typedef unsigned char (*paypass_RECOVERAC)(EMVBASE_APDU_SEND *apdu_s);
typedef unsigned char (*core_deletetornlog)(void);
typedef void (*core_addtornlog)(unsigned int AppUnionStructaddr);
typedef void (*core_senduserinterfacerequestdata)(unsigned int step);
typedef void (*core_sendoutparamset)(unsigned int step);
typedef void (*paypass_sendDisData)(int mode);
typedef void (*core_sendendapplicationdatarecord)(void);
typedef unsigned char (*core_copytornrecorddata)(void);
typedef unsigned char (*core_freetornlogtempdata)(void);
typedef int (*core_rf_card_poweroff)(void);
typedef int (*core_displayremovecard)(void);
typedef unsigned int (*core_paypass_getCurMsTimer)(void);
typedef int (*paypass_GetPinRes)(void);
typedef int (*paypass_GetVerifyCardNoRes)(void);
typedef int (*paypass_GetVerifyCAPKRes)(void);


#if 1

//DF8129
#define PAYPASS_OPS_STATUS_APPROVED   1
#define PAYPASS_OPS_STATUS_DECLINED   2
#define PAYPASS_OPS_STATUS_ONLINE     3
#define PAYPASS_OPS_STATUS_ENDAPPLICATION 4
#define PAYPASS_OPS_STATUS_SELECTNEXT 5
#define PAYPASS_OPS_STATUS_TRYANOTHERINTERFACE     6
#define PAYPASS_OPS_STATUS_TRYAGAIN 7
#define PAYPASS_OPS_STATUS_NA 0x0F


#define PAYPASS_OPS_START_A  0x00
#define PAYPASS_OPS_START_B  0x01
#define PAYPASS_OPS_START_C  0x02
#define PAYPASS_OPS_START_D  0x03
#define PAYPASS_OPS_START_NA 0x0F


#define PAYPASS_OPS_CVM_NOCVMREQ        0
#define PAYPASS_OPS_CVM_OBTAINSIGNATURE 1
#define PAYPASS_OPS_CVM_ONLINEPIN       2
#define PAYPASS_OPS_CVM_CONFVERIFIED    3
#define PAYPASS_OPS_CVM_NA              0x0F



#define PAYPASS_ERRID_L1_OK           0
#define PAYPASS_ERRID_L1_TIMEOUTERR   1
#define PAYPASS_ERRID_L1_TRANSERR     2
#define PAYPASS_ERRID_L1_PROTOERR     3



#define PAYPASS_ERRID_L2_SUCCESS         0
#define PAYPASS_ERRID_L2_CARDDATAMISSING 1
#define PAYPASS_ERRID_L2_CAMFAILED       2
#define PAYPASS_ERRID_L2_STATUSBYTES     3
#define PAYPASS_ERRID_L2_PARSINGERROR    4
#define PAYPASS_ERRID_L2_MAXLIMITEXCEEDED 5
#define PAYPASS_ERRID_L2_CARDDATAERR     6
#define PAYPASS_ERRID_L2_MAGNOTSUP       7
#define PAYPASS_ERRID_L2_NOPPSE          8
#define PAYPASS_ERRID_L2_PPSEFAULT       9
#define PAYPASS_ERRID_L2_EMPTYCANLIST    10
#define PAYPASS_ERRID_L2_IDSREADERR      11
#define PAYPASS_ERRID_L2_IDSWRITEERR     12
#define PAYPASS_ERRID_L2_IDSDATAERR      13
#define PAYPASS_ERRID_L2_IDSNOMATCHAC    14
#define PAYPASS_ERRID_L2_TERMINALDATAERR 15


#define PAYPASS_ERRID_L3_OK    0
#define PAYPASS_ERRID_L3_TIMEOUT 1
#define PAYPASS_ERRID_L3_STOP  2
#define PAYPASS_ERRID_L3_AMOUNTNOTPRESENT 3



#define PAYPASS_USERREQDATA_MSGID_CARDREADOK      0x17
#define PAYPASS_USERREQDATA_MSGID_TRYAGAIN        0x21   //33
#define PAYPASS_USERREQDATA_MSGID_APPROVED        0x03
#define PAYPASS_USERREQDATA_MSGID_APPROVEDSIGN    0x1A   //26
#define PAYPASS_USERREQDATA_MSGID_DECLINED        0x07
#define PAYPASS_USERREQDATA_MSGID_ERROR_OTHERCARD 0x1C   //28
#define PAYPASS_USERREQDATA_MSGID_INSERTCARD      0x1D   // 29
#define PAYPASS_USERREQDATA_MSGID_SEEPHONE        0x20   // 32
#define PAYPASS_USERREQDATA_MSGID_AUTHORISINGPLS  0x1B   // 27
#define PAYPASS_USERREQDATA_MSGID_CLEARDISP       0x1E   // 30



//#define PAYPASS_ERRID_MSGONERR_OTHERCARD 128
//#define PAYPASS_ERRID_MSGONERR_TRYAGAIN  129


#define PAYPASS_USERREQDATA_STATUS_NOTREADY    0
#define PAYPASS_USERREQDATA_STATUS_IDLE        1
#define PAYPASS_USERREQDATA_STATUS_READYTOREAD 2
#define PAYPASS_USERREQDATA_STATUS_PROCESSING  3
#define PAYPASS_USERREQDATA_STATUS_CARDREADSUCCESS 4
#define PAYPASS_USERREQDATA_STATUS_PROCESSINGERR   5



#define PAYPASS_USERREQDATA_VQ_NONE     0
#define PAYPASS_USERREQDATA_VQ_AMOUNT   1
#define PAYPASS_USERREQDATA_VQ_BALANCE  2


#define PAYPASS_USERREQDATA_MSGHOLDTIME_DEFAULT "\x00\x00\x13"
#define PAYPASS_TORNLOG_MAXLIFETIME_DEFAULT     "\x01\x2C"

#ifdef PAYPASS_OUTCOME_SUPPORT_FLAG
typedef struct _DISCRETIONARYITEMITEM_OUTCOME
{
    unsigned char errindicationflag;
    unsigned char emvdataflag;
    unsigned char msgdataflag;
} DISCRETIONARYITEMITEM_OUTCOME;
#endif

typedef struct _PAYPASS_OUTPARAMETERSET
{

    #ifdef PAYPASS_OUTCOME_SUPPORT_FLAG
    unsigned char OutcomeParSet[8];
    #endif
    unsigned char status;
    unsigned char start;
    unsigned char CVM;

    #ifdef PAYPASS_OUTCOME_SUPPORT_FLAG
    unsigned char UIRequestOnOutPresent;
    unsigned char UIRequestOnRestartPresent;
    unsigned char DataRecordPresent;
    unsigned char DisDataPresent;
    unsigned char ReceiptinoutParameter;
    unsigned char AIPinoutParameter;
    unsigned char FieldoffinoutParameter;
    unsigned char RemovalTimeoutinoutParameter;
    unsigned char OnlineResDatainoutParameter;
    unsigned char opssendflag;
    struct _DISCRETIONARYITEMITEM_OUTCOME discretionaryitem;
    #endif
    unsigned char selectsucdata;
	#ifdef PAYPASS_L2_CERT_USE
    unsigned short FCIdatalen;
    unsigned char FCIdata[256];
    unsigned char SelectCombDatalen;
    unsigned char SelectCombData[20];
	#endif
} PAYPASS_OUTPARAMETERSET;



typedef struct _PAYPASS_USERINTERFACEREQDATA
{
    unsigned char MessageID;
    unsigned char Status;
    unsigned char HoldTime[3];
    unsigned char LanguagePreference[8];
    unsigned char ValueQualifier;
    unsigned char value[6];
    unsigned char CurrencyCode[2];
    unsigned char LanguagePreferenceLen;
    unsigned char sendMsgFlag;
} PAYPASS_USERINTERFACEREQDATA;


typedef struct _PAYPASS_OUTCOME_ERRORID   //DF8115 - 6
{
    unsigned char L1;
    unsigned char L2;
    unsigned char L3;
    unsigned char SW12[2];
    unsigned char MsgOnError;
} PAYPASS_OUTCOME_ERRORID;

typedef struct _PAYPASS_APDU_ERRORID
{
    unsigned int Step;
    unsigned char Status;
    unsigned char L1;
    unsigned char L2;
    unsigned char L3;
    unsigned char SW12[2];
    unsigned char MsgOnError;
} PAYPASS_APDU_ERRORID;

#define PAYPASS_MAXPHONEMESSAGETABLENUM 5
typedef struct
{
    unsigned char PCIIMASK[3];
    unsigned char PCIIVALUE[3];
    unsigned char MESSAGE;
    unsigned char STATUS;
} PAYPASS_PHONEMESSAGETABLE;


typedef struct
{
    unsigned int phonemessagetablenum;
    PAYPASS_PHONEMESSAGETABLE phonemessagetable[PAYPASS_MAXPHONEMESSAGETABLENUM];
} PAYPASS_PHONEMESSAGETABLEINFO;

#endif

typedef struct
{

    PAYPASSDLLTRADEPARAMETER *PaypassTradeParam;

    unsigned char *paypassccctimecnt;


    PAYPASS_APDU_ERRORID *EmvErrID;
    PAYPASS_OUTPARAMETERSET *pOutParameterSet;
    #ifdef PAYPASS_OUTCOME_SUPPORT_FLAG

    PAYPASS_USERINTERFACEREQDATA *pUserInterfaceReqData;

    #endif

    PAYPASS_OUTCOME_ERRORID	*pErrorID;
    PAYPASS_PHONEMESSAGETABLEINFO *pphonemsgtable;

    core_senduserinterfacerequestdata senduserinterfacerequestdata;
    core_sendoutparamset sendoutparamset;
    paypass_sendDisData sendDisData;
    core_sendendapplicationdatarecord sendendapplicationdatarecord;

    unsigned char *pasTradeAmount;

    EMVBASE_PaypassIsoCommand IsoCommand;
    EMVBCORE_InputCreditPwd EMVB_InputCreditPwd;
    Paypass_RandomNum EMVB_RandomNum;
	paypass_GetPinRes GetPinRes;
	paypass_GetVerifyCardNoRes GetVerifyCardNoRes;
	paypass_GetVerifyCAPKRes GetVerifyCAPKRes;

    core_rf_card_poweroff rf_card_poweroff;
    core_displayremovecard displayRemoveCard;

    core_checkifcardintornlog checkifcardintornlog;
    paypass_RECOVERAC       RECOVERAC;
    core_freetornlogtempdata freetornlogtempdata;
    core_deletetornlog   deletetornlog;
    core_copytornrecorddata copytornrecorddata;
    core_addtornlog      addtornlog;

    Paypass_termipkrevokecheck termipkrevokecheck;

    core_paypassccctimergetstartid    paypassCcctimerGetStartId;//luohuidong 2017.03.21 20:0
    core_paypassdelay    paypassdelay;
    unsigned char ucPaypassintornlog;
    core_gettornlogtempdata gettornlogtempdata;

    core_paypass_dataexchange paypassDataExchange;
    core_paypass_get_dek_total_num   paypassGetDekTotalNum;
    core_paypass_bIs_support_de paypassBisSupportDe;
    core_paypass_resv_stop_signal paypassResvStopSignal;//20160716_lhd

    unsigned char *ppaypassbIsDoNotSaveCurRecvCMDLog;
    core_paypass_save_datachange_cmd paypassSaveDataExchangeCmd;

    core_paypass_OpenTimer sdkPaypassOpenTimer;
    core_paypass_CloseTimer sdkPaypassCloseTimer;
    core_paypass_GetTimeTaken sdkPaypassGetTimeTaken;
    core_paypass_getCurMsTimer getCurMsTimer;

} PAYPASSTradeUnionStruct;

extern unsigned char Paypass_GetKernelVerInfo(int machine_code, unsigned char ver[32]);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif


