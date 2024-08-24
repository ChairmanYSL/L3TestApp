package com.example.emvl3app;

import android.content.Intent;
import android.os.Bundle;
import android.text.InputFilter;
import android.util.Log;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;


public class InputIPAddrActivity extends AppCompatActivity {

    private Button buttonConfirm;
    private Button buttonCancel;
    private EditText editTextIPAddr;
    private EditText editTextPort;
    private Boolean bHasDefaultValue;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_inputipaddr);

        buttonConfirm = (Button) findViewById(R.id.button_confirm);
        buttonCancel = (Button) findViewById(R.id.button_cancel);
        editTextIPAddr = (EditText)findViewById(R.id.editTextIpAddr);
        editTextPort = (EditText)findViewById(R.id.editTextIPPort);


        InputFilter[] filters = new InputFilter[] {new IPAddressFilter()};
        editTextIPAddr.setFilters(filters);

        Intent resultIntent = new Intent();

        bHasDefaultValue = resultIntent.getBooleanExtra("bHasDefaultValue", false);
        if(bHasDefaultValue == true){
            editTextIPAddr.setText(resultIntent.getStringExtra("defaultIpAddr"));
            editTextPort.setText(String.valueOf(resultIntent.getIntExtra("defaultIPPort", 8182)));
        }

        buttonConfirm.setOnClickListener(v->{
            if(editTextIPAddr.getText() != null && editTextPort.getText() != null){
                resultIntent.putExtra("IpAddr", editTextIPAddr.getText().toString());
                resultIntent.putExtra("Port", editTextPort.getText().toString());
                resultIntent.putExtra("retCode", TypeDefine.EMV_OK);
                Log.d("lishiyao", "uploadIPInfo: get text from dialog success");
            }else{
                resultIntent.putExtra("retCode", TypeDefine.EMV_ERR);
                Log.d("lishiyao", "uploadIPInfo: get text from dialog fail");
            }
            setResult(RESULT_OK, resultIntent);
            finish();
        });

        buttonCancel.setOnClickListener(v->{
            resultIntent.putExtra("retCode", TypeDefine.EMV_ERR);
            setResult(RESULT_OK, resultIntent);
            finish();
        });

    }

}
