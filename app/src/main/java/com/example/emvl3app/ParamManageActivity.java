package com.example.emvl3app;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

public class ParamManageActivity extends AppCompatActivity {
    private int commuType;
    private DataTransformer dataTransformer;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_param_manage);

        commuType = getIntent().getIntExtra("communicateType", TypeDefine.communicateWithNone);
        dataTransformer = new DataTransformer(commuType, this);

        Button buttonDownloadAID = (Button) findViewById(R.id.button_DownloadAID);
        Button buttonDownloadCAPK = (Button) findViewById(R.id.button_DownloadCAPK);
        Button buttonDownloadExcpFile = (Button) findViewById(R.id.button_DownloadExcpFile);
        Button buttonDownloadRevokey = (Button) findViewById(R.id.button_DownloadRevokey);

        buttonDownloadAID.setOnClickListener(v->{
            downloadAID();
        });

        buttonDownloadCAPK.setOnClickListener(v->{
            downloadCAPK();
        });


        buttonDownloadExcpFile.setOnClickListener(v->{
            downloadExcptionFile();
        });

        buttonDownloadRevokey.setOnClickListener(v->{
            downloadRevokey();
        });
    }

    private void downloadAID(){
        byte [] sendData = new byte[]{(byte) 0x02, (byte) 0xC3, (byte) 0x00, (byte) 0x00};
        byte [] recvData = new byte[512];
        int ret;

        Log.d("lishiyao", "downloadAID: start send protocol");
        dataTransformer.send(sendData, 4);
        ret = dataTransformer.receive(recvData, 512, 2000);
        if(ret > 0){
            TransFlowProcessor transFlowProcessor = new TransFlowProcessor(recvData, this, commuType);
            transFlowProcessor.parseProtocol();
        }else{
            Log.d("lishiyao", "downloadAID: download AID error");
        }
    }

    private void downloadCAPK(){
        byte [] sendData = new byte[]{(byte) 0x02, (byte) 0xC2, (byte) 0x00, (byte) 0x00};
        byte [] recvData = new byte[1024];
        int ret;

        dataTransformer.send(sendData, 4);
        ret = dataTransformer.receive(recvData, 1024, 2000);
        if(ret > 0){
            TransFlowProcessor transFlowProcessor = new TransFlowProcessor(recvData, this, commuType);
            transFlowProcessor.parseProtocol();
        }else{
            Log.d("lishiyao", "downloadCAPK: download CAPK error");
        }
    }

    private void downloadExcptionFile(){
        byte [] sendData = new byte[]{(byte) 0x02, (byte) 0xC5, (byte) 0x00, (byte) 0x00};
        byte [] recvData = new byte[256];
        int ret;

        dataTransformer.send(sendData, 4);
        ret = dataTransformer.receive(recvData, 256, 2000);
        if(ret > 0){
            TransFlowProcessor transFlowProcessor = new TransFlowProcessor(recvData, this, commuType);
            transFlowProcessor.parseProtocol();
        }else{
            Log.d("lishiyao", "downloadExcptionFile: download Exception File error");
        }
    }

    private void downloadRevokey(){
        byte [] sendData = new byte[]{(byte) 0x02, (byte) 0xC6, (byte) 0x00, (byte) 0x00};
        byte [] recvData = new byte[256];
        int ret;

        dataTransformer.send(sendData, 4);
        ret = dataTransformer.receive(recvData, 256, 2000);
        if(ret > 0){
            TransFlowProcessor transFlowProcessor = new TransFlowProcessor(recvData, this, commuType);
            transFlowProcessor.parseProtocol();
        }else{
            Log.d("lishiyao", "downloadRevokey: download Exception File error");
        }
    }
}
