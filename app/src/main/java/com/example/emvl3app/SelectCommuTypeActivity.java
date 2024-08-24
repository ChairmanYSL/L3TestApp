package com.example.emvl3app;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.widget.Button;

import androidx.appcompat.app.AppCompatActivity;

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

        mainApplication = MainApplication.getInstance();

        tcpClient = TCPClient.getInstance();

        Intent resultIntent = new Intent();

        buttonSerial.setOnClickListener(v -> {
            resultIntent.putExtra("communicateType", TypeDefine.communicateWithSerialPort);
            setResult(RESULT_OK, resultIntent);
            Log.d("lishiyao", "onCreate: start open serial port");
//            openSerial();
            Log.d("lishiyao", "onCreate: finish open serial port");
            finish();
        });

        buttonTcp.setOnClickListener(v -> {
            Intent tcpInfoIntent = new Intent(this, InputIPAddrActivity.class);

            if(tcpClient != null){
                tcpInfoIntent.putExtra("defaultIpAddr", tcpClient.getIpAddress());
                tcpInfoIntent.putExtra("defaultIPPort", tcpClient.getPort());
                tcpInfoIntent.putExtra("bHasDefaultValue", true);
            }else{
                tcpInfoIntent.putExtra("bHasDefaultValue", false);
            }

            Log.d("lishiyao", "onCreate: start open TCP");
            startActivityForResult(tcpInfoIntent, REQUEST_CODE_SETTCP_ACTIVITY);
            Log.d("lishiyao", "onCreate: finish open TCP");
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        int ret;

        if(requestCode == REQUEST_CODE_SETTCP_ACTIVITY && resultCode == RESULT_OK){
            //从Intent中获取IP地址和端口信息
            ret = data.getIntExtra("retCode", TypeDefine.EMV_ERR);
            Log.d("lishiyao", "onActivityResult: get retCode from InputIPAddrActivity:"+ ret);
            if(ret == TypeDefine.EMV_OK){
                ipAddr = data.getStringExtra("IpAddr");
                tcpPort = data.getStringExtra("Port");
                try {
                    setTcpSetting(ipAddr, tcpPort);

                    Intent resultIntent = new Intent();
                    resultIntent.putExtra("communicateType", TypeDefine.communicateWithNetwork);
                    setResult(RESULT_OK, resultIntent);
                    finish();
                } catch (IOException e) {
                    Log.d("lishiyao", "setTcpSetting Exception: "+e.getMessage());
                    e.printStackTrace();
                }
            }else{
                // 处理错误或用户取消操作
                setResult(RESULT_CANCELED);
                finish();
            }
        }
    }

    private void setTcpSetting(String ipAddr, String tcpPort) throws IOException {
        tcpClient.connect(ipAddr, Integer.parseInt(tcpPort));
    }
}