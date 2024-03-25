package com.example.emvl3app;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

public class Menu0Fragment extends Fragment {

    public Menu0Fragment() {
        // Required empty public constructor
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        return inflater.inflate(R.layout.activity_menu0, container, false);
    }

    @Override
    public void onViewCreated(View view, Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        // Initialize your buttons here
        Button buttonStartTrans = view.findViewById(R.id.button_StartTrans);
        // Ensure the activity is a MainActivity instance before casting
        if (getActivity() instanceof MainActivity) {
            MainActivity mainActivity = (MainActivity) getActivity();
            // Use your custom OnClickListener
            MyOnClickListenerStartTrans myOnClickListener = new MyOnClickListenerStartTrans(mainActivity);
            buttonStartTrans.setOnClickListener(myOnClickListener);
        }

        Button buttonSelectCommuType = view.findViewById(R.id.button_CommuType);
        if(getActivity() instanceof MainActivity){
            MainActivity mainActivity = (MainActivity) getActivity();
            MyOnClickListenerSelectCommuType myOnClickListenerSelectCommuType = new MyOnClickListenerSelectCommuType(mainActivity);
            buttonSelectCommuType.setOnClickListener(myOnClickListenerSelectCommuType);
        }

        Button buttonParamManage = view.findViewById(R.id.button_ParamManage);
        if(getActivity() instanceof MainActivity){
            MainActivity mainActivity = (MainActivity) getActivity();
            MyOnClickListenerParamManage onClickListenerParamManage = new MyOnClickListenerParamManage(mainActivity);
            buttonParamManage.setOnClickListener(onClickListenerParamManage);
        }


        // Initialize other buttons similarly
    }


}
