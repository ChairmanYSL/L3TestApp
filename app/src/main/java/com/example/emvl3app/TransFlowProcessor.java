package com.example.emvl3app;

import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.util.Log;

import com.example.emvl3app.TypeDefine;
import com.szzt.android.util.HexDump;
import com.szzt.sdk.device.card.ContactlessCardReader;
import com.szzt.sdk.device.card.SmartCardReader;
import com.szzt.sdk.device.emv.EMV_CARDBLACK;
import com.szzt.sdk.device.emv.EMV_CERTBLACK;
import com.szzt.sdk.device.emv.EMV_CONSTANTS;
import com.szzt.sdk.device.emv.EMV_STATUS;
import com.szzt.sdk.device.emv.EmvInterface;

import java.lang.reflect.Type;

public class TransFlowProcessor {
    public byte [] recvData;
    public byte [] sendData;
    private EmvInterface emvInterface;
    private EMVHandler emvHandler;
    private Context context;
    private byte [] pinBuff;
    private byte[] filed55N;
    private MainApplication mainApplication;
    private SmartCardReader smartCardReader;
    private ContactlessCardReader contactlessCardReader;
    private int commuType;
    private DataTransformer dataTransformer;

    public TransFlowProcessor(byte [] recvData, Context context, int commuType) {
        this.recvData = recvData;
        this.context = context;
        this.mainApplication =MainApplication.getInstance();
        this.smartCardReader = mainApplication.getSmartCardReader();
        this.contactlessCardReader = mainApplication.getContactlessCardReader();
        this.commuType = commuType;
        this.dataTransformer = new DataTransformer(this.commuType, this.context);
        this.emvInterface =this.mainApplication.getEmvInterface();
    }

    public int parseProtocol(){
        int ret = TypeDefine.EMV_ERR;

        if(recvData[0] != 0x02){
            return TypeDefine.EMV_ERR;
        }

        switch (recvData[1]){
            case (byte)TypeDefine.PROTOCOL_DOWNLOAD_AID_RECV:
                ret = downloadAID();
                break;

            case (byte)TypeDefine.PROTOCOL_DOWNLOAD_CAPK_RECV:
                ret = downloadCAPK();
                break;

            case (byte)TypeDefine.PROTOCOL_DOWNLOAD_TERMINFO_RECV:
                ret = downloadTermParam();
                break;

            case (byte) TypeDefine.PROTOCOL_DOWNLOAD_EXCEPTIONFILE_RECV:
                ret = downloadExceptionFile();
                break;

            case (byte) TypeDefine.PROTOCOL_DOWNLOAD_REVPKEY_RECV:
                ret = downloadRevokey();
                break;

            case (byte) TypeDefine.PROTOCOL_START_TRANS_RECV:
                startTrans();
                ret = TypeDefine.EMV_OK;
                break;

            case (byte) TypeDefine.PROTOCOL_FINANCIALREQ_RECV:
                dealFinanceReq();
                ret = TypeDefine.EMV_OK;
                break;

            default:
                Log.d("Protocol", "parseProtocol: invalid protocol type");
        }

        return ret;
    }

    private int downloadAID(){
        int dataLen = recvData[2] * 256 + recvData[3];
        byte [] tlvData = new byte[dataLen];
        int ret;

        System.arraycopy(recvData, 4, tlvData, 0, dataLen);
        ret = emvInterface.updateAidParam(EMV_CONSTANTS.ParamOper.ADD_PARAM, tlvData);
        Log.d("Download AID", "downloadAID: add AID ret = " + ret);
        if(ret == EMV_STATUS. EMV_OK){
            return TypeDefine.EMV_OK;
        }else{
            return TypeDefine.EMV_ERR;
        }
    }

    private int downloadCAPK(){
        int dataLen = recvData[2] * 256 + recvData[3];
        byte [] tlvData = new byte[dataLen];
        int ret;

        System.arraycopy(recvData, 4, tlvData, 0, dataLen);
        ret = emvInterface.updateCAPKParam(EMV_CONSTANTS.ParamOper.ADD_PARAM, tlvData);
        Log.d("Download CAPK", "downloadCAPK: add CAPK ret = " + ret);
        if(ret == EMV_STATUS. EMV_OK){
            return TypeDefine.EMV_OK;
        }else{
            return TypeDefine.EMV_ERR;
        }
    }

    private int downloadTermParam(){
        int dataLen = recvData[2] * 256 + recvData[3];
        byte [] tlvData = new byte[dataLen];
        int ret;

        System.arraycopy(recvData, 4, tlvData, 0, dataLen);
        ret = emvInterface.setTerminalParam(tlvData);
        Log.d("Download Term Param", "downloadTermParam: add Term Param ret = " + ret);
        if(ret == EMV_STATUS. EMV_OK){
            return TypeDefine.EMV_OK;
        }else{
            return TypeDefine.EMV_ERR;
        }
    }

    private int downloadExceptionFile(){
        int dataLen = recvData[2] * 256 + recvData[3];
        byte [] tlvData = new byte[dataLen];
        int ret = TypeDefine.EMV_ERR;
        EMV_CARDBLACK emv_cardblack = new EMV_CARDBLACK();

        System.arraycopy(recvData, 4, tlvData, 0, dataLen);
        TLVObject tlvObject = new TLVObject();

        if(true == tlvObject.parse_tlvBCD(tlvData, dataLen)){
            if(tlvObject.exist("5A")){
                emv_cardblack.pan = tlvObject.get("5A");
            }
            if(tlvObject.exist("5F34")){
                byte [] tmp = tlvObject.get("5F34").getBytes();
                emv_cardblack.psn = tmp[0];
            }
            ret = emvInterface.addCardBlack(emv_cardblack);
            Log.d("Download Exception File", "downloadExceptionFile: add Exception File ret = " + ret);
            if(ret == EMV_STATUS. EMV_OK){
                return TypeDefine.EMV_OK;
            }else{
                return TypeDefine.EMV_ERR;
            }
        }

        return ret;
    }

    private int downloadRevokey(){
        int dataLen = recvData[2] * 256 + recvData[3];
        byte [] tlvData = new byte[dataLen];
        int ret = TypeDefine.EMV_ERR;
        EMV_CERTBLACK emv_certblack = new EMV_CERTBLACK();

        System.arraycopy(recvData, 4, tlvData, 0, dataLen);
        TLVObject tlvObject = new TLVObject();

        if(true == tlvObject.parse_tlvBCD(tlvData, dataLen)){
            if(tlvObject.exist("9F06")){
                emv_certblack.rid = tlvObject.get("9F06").getBytes();
            }
            if(tlvObject.exist("8F")){
                byte [] tmp = tlvObject.get("8F").getBytes();
                emv_certblack.pki = tmp[0];
            }
            if(tlvObject.exist("DF8105")){
                emv_certblack.csn = tlvObject.get("DF8105").getBytes();
            }

            ret = emvInterface.addCertBlack(emv_certblack);
            Log.d("Download Revokey", "downloadRevokey: add Revokey ret = " + ret);
            if(ret == EMV_STATUS. EMV_OK){
                return TypeDefine.EMV_OK;
            }else{
                return TypeDefine.EMV_ERR;
            }
        }

        return ret;
    }

    private int openField(){
        int ret = TypeDefine.EMV_OK;

        ret = contactlessCardReader.open();
        if(ret < TypeDefine.DEVICE_OK){
            Log.d("lishiyao", "openField: open RF failed");
        }

        SmartCardReader.SCReaderListener listener = new SmartCardReader.SCReaderListener() {
            @Override
            public void notify(int nSlotIndex,int nEvent) {
                if(nEvent ==SmartCardReader.EVENT_SMARTCARD_NOT_READY){

                }else if(nEvent == SmartCardReader.EVENT_SMARTCARD_READY){
                }
            }
        };

        ret = smartCardReader.open(0, listener);
        if(ret < TypeDefine.DEVICE_OK){
            Log.d("lishiyao", "openField: open IC failed");
        }

        return ret;
    }


    private int readCard(int timeoutMSWaitCard, int timeoutMSReadCard){
        int ret=TypeDefine.EMV_ERR;


        long startTime = System.currentTimeMillis(); // 记录起始时间

        while (true){
            ret = smartCardReader.waitForCard(0, timeoutMSWaitCard);
            if(ret == 0){
                byte [] atr = new byte[256];
                ret = smartCardReader.powerOn(0, atr);
                if(ret >= 0){
                    ret = TypeDefine.EMV_OK;
                    break;
                }
            }

            try {
                Thread.sleep(1000);
            }catch (InterruptedException e){
                e.printStackTrace();
            }

            // 再次检查是否超时，可选，视你的需求而定
            if (System.currentTimeMillis() - startTime > timeoutMSReadCard) {
                // 超时后的处理逻辑，例如跳出循环
                break;
            }

            ret = contactlessCardReader.waitForCard(timeoutMSWaitCard);
            if(ret == 0){
                byte [] atr = new byte[256];
                ret = contactlessCardReader.powerOn(atr);
                if(ret >= 0){
                    ret = TypeDefine.EMV_OK;
                    break;
                }
            }
        }

        return ret;
    }

    private void closeField(){
        smartCardReader.close(0);
        contactlessCardReader.close();
    }

    private void startTrans(){
        int dataLen = recvData[2] * 256 + recvData[3];
        byte [] tlvData = new byte[dataLen];
        int ret;
        long transAmt=0;

        TLVObject tlvObject = new TLVObject();

        if(true == tlvObject.parse_tlvBCD(tlvData, dataLen)){
            if(tlvObject.exist("9F02")){
                String amountStr = tlvObject.get("9F02");
                byte [] amountBCD = new byte[amountStr.length() / 2];
                for(int i = 0; i < amountBCD.length; i++){
                    int high = Character.digit(amountStr.charAt(2 * i), 10) << 4;
                    int low = Character.digit(amountStr.charAt(2 * i + 1), 10);
                    amountBCD[i] = (byte) (high | low);
                }

                for(int i = 0; i < amountBCD.length; i++){
                    int high = (amountBCD[i] & 0xF0) >> 4; // 高四位
                    int low = (amountBCD[i] & 0x0F); // 低四位
                    transAmt = transAmt * 100 + high * 10 + low;
                }

                emvInterface.setTransAmount(transAmt);
            }

            if(tlvObject.exist("9F03")){
                String amountStr = tlvObject.get("9F03");
                byte [] amountBCD = new byte[amountStr.length() / 2];
                for(int i = 0; i < amountBCD.length; i++){
                    int high = Character.digit(amountStr.charAt(2 * i), 10) << 4;
                    int low = Character.digit(amountStr.charAt(2 * i + 1), 10);
                    amountBCD[i] = (byte) (high | low);
                }

                for(int i = 0; i < amountBCD.length; i++){
                    int high = (amountBCD[i] & 0xF0) >> 4; // 高四位
                    int low = (amountBCD[i] & 0x0F); // 低四位
                    transAmt = transAmt * 100 + high * 10 + low;
                }

                emvInterface.setOtherAmount(transAmt);
            }

            if(tlvObject.exist("9C")){
                byte [] tmp = tlvObject.get("9C").getBytes();
                emvInterface.setTransType(tmp[0]);
            }

            emvInterface.initialize(emvHandler);
            emvInterface.preprocess(0);

            ret = readCard(1000, 30000);
            if(ret != TypeDefine.EMV_OK){
                Log.d("lishiyao", "startTrans: read card failed");
                return;
            }

            while (true){
                ret = emvInterface.process();
                Log.d("Transaction", "startTrans: kernel retCode = " + ret);
                if(ret > 0 && ret < 100){
                    if(ret == EMV_STATUS.EMV_REQ_GO_ONLINE){
                        sendFinanceReq();
                        recvData = new byte[256];
                        dataTransformer.receive(recvData, 256, 2000);
                        parseProtocol();
                    }
                    else if(ret == EMV_STATUS.EMV_REQ_ONLINE_PIN){
                        ret = startOnlinePinEntryActivity();
                        if(ret == EMV_CONSTANTS.EMV_OPERRESULTS. EMV_OPER_BYPASS){
                            emvInterface.setPinByPassConfirmed(1);
                        }
                        else{
                            emvInterface.setOnlinePinEntered(ret, pinBuff, pinBuff.length);
                        }
                    }
                    continue;
                }
                else if(ret < 0){
                    break;
                }
                else if(ret > 100){
                    break;
                }
                else {
                    break;
                }
            }

            byte[] data55=new byte[256];
            byte[] recvFiled55N=new byte[256];
            int index55=0;

            int[] tag55=new int[]{0x9F26,0x9F27,0x9F10,0x9F37,0x9F36,0x95,0x9A,0x9C,0x9F02,0x5F2A,0x82,0x9F1A,0x9F34,0x9F33,0x9F35,0x9F1E,0x84,0x9F09,0x9F03,0x5F34,0x91,0x9F6E};
            for(int i=0;i<tag55.length;i++){
                int len=emvInterface.getTagData(tag55[i], data55);
                if(len>0){
                    int tagIndex;
                    if(tag55[i]>0xFF){
                        tagIndex=recvFiled55N[index55++]=(byte)(tag55[i]/0xff);
                    }
                    recvFiled55N[index55++]=(byte)(tag55[i]&0xff);
                    recvFiled55N[index55++]=(byte) len;
                    System.arraycopy(data55, 0, recvFiled55N, index55, len);
                    index55+=len;
                }
            }
            filed55N=new byte[index55];
            System.arraycopy(recvFiled55N, 0, filed55N, 0, index55);
            Log.d("Backgroud Protocol", "startTrans: filed 55 buff"+ HexDump.toHexString(filed55N));
        }
    }

    private int startOnlinePinEntryActivity(){
        Intent intent = new Intent(context, InputPINActivity.class);
        context.startActivity(intent);

        pinBuff = intent.getByteArrayExtra("pin_buff");
        return intent.getIntExtra("pin_result", EMV_CONSTANTS.EMV_OPERRESULTS. EMV_OPER_OK);
    }

    private void sendFinanceReq(){
        sendData = new byte[512];
        byte [] tlvData = new byte[512];
        int i = 0;

        sendData[0] = (byte) 0x02;
        sendData[1] = (byte) TypeDefine.PROTOCOL_FINANCIALREQ_SEND;

        if(pinBuff.length > 0){
            tlvData[i++] = (byte) 0x99;
            if(pinBuff.length < 128){
                tlvData[i++] = (byte) pinBuff.length;
            }
            else if(pinBuff.length >= 128 && pinBuff.length <= 255){
                tlvData[i++]= (byte) 0x81;
                tlvData[i++] = (byte) pinBuff.length;
            }
            else{
                tlvData[i++] = (byte) 0x82;
                tlvData[i++] = (byte) ((pinBuff.length >> 8) & 0xFF);
                tlvData[i++] = (byte) (pinBuff.length & 0xFF);
            }
            System.arraycopy(pinBuff, 0, tlvData, i, pinBuff.length);
            i+=pinBuff.length;
        }

        if(filed55N.length > 0){
            System.arraycopy(filed55N, 0, tlvData, i, filed55N.length);
        }

        sendData[2] = (byte) ((tlvData.length >> 8) & 0xFF);
        sendData[3] = (byte) (tlvData.length & 0xFF);

        System.arraycopy(tlvData, 0, sendData, 4, tlvData.length);
        dataTransformer.send(sendData, 512);
    }

    private int dealFinanceReq(){
        int ret = TypeDefine.EMV_OK;
        int dataLen = recvData[2] * 256 + recvData[3];
        byte [] tlvData = new byte[dataLen];

        System.arraycopy(recvData, 4, tlvData, 0, dataLen);
        ret = emvInterface.setOnlineResult(EMV_CONSTANTS.EMV_ONLINERESULTS.EMV_ONLINE_SUCC_ACCEPT, tlvData, dataLen);
        if(ret == EMV_STATUS.EMV_OK){
            ret = TypeDefine.EMV_OK;
        }else{
            ret = TypeDefine.EMV_ERR;
        }

        return ret;
    }

}
