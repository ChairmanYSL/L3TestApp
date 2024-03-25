package com.example.emvl3app;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.Button;

import com.szzt.sdk.device.port.SerialPort;

import java.io.IOException;


public class SelectCommuTypeActivity extends AppCompatActivity {
    private MainApplication mainApplication;
    private static final int REQUEST_CODE_SETTCP_ACTIVITY = 1;
    private String ipAddr;
    private String tcpPort;
    private TCPClient tcpClient;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_selectcommutype);

        Button buttonSerial = (Button) findViewById(R.id.button_serial);
        Button buttonTcp = (Button)findViewById(R.id.button_tcp);

        mainApplication = SZZTApplication.getInstance().getMainApplication();

        tcpClient = mainApplication.getTcpClient();

        buttonSerial.setOnClickListener(v -> {
            Intent resultIntent = new Intent();
            resultIntent.putExtra("communicateType", TypeDefine.communicateWithSerialPort);
            setResult(RESULT_OK, resultIntent);
            Log.d("lishiyao", "onCreate: start open serial port");
//            openSerial();
            Log.d("lishiyao", "onCreate: finish open serial port");
            finish();
        });

        buttonTcp.setOnClickListener(v -> {
            Intent intent = new Intent(this, InputIPAddrActivity.class);
            intent.putExtra("communicateType", TypeDefine.communicateWithNetwork);
            setResult(RESULT_OK, intent);

            if(tcpClient != null){
                intent.putExtra("defaultIpAddr", tcpClient.getIpAddress());
                intent.putExtra("defaultIPPort", tcpClient.getPort());
                intent.putExtra("bHasDefaultValue", true);
            }else{
                intent.putExtra("bHasDefaultValue", false);
            }

            Log.d("lishiyao", "onCreate: start open TCP");
            startActivityForResult(intent, REQUEST_CODE_SETTCP_ACTIVITY);
            Log.d("lishiyao", "onCreate: finish open TCP");
            finish();
        });
    }

//    private void openSerial(){
//        int ret;
//
//        if(serialPort == null){
//            Log.e("lishiyao", "openSerial: serial port instance is null");
//        }
//
//        serialPortNum = serialPort.open("/dev/ttyUSB1", 115200);
//        Log.d("lishiyao", "openSerial: return serial port num:" + serialPortNum);
//        if(serialPortNum < 0){
//            Log.e("lishiyao", "openSerial: open serial port failed");
//            return;
//        }
//
//        ret = serialPort.setOptions(serialPortNum, 115200, 8, 0, 1);
//        if(ret < 0){
//            Log.e("SerialPort", "send: set serial port option failed");
//            return ;
//        }
//    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        int ret;

        if(requestCode == REQUEST_CODE_SETTCP_ACTIVITY && resultCode == RESULT_OK){
            // 从Intent中获取返回的值
            ret = data.getIntExtra("retCode", TypeDefine.EMV_ERR);
            Log.d("lishiyao", "onActivityResult: get retCode from InputIPAddrActivity:"+ ret);
            if(ret == TypeDefine.EMV_OK){
                ipAddr = data.getStringExtra("IpAddr");
                tcpPort = data.getStringExtra("Port");
                try {
                    setTcpSetting(ipAddr, tcpPort);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }else{

            }
        }
    }

    private void setTcpSetting(String ipAddr, String tcpPort) throws IOException {
        tcpClient.connect(ipAddr, Integer.parseInt(tcpPort), 1024);
    }
}