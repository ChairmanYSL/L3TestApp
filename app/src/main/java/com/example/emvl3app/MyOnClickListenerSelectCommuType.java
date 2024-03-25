package com.example.emvl3app;

import android.view.View;

public class MyOnClickListenerSelectCommuType implements View.OnClickListener {
    private MainActivity mainActivity;

    public MyOnClickListenerSelectCommuType(MainActivity mainActivity){
        this.mainActivity = mainActivity;
    }

    @Override
    public void onClick(View view) {
        this.mainActivity.selectCommuType();
    }
}
