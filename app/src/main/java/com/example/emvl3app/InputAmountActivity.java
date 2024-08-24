package com.example.emvl3app;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.KeyEvent;
import android.widget.Button;
import android.widget.TextView;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import com.szzt.sdk.device.emv.EmvInterface;

public class InputAmountActivity extends AppCompatActivity {

    private Button buttonConfirm;
    private Button buttonCancel;
    private TextView textAmt;
    private TextView textAmtOth;
    private TextView textTransType;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_input_amount);


        buttonConfirm = (Button) findViewById(R.id.btn_confirm_trans);
        buttonCancel = (Button) findViewById(R.id.btn_cancel_trans);
        textAmt = (TextView) findViewById(R.id.text_Amount);
        textAmtOth = (TextView) findViewById(R.id.text_AmountOther);
        textTransType = (TextView) findViewById(R.id.text_TransType);

        buttonConfirm.setOnClickListener( v->{
            Intent intent = new Intent();
            intent.putExtra("transAmount", textAmt.getText().toString());
            intent.putExtra("transAmountOther", textAmtOth.getText().toString());
            intent.putExtra("transType", textTransType.getText().toString());

            setResult(RESULT_OK, intent);
            finish();
        });

        buttonCancel.setOnClickListener( v -> {
            setResult(RESULT_CANCELED);
            finish();
        });
    }
}
