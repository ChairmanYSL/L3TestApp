package com.example.emvl3app;

import android.view.View;

public class MyOnClickListenerStartTrans implements View.OnClickListener{
    private final MainActivity mainActivity;

    public MyOnClickListenerStartTrans(MainActivity mainActivity){
        this.mainActivity = mainActivity;
    }

    @Override
    public void onClick(View view) {
        this.mainActivity.startTrans();
    }

}
