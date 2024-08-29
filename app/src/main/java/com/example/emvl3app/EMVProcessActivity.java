package com.example.emvl3app;

import android.app.Activity;
import android.content.Intent;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.activity.result.ActivityResultCaller;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContract;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.appcompat.app.AppCompatActivity;

import com.szzt.sdk.device.card.ContactlessCardReader;
import com.szzt.sdk.device.card.SmartCardReader;
import com.szzt.sdk.device.emv.EMV_CONSTANTS;
import com.szzt.sdk.device.emv.EMV_STATUS;
import com.szzt.sdk.device.emv.EmvInterface;

import java.util.Locale;


public class EMVProcessActivity extends AppCompatActivity {
    private EmvInterface emvInterface;
    private TextView textView;
    private long transAmount;
    private long transAmountOther;
    private byte transType;
    private boolean bIsSeePhone = false;
    private boolean bIsMultiCardColl = false;
    private boolean bIsSecondTap = false;
    private ImageView imageView;
    private SmartCardReader smartCardReader;
    private ContactlessCardReader contactlessCardReader;
    private int cardType;
    private Button buttonClear;
    private Button buttonExit;
    private ActivityResultLauncher<Intent> inputPINActivityResultLauncher;
    private String pinResult;
    private String pinStr;

    private class MyHandler extends Handler {
        public MyHandler(Looper looper) {
            super(looper);
        }

        public void handleMessage(Message m) {
            super.handleMessage(m);
            switch (m.what) {
                case 1:
                    break;
                case 2:
                    break;
                default:
                    break;
            }
        }
    }

    private MyHandler myHandler;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_show_outcome);

        myHandler = new MyHandler(Looper.getMainLooper());

        emvInterface = MainApplication.getInstance().getEmvInterface();
        smartCardReader = MainApplication.getInstance().getSmartCardReader();
        contactlessCardReader = MainApplication.getInstance().getContactlessCardReader();
        textView = (TextView) findViewById(R.id.text_outcome);
        imageView = (ImageView) findViewById(R.id.image_rflogo);
        buttonClear = (Button) findViewById(R.id.button_clear_info);
        buttonExit = (Button) findViewById(R.id.button_exit_trans);

        buttonClear.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                textView.setText("");
            }
        });

        buttonExit.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                emvInterface.processExit();
                smartCardReader.powerOff(0);
                smartCardReader.close(0);
                contactlessCardReader.powerOff();
                contactlessCardReader.close();
                finish();
            }
        });

        inputPINActivityResultLauncher = registerForActivityResult(
                new ActivityResultContracts.StartActivityForResult(),
                result -> {
                    if(result.getResultCode() == Activity.RESULT_OK){
                        Intent data = result.getData();
                    }
                }
        );

        DealTrade();
    }

    @Override
    protected void onStop() {
        super.onStop();

        smartCardReader.powerOff(0);
        smartCardReader.close(0);
        contactlessCardReader.powerOff();
        contactlessCardReader.close();
    }

    private void parseIntentData() {
        Intent intent = getIntent(); // Use the current activity's intent

        transAmount = parseAmount(intent.getStringExtra("transAmount"));
        transAmountOther = parseAmount(intent.getStringExtra("transAmountOther"));
        transType = Byte.parseByte(intent.getStringExtra("transType"));
    }

    private long parseAmount(String amountStr) {
        if (amountStr != null && amountStr.contains(".")) {
            amountStr = amountStr.replace(".", "");
        }
        try {
            return Long.parseLong(amountStr);
        } catch (NumberFormatException e) {
            Log.e("EMVProcessActivity", "Invalid amount format", e);
            return 0; // default value or error handling
        }
    }

    private void DealTrade() {
        int ret;
        boolean readCardAgain=false;

        emvInterface.initialize(myHandler);
        ImportTradeAmount();

        ret = emvInterface.preprocess(0);

        cardType = 0;

        while (true){

            ReadCardDisp();
            ret = IccReadCard();

            if(cardType == TypeDefine.CARD_TYPE_CONTACTLESS){
                if(ret == TypeDefine.ICC_MULTICARD){
                    closeRFLogo();
                    textView.append("Multi Card Collision");
                    contactlessCardReader.powerOff();
                    bIsMultiCardColl = true;
                    continue;
                }else if(ret == TypeDefine.ICC_RESET_CARD_ERR){
                    closeRFLogo();
                    textView.append("Read Card error,Tx Stop");
                    contactlessCardReader.powerOff();
                    contactlessCardReader.close();
                    finish();
                }else{
                    break;
                }
            }
            else if(cardType == TypeDefine.CARD_TYPE_CONTACT){
                if(ret == TypeDefine.ICC_RESET_CARD_ERR){
                    closeRFLogo();
                    textView.append("Read Card error,Tx Stop");
                    smartCardReader.powerOff(0);
                    smartCardReader.close();
                    finish();
                }else{
                    break;
                }
            }
            else {
                closeRFLogo();
                textView.append("Timeout exit");
                finish();
            }

            closeRFLogo();
            textView.append("Processing");

            if(cardType == TypeDefine.CARD_TYPE_CONTACT){
                smartCardReader.powerOff(0);
                smartCardReader.close(0);
                Log.d("lishiyao", "Contact Function not valid now");
            }
            else if(cardType == TypeDefine.CARD_TYPE_CONTACTLESS){
                if(!bIsSecondTap){
                    emvInterface.setCardType(TypeDefine.CARD_TYPE_CONTACTLESS);
                }

                while (true){
                    ret = emvInterface.process();
                    Log.d("lishiyao", "emvInterface.process ret = "+ret);
                    if(ret < 0){
                        contactlessCardReader.powerOff();
                        contactlessCardReader.close();

                        if(ret == TypeDefine.EMV_APDU_TIMEOUT){
                            textView.append("Present Card Again\n");
                            readCardAgain = true;
                            break;
                        }

                        textView.append("End Application\n");
                        break;
                    }else if(ret > EMV_STATUS.EMV_OK && ret < EMV_STATUS.EMV_COMPLETED){
                        switch (ret){
                            case EMV_STATUS.EMV_STA_APP_SELECTED:
                                PAYPASSSetBeforeGPO();
                                break;
                            case EMV_STATUS.EMV_REQ_ONLINE_PIN:
                                Intent intent = new Intent(this, InputPINActivity.class);
                                inputPINActivityResultLauncher.launch(intent);
                                if(pinResult == null){
                                    if(pinStr == null){
//                                        emvInterface.setOnlinePinEntered(EMV_CONSTANTS.EMV_OPERRESULTS.EMV_OPER_CANCEL, )
                                    }
                                }

                        }
                    }
                }
            }

            if(readCardAgain){
                continue;
            }else{

            }


        }


    }

    private int ReadCardAgain(){



        return 0;
    }



    private void ImportTradeAmount() {
        emvInterface.setTransAmount(transAmount);
        emvInterface.setOtherAmount(transAmountOther);
        emvInterface.setTransType(transType);
    }

    private void ReadCardDisp() {
        long plus = transAmount + transAmountOther;
        double fmt = plus / 100.0;
        String s = String.format(Locale.US, "%.2f", fmt);

        textView.setText("");

        if (bIsSeePhone) {
            textView.append("See Phone\n");
            textView.append("Present Card Again\n");
            textView.append(s + "\n");
        } else {
            if (bIsMultiCardColl) {
                textView.append("Please Present One Card Only\n");
                textView.append("Ready to Read\n");
            } else {
                if (bIsSecondTap) {
                    textView.append("Additional Tap\n");
                    textView.append("Plz present card again\n");
                    textView.append("Ready to Read\n");
                } else {
                    textView.append("Present Card\n");
                    textView.append("Ready to Read\n");
                    textView.append(s + "\n");
                }
            }
        }

        // 延迟一段时间后显示非接Logo
        myHandler.postDelayed(() -> {
            textView.setVisibility(View.GONE); // 隐藏文本视图
            imageView.setVisibility(View.VISIBLE); // 显示图片
        }, 100); // 延迟100毫秒
    }

    private void closeRFLogo() {
        imageView.setVisibility(View.GONE); // 隐藏图片
        textView.setVisibility(View.VISIBLE); // 显示文本视图
    }

    private int IccReadCard(){
        int ret = TypeDefine.EMV_ERR;
        byte [] atr = new byte[256];

        contactlessCardReader.open();

        smartCardReader.open(0, new SmartCardReader.SCReaderListener() {
            @Override
            public void notify(int nSlotIndex,int nEvent) {
                if(nEvent == SmartCardReader.EVENT_SMARTCARD_NOT_READY){

                }else if(nEvent == SmartCardReader.EVENT_SMARTCARD_READY){
                    contactlessCardReader.close();
                }
            }
        });

        long startTime = System.currentTimeMillis();
        long timeOut = 10000;   //10s

        while (System.currentTimeMillis() - startTime < timeOut){
            ret = contactlessCardReader.waitForCard(500);
            if(ret >= 0){
                cardType = TypeDefine.CARD_TYPE_CONTACTLESS;   //检测到非接
                smartCardReader.close();
                break;
            }

            ret = smartCardReader.waitForCard(0,500);
            if(ret >= 0){
                cardType = TypeDefine.CARD_TYPE_CONTACT;
                contactlessCardReader.close();
                break;
            }
        }

        if(cardType == 0){
            smartCardReader.close(0);
            contactlessCardReader.close();
            ret = TypeDefine.ICC_NO_CARD;
        }else if(cardType == TypeDefine.CARD_TYPE_CONTACTLESS) {
            ret = contactlessCardReader.powerOn(atr);
            if (ret >= 0) {
                if (atr[0] == 0xFF && atr[1] == 0xFF) {
                    ret = TypeDefine.ICC_MULTICARD;
                } else {
                    ret = TypeDefine.EMV_OK;
                }
            }else{
                ret = TypeDefine.ICC_RESET_CARD_ERR;
            }
        }else if(cardType == TypeDefine.CARD_TYPE_CONTACT){
            ret = smartCardReader.powerOn(0, atr);
            if(ret >= 0){
                ret = TypeDefine.EMV_OK;
            }else{
                ret = TypeDefine.ICC_RESET_CARD_ERR;
            }
        }

        return ret;
    }

    private void PAYPASSSetBeforeGPO(){
        byte transType=0x00;
        byte buf [] = new byte[256];
        byte aid [] = new byte[16];
        int aidLen;

        emvInterface.getTagData(0x9C, buf);
        transType = buf[0];
        aidLen = emvInterface.getTagData(0x4F, buf);
        System.arraycopy(buf, 0, aid, 0, aidLen);

        //TODO: 添加万事达自动化测试加载参数的处理

    }

}