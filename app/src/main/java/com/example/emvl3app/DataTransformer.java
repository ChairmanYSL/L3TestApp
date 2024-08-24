package com.example.emvl3app;

import static android.content.ContentValues.TAG;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.util.Log;

import com.szzt.android.util.HexDump;
import com.szzt.sdk.device.Constants;
import com.szzt.sdk.device.port.SerialPort;

import java.io.IOException;
import java.nio.channels.SocketChannel;

public class DataTransformer {

    private int communicateType = TypeDefine.communicateWithNone;

    private int resendCounter;
    private Context context;
    private MainApplication mainApplication;
    private TCPClient tcpClient;
    private SerialPort serialPort;
    private int serialPortNum;

    public DataTransformer(int type, Context context){
        this.communicateType = type;
        this.context = context;

        if(communicateType == TypeDefine.communicateWithNetwork){
            Log.d("lishiyao", "DataTransformer: tcp client instance pointer is:" + tcpClient);
        }
        else if(communicateType == TypeDefine.communicateWithSerialPort){
            Log.d("lishiyao", "DataTransformer: serial port instance pointer is:" + serialPort);
        }
    }

    public int send(byte[] data, int maxLen){
        int ret = TypeDefine.EMV_ERR;

        Log.d("lishiyao", "send: current communicate type is : "+ communicateType);

        if(communicateType == TypeDefine.communicateWithSerialPort)
        {
            if(serialPort == null){
                Log.e("SerialPort", "sendProtocolContext: The serial port object is null");
                AlertDialog.Builder builder = new AlertDialog.Builder(this.context);
                builder.setTitle("设置错误");
                builder.setMessage("串口未实例化或未打开");
                builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {

                    }
                });

                AlertDialog alert = builder.create();
                alert.show();
                return -1;
            }

            resendCounter = 3;
            while (resendCounter > 0){
                ret = serialPort.send(serialPortNum, data, maxLen);
                if(ret > 0){
                    break;
                }
                resendCounter--;
            }

            if(ret <= 0){
                Log.e("SerialPort", "sendProtocolContext: send protocol error");
                return ret;
            }
        }
        else if(communicateType == TypeDefine.communicateWithNetwork)
        {
            Log.d("lishiyao", "DataTransformer: tcp client instance pointer is:" + tcpClient);
            if(tcpClient != null){
                Log.d("lishiyao", "DataTransformer: socket status is:" + tcpClient.getStatus());
                if(tcpClient.getStatus() == true){
                    try{
                        tcpClient.send(data);
                        return data.length;
                    }catch (IOException e){
                        e.printStackTrace();
                    }
                }else{
                    Log.d("lishiyao", "send: TCP socket is closed");
                }
            }else{
                Log.d("lishiyao", "send: TCP Socket instance is null");
            }
        }
        else
        {
            Log.e("Protocol", "sendProtocolContext: Invalid Communicate Type");
        }

        return ret;
    }

    public int receive(byte [] data, int maxLen, int timeoutMS){
        int ret = TypeDefine.EMV_ERR;

        if(communicateType == TypeDefine.communicateWithSerialPort){
            if (serialPortNum < 0){

                serialPortNum = serialPort.open("/dev/ttyUSB0", 115200);
                if(serialPortNum < 0){
                    Log.e("SerialPort", "sendProtocolContext: The serial port object is null");
                    AlertDialog.Builder builder = new AlertDialog.Builder(this.context);
                    builder.setTitle("设置错误");
                    builder.setMessage("串口未实例化或未打开");
                    builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialogInterface, int i) {

                        }
                    });

                    AlertDialog alert = builder.create();
                    alert.show();
                    return TypeDefine.EMV_ERR;
                }
                ret = serialPort.setOptions(serialPortNum, 115200, 8, 0, 1);
                if(ret < 0){
                    Log.e("SerialPort", "send: set serial port option failed");
                    return ret;
                }
            }

            resendCounter = 3;
            while (resendCounter > 0){
                ret = serialPort.receive(serialPortNum, data, maxLen, timeoutMS);
                if(ret > 0){
                    break;
                }
                resendCounter--;
            }

            if(ret <= 0){
                Log.e("SerialPort", "sendProtocolContext: receive protocol error");
                return ret;
            }
        }else if(communicateType == TypeDefine.communicateWithNetwork){
            if(tcpClient != null){
                if(tcpClient.getStatus() == true){
                    try{
                        ret = tcpClient.read(data);
                    }catch (IOException e){
                        e.printStackTrace();
                    }
                }
            }
        }
        else {
            Log.d("lishiyao", "receive: Invalid Communicate Type");
        }

        return ret;
    }
}
