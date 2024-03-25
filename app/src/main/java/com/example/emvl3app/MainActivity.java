package com.example.emvl3app;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.provider.ContactsContract;
import android.support.v4.view.ViewPager;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.Button;
import android.widget.TextView;
import android.content.Intent;

import com.szzt.sdk.device.Device;
import com.szzt.sdk.device.DeviceManager;
import com.szzt.sdk.device.card.ContactlessCardReader;

import com.szzt.sdk.device.card.SmartCardReader;
import com.szzt.sdk.device.emv.EMV_STATUS;


public class MainActivity extends AppCompatActivity {

    private ViewPager viewPager;
    private TextView textView;
    private MyOnPageChangeListener pageChangeListener;

    private int communicateType;  //0-None  1-SerialPort  2-NetWork

    private static final int REQUEST_CODE_SELECT_COMMUNICATE_ACTIVITY = 1;
    private static final int REQUEST_CODE_PARAM_MANAGE_ACTIVITY = 2;
    private DeviceManager mDeviceManager;
    private MainApplication mainApplication;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initAppParam();

    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if(requestCode == REQUEST_CODE_SELECT_COMMUNICATE_ACTIVITY && resultCode == RESULT_OK){
            // 从Intent中获取返回的值
            int returnedValue = data.getIntExtra("communicateType", 0);
            setCommunicateType(returnedValue);
        }

        if(requestCode == REQUEST_CODE_PARAM_MANAGE_ACTIVITY && resultCode == RESULT_OK){
            //
        }
    }

    // 根据页面位置更新标题
    public void updateTitle(int position) {
        switch (position) {
            case 0:
                textView.setText(getString(R.string.menu0));
                break;
            case 1:
                textView.setText(getString(R.string.menu1));
                break;
            default:
                textView.setText(getString(R.string.menu0));
            // 添加更多的菜单页面处理
        }
    }

    //初始化
    private void initAppParam(){
        viewPager = (ViewPager) findViewById(R.id.viewPager);
        textView = (TextView)findViewById(R.id.textView_title);
        pageChangeListener = new MyOnPageChangeListener(this);

        //设置页面转换器
        MyPagerAdapter pagerAdapter = new MyPagerAdapter(getSupportFragmentManager());
        viewPager.setAdapter(pagerAdapter);

        // 设置自定义的页面切换监听器
        viewPager.addOnPageChangeListener(pageChangeListener);
        //初始化页面显示标题
        updateTitle(viewPager.getCurrentItem());

        communicateType = 0;
        mainApplication = (MainApplication) getApplication();
        SZZTApplication.getInstance().mainActivity = this;
        SZZTApplication.getInstance().setMainApplication(mainApplication);
    }

    public SmartCardReader getSmartCardReader(){
        Device[] smartCardReaders = mDeviceManager
                .getDeviceByType(Device.TYPE_SMARTCARDREADER);
        if (smartCardReaders != null)
            return (SmartCardReader) smartCardReaders[0];
        return null;
    }

    public ContactlessCardReader getContactlessCardReader() {
        Device[] contactlessCards = mDeviceManager
                .getDeviceByType(Device.TYPE_CONTACTLESSCARDREADER);
        if (contactlessCards != null)
            return (ContactlessCardReader) contactlessCards[0];
        return null;
    }

    public void selectCommuType(){
        Intent intent = new Intent(this, SelectCommuTypeActivity.class);
        startActivityForResult(intent, REQUEST_CODE_SELECT_COMMUNICATE_ACTIVITY);
    }

    public void pageParamManage(){
        Intent intent = new Intent(this, ParamManageActivity.class);
        intent.putExtra("communicateType", getCommunicateType());
        startActivityForResult(intent, REQUEST_CODE_PARAM_MANAGE_ACTIVITY );
    }

    //开始交易
    public void startTrans(){
        int ret = 0;
        byte [] sendData = new byte[4];
        sendData[0] = (byte) 0x02;
        sendData[1] = (byte) TypeDefine.PROTOCOL_START_TRANS_SEND;
        DataTransformer dataTransformer = new DataTransformer(communicateType, this);
        ret = dataTransformer.send(sendData, sendData.length);
        if(ret > 0){

        }else{
            Log.d("lishiyao", "startTrans: send protocol error");
        }

        byte [] recvData = new byte[256];
        ret = dataTransformer.receive(recvData, recvData.length, 1000);
        if(ret > 0){
            TransFlowProcessor transFlowProcessor = new TransFlowProcessor(recvData, this, communicateType);
            transFlowProcessor.parseProtocol();
        }else{
            Log.d("lishiyao", "startTrans: receive protocol error");
        }
    }

    public void setCommunicateType(int communicateType){
        Log.d("lishiyao", "setCommunicateType: current communicate type: "+ communicateType);
        this.communicateType = communicateType;
        // 根据需要更新UI或执行其他逻辑
    }

    public int getCommunicateType(){
        return this.communicateType;
    }

}