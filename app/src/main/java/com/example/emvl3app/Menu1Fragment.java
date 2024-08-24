package com.example.emvl3app;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;


public class Menu1Fragment extends Fragment {

    public Menu1Fragment(){

    }

    @Override
    public View onCreateView(LayoutInflater inflater,  ViewGroup container, Bundle savedInstanceState) {
         View view = inflater.inflate(R.layout.activity_menu1, container, false);
         return view;
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        // Initialize buttons
        Button buttonSelCommuType = (Button) view.findViewById(R.id.button_CommuType);
        // Ensure the activity is a MainActivity instance before casting
        if (getActivity() instanceof MainActivity) {
            MainActivity mainActivity = (MainActivity) getActivity();
            // Use your custom OnClickListener
            MyOnClickListenerSelectCommuType myOnClickListener = new MyOnClickListenerSelectCommuType(mainActivity);
            buttonSelCommuType.setOnClickListener(myOnClickListener);
        }
    }
}
