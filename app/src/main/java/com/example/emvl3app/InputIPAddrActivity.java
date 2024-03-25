package com.example.emvl3app;

import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class InputIPAddrActivity extends AppCompatActivity {

    Button buttonConfirm;
    Button buttonCancel;
    EditText editTextIPAddr;
    EditText editTextPort;
    Boolean bHasDefaultValue;
    MainApplication mainApplication;
    TCPClient tcpClient;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_inputipaddr);

        buttonConfirm = (Button) findViewById(R.id.button_confirm);
        buttonCancel = (Button) findViewById(R.id.button_cancel);
        editTextIPAddr = (EditText)findViewById(R.id.editTextIpAddr);
        editTextPort = (EditText)findViewById(R.id.editTextIPPort);

        mainApplication = SZZTApplication.getInstance().getMainApplication();
        tcpClient = mainApplication.getTcpClient();

        Intent resultIntent = new Intent();

        bHasDefaultValue = resultIntent.getBooleanExtra("bHasDefaultValue", false);
        if(bHasDefaultValue == true){
            editTextIPAddr.setText(resultIntent.getStringExtra("defaultIpAddr"));
            editTextPort.setText(String.valueOf(resultIntent.getIntExtra("defaultIPPort", 8182)));
        }

        buttonConfirm.setOnClickListener(v->{
            uploadIPInfo();
            finish();
        });

        buttonCancel.setOnClickListener(v->{
            resultIntent.putExtra("retCode", TypeDefine.EMV_ERR);
            setResult(RESULT_OK, resultIntent);
            finish();
        });

    }

    private void uploadIPInfo(){
        int retCode = TypeDefine.EMV_ERR;
        Intent resultIntent = new Intent();

        if(editTextIPAddr.getText() != null && editTextPort.getText() != null){
            retCode = TypeDefine.EMV_OK;
            resultIntent.putExtra("IpAddr", editTextIPAddr.getText());
            resultIntent.putExtra("Port", editTextPort.getText());
            resultIntent.putExtra("retCode", retCode);
            Log.d("lishiyao", "uploadIPInfo: get text from dialog success");
        }else{
            resultIntent.putExtra("retCode", retCode);
            Log.d("lishiyao", "uploadIPInfo: get text from dialog fail");
        }
        setResult(RESULT_OK, resultIntent);
    }
}
