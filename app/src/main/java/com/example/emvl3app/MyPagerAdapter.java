package com.example.emvl3app;

import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentStatePagerAdapter;

public class MyPagerAdapter extends FragmentStatePagerAdapter {

    public MyPagerAdapter(FragmentManager fragmentManager){
        super(fragmentManager);
    }


    @Override
    public Fragment getItem(int position){
        switch (position){
            case 0:
                return new Menu0Fragment();
            case 1:
                return new Menu1Fragment();
            default:
                return null;
        }
    }

    @Override
    public int getCount(){
        return 2;
    }
}
