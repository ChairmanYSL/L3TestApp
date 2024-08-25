#ifndef _DLLPAYPASSPRIVATE_H_
#define _DLLPAYPASSPRIVATE_H_

#define GenerateAC1  1
#define GenerateAC2  2


#define MAXSCRIPTLEN 128


#define CMD_NONE 		(0x00)
#define CMD_GET_DATA	(0x01)
#define CMD_READRECORD 	(0x02)


//taa result
#define TAA_OFFLINE 0x40
#define TAA_ONLINE  0x80
#define TAA_DENIAL  0x00


#define MAXPAYPASSCCCTIMES  (5)

#define CVR_UNKNOWN             0
#define CVR_FAIL                1
#define CVR_SUCCESS             2



typedef struct
{
    unsigned char ICCDynNumLen;
    unsigned char ICCDynNum[8];
    unsigned char CryptInfo;
    unsigned char AppCrypt[8];
    unsigned char HashResult[20];
    unsigned char DSSummary2Len;
    unsigned char DSSummary2[16];
    unsigned char DSSummary3Len;
    unsigned char DSSummary3[16];
    unsigned char TerminalRelayResistanceEntropy[4];
    unsigned char DeviceRelayResistanceEntropy[4];
    unsigned char MinTimeForPRRAPDU[2];
    unsigned char MaxTimeForPRRAPDU[2];
    unsigned char DeviceEstimatedTransTFRRRAPDU[2];
} ICC_DYN_DATA;


typedef struct
{
    unsigned char method;
    unsigned char condition;
} CVMR;

typedef struct
{
    CVMR CVRList[120];
    unsigned char CVRListLen;
    unsigned char CVM_X[6];
    unsigned char CVM_Y[6];
} CVMSTRCUT;


#endif

