package com.example.emvl3app;

import android.support.v4.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;


public class Menu1Fragment extends Fragment {

    public Menu1Fragment(){

    }

    @Override
    public View onCreateView(LayoutInflater inflater,  ViewGroup container, Bundle savedInstanceState) {
         View view = inflater.inflate(R.layout.activity_menu1, container, false);
         return view;
    }

}
