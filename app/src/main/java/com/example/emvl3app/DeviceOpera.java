package com.example.emvl3app;

import android.util.Log;

import com.szzt.android.util.HexDump;
import com.szzt.sdk.device.card.ContactlessCardReader;
import com.szzt.sdk.device.port.SerialPort;

import java.io.IOException;

public class DeviceOpera {
    private MainApplication mainApplication;
    private ContactlessCardReader mCard;
    private SerialPort port;
    public int portId;
    public TCPClient tcpClient;

        public DeviceOpera(){
            mainApplication = SZZTApplication.getInstance().getMainApplication();
            mCard=mainApplication.getContactlessCardReader();
            port=mainApplication.getSerialPortWrapperImpl();
            mainApplication.getFileSystemWrapperImpl();
            tcpClient = new TCPClient();
        }

        public int openRF() {
            int ret;
            if(mCard!=null)
            {
                ret = mCard.open();
                Log.d("lishiyao", "openRF ret: "+ret);
                return ret;
            }
            else
            {
                return -1200;
            }
        }

        //非接复位
        public int ResetRF(){
            int ret=mCard.waitForCard(5*1000);
            if(ret==0){
                ret = mCard.powerOn(new byte[256]);
                Log.d("lishiyao", "ResetRF ret: "+ret);
            }
            return ret;
        }

        //关闭非接
        public void CloseRF(){
            mCard.cancel();
        }

        public int OpenSerial(){
//        Log.d("lishiyao", "call OpenSerial: ");
//        Log.d("lishiyao", "port: "+port.toString());
            if(port!=null)
            {
                portId = port.open("/dev/ttyUSB1", 115200);
                if(portId < 0)
                {
                    port.close();
                    port.open("/dev/ttyUSB1", 115200);
                }
                return portId;
            }
            else
            {
                return -100;
            }
        }

        public int SendSerial(byte [] sendData, int sendLen){
//        Log.d("lishiyao", "Call SendSerial: ");
            int ret;
            Log.d("lishiyao", "SendSerial data:"+ HexDump.getHexString(sendData));
//        Log.d("lishiyao", "portId: "+portId);
            ret = port.send(portId, sendData, sendLen);
            Log.d("lishiyao", "ret: "+ret);
            return ret;
        }


        public byte [] ReadSerial(int readLen, int timeout){
//        Log.d("lishiyao", "Call ReadSerial: ");
            int ret;
            byte [] receiveData = new byte[readLen];
            ret = port.receive(portId, receiveData, readLen, timeout);
//        Log.d("lishiyao", "data: "+HexDump.dumpHex(receiveData));
//        Log.d("lishiyao", "ret: "+ret);
            if(receiveData.toString() == "")

                if(ret <= 0)
                {
                    return null;
                }
            if(ret>=4){
                if(receiveData[0]!=0x02){
                    byte[] data=new byte[ret];
                    System.arraycopy(receiveData,0,data,0,ret);
                    Log.d("txq","data:"+HexDump.toHexString(data));
                    return data;
                }
                Log.d("txq","data:"+HexDump.toHexString(receiveData,0,ret));
                byte[] d=new byte[4];
                d[2]=receiveData[2];
                d[3]=receiveData[3];
                Log.d("txq","data:"+HexDump.toHexString(d));
                int len=HexDump.byteToInt(d);
                long start=System.currentTimeMillis();
                Log.d("txq","len:"+len+":"+ret);
                int index=0;
                byte[] retData=new byte[len+4];
                System.arraycopy(receiveData,0,retData,index,ret);
                index+=ret;
                while (true){
                    Log.d("txq","index:"+index);
                    if(index>=retData.length){
                        return retData;
                    }
                    else{
                        ret=port.receive(portId, receiveData, readLen, timeout);
                        System.arraycopy(receiveData,0,retData,index,ret);
                        index+=ret;

                    }
                }
            }
            return null;

        }

        public void CloseSerial(int portId){
            port.close(portId);
        }

        public void openTCP(String IP, int port, int bufferSize) throws IOException {
            tcpClient.connect(IP,port,bufferSize);
        }

        public void sendTCP(byte [] data, int len) throws IOException {
            tcpClient.send(data);
        }

        public int readTCP(byte [] data){
            try{
                return tcpClient.read(data);
            }catch (IOException e){
                Log.e("kdkdkjd","readTCP:"+e.getMessage());
                e.printStackTrace();
                return TypeDefine.EMV_ERR;
            }
        }

        public void closeTCP() throws IOException {
            tcpClient.close();
        }
    }


