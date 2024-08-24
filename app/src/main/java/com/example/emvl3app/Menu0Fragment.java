package com.example.emvl3app;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import androidx.fragment.app.Fragment;

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

        // Initialize buttons
        Button buttonStartTrans = (Button) view.findViewById(R.id.button_StartTrans);
        // Ensure the activity is a MainActivity instance before casting
        if (getActivity() instanceof MainActivity) {
            MainActivity mainActivity = (MainActivity) getActivity();
            // Use your custom OnClickListener
            MyOnClickListenerStartTrans myOnClickListener = new MyOnClickListenerStartTrans(mainActivity);
            buttonStartTrans.setOnClickListener(myOnClickListener);
        }

        Button buttonParamManage = (Button)view.findViewById(R.id.button_ParamManage);
        if(getActivity() instanceof MainActivity){
            MainActivity mainActivity = (MainActivity) getActivity();
            MyOnClickListenerParamManage onClickListenerParamManage = new MyOnClickListenerParamManage(mainActivity);
            buttonParamManage.setOnClickListener(onClickListenerParamManage);
        }

        Button buttonKernelVersion = (Button)view.findViewById(R.id.button_KernelVer);
        if(getActivity() instanceof MainActivity){
            MainActivity mainActivity = (MainActivity) getActivity();
            MyOnClickListenerShowKernelVersion onClickListenerShowKernelVersion = new MyOnClickListenerShowKernelVersion(mainActivity);
            buttonKernelVersion.setOnClickListener(onClickListenerShowKernelVersion);
        }


        // Initialize other buttons similarly
    }


}
