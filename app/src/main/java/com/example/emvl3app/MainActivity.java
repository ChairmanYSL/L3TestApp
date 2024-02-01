package com.example.emvl3app;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.widget.TextView;
import android.os.Handler;
import android.content.Intent;
import android.os.Looper;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    @Override
    protected void onResume() {
        super.onResume();
//        goNextPage();
    }

//    private void goNextPage(){
//        TextView tv_hello = findViewById(R.id.tv_hello);
//        tv_hello.setText("3s后进入下个页面");
//        new Handler(Looper.myLooper()).postDelayed(taskGoNextPage, 3000);
//    }

//    private Runnable taskGoNextPage = new Runnable() {
//        @Override
//        public void run() {
//            Intent switchPage =  new Intent(MainActivity.this, OtherActivity.class);
//            startActivity(switchPage);
//        }
//    };
}