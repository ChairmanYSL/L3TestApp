package com.example.emvl3app;

import android.os.AsyncTask;
import android.util.Log;

import com.szzt.sdk.device.port.SerialPort;

public class SerialCommunicationTask extends AsyncTask<Void, Void, Integer>{

    private String sendData;
    private SerialPort serialPort;
    private int serialPortNum;

    public SerialCommunicationTask(String sendData, SerialPort serialPort, int serialPortNum) {
        this.sendData = sendData;
        this.serialPort = serialPort;
        this.serialPortNum = serialPortNum;
    }

    @Override
    protected Integer doInBackground(Void... voids) {
        byte[] sendBytes = sendData.getBytes();
        byte[] recvBytes = new byte[64];
        int ret;

        // 在后台线程中进行串口通信
        ret = serialPort.send(serialPortNum, sendBytes, sendBytes.length);

        if (ret > 0) {
            // 发送成功，接收数据
            ret = serialPort.receive(serialPortNum, recvBytes, recvBytes.length, 500);

            if (ret > 0) {
                // 成功接收数据
                String receivedData = new String(recvBytes, 0, ret);
                Log.d("SerialCommunication", "Received Data: " + receivedData);
            } else {
                Log.e("SerialCommunication", "Receive Error");
            }
        } else {
            Log.e("SerialCommunication", "Send Error");
        }

        return ret;
    }

    @Override
    protected void onPostExecute(Integer integer) {
        super.onPostExecute(integer);
    }
}
