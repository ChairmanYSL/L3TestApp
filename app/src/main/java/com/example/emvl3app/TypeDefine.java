package com.example.emvl3app;

public class TypeDefine {
    //Host Communicate Type
    public static final int communicateWithNone = 0;
    public static final int communicateWithSerialPort = 1;
    public static final int communicateWithNetwork = 2;
    public static final int errorCode_NullInstance = -1;

    //
    public static final int DEVICE_OK = 0;
    public static final int EMV_OK = 1;
    public static final int EMV_ERR = -1;
    public static final int EMV_PARAM_INVALID = -2;

    //Protocol Type Define,Management
    public static final int PROTOCOL_START_TRANS_SEND = 0xC0;
    public static final int PROTOCOL_START_TRANS_RECV = 0x80;
    public static final int PROTOCOL_TRANS_RESULT_SEND = 0xC1;
    public static final int PROTOCOL_TRANS_RESULT_RECV = 0x81;
    public static final int PROTOCOL_DOWNLOAD_CAPK_SEND = 0xC2;
    public static final int PROTOCOL_DOWNLOAD_CAPK_RECV = 0x82;
    public static final int PROTOCOL_DOWNLOAD_AID_SEND = 0xC3;
    public static final int PROTOCOL_DOWNLOAD_AID_RECV = 0x83;
    public static final int PROTOCOL_DOWNLOAD_TERMINFO_SEND = 0xC4;
    public static final int PROTOCOL_DOWNLOAD_TERMINFO_RECV = 0x84;
    public static final int PROTOCOL_DOWNLOAD_EXCEPTIONFILE_SEND = 0xC5;
    public static final int PROTOCOL_DOWNLOAD_EXCEPTIONFILE_RECV = 0x85;
    public static final int PROTOCOL_DOWNLOAD_REVOKEY_SEND = 0xC6;
    public static final int PROTOCOL_DOWNLOAD_REVPKEY_RECV = 0x86;
    public static final int PROTOCOL_UPLOAD_ECCHIP_SEND = 0xC7;
    public static final int PROTOCOL_UPLOAD_ECCHIP_RECV = 0x87;
    public static final int PROTOCOL_DOWNLOAD_DRL_SEND = 0xC8;
    public static final int PROTOCOL_DOWNLOAD_DRL_RECV = 0x88;
    public static final int PROTOCOL_DISPLAYINFO_SEND = 0xC9;
    public static final int PROTOCOL_DISPLAYINFO_RECV = 0x89;

    //Protocol Type Define,Transaction
    public static final int PROTOCOL_FINANCIALREQ_SEND = 0x41;
    public static final int PROTOCOL_FINANCIALREQ_RECV = 0x01;
    public static final int PROTOCOL_AUTHENTICATEREQ_SEND = 0x42;
    public static final int PROTOCOL_AUTHENTICATEREQ_RECV = 0x02;
    public static final int PROTOCOL_FINANCIALREQ_CONFIRM_SEND = 0x43;
    public static final int PROTOCOL_FINANCIALREQ_CONFIRM_RECV = 0x03;
    public static final int PROTOCOL_BATCHUPLOAD_SEND = 0x44;





}
