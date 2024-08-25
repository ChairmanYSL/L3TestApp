#ifndef SDKPAYPASSPRIVATE_H
#define SDKPAYPASSPRIVATE_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define SDKPAYPASS_FILE_PATH	"SDKPAYPASS/"

#define SDK_PAYPASS_STATUS_PPSE 				0x00
#define SDK_PAYPASS_STATUS_SELECTAID 			0x01
#define SDK_PAYPASS_STATUS_GPO		 			0x02
#define SDK_PAYPASS_STATUS_RRP					0x03
#define SDK_PAYPASS_STATUS_READRECORD			0x04
#define SDK_PAYPASS_STATUS_TRM					0x05
#define SDK_PAYPASS_STATUS_PROCESSREST			0x06
#define SDK_PAYPASS_STATUS_TAA					0x07
#define SDK_PAYPASS_STATUS_ODA					0x08
#define SDK_PAYPASS_STATUS_CVM					0x09
#define SDK_PAYPASS_STATUS_ISSUERUPDATE			0x0A
#define SDK_PAYPASS_STATUS_RESELECTAID			0x0B
#define SDK_PAYPASS_STATUS_CCC					0x0C
#define SDK_PAYPASS_STATUS_PREGACBALANCEREAD	0x0D
#define SDK_PAYPASS_STATUS_CHECKFLOORLMT		0x0F
#define SDK_PAYPASS_STATUS_PREGACPUTDATA		0x10
#define SDK_PAYPASS_STATUS_POSTGACBALANCEREAD	0x20
#define SDK_PAYPASS_STATUS_POSTGACPUTDATA		0x30
#define SDK_PAYPASS_STATUS_TRANSANALYS			0x40
#define SDK_PAYPASS_DIFFEMVBASE					0x50

#define DEK_TYPE						(0x01)
#define DET_TYPE						(0x02)

#define DEKDET_TYPE_OFFSET				(0)
#define DEKDET_NUMLIST_OFFSET			(1)
#define DEKDET_SUBNUMLIST_OFFSET		(2)
#define DEKDET_TOTALPACKET_OFFSET		(3)
#define DEKDET_SUBPACKET_OFFSET			(4)

#define DEKSUBDETMAXNUM		(20)
#define DEKDETINDEXNUM		(20)

#define DEKFILE			"dek"
#define DETFILE			"det"
#define DETDEKEXCHANGELOG	"dekdetexchange"	//20160629_lhd  文件总长度4字节，偏移4字节 预留4字节

struct _DETData
{
	u8		bIsexist;			//当前det是否存在
	u16 	usDetLen;			//数据长度
	u32 	uiDetOffset;		//数据的偏移
};
struct _DEKData
{
	u8		bIsexist;			//当前dek是否存在
	u16 	usDekLen;			//数据长度
	u32 	uiDekOffset;		//数据的偏移
	struct  _DETData detdata[DEKSUBDETMAXNUM];
};
//extern struct _DEKData *gstDEKIndex; //[DEKDETINDEXNUM];



typedef struct
{
    s32 (*setBeforeGPO)(void);		//set something before GPO
    s32 (*DispPromptData)(unsigned char ePromptType);
} SDK_PAYPASS_TRADE_PARAM;



//paypass recovery data
#define PAYPASS_TORNRECORDFLAG   "\x55\xaa\x66\xbb"

typedef struct _PaypassTornTransactionLogRecord
{
    unsigned char flag[4];              //\x55\xaa\x55\xaa????屑??
    unsigned char recordseqnum[6];
    unsigned char transdatetime[7];
    unsigned char AmtAuthNum[6];                //tag'9F02' Authorised amount of BCD numeric
    unsigned char AmtOtherNumexist;
    unsigned char AmtOtherNum[6];                     //tag'9F03' Other(cashback) amount of BCD numeric
    unsigned char PANLen;
    unsigned char PAN[10];
    unsigned char PANSeqexist;
    unsigned char PANSeq;                                               //identify card with same pan.
    unsigned char PayPassPreBalanceExist;               //
    unsigned char PayPassPreBalance[6];                 //paypass tag '9f50'  jiguang 20121220 add
    unsigned char CDOL1RelatedLen;
    unsigned char CDOL1RelatedData[256];              //?????????????
    unsigned char CVMResult[3];                                 //tag'9F34' cardholder verification methods perform result
    unsigned char DRDOLRelatedLen;
    unsigned char DRDOLRelatedData[256];              //?????????????
    unsigned char DSSummary1Len;                //8  or 16
    unsigned char DSSummary1[16];
    unsigned char IDSStatus;
    unsigned char InterfaceDeviceSerialNumberExist;
    unsigned char InterfaceDeviceSerialNumber[8];
    unsigned char PDOLRelatedLen;
    unsigned char PDOLRelatedData[256];
    unsigned char ReferenceControlParameter;
    unsigned char TermCapab[3];                                   //tag'9F33' terminal capability
    unsigned char CountryCodeexist;
    unsigned char CountryCode[2];                         //tag'9F1A' Terminal country code BCD
    unsigned char TermType;                                       //tag'9F35' Terminal type
    unsigned char TVR[5];                                         //tag'95'   Terminal Verification Results
    unsigned char TransCateCodeexist;
    unsigned char TransCateCode;                      //tag '9F53' Transaction Category Code, Mastercard M/Chip private data.
    unsigned char TransCurcyCodeexist;
    unsigned char TransCurcyCode[2];                  //tag'5F2A'
    unsigned char TransDate[3];                       //tag'9A'   YYMMDD
    unsigned char TransTime[3];                       //tag'9F21',HHMMSS,BCD
    unsigned char TransTypeValue;                     //tag'9C',
    unsigned char UnpredictNum[4];                    //tag'9F37' Terminal created for each transaction.
    unsigned char CDOL1Len;
    unsigned char CDOL1[256];
    unsigned char TerminalRelayResistanceEntropy[4];
    unsigned char DeviceRelayResistanceEntropy[4];
    unsigned char MinTimeForProcessingRelayResistanceAPDU[2];
    unsigned char MaxTimeForProcessingRelayResistanceAPDU[2];
    unsigned char DeviceEstimatedTransTForRRRAPDU[2];
    unsigned char MeasureRelayResistanceProcessingTime[2];
    unsigned char RRP_counter;
    unsigned char crc[2];
} PaypassTornTransactionLogRecord;


#define PaypassTornTransactionRecordLen sizeof(PaypassTornTransactionLogRecord)
#define PaypassTornTransaction "TornTrans"


typedef struct
{
    unsigned int index;
    unsigned int exit;
    unsigned char PANLen;
    unsigned char PAN[10];
    unsigned char PANseqexist;
    unsigned char PANseq;
    unsigned char recordseqnum[6];
    unsigned char transdatetime[7];
} TORNTRANSACTIONMAP;

typedef struct
{
	unsigned char AidLen;
	unsigned char Aid[16];
	unsigned char TransCurcyCode[2];			 //tag'5F2A'
	unsigned char TransCurcyExp;				 //tag'5F36'
	unsigned char TransReferCurcyCode[2];		 //tag'9F3C'
	unsigned char TransReferCurcyExp;			 //tag'9F3D'
	unsigned char AcquireID[6]; 				 //tag'9F01'
	unsigned char TermID[8];					 //tag'9F1C'
	unsigned char MerchCateCode[2]; 			 //tag'9F15'
	unsigned char MerchID[15];					 //tag'9F16'
	unsigned char MerchantNameLen;
	unsigned char MerchantName[20];
	unsigned char TRMDataLen;
	unsigned char TRMData[8];					//tag'9F1D'
	unsigned char TermTDOLLen;
	unsigned char TermTDOL[64];
	unsigned char TermTransPredicable[4];
	unsigned char terminalcapability[3];
	unsigned char terminaladdcapability[5];
	unsigned char terminaltype;
	unsigned char tradetypevalue;		//if for all transcation type set transvaule 0xFF, otherwise set transvaule with tag9C
	unsigned char reservedlen;
	unsigned char reserved[128];

	unsigned char ondevRTCL[6];
	unsigned char KernelID;
	unsigned char KernelConfigure;
	unsigned char magcvmrequirer;
	unsigned char magnocvmrequirer;
	unsigned char PaypassMagVer[2]; 			//paypass magstripe version
	unsigned char trm[8];				//20151207_lhd add this line
}SDK_PAYPASS_APPEX_AID_STRUCT;


typedef struct _TORNTRANSACTIONMAPLIST
{
    TORNTRANSACTIONMAP TornTransactionInfo;
    struct _TORNTRANSACTIONMAPLIST *next;
} TORNTRANSACTIONMAPLIST;


typedef struct
{
    unsigned char *flashusedflag;
    unsigned int TornMaxNum;
    unsigned int currentusedindex;
    TORNTRANSACTIONMAPLIST *tornhead;
} PAYPASS_TORNTRANSACTIONMAPTOTALINFO;


extern	void sdkPaypass_OutCome_packoutsignal(unsigned int step);
extern 	void sdkPaypass_OutCome_SetErrPar(unsigned int step);
extern	int sdkPaypassSetAppExAidParam(SDK_PAYPASS_APPEX_AID_STRUCT *p);
extern s32 sdkPaypassSetSendUserInterfaceRequestData(void(*fun_setSendUserInterfaceRequestData)(u32 step));
extern s32 sdkPaypassSetSendOutParamset(void(*fun_setSendOutParamset)(u32 step));
extern s32 sdkPaypassSetSendDisData(void(*fun_setSendDisData)(int mode));
extern s32 sdkPaypassSetSendEndApplicationDataRecord(void(*fun_setSendEndApplicationDataRecord)(void));
extern bool sdkPaypassGetPayMode(u8 *mode);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif

