package com.example.emvl3app;

import android.view.View;

public class MyOnClickListenerShowKernelVersion implements View.OnClickListener{
    private MainActivity mainActivity;

    public MyOnClickListenerShowKernelVersion(MainActivity mainActivity){
        this.mainActivity = mainActivity;
    }

    @Override
    public void onClick(View v) {
        mainActivity.getKernelVersion();
    }
}
