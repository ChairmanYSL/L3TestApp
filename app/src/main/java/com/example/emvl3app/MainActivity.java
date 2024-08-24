package com.example.emvl3app;

import android.app.AlertDialog;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;
import androidx.viewpager.widget.ViewPager;

import com.szzt.sdk.device.Device;
import com.szzt.sdk.device.DeviceManager;
import com.szzt.sdk.device.card.ContactlessCardReader;
import com.szzt.sdk.device.card.SmartCardReader;
import com.szzt.sdk.device.emv.EmvInterface;


public class MainActivity extends AppCompatActivity {

    private ViewPager viewPager;
    private TextView textView;
    private MyOnPageChangeListener pageChangeListener;

    private int communicateType;  //0-None  1-SerialPort  2-NetWork

    private static final int REQUEST_CODE_SELECT_COMMUNICATE_ACTIVITY = 1;
    private static final int REQUEST_CODE_PARAM_MANAGE_ACTIVITY = 2;
    private static final int REQUEST_CODE_STARTTRANS_ACTIVITY = 3;
    private static final int REQUEST_CODE_EMVPROCESS_ACTIVITY = 4;
    private MainApplication mainApplication;
    private EmvInterface emvInterface;
    private boolean isBackPressedOnce = false;
    private AlertDialog exitDialog;

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

        if(requestCode == REQUEST_CODE_STARTTRANS_ACTIVITY && resultCode == RESULT_OK){
            String transAmount = data.getStringExtra("transAmount");
            String transAmountOther = data.getStringExtra("transAmountOther");
            String transType = data.getStringExtra("transType");
            processTrans(transAmount, transAmountOther, transType);
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
            case 2:
                textView.setText(getString(R.string.menu2));
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

        mainApplication = MainApplication.getInstance();
        this.emvInterface = this.mainApplication.getEmvInterface();

        // 初始化退出对话框
        exitDialog = new AlertDialog.Builder(this)
                .setMessage(getString(R.string.prompt_exit))
                .setPositiveButton(getString(R.string.confirm), (dialog, which) -> {
                    finishAffinity(); // 关闭应用
                })
                .setNegativeButton(getString(R.string.cancel), (dialog, which) -> {
                    dialog.dismiss();
                    isBackPressedOnce = false; // 重置返回键状态
                })
                .create();
    }

    public void selectCommuType(){
        Intent intent = new Intent(this, SelectCommuTypeActivity.class);
        startActivityForResult(intent, REQUEST_CODE_SELECT_COMMUNICATE_ACTIVITY);
    }

//    private void openSerial(){
//    int ret;
//
//    if(serialPort == null){
//        Log.e("lishiyao", "openSerial: serial port instance is null");
//    }
//
//    serialPortNum = serialPort.open("/dev/ttyUSB1", 115200);
//    Log.d("lishiyao", "openSerial: return serial port num:" + serialPortNum);
//    if(serialPortNum < 0){
//        Log.e("lishiyao", "openSerial: open serial port failed");
//        return;
//    }
//
//    ret = serialPort.setOptions(serialPortNum, 115200, 8, 0, 1);
//    if(ret < 0){
//        Log.e("SerialPort", "send: set serial port option failed");
//        return ;
//    }
//}

    public void pageParamManage(){
        Intent intent = new Intent(this, ParamManageActivity.class);
        intent.putExtra("communicateType", getCommunicateType());
        startActivityForResult(intent, REQUEST_CODE_PARAM_MANAGE_ACTIVITY );
    }

    @Override
    public void onActivityReenter(int resultCode, Intent data) {
        super.onActivityReenter(resultCode, data);
    }

    //开始交易
    public void startTrans(){
        Intent intent = new Intent(this, InputAmountActivity.class);
        startActivityForResult(intent, REQUEST_CODE_STARTTRANS_ACTIVITY);
    }

    public void setCommunicateType(int communicateType){
        Log.d("lishiyao", "setCommunicateType: current communicate type: "+ communicateType);
        this.communicateType = communicateType;
        // 根据需要更新UI或执行其他逻辑
    }

    public int getCommunicateType(){
        return this.communicateType;
    }

    public void getKernelVersion(){
        Log.d("lishiyao", "getKernelVersion: emvInterface pointer: "+ this.emvInterface);
        TextView textView = (TextView) findViewById(R.id.textView_title);
        textView.setText(this.emvInterface.getVersion());
    }

    @Override
    public void onBackPressed() {
        if (isBackPressedOnce) {
            super.onBackPressed(); // 用户第二次点击返回键，退出应用
            return;
        }

        isBackPressedOnce = true;
        exitDialog.show(); // 显示退出确认对话框

        // 设置一个计时器，2秒内用户未再点击返回键则重置状态
        new Handler().postDelayed(() -> isBackPressedOnce = false, 2000);
    }

    private void processTrans(String transAmt, String amtOther, String type){
        Intent intent = new Intent(this, EMVProcessActivity.class);
        intent.putExtra("transAmount", transAmt);
        intent.putExtra("transAmountOther", amtOther);
        intent.putExtra("transType", type);
        startActivityForResult(intent, REQUEST_CODE_EMVPROCESS_ACTIVITY);
    }
}