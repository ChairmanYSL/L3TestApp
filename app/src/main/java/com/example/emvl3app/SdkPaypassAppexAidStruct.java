package com.example.emvl3app;

public class SdkPaypassAppexAidStruct {
    public byte aidLen;
    public byte[] aid = new byte[16];
    public byte[] transCurcyCode = new byte[2];
    public byte transCurcyExp;
    public byte[] transReferCurcyCode = new byte[2];
    public byte transReferCurcyExp;
    public byte[] acquireID = new byte[6];
    public byte[] termID = new byte[8];
    public byte[] merchCateCode = new byte[2];
    public byte[] merchID = new byte[15];
    public byte merchantNameLen;
    public byte[] merchantName = new byte[20];
    public byte trmDataLen;
    public byte[] trmData = new byte[8];
    public byte termTDOLLen;
    public byte[] termTDOL = new byte[64];
    public byte[] termTransPredicable = new byte[4];
    public byte[] terminalCapability = new byte[3];
    public byte[] terminalAddCapability = new byte[5];
    public byte terminalType;
    public byte tradeTypeValue;
    public byte reservedLen;
    public byte[] reserved = new byte[128];
    public byte[] ondevRTCL = new byte[6];
    public byte kernelID;
    public byte kernelConfigure;
    public byte magCvmRequirer;
    public byte magNoCvmRequirer;
    public byte[] paypassMagVer = new byte[2];
    public byte[] trm = new byte[8]; // terminal risk manage data
}

