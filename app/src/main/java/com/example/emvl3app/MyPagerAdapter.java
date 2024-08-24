package com.example.emvl3app;


import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentStatePagerAdapter;

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
            case 2:
                return new Menu2Fragment();
            default:
                return null;
        }
    }

    @Override
    public int getCount(){
        return 3;
    }
}
