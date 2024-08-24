package com.example.emvl3app;

import android.util.Log;

import androidx.viewpager.widget.ViewPager;

public class MyOnPageChangeListener implements ViewPager.OnPageChangeListener {
    private final MainActivity activity;

    public MyOnPageChangeListener(MainActivity mainActivity){
        this.activity = mainActivity;
    }

    @Override
    public void onPageScrolled(int position, float positionOffset, int positionOffsetPixels) {

    }

    @Override
    public void onPageSelected(int position) {
        Log.d("lishiyao", "onPageSelected: position = "+ position);
        activity.updateTitle(position);
    }

    @Override
    public void onPageScrollStateChanged(int state) {

    }
}
