package com.example.emvl3app;

import android.app.Application;

public class SZZTApplication extends Application {
    private static SZZTApplication singleton = new SZZTApplication();
    public MainActivity mainActivity;
    public MainApplication mainApplication;

    public static SZZTApplication getInstance(){
        return singleton;
    }

    public void setMainApplication(MainApplication mainApplication){this.mainApplication = mainApplication;}
    public MainApplication getMainApplication(){return mainApplication;}

}
