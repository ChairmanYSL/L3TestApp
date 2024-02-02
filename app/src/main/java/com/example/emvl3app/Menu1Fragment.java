package com.example.emvl3app;

import android.support.v4.app.Fragment;
import android.os.Bundle;
import android.support.v4.view.ViewPager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.w3c.dom.Text;

public class Menu1Fragment extends Fragment {

    public Menu1Fragment(){

    }

    @Override
    public View onCreateView(LayoutInflater inflater,  ViewGroup container, Bundle savedInstanceState) {
         View view = inflater.inflate(R.layout.activity_menu1, container, false);
         TextView textView = getActivity().findViewById(R.id.textView_title);

         if(textView != null) {
             String title = getResources().getString(R.string.menu1);
             Log.d("lishiyao", "onCreateView: title = "+title);
             textView.setText(title);
         }

         return view;
    }

}
