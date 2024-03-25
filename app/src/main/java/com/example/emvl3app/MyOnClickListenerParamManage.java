package com.example.emvl3app;

import android.view.View;

public class MyOnClickListenerParamManage implements View.OnClickListener {
    private MainActivity mainActivity;

    public MyOnClickListenerParamManage(MainActivity mainActivity){this.mainActivity = mainActivity;}

    @Override
    public void onClick(View view) {
        mainActivity.pageParamManage();
    }
}
